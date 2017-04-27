/**
 * Copyright (C) 2011-2012  Charlie Sharpsteen, Stefan Löffler
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

#include <PDFBackend.h>
#include <QPainter>
#include <QApplication>

namespace QtPDF {

namespace Backend {

// TODO: Find a better place to put this
QBrush * pageDummyBrush = NULL;

QDateTime fromPDFDate(QString pdfDate)
{
  QDate date;
  QTime time;
  QString format;
  QDateTime retVal;
  int sign = 0;
  int hourOffset, minuteOffset = 0;
  bool ok;

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

#ifdef DEBUG
void PDFPageProcessingThread::dumpWorkStack(const QStack<PageProcessingRequest*> & ws)
{
  int i;
  QStringList strList;
  for (i = 0; i < ws.size(); ++i) {
    PageProcessingRequest * request = ws[i];
    if (!request)
      strList << QString::fromUtf8("NULL");
    else {
      strList << *request;
    }
  }
  qDebug() << strList;
}
#endif




// Fonts
// =================

PDFFontDescriptor::PDFFontDescriptor(const QString fontName /* = QString() */) :
  _name(fontName),
  _stretch(FontStretch_Normal),
  _weight(400),
  _italicAngle(0),
  _ascent(0),
  _descent(0),
  _leading(0),
  _capHeight(0),
  _xHeight(0),
  _stemV(0),
  _stemH(0),
  _avgWidth(0),
  _maxWidth(0),
  _missingWidth(0)
{
}

bool PDFFontDescriptor::isSubset() const
{
  // Subset fonts have a tag of 6 upper-case letters, followed by a '+',
  // prefixed to the font name
  if (_name.length() < 7 || _name[6] != QChar::fromLatin1('+'))
    return false;
  for (int i = 0; i < 6; ++i) {
    if (!_name[i].isUpper())
      return false;
  }
  return true;
}

QString PDFFontDescriptor::pureName() const
{
  if (!isSubset())
    return _name;
  else
    return _name.mid(7);
}


// Backend Rendering
// =================
// The `PDFPageProcessingThread` is a thread that processes background jobs.
// Each job is represented by a subclass of `PageProcessingRequest` and
// contains an `execute` method that performs the actual work.
PDFPageProcessingThread::PDFPageProcessingThread() :
  _idle(true),
  _quit(false)
{
}

PDFPageProcessingThread::~PDFPageProcessingThread()
{
  _mutex.lock();
  _quit = true;
  _waitCondition.wakeAll();
  _mutex.unlock();
  wait();
}

void PDFPageProcessingThread::addPageProcessingRequest(PageProcessingRequest * request)
{

  if (!request)
    return;

  // `request` must live in the main (GUI) thread, or else destroying it later
  // on will fail
  Q_ASSERT(request->thread() == QApplication::instance()->thread());

  QMutexLocker locker(&(this->_mutex));
  // Note: Commenting the "remove identical requests in the stack" code for now.
  // This should be handled by the caching routine elsewhere automatically. If
  // in doubt, it's better to render a tile twice than to not render it at all
  // (thereby leaving the dummy image in the cache indefinitely)
/*
  // remove any instances of the given request type before adding the new one to
  // avoid processing it several times
  // **TODO:** Could it be that we require several concurrent versions of the
  //           same page?
  int i;
  for (i = _workStack.size() - 1; i >= 0; --i) {
    if (*(_workStack[i]) == *request) {
      // Using deleteLater() doesn't work because we have no event queue in this
      // thread. However, since the object is still on the stack, it is still
      // sleeping and directly deleting it should therefore be safe.
      delete _workStack[i];
      _workStack.remove(i);
    }
  }
*/

  _workStack.push(request);
#ifdef DEBUG
  qDebug() << "new request:" << *request;
#endif

  locker.unlock();
  if (!isRunning())
    start();
  else
    _waitCondition.wakeOne();
}

void PDFPageProcessingThread::run()
{
  PageProcessingRequest * workItem;

  _mutex.lock();
  _idle = false;
  while (!_quit) {
    // mutex must be locked at start of loop
    if (_workStack.size() > 0) {
      workItem = _workStack.pop();
      _mutex.unlock();

#ifdef DEBUG
      qDebug() << "processing work item" << *workItem << "; remaining items:" << _workStack.size();
      _renderTimer.start();
#endif
      workItem->execute();
#ifdef DEBUG
      QString jobDesc;
      switch (workItem->type()) {
        case PageProcessingRequest::LoadLinks:
          jobDesc = QString::fromUtf8("loading links");
          break;
        case PageProcessingRequest::PageRendering:
          jobDesc = QString::fromUtf8("rendering page");
          break;
      }
      qDebug() << "finished " << jobDesc << "for page" << workItem->page->pageNum() << ". Time elapsed: " << _renderTimer.elapsed() << " ms.";
#endif

      // Delete the work item as it has fulfilled its purpose
      // Note that we can't delete it here or we might risk that some emitted
      // signals are invalidated; to ensure they reach their destination, we
      // need to call deleteLater().
      // Note: workItem *must* live in the main (GUI) thread for this!
      Q_ASSERT(workItem->thread() == QApplication::instance()->thread());
      workItem->deleteLater();

      _mutex.lock();
    }
    else {
#ifdef DEBUG
      qDebug() << "going to sleep";
#endif
      _idle = true;
      _idleCondition.wakeAll();
      _waitCondition.wait(&_mutex);
      _idle = false;
#ifdef DEBUG
      qDebug() << "waking up";
#endif
    }
  }
  _mutex.unlock();
}

void PDFPageProcessingThread::clearWorkStack()
{
  _mutex.lock();

  foreach(PageProcessingRequest * workItem, _workStack) {
    if (!workItem)
      continue;
    Q_ASSERT(workItem->thread() == QApplication::instance()->thread());
    workItem->deleteLater();
  }
  _workStack.clear();

  if (!_idle) {
    // Wait until the current operation finishes
    _idleCondition.wait(&_mutex);
  }
  _mutex.unlock();
}


// Asynchronous Page Operations
// ----------------------------
//
// The `execute` functions here are called by the processing theread to perform
// background jobs such as page rendering or link loading. This alows the GUI
// thread to stay unblocked and responsive. The results of background jobs are
// posted as events to a `listener` which can be any subclass of `QObject`. The
// `listener` will need a custom `event` function that is capable of picking up
// on these events.

bool PageProcessingRequest::operator==(const PageProcessingRequest & r) const
{
  // TODO: Should we care about the listener here as well?
  return (type() == r.type() && page == r.page);
}

bool PageProcessingRenderPageRequest::operator==(const PageProcessingRequest & r) const
{
  if (!PageProcessingRequest::operator==(r))
    return false;
  const PageProcessingRenderPageRequest * rr = static_cast<const PageProcessingRenderPageRequest*>(&r);
  // TODO: Should we care about the listener here as well?
  return (xres == rr->xres && yres == rr->yres && render_box == rr->render_box && cache == rr->cache);
}

#ifdef DEBUG
PageProcessingRenderPageRequest::operator QString() const
{
  return QString::fromUtf8("RP:%1.%2_%3").arg(page->pageNum()).arg(render_box.topLeft().x()).arg(render_box.topLeft().y());
}
#endif

// ### Custom Event Types
// These are the events posted by `execute` functions.
const QEvent::Type PDFPageRenderedEvent::PageRenderedEvent = static_cast<QEvent::Type>( QEvent::registerEventType() );
const QEvent::Type PDFLinksLoadedEvent::LinksLoadedEvent = static_cast<QEvent::Type>( QEvent::registerEventType() );

bool PageProcessingRenderPageRequest::execute()
{
  // TODO: Aborting renders doesn't really work right now---the backend knows
  // nothing about the PDF scenes.
  //
  // Idea: Perhaps allow page render requests to provide a pointer to a function
  // that returns a `bool` value indicating if the request is still valid? Then
  // the `PDFPageGraphicsItem` could have a function that indicates if the item
  // is anywhere near a viewport.
  QImage rendered_page = page->renderToImage(xres, yres, render_box, cache);
  QCoreApplication::postEvent(listener, new PDFPageRenderedEvent(xres, yres, render_box, rendered_page));

  return true;
}

bool PageProcessingLoadLinksRequest::execute()
{
  QCoreApplication::postEvent(listener, new PDFLinksLoadedEvent(page->loadLinks()));
  return true;
}

#ifdef DEBUG
PageProcessingLoadLinksRequest::operator QString() const
{
  return QString::fromUtf8("LL:%1").arg(page->pageNum());
}
#endif

#ifdef DEBUG
PDFPageTile::operator QString() const
{
  return QString::fromUtf8("p%1,%2x%3,r%4|%5x%6|%7").arg(page_num).arg(xres).arg(yres).arg(render_box.x()).arg(render_box.y()).arg(render_box.width()).arg(render_box.height());
}
#endif

// Taken from Qt 4.7.2 sources (<Qt>/src/corelib/tools/qhash.cpp)
static uint hash(const uchar *p, int n)
{
  uint h = 0;

  while (n--) {
    h = (h << 4) + *p++;
    h ^= (h & 0xf0000000) >> 23;
    h &= 0x0fffffff;
  }
  return h;
}

inline uint qHash(const QRect &key) {
  return qHash(
        QPair< QPair< int, int >, QPair< int, int > >(
          QPair< int, int >(key.x(), key.y()),
          QPair< int, int >(key.width(), key.height())
        )
        );
}

inline uint qHash(const double &d)
{
  // We interpret the double as an array of bytes and use the hash() function on
  // it.
  // NOTE: Due to rounding errors, this is not 100% reliable - two doubles that
  // _look_ the same may actually differ in their bit representations (e.g., if
  // the same value was calculated in two different ways). So this function may
  // report different hashes for doubles that look the same (which should not be
  // a problem in our case, however).
  // Note also that the QDataStream approach used previously also works on the
  // binary representation of doubles internally and so the same problem would
  // occur there as well.
  return hash((const uchar*)&d, sizeof(d));
}

// ### Cache for Rendered Images
inline uint qHash(const PDFPageTile &tile)
{
  uint h1 = qHash(QPair<uint, uint>(qHash(tile.xres), qHash(tile.yres)));
  uint h2 = qHash(QPair<uint,int>(qHash(tile.render_box), tile.page_num));
  return qHash(QPair<uint, uint>(h1, h2));
}

QSharedPointer<QImage> PDFPageCache::getImage(const PDFPageTile & tile) const
{
  _lock.lockForRead();
  QSharedPointer<QImage> * retVal = object(tile);
  _lock.unlock();
  if (retVal)
    return *retVal;
  return QSharedPointer<QImage>();
}

PDFPageCache::TileStatus PDFPageCache::getStatus(const PDFPageTile & tile) const
{
  PDFPageCache::TileStatus retVal = UNKNOWN;
  _lock.lockForRead();
  if (_tileStatus.contains(tile))
    retVal = _tileStatus[tile];
  _lock.unlock();
  return retVal;
}

QSharedPointer<QImage> PDFPageCache::setImage(const PDFPageTile & tile, QImage * image, const TileStatus status, const bool overwrite /* = true */)
{
  _lock.lockForWrite();
  QSharedPointer<QImage> retVal;
  if (contains(tile))
    retVal = *object(tile);
  // If the key is not in the cache yet add it. Otherwise overwrite the cached
  // image but leave the pointer intact as that can be held/used elsewhere
  if (!retVal) {
    QSharedPointer<QImage> * toInsert = new QSharedPointer<QImage>(image);
    insert(tile, toInsert, (image ? image->byteCount() : 0));
    _tileStatus.insert(tile, status);
    retVal = *toInsert;
  }
  else if (retVal.data() == image) {
    // Trying to overwrite an image with itself - just update the status
    _tileStatus.insert(tile, status);
  }
  else if (overwrite) {
    // TODO: overwriting an image with a different one can change its size (and
    // therefore its cost in the cache). There doesn't seem to be a method to
    // hande that in QCache, though, and since we only use one tile size this
    // shouldn't pose a problem.
    if (image)
      *retVal = *image;
    else {
      QSharedPointer<QImage> * toInsert = new QSharedPointer<QImage>;
      insert(tile, toInsert, 0);
      retVal = *toInsert;
    }
    _tileStatus.insert(tile, status);
  }
  _lock.unlock();
  return retVal;
}

void PDFPageCache::markOutdated()
{
  QWriteLocker l(&_lock);
  QMap<PDFPageTile, TileStatus>::iterator it;
  for (it = _tileStatus.begin(); it != _tileStatus.end(); ++it)
    it.value() = OUTDATED;
}


// PDF ABCs
// ========

// Document Class
// --------------
//
// This class is thread-safe. Data access is governed by the QReadWriteLock
// _docLock.
Document::Document(QString fileName):
  _numPages(-1),
  _fileName(fileName),
  _meta_trapped(Trapped_Unknown),
  _docLock(new QReadWriteLock(QReadWriteLock::Recursive))
{
  Q_ASSERT(_docLock != NULL);

#ifdef DEBUG
//  qDebug() << "Document::Document(" << fileName << ")";
#endif

  // Set cache for rendered pages to be 1GB. This is enough for 256 RGBA tiles
  // (1024 x 1024 pixels x 4 bytes per pixel).
  //
  // NOTE: The application seems to exceed 1 GB---usage plateaus at around 2GB. No idea why. Perhaps freed
  // blocks are not garbage collected?? Perhaps my math is off??
  _pageCache.setMaxSize(1024 * 1024 * 1024);
}

Document::~Document()
{
#ifdef DEBUG
//  qDebug() << "Document::~Document()";
#endif
  clearPages();
}

int Document::numPages() { QReadLocker docLocker(_docLock.data()); return _numPages; }
PDFPageProcessingThread &Document::processingThread() { QReadLocker docLocker(_docLock.data()); return _processingThread; }
PDFPageCache &Document::pageCache() { QReadLocker docLocker(_docLock.data()); return _pageCache; }

QList<SearchResult> Document::search(QString searchText, SearchFlags flags, int startPage)
{
  QReadLocker docLocker(_docLock.data());
  QList<SearchResult> results;
  int i, start, end, step;

  start = startPage;
  end = (flags & Search_Backwards ? -1 : _numPages);
  step = (flags & Search_Backwards ? -1 : +1);

  for (i = start; i != end; i += step) {
    QSharedPointer<Page> page(_pages[i]);
    if (!page)
      continue;
    results << page->search(searchText, flags);
  }

  if (flags & Search_WrapAround) {
    start = (flags & Search_Backwards ? _numPages - 1 : 0);
    end = startPage;
    for (i = start; i != end; i += step) {
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
// Note that the Page may exist in a detached state, i.e., _parent == NULL. This
// is typically the case when the document discarded the page object but some
// other object (typically in another thread) still holds a QSharedPointer to it.
Page::Page(Document *parent, int at, QSharedPointer<QReadWriteLock> docLock):
  _parent(parent),
  _n(at),
  _transition(NULL),
  _pageLock(new QReadWriteLock(QReadWriteLock::Recursive)),
  _docLock(docLock)
{
  Q_ASSERT(_pageLock);

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

Page::~Page()
{
#ifdef DEBUG
//  qDebug() << "Page::~Page(" << _n << ")";
#endif
}

int Page::pageNum() { QReadLocker pageLocker(_pageLock); return _n; }

void Page::detachFromParent()
{
  QWriteLocker pageLocker(_pageLock);
  _parent = NULL;
}

QSharedPointer<QImage> Page::getCachedImage(double xres, double yres, QRect render_box /* = QRect() */, PDFPageCache::TileStatus * status /* = NULL */)
{
  QReadLocker docLocker(_docLock.data());
  QReadLocker pageLocker(_pageLock);
  if (!_parent) {
    if (status)
      *status = PDFPageCache::UNKNOWN;
    return QSharedPointer<QImage>();
  }
  PDFPageTile tile(xres, yres, render_box, _n);
  if (status)
    *status = _parent->pageCache().getStatus(tile);
  return _parent->pageCache().getImage(tile);
}

void Page::asyncRenderToImage(QObject *listener, double xres, double yres, QRect render_box, bool cache)
{
  QReadLocker docLocker(_docLock.data());
  QReadLocker pageLocker(_pageLock);
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
  QReadLocker pageLocker(_pageLock);

  // If the render_box is empty, use the whole page
  if (render_box.isNull())
    render_box = QRectF(0, 0, pageSizeF().width() * xres / 72., pageSizeF().height() * yres / 72.).toAlignedRect();

  // If the tile is cached, return it if
  // 1) it is current
  // 2) it is a placeholder (in this case, it is currently rendering in the
  // background and we don't need to do anything)
  PDFPageCache::TileStatus status;
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
      _parent->pageCache().setImage(PDFPageTile(xres, yres, render_box, _n), retVal.data(), PDFPageCache::PLACEHOLDER, false);
    }
    else {
      // otherwise construct a dummy image
      QImage * tmpImg = new QImage(render_box.width(), render_box.height(), QImage::Format_ARGB32);
      QPainter p(tmpImg);
      p.fillRect(tmpImg->rect(), *pageDummyBrush);

      // Look through the cache to find tiles we can reuse (by scaling) for our
      // dummy tile
      // TODO: Benchmark this. If it is actualy too slow (i.e., just keeping the
      // rendered image from popping up due to the write lock we hold) disable it
      if (_parent) {
        QList<PDFPageTile> tiles = _parent->pageCache().tiles();
        for (QList<PDFPageTile>::iterator it = tiles.begin(); it != tiles.end(); ) {
          if (it->page_num != pageNum()) {
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
        qSort(tiles.begin(), tiles.end(), higherResolutionThan);
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
      retVal = _parent->pageCache().setImage(PDFPageTile(xres, yres, render_box, _n), tmpImg, PDFPageCache::PLACEHOLDER, false);
      if (retVal != tmpImg)
        delete tmpImg;
    }
    return retVal;
  }
  else {
    renderToImage(xres, yres, render_box, true);
    return getCachedImage(xres, yres, render_box);
  }
}

void Page::asyncLoadLinks(QObject *listener)
{
  QReadLocker docLocker(_docLock.data());
  QReadLocker pageLocker(_pageLock);
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

