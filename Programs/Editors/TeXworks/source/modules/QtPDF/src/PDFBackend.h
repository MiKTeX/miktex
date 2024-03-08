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
#ifndef PDFBackend_H
#define PDFBackend_H

#include "PDFAnnotations.h"
#include "PDFFontInfo.h"
#include "PDFPageCache.h"
#include "PDFPageProcessingThread.h"
#include "PDFToC.h"
#include "PDFTransitions.h"

#include <QAbstractItemModel>
#include <QEvent>
#include <QFileInfo>
#include <QImage>
#include <QMutex>
#include <QReadLocker>
#include <QStack>
#include <QThread>
#include <QWaitCondition>
#include <QWeakPointer>

namespace QtPDF {

namespace Backend {

// Backend Rendering
// =================

class Page;
class Document;
struct SearchRequest;
struct SearchResult;

// TODO: Find a better place to put this
QDateTime fromPDFDate(QString pdfDate);


enum SearchFlag { Search_WrapAround = 0x01, Search_CaseInsensitive = 0x02, Search_Backwards = 0x04};
Q_DECLARE_FLAGS(SearchFlags, SearchFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(SearchFlags)


// PDF ABCs
// ========
// This header file defines a set of Abstract Base Classes (ABCs) for PDF
// documents. Having a set of abstract classes allows tools like GUI viewers to
// be written that are agnostic to the library that provides the actual PDF
// implementation: Poppler, MuPDF, etc.
// TODO: Should this class be derived from QObject to emit signals (e.g.,
// documentChanged() after reload, unlocking, etc.)?

// This class is thread-safe. See implementation for internals.
class Document
{
  friend class Page;

public:
  using size_type = QVector<QSharedPointer<Page>>::size_type;

  enum TrappedState { Trapped_Unknown, Trapped_True, Trapped_False };
  enum Permission { Permission_Print = 0x0004,
                    Permission_Change = 0x0008,
                    Permission_Extract = 0x0010, // text and graphics
                    Permission_Annotate = 0x0020, // Also includes filling forms
                    Permission_FillForm = 0x0100,
                    Permission_ExtractForAccessibility = 0x0200,
                    Permission_Assemble = 0x0400,
                    Permission_PrintHighRes = 0x0800
                  };
  Q_DECLARE_FLAGS(Permissions, Permission)

  static QSharedPointer<Backend::Document> newDocument(const QString & fileName, const QString & backend = {});
  static QStringList backends();
  static QString defaultBackend();
  static void setDefaultBackend(const QString & backend);

  virtual ~Document();

  // Uses doc-read-lock
  size_type numPages() const;
  // Uses doc-read-lock
  QString fileName() const { QReadLocker docLocker(_docLock.data()); return _fileName; }
  // Uses doc-read-lock
  PDFPageProcessingThread& processingThread();
  static PDFPageCache& pageCache() { return _pageCache; }

  // Uses doc-read-lock and may use doc-write-lock
  // NB: no const variant exists as we may need to create a new Page (if it was
  // not cached in _pages), which requires a non-const `this` pointer as parent
  virtual QWeakPointer<Page> page(size_type at);
  virtual PDFDestination resolveDestination(const PDFDestination & namedDestination) const {
    return (namedDestination.isExplicit() ? namedDestination : PDFDestination());
  }


  // Uses doc-read-lock
  Permissions permissions() const { QReadLocker docLocker(_docLock.data()); return _permissions; }
  // Uses doc-read-lock
  Permissions& permissions() { QReadLocker docLocker(_docLock.data()); return _permissions; }

  // Uses doc-read-lock
  virtual bool isValid() const = 0;
  // Uses doc-read-lock
  virtual bool isLocked() const = 0;
  // Uses doc-write-lock
  virtual void reload() = 0;

  // Returns `true` if unlocking was successful and `false` otherwise.
  // Uses doc-read-lock and may use doc-write-lock
  virtual bool unlock(const QString password) = 0;

  // Override in derived class if it provides access to the document outline
  // strutures of the pdf file.
  virtual PDFToC toc() const { return PDFToC(); }
  virtual QList<PDFFontInfo> fonts() const { return QList<PDFFontInfo>(); }
  virtual QAbstractItemModel * optionalContentModel() const { return nullptr; }

  // <metadata>
  QString title() const { QReadLocker docLocker(_docLock.data()); return _meta_title; }
  QString author() const { QReadLocker docLocker(_docLock.data()); return _meta_author; }
  QString subject() const { QReadLocker docLocker(_docLock.data()); return _meta_subject; }
  QString keywords() const { QReadLocker docLocker(_docLock.data()); return _meta_keywords; }
  QString creator() const { QReadLocker docLocker(_docLock.data()); return _meta_creator; }
  QString producer() const { QReadLocker docLocker(_docLock.data()); return _meta_producer; }
  QDateTime creationDate() const { QReadLocker docLocker(_docLock.data()); return _meta_creationDate; }
  QDateTime modDate() const { QReadLocker docLocker(_docLock.data()); return _meta_modDate; }
  QSizeF pageSize() const { QReadLocker docLocker(_docLock.data()); return _meta_pageSize; }
  qint64 fileSize() const { QReadLocker docLocker(_docLock.data()); return _meta_fileSize; }
  TrappedState trapped() const { QReadLocker docLocker(_docLock.data()); return _meta_trapped; }
  QMap<QString, QString> metaDataOther() const { QReadLocker docLocker(_docLock.data()); return _meta_other; }
  // </metadata>

  virtual QColor paperColor() const { return Qt::white; }
  virtual void setPaperColor(const QColor & color) { Q_UNUSED(color); }

  // Searches the entire document for the given string and returns a list of
  // boxes that contain that text.
  //
  // TODO:
  //
  //   - Implement as a function that returns a generator object which can
  //     return the search results one at a time rather than all at once.
  //
  //   - See TODO list in `Page::search`
  virtual QList<SearchResult> search(const QString & searchText, const SearchFlags & flags, const size_type startPage = 0);

protected:
  Document(const QString fileName);

  void clearPages();
  virtual void clearMetaData();

  size_type _numPages{-1};
  PDFPageProcessingThread _processingThread;
  static PDFPageCache _pageCache;
  QVector< QSharedPointer<Page> > _pages;
  Permissions _permissions;

  QString _fileName;

  QString _meta_title;
  QString _meta_author;
  QString _meta_subject;
  QString _meta_keywords;
  QSizeF _meta_pageSize;
  QString _meta_creator;
  QString _meta_producer;
  QDateTime _meta_creationDate;
  QDateTime _meta_modDate;
  qint64 _meta_fileSize{0};
  TrappedState _meta_trapped{Trapped_Unknown};
  QMap<QString, QString> _meta_other;
  QSharedPointer<QReadWriteLock> _docLock{new QReadWriteLock(QReadWriteLock::Recursive)};
};

// This class is thread-safe. See implementation for internals.
class Page
{
  friend class Document;

protected:
  using size_type = Document::size_type;

  Document *_parent{nullptr};
  const size_type _n{-1};
  std::unique_ptr<Transition::AbstractTransition> _transition;
  mutable QReadWriteLock _pageLock{QReadWriteLock::Recursive};
  const QSharedPointer<QReadWriteLock> _docLock;

  // Getter for derived classes (that are not friends of Document)
  QSharedPointer<QReadWriteLock> docLock() const { return _docLock; }
  // The caller must hold a doc-lock. Uses a page-write-lock.
  virtual void detachFromParent();

  Page(Document *parent, size_type at, QSharedPointer<QReadWriteLock> docLock);

  // Uses doc-read-lock and page-read-lock.
  QSharedPointer<QImage> getCachedImage(double xres, double yres, QRect render_box = QRect(), PDFPageCache::TileStatus * status = nullptr);

  // Uses doc-read-lock and page-read-lock.
  virtual void asyncRenderToImage(QObject *listener, double xres, double yres, QRect render_box = QRect(), bool cache = false);

public:
  // Class to encapsulate boxes, e.g., for selecting
  class Box {
  public:
    QRectF boundingBox;
    QList<Box> subBoxes;

    bool operator==(const Box & o) const {
      return (boundingBox == o.boundingBox && subBoxes == o.subBoxes);
    }
  };
  using BoxBoundaryList = QVector<QRectF>;

  virtual ~Page() = default;

  Document * document() { QReadLocker pageLocker(&_pageLock); return _parent; }
  size_type pageNum() const;
  virtual QSizeF pageSizeF() const = 0;
  virtual QRectF getContentBoundingBox() const;
  Transition::AbstractTransition * transition() const { QReadLocker pageLocker(&_pageLock); return _transition.get(); }

  virtual QList< QSharedPointer<Annotation::Link> > loadLinks() = 0;
  // Uses doc-read-lock and page-read-lock.
  virtual void asyncLoadLinks(QObject *listener);

  // Returns a list of boxes (e.g., for the purpose of selecting text)
  // Box rectangles are in pdf coordinates (i.e., bp)
  // The backend may return big boxes comprised of subboxes (e.g., words made up
  // of characters) to speed up hit calculations. Only one level of subboxes is
  // currently supported. The big box boundingBox must completely encompass all
  // subBoxes' boundingBoxes.
  virtual QList<Box> boxes() const { return QList<Box>(); }
  // Return selected text
  // The returned text should contain all characters inside (at least) one of
  // the `selection` polygons.
  // The `selection` polygons must be in pdf coords (i.e., in bp)
  // Optionally, the function can also return wordBoxes and/or charBoxes for
  // each character (i.e., a rect enclosing the word the character is part of
  // and/or a rect enclosing the actual character)
  virtual QString selectedText(const QList<QPolygonF> & selection, BoxBoundaryList * wordBoxes = nullptr, BoxBoundaryList * charBoxes = nullptr, const bool onlyFullyEnclosed = false) const {
    Q_UNUSED(selection)
    Q_UNUSED(onlyFullyEnclosed)
    if (wordBoxes) wordBoxes->clear();
    if (charBoxes) charBoxes->clear();
    return QString();
  }

  // Uses page-read-lock and doc-read-lock.
  virtual QImage renderToImage(double xres, double yres, QRect render_box = QRect(), bool cache = false) const = 0;

  // Returns either a cached image (if it exists), or triggers a render request.
  // If listener != nullptr, this is an asynchronous render request and the method
  // returns a dummy image (which is added to the cache to speed up future
  // requests). Otherwise, the method renders the page synchronously and returns
  // the result.
  // Uses page-read-lock and doc-read-lock.
  QSharedPointer<QImage> getTileImage(QObject * listener, const double xres, const double yres, QRect render_box = QRect());

  virtual QList< QSharedPointer<Annotation::AbstractAnnotation> > loadAnnotations() { return QList< QSharedPointer<Annotation::AbstractAnnotation> >(); }

  // Searches the page for the given text string and returns a list of boxes
  // that contain that text.
  //
  // TODO:
  //
  // Implement as a function that returns a generator object which can return
  // the search results one at a time rather than all at once which is time
  // consuming. Even better, allow the returned object to be used as a C++
  // iterator---then we could pass it off to QtConcurrent to generate results
  // in the background and access them through a QFuture.
  //
  // This is very tricky to do in C++. God I miss Python and its `itertools`
  // library.
  virtual QList<SearchResult> search(const QString & searchText, const SearchFlags & flags) const = 0;
  static QList<SearchResult> executeSearch(SearchRequest request);
};

struct SearchRequest
{
  QWeakPointer<Document> doc;
  Document::size_type pageNum;
  QString searchString;
  SearchFlags flags;
};

struct SearchResult
{
  Document::size_type pageNum;
  QRectF bbox;

  bool operator==(const SearchResult & o) const {
    return (pageNum == o.pageNum && bbox == o.bbox);
  }
};

} // namespace Backend

class BackendInterface : public QObject
{
  Q_OBJECT
public:
  ~BackendInterface() override = default;
  virtual QSharedPointer<Backend::Document> newDocument(const QString & fileName) = 0;
  virtual QString name() const = 0;
  virtual bool canHandleFile(const QString & fileName) = 0;
};

} // namespace QtPDF

Q_DECLARE_INTERFACE(QtPDF::BackendInterface, "org.tug.QtPDF/1.0")

// Backend Implementations
// =======================
// These provide library-specific concrete impelemntations of the abstract base
// classes defined here.
// NOTE: The backend implementations must be included _outside_ the namespace,
// as that could otherwise interfere with other header files (e.g., those of
// poppler-qt4)
#ifdef USE_POPPLERQT
#include <backends/PopplerQtBackend.h> // Invokes GPL v2+ License
#endif
#ifdef USE_MUPDF
#include <backends/MuPDFBackend.h>   // Invokes GPL v3 License
#endif

#endif // End header guard
// vim: set sw=2 ts=2 et

