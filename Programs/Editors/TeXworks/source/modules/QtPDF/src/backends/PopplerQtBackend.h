/**
 * Copyright (C) 2013-2018  Charlie Sharpsteen, Stefan Löffler
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
#if QT_VERSION < 0x050000
  #include <poppler-qt4.h>
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

  void recursiveConvertToC(QList<PDFToCItem> & items, QDomNode node) const;

protected:
  // Poppler is not threadsafe, so some operations need to be serialized with a
  // mutex.
  QMutex * _poppler_docLock;
  QList<PDFFontInfo> _fonts;
  bool _fontsLoaded;

  // The following two methods are not thread-safe because they don't acquire a
  // read lock. This is to enable methods that have a write lock to use them.
  bool _isValid() const { return (_poppler_doc != NULL); }
  bool _isLocked() const { return (_poppler_doc ? _poppler_doc->isLocked() : false); }

public:
  Document(QString fileName);
  ~Document();

  bool isValid() const { QReadLocker docLocker(_docLock.data()); return _isValid(); }
  bool isLocked() const { QReadLocker docLocker(_docLock.data()); return _isLocked(); }

  void reload();
  bool unlock(const QString password);

  QWeakPointer<Backend::Page> page(int at);
  QWeakPointer<Backend::Page> page(int at) const;
  PDFDestination resolveDestination(const PDFDestination & namedDestination) const;

  PDFToC toc() const;
  QList<PDFFontInfo> fonts() const;

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
  bool _annotationsLoaded;
  bool _linksLoaded;

  void loadTransitionData();

protected:
  Page(Document *parent, int at, QSharedPointer<QReadWriteLock> docLock);

public:
  ~Page();

  QSizeF pageSizeF() const;

  QImage renderToImage(double xres, double yres, QRect render_box = QRect(), bool cache = false) const;

  QList< QSharedPointer<Annotation::Link> > loadLinks();
  QList< QSharedPointer<Annotation::AbstractAnnotation> > loadAnnotations();
  QList< Backend::Page::Box > boxes();
  QString selectedText(const QList<QPolygonF> & selection, QMap<int, QRectF> * wordBoxes = NULL, QMap<int, QRectF> * charBoxes = NULL, const bool onlyFullyEnclosed = false);

  QList<Backend::SearchResult> search(QString searchText, SearchFlags flags);
};

} // namespace PopplerQt

} // namespace Backend

class PopplerQtBackend : public BackendInterface
{
  Q_OBJECT
  Q_INTERFACES(QtPDF::BackendInterface)
public:
  PopplerQtBackend() { }
  virtual ~PopplerQtBackend() { }

  virtual QSharedPointer<Backend::Document> newDocument(const QString & fileName) {
    return QSharedPointer<Backend::Document>(new Backend::PopplerQt::Document(fileName));
  }

  virtual QString name() const { return QString::fromLatin1("poppler-qt"); }
  virtual bool canHandleFile(const QString & fileName) { return QFileInfo(fileName).suffix() == QString::fromLatin1("pdf"); }
};

} // namespace QtPDF

#endif // End header guard
// vim: set sw=2 ts=2 et
