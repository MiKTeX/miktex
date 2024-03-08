/**
 * Copyright (C) 2013-2023  Charlie Sharpsteen, Stefan Löffler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include "PDFBackend.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>
#include <memory>
#include <list>

namespace QtPDF {

namespace Backend {

class BackendManager
{
  std::list< std::unique_ptr<BackendInterface> > m_backendInterfaces;
public:
  BackendManager() {
#ifdef USE_POPPLERQT
    m_backendInterfaces.push_back(std::unique_ptr<BackendInterface>(new PopplerQtBackend));
#endif
#ifdef USE_MUPDF
    m_backendInterfaces.push_back(std::unique_ptr<BackendInterface>(new MuPDFBackend));
#endif
  }
  BackendInterface * backend(const QString & name = {})
  {
    if (!name.isEmpty()) {
      for (const std::unique_ptr<BackendInterface> & b : m_backendInterfaces) {
        if (b && b->name() == name)
          return b.get();
      }
    }
    return (m_backendInterfaces.empty() ? nullptr : m_backendInterfaces.front().get());
  }
  QStringList backendNames() const
  {
    QStringList rv;
    for (const std::unique_ptr<BackendInterface> & bi : m_backendInterfaces) {
      if (!bi) {
        continue;
      }
      rv.append(bi->name());
    }
    return rv;
  }
  QString defaultBackendName() const
  {
    return (m_backendInterfaces.empty() ? QString() : m_backendInterfaces.front()->name());
  }
  void setDefaultBackend(const QString & name)
  {
    auto it = std::find_if(m_backendInterfaces.begin(), m_backendInterfaces.end(),
      [name](const std::unique_ptr<BackendInterface> & bi) { return (bi && bi->name() == name); });
    if (it == m_backendInterfaces.end()) {
      // No such backend found
      return;
    }
    if (it == m_backendInterfaces.begin()) {
      // It's already the default
      return;
    }
    std::unique_ptr<BackendInterface> placeholder;
    // Swap backend into placeholder
    std::swap(placeholder, *it);
    // Erase the (now empty) entry in the list
    m_backendInterfaces.erase(it);
    // Insert the placeholder back in the front
    m_backendInterfaces.insert(m_backendInterfaces.begin(), std::move(placeholder));
  }
};
static BackendManager backendManager;

// TODO: Find a better place to put this
static QBrush * pageDummyBrush = nullptr;

QDateTime fromPDFDate(QString pdfDate)
{
  QDate date;
  QTime time;
  QString format;
  int sign{0};
  // QDateTime::addSecs() uses qint64, so use that type from the start
  qint64 hourOffset{0}, minuteOffset{0};
  bool ok{false};

  // "D:" prefix is strongly recommended, but optional; we don't need it here
  if (pdfDate.startsWith(QString::fromUtf8("D:")))
    pdfDate.remove(0, 2);

  // Parse the date
  if (pdfDate.length() < 4)
    return QDateTime();
  format = QString::fromUtf8("yyyy");
  if (pdfDate.length() >= 6)
    format += QString::fromUtf8("MM");
  if (pdfDate.length() >= 8)
    format += QString::fromUtf8("dd");
  date = QDate::fromString(pdfDate.left(format.length()), format);
  pdfDate.remove(0, format.length());

  // Parse the time
  if (pdfDate.length() < 2)
    return QDateTime(date, time);
  format = QString::fromUtf8("hh");
  if (pdfDate.length() >= 4)
    format += QString::fromUtf8("mm");
  if (pdfDate.length() >= 6)
    format += QString::fromUtf8("ss");
  time = QTime::fromString(pdfDate.left(format.length()), format);
  pdfDate.remove(0, format.length());

  // Parse time zone data
  if (pdfDate.length() == 0)
    return QDateTime(date, time);
  switch (pdfDate[0].toLatin1()) {
    case 'Z':
      return QDateTime(date, time, Qt::UTC).toLocalTime();
    case '+':
      // Note: A `+` signifies that pdfDate is later than UTC. Since we will
      // specify the QDateTime in UTC below, we have to _subtract_ the offset
      sign = -1;
      break;
    case '-':
      sign = +1;
      break;
    default:
      return QDateTime(date, time);
  }
  pdfDate.remove(0, 1);
  if (pdfDate.length() < 3 || pdfDate[2] != QChar::fromLatin1('\''))
    return QDateTime(date, time);
  hourOffset = pdfDate.left(2).toInt(&ok);
  if (!ok)
    return QDateTime(date, time);
  pdfDate.remove(0, 3);
  if (pdfDate.length() >= 2)
    minuteOffset = pdfDate.left(2).toInt();
  return QDateTime(date, time, Qt::UTC).addSecs(sign * (hourOffset * 3600 + minuteOffset * 60)).toLocalTime();
}

// PDF ABCs
// ========

// Document Class
// --------------
//
// This class is thread-safe. Data access is governed by the QReadWriteLock
// _docLock.

PDFPageCache Document::_pageCache;

Document::Document(QString fileName):
  _fileName(fileName)
{
  Q_ASSERT(_docLock != nullptr);

#ifdef DEBUG
//  qDebug() << "Document::Document(" << fileName << ")";
#endif
}

// FIXME: Consider porting Document to a PIMPL design in which we could just
// call the constructor to construct a document
QSharedPointer<Document> Document::newDocument(const QString & fileName, const QString & backend)
{
  BackendInterface * bi = backendManager.backend(backend);
  return (bi ? bi->newDocument(fileName) : QSharedPointer<Document>());
}

QStringList Document::backends()
{
  return backendManager.backendNames();
}

QString Document::defaultBackend()
{
  return backendManager.defaultBackendName();
}

void Document::setDefaultBackend(const QString & backend)
{
  backendManager.setDefaultBackend(backend);
}

Document::~Document()
{
#ifdef DEBUG
//  qDebug() << "Document::~Document()";
#endif
  clearPages();
  _pageCache.removeDocumentTiles(this);
}

Document::size_type Document::numPages() const { QReadLocker docLocker(_docLock.data()); return _numPages; }
PDFPageProcessingThread &Document::processingThread() { QReadLocker docLocker(_docLock.data()); return _processingThread; }

QWeakPointer<Page> Document::page(size_type at)
{
  QReadLocker l(_docLock.data());
  if (at < 0 || at >= _pages.size()) {
    return QWeakPointer<Page>();
  }
  return _pages[at];
}

QList<SearchResult> Document::search(const QString & searchText, const SearchFlags & flags, const size_type startPage)
{
  QReadLocker docLocker(_docLock.data());
  QList<SearchResult> results;
  size_type start = startPage;
  size_type end = (flags.testFlag(Search_Backwards) ? -1 : _numPages);
  size_type step = (flags.testFlag(Search_Backwards) ? -1 : +1);

  for (size_type i = start; i != end; i += step) {
    QSharedPointer<Page> page(_pages[i]);
    if (!page)
      continue;
    results << page->search(searchText, flags);
  }

  if (flags.testFlag(Search_WrapAround)) {
    start = ((flags & Search_Backwards) ? _numPages - 1 : 0);
    end = startPage;
    for (size_type i = start; i != end; i += step) {
      QSharedPointer<Page> page(_pages[i]);
      if (!page)
        continue;
      results << page->search(searchText, flags);
    }
  }

  return results;
}

void Document::clearPages()
{
  // Clear the processing thread to ensure no task still needs the pages we are
  // about to destroy.
  // NB: Do this before acquiring _docLock. See clearWorkStack() documentation.
  // This should not cause any problems as we are supposed to currently be in
  // the main (GUI) thread, and only this thread is supposed to add items to the
  // work stack.
  _processingThread.clearWorkStack();

  QWriteLocker docLocker(_docLock.data());
  foreach(QSharedPointer<Page> page, _pages) {
    if (page.isNull())
      continue;
    page->detachFromParent();
  }
  // Note: clear() releases all QSharedPointer to pages, thereby destroying them
  // (if they are not used elsewhere)
  _pages.clear();
}

void Document::clearMetaData()
{
  QWriteLocker docLocker(_docLock.data());
  _meta_title = QString();
  _meta_author = QString();
  _meta_subject = QString();
  _meta_keywords = QString();
  _meta_creator = QString();
  _meta_producer = QString();
  _meta_fileSize = 0;

  _meta_creationDate = QDateTime();
  _meta_modDate = QDateTime();
  _meta_trapped = Trapped_Unknown;
  _meta_other.clear();
}

// Page Class
// ----------
//
// This class is thread-safe. Data access is governed by the QReadWriteLock
// _pageLock. When accessing the parent document directly (i.e., not via public
// member functions), the QSharedPointer<QReadWriteLock> _docLock must also be
// acquired. Note that if _docLock and _pageLock are to be acquired, _docLock
// must be acquired first.
// Note that the Page may exist in a detached state, i.e., _parent == nullptr. This
// is typically the case when the document discarded the page object but some
// other object (typically in another thread) still holds a QSharedPointer to it.
Page::Page(Document *parent, size_type at, QSharedPointer<QReadWriteLock> docLock):
  _parent(parent),
  _n(at),
  _docLock(docLock)
{
#ifdef DEBUG
//  qDebug() << "Page::Page(" << parent << ", " << at << ")";
#endif

  if (!pageDummyBrush) {
    pageDummyBrush = new QBrush();

    // Make a texture brush which can be used to print "rendering page" all over
    // the dummy tiles that are shown while the rendering thread is doing its
    // work
    QImage brushTex(1024, 1024, QImage::Format_ARGB32);
    QRectF textRect;
    QPainter p;
    p.begin(&brushTex);
    p.fillRect(brushTex.rect(), Qt::white);
    p.setPen(Qt::lightGray);
    p.drawText(brushTex.rect(), Qt::AlignCenter | Qt::AlignVCenter | Qt::TextSingleLine, QCoreApplication::translate("QtPDF::PDFDocumentScene", "rendering page"), &textRect);
    p.end();
    textRect.adjust(-textRect.width() * .05, -textRect.height() * .1, textRect.width() * .05, textRect.height() * .1);
    brushTex = brushTex.copy(textRect.toAlignedRect());

    pageDummyBrush->setTextureImage(brushTex);
    pageDummyBrush->setTransform(QTransform().rotate(-45));
  }
}

Page::size_type Page::pageNum() const { QReadLocker pageLocker(&_pageLock); return _n; }

void Page::detachFromParent()
{
  QWriteLocker pageLocker(&_pageLock);
  _parent = nullptr;
}

QRectF Page::getContentBoundingBox() const
{
  QSizeF pageSize(pageSizeF());
  // render the page into a 100x100 px image (this should be fast and will allow
  // estimating the content bounding box to about 1% of the page size)
  QImage img = renderToImage(100. * 72 / pageSize.width(), 100. * 72 / pageSize.height());

  if (img.isNull())
    return QRectF();

  // Make sure the image is in a format we can handle here
  switch (img.format()) {
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
    case QImage::Format_RGB32:
      break;
    default:
      img = img.convertToFormat(QImage::Format_ARGB32);
      break;
  }

  // Get the background color (assumed to be the color of the top left pixel)
  QRgb bg = img.pixel(0, 0);

  // Make sure the same color is used in the other three corners (otherwise we
  // can't be sure it's really the global background color; in that case we
  // assume that everything is content)
  if (bg != img.pixel(img.width() - 1, 0) || bg != img.pixel(0, img.height() - 1 || bg != img.pixel(img.width() - 1, img.height() - 1)))
    return QRectF(QPointF(0, 0), pageSize);

  // Find the bounding box (min/max values for x and y) of the content
  int x0 = img.width(), x1 = 0, y0 = img.height(), y1 = 0;
  for (int y = 0; y < img.height(); ++y) {
    const QRgb * row = reinterpret_cast<const QRgb*>(img.constScanLine(y));
    for (int x = 0; x < img.width(); ++x) {
      if (row[x] != bg) {
        if (x0 > x) x0 = x;
        if (x1 < x) x1 = x;
        if (y0 > y) y0 = y;
        if (y1 < y) y1 = y;
      }
    }
  }

  return QRectF(x0 * pageSize.width() / 100., y0 * pageSize.height() / 100., (x1 - x0 + 1) * pageSize.width() / 100., (y1 - y0 + 1) * pageSize.height() / 100.);
}

QSharedPointer<QImage> Page::getCachedImage(double xres, double yres, QRect render_box /* = QRect() */, PDFPageCache::TileStatus * status /* = nullptr */)
{
  QReadLocker docLocker(_docLock.data());
  QReadLocker pageLocker(&_pageLock);
  if (!_parent) {
    if (status)
      *status = PDFPageCache::UNKNOWN;
    return QSharedPointer<QImage>();
  }
  const PDFPageTile tile(xres, yres, render_box, _parent, _n);
  if (status)
    *status = _parent->pageCache().getStatus(tile);
  return _parent->pageCache().getImage(tile);
}

void Page::asyncRenderToImage(QObject *listener, double xres, double yres, QRect render_box, bool cache)
{
  QReadLocker docLocker(_docLock.data());
  QReadLocker pageLocker(&_pageLock);
  if (!_parent)
    return;
  _parent->processingThread().addPageProcessingRequest(new PageProcessingRenderPageRequest(this, listener, xres, yres, render_box, cache));
}

bool higherResolutionThan(const PDFPageTile & t1, const PDFPageTile & t2)
{
  // Note: We silently assume that xres and yres behave the same way
  return t1.xres > t2.xres;
}

QSharedPointer<QImage> Page::getTileImage(QObject * listener, const double xres, const double yres, QRect render_box /* = QRect() */)
{
  QReadLocker docLocker(_docLock.data());
  QReadLocker pageLocker(&_pageLock);

  // If the render_box is empty, use the whole page
  if (render_box.isNull())
    render_box = QRectF(0, 0, pageSizeF().width() * xres / 72., pageSizeF().height() * yres / 72.).toAlignedRect();

  // If the tile is cached, return it if
  // 1) it is current
  // 2) it is a placeholder (in this case, it is currently rendering in the
  // background and we don't need to do anything)
  PDFPageCache::TileStatus status{PDFPageCache::UNKNOWN};
  QSharedPointer<QImage> retVal = getCachedImage(xres, yres, render_box, &status);
  if (retVal && (status == PDFPageCache::CURRENT || status == PDFPageCache::PLACEHOLDER))
    return retVal;

  if (listener) {
    // Render asyncronously, but add a dummy image to the cache first and return
    // that in the end
    // Note: Start the rendering in the background before constructing the image
    // to take advantage of multi-core CPUs. Since we hold the write lock here
    // there's nothing to worry about
    asyncRenderToImage(listener, xres, yres, render_box, true);

    if (retVal && status == PDFPageCache::OUTDATED) {
      // If we have an outdated image, use that as a placeholder
      _parent->pageCache().setImage(PDFPageTile(xres, yres, render_box, _parent, _n), retVal, PDFPageCache::PLACEHOLDER, false);
    }
    else {
      // otherwise construct a dummy image
      QSharedPointer<QImage> tmpImg{new QImage(render_box.width(), render_box.height(), QImage::Format_ARGB32)};
      QPainter p(tmpImg.data());
      p.fillRect(tmpImg->rect(), *pageDummyBrush);

      // Look through the cache to find tiles we can reuse (by scaling) for our
      // dummy tile
      // TODO: Benchmark this. If it is actualy too slow (i.e., just keeping the
      // rendered image from popping up due to the write lock we hold) disable it
      if (_parent) {
        QList<PDFPageTile> tiles = _parent->pageCache().tiles();
        for (QList<PDFPageTile>::iterator it = tiles.begin(); it != tiles.end(); ) {
          if (it->doc != _parent || it->page_num != pageNum()) {
            it = tiles.erase(it);
            continue;
          }
          // See if it->render_box intersects with render_box (after proper scaling)
          QRect scaledRect = QTransform::fromScale(xres / it->xres, yres / it->yres).mapRect(it->render_box);
          if (!scaledRect.intersects(render_box)) {
            it = tiles.erase(it);
            continue;
          }
          ++it;
        }
        // Sort the remaining tiles by size, high-res first
        std::sort(tiles.begin(), tiles.end(), higherResolutionThan);
        // Finally, crop, scale and paint each image until the whole area is
        // filled or no images are left in the list
        QPainterPath clipPath;
        clipPath.addRect(0, 0, render_box.width(), render_box.height());
        foreach (PDFPageTile tile, tiles) {
          QSharedPointer<QImage> tileImg = _parent->pageCache().getImage(tile);
          if (!tileImg)
            continue;

          // cropRect is the part of `tile` that overlaps the tile-to-paint (after
          // proper scaling).
          // paintRect is the part `tile` fills of the area we paint to (after
          // proper scaling).
          QRect cropRect = QTransform::fromScale(tile.xres / xres, tile.yres / yres).mapRect(render_box).intersected(tile.render_box).translated(-tile.render_box.left(), -tile.render_box.top());
          QRect paintRect = QTransform::fromScale(xres / tile.xres, yres / tile.yres).mapRect(tile.render_box).intersected(render_box).translated(-render_box.left(), -render_box.top());

          // Get the actual image and paint it onto the dummy tile
          QImage tmp(tileImg->copy(cropRect).scaled(paintRect.size()));
          p.setClipPath(clipPath);
          p.drawImage(paintRect.topLeft(), tmp);

          // Confine the clipping path to the part we have not painted to yet.
          QPainterPath pp;
          pp.addRect(paintRect);
          clipPath = clipPath.subtracted(pp);
          if (clipPath.isEmpty())
            break;
        }
      }
      // stop painting or else we couldn't (possibly) delete tmpImg below
      p.end();

      // Add the dummy tile to the cache
      // Note: In the meantime the asynchronous rendering could have finished and
      // insert the final image in the cache---we must handle that case and delete
      // our temporary image
      retVal = _parent->pageCache().setImage(PDFPageTile(xres, yres, render_box, _parent, _n), tmpImg, PDFPageCache::PLACEHOLDER, false);
    }
    return retVal;
  }
  renderToImage(xres, yres, render_box, true);
  return getCachedImage(xres, yres, render_box);
}

void Page::asyncLoadLinks(QObject *listener)
{
  QReadLocker docLocker(_docLock.data());
  QReadLocker pageLocker(&_pageLock);
  if (!_parent)
    return;
  _parent->processingThread().addPageProcessingRequest(new PageProcessingLoadLinksRequest(this, listener));
}

//static
QList<SearchResult> Page::executeSearch(SearchRequest request)
{
  QSharedPointer<Document> doc(request.doc.toStrongRef());
  if (!doc)
    return QList<SearchResult>();
  QSharedPointer<Page> page = doc->page(request.pageNum).toStrongRef();
  if (!page)
    return QList<SearchResult>();
  return page->search(request.searchString, request.flags);
}

} // namespace Backend

} // namespace QtPDF

// vim: set sw=2 ts=2 et

