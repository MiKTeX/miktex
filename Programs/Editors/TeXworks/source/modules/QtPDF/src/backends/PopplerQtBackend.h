/**
 * Copyright (C) 2013-2020  Charlie Sharpsteen, Stefan Löffler
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

// NOTE:
//
// ** THIS HEADER IS NOT MENT TO BE INCLUDED DIRECTLY **
//
// Instead, include `PDFBackend.h` which defines classes that this header
// relies on.
#ifndef PopplerBackend_H
#define PopplerBackend_H

#include "PDFBackend.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <poppler-qt6.h>
#else
#include <poppler-qt5.h>
#endif

namespace QtPDF {

namespace Backend {

namespace PopplerQt {

class Document;
class Page;

class Document: public Backend::Document
{
  typedef Backend::Document Super;
  friend class Page;

  QSharedPointer< ::Poppler::Document > _poppler_doc;

#if POPPLER_HAS_OUTLINE
  void recursiveConvertToC(QList<PDFToCItem> & items, const QVector<Poppler::OutlineItem> & popplerItems) const;
#else
  void recursiveConvertToC(QList<PDFToCItem> & items, QDomNode node) const;
#endif

protected:
  // Poppler is not threadsafe, so some operations need to be serialized with a
  // mutex.
  QMutex * _poppler_docLock{new QMutex};
  // Since ::Poppler::Document::fonts() is extremely slow, we need to cache the
  // result.
  mutable QList<PDFFontInfo> _fonts;
  mutable bool _fontsLoaded{false};

  // The following two methods are not thread-safe because they don't acquire a
  // read lock. This is to enable methods that have a write lock to use them.
  bool _isValid() const { return (_poppler_doc != nullptr); }
  bool _isLocked() const { return (_poppler_doc ? _poppler_doc->isLocked() : false); }

public:
  Document(const QString & fileName);
  ~Document() override;

  bool isValid() const override { QReadLocker docLocker(_docLock.data()); return _isValid(); }
  bool isLocked() const override { QReadLocker docLocker(_docLock.data()); return _isLocked(); }

  void reload() override;
  bool unlock(const QString password) override;

  QWeakPointer<Backend::Page> page(int at) override;
  PDFDestination resolveDestination(const PDFDestination & namedDestination) const override;

  PDFToC toc() const override;
  QList<PDFFontInfo> fonts() const override;

private:
  void parseDocument();
};


class Page: public Backend::Page
{
  friend class Document;

  typedef Backend::Page Super;
  QSharedPointer< ::Poppler::Page > _poppler_page;
  QList< QSharedPointer<Annotation::AbstractAnnotation> > _annotations;
  QList< QSharedPointer<Annotation::Link> > _links;
  bool _annotationsLoaded{false};
  bool _linksLoaded{false};

  void loadTransitionData();

protected:
  Page(Document *parent, int at, QSharedPointer<QReadWriteLock> docLock);

public:
  ~Page() override;

  QSizeF pageSizeF() const override;

  QImage renderToImage(double xres, double yres, QRect render_box = QRect(), bool cache = false) const override;

  QList< QSharedPointer<Annotation::Link> > loadLinks() override;
  QList< QSharedPointer<Annotation::AbstractAnnotation> > loadAnnotations() override;
  QList< Backend::Page::Box > boxes() override;
  QString selectedText(const QList<QPolygonF> & selection, QMap<int, QRectF> * wordBoxes = nullptr, QMap<int, QRectF> * charBoxes = nullptr, const bool onlyFullyEnclosed = false) override;

  QList<Backend::SearchResult> search(const QString & searchText, const SearchFlags & flags) override;
};

} // namespace PopplerQt

} // namespace Backend

class PopplerQtBackend : public BackendInterface
{
  Q_OBJECT
  Q_INTERFACES(QtPDF::BackendInterface)
public:
  QSharedPointer<Backend::Document> newDocument(const QString & fileName) override;

  QString name() const override { return QString::fromLatin1("poppler-qt"); }
  bool canHandleFile(const QString & fileName) override { return QFileInfo(fileName).suffix() == QString::fromLatin1("pdf"); }
};

} // namespace QtPDF

#endif // End header guard
// vim: set sw=2 ts=2 et
