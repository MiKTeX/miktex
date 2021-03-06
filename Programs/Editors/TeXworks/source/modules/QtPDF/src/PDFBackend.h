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
#ifndef PDFBackend_H
#define PDFBackend_H

#include "PDFAnnotations.h"
#include "PDFFontDescriptor.h"
#include "PDFPageTile.h"
#include "PDFToC.h"
#include "PDFTransitions.h"

#include <QCache>
#include <QEvent>
#include <QFileInfo>
#include <QImage>
#include <QMap>
#include <QMutex>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <QStack>
#include <QThread>
#include <QWaitCondition>
#include <QWeakPointer>
#include <QWriteLocker>

namespace QtPDF {

namespace Backend {

// Backend Rendering
// =================

class Page;
class Document;

// TODO: Find a better place to put this
QDateTime fromPDFDate(QString pdfDate);

// Note: This is a hack, but since all the information (with the exception of
// the type of font) we use (and that is provided by poppler) is encapsulated in
// PDFFontDescriptor, there is no use right now to completely implement all the
// different font structures
class PDFFontInfo
{
public:
  enum FontType { FontType_Type0, FontType_Type1, FontType_MMType1, \
                  FontType_Type3, FontType_TrueType };
  enum CIDFontType { CIDFont_None, CIDFont_Type0, CIDFont_Type2 };
  enum FontProgramType { ProgramType_None, ProgramType_Type1, \
                         ProgramType_TrueType, ProgramType_Type1CFF, \
                         ProgramType_CIDCFF, ProgramType_OpenType };
  enum FontSource { Source_Embedded, Source_File, Source_Builtin };

  PDFFontInfo() = default;
  virtual ~PDFFontInfo() = default;

  FontType fontType() const { return _fontType; }
  CIDFontType CIDType() const { return _CIDType; }
  FontProgramType fontProgramType() const { return _fontProgramType; }
  PDFFontDescriptor descriptor() const { return _descriptor; }
  // returns the path to the file used for rendering this font, or an invalid
  // QFileInfo for embedded fonts
  QFileInfo fileName() const { return _substitutionFile; }

  bool isSubset() const { return _descriptor.isSubset(); }
  FontSource source() const { return _source; }

  // TODO: Implement some advanced logic; e.g., non-embedded fonts have no font
  // program type
  void setFontType(const FontType fontType) { _fontType = fontType; }
  void setCIDType(const CIDFontType CIDType) { _CIDType = CIDType; }
  void setFontProgramType(const FontProgramType programType) { _fontProgramType = programType; }
  void setDescriptor(const PDFFontDescriptor & descriptor) { _descriptor = descriptor; }
  void setFileName(const QFileInfo & file) { _source = Source_File; _substitutionFile = file; }
  void setSource(const FontSource source) { _source = source; }

  bool operator==(const PDFFontInfo & o) const {
    return (_source == o._source && _descriptor == o._descriptor &&
      _substitutionFile == o._substitutionFile && _fontType == o._fontType &&
      _CIDType == o._CIDType && _fontProgramType == o._fontProgramType);
  }

protected:
  FontSource _source{Source_Builtin};
  PDFFontDescriptor _descriptor;
  QFileInfo _substitutionFile;
  FontType _fontType{FontType_Type1};
  CIDFontType _CIDType{CIDFont_None};
  FontProgramType _fontProgramType{ProgramType_None};
};

// This class is thread-safe
class PDFPageCache : protected QCache<PDFPageTile, QSharedPointer<QImage> >
{
  typedef QCache<PDFPageTile, QSharedPointer<QImage> > Super;
public:
  enum TileStatus { UNKNOWN, PLACEHOLDER, CURRENT, OUTDATED };

  PDFPageCache() = default;
  virtual ~PDFPageCache() = default;

  // Note: Each image has a cost of 1
  int maxSize() const { return maxCost(); }
  void setMaxSize(const int num) { setMaxCost(num); }

  // Returns the image under the key `tile` or nullptr if it doesn't exist
  QSharedPointer<QImage> getImage(const PDFPageTile & tile) const;
  TileStatus getStatus(const PDFPageTile & tile) const;
  // Returns the pointer to the image in the cache under they key `tile` after
  // the insertion. If overwrite == true, this will always be image, otherwise
  // it can be different
  QSharedPointer<QImage> setImage(const PDFPageTile & tile, QImage * image, const TileStatus status, const bool overwrite = true);


  void lock() const { _lock.lockForRead(); }
  void unlock() const { _lock.unlock(); }

  void clear() { QWriteLocker l(&_lock); Super::clear(); _tileStatus.clear(); }
  // Mark all tiles outdated
  void markOutdated();

  QList<PDFPageTile> tiles() const { return keys(); }
protected:
  mutable QReadWriteLock _lock;
  // Map to keep track of the current status of tiles; note that the status
  // information is not deleted when the QCache scraps images to save memory.
  QMap<PDFPageTile, TileStatus> _tileStatus;
};

class PageProcessingRequest : public QObject
{
  Q_OBJECT
  friend class PDFPageProcessingThread;

  // Protect c'tor and execute() so we can't access them except in derived
  // classes and friends
protected:
  PageProcessingRequest(Page *page, QObject *listener) : page(page), listener(listener) { }
  // Should perform whatever processing it is designed to do
  // Returns true if finished successfully, false otherwise
  virtual bool execute() = 0;

public:
  enum Type { PageRendering, LoadLinks };

  ~PageProcessingRequest() override = default;
  virtual Type type() const = 0;

  Page *page;
  QObject *listener;

  virtual bool operator==(const PageProcessingRequest & r) const;
#ifdef DEBUG
  virtual operator QString() const = 0;
#endif
};

class PageProcessingRenderPageRequest : public PageProcessingRequest
{
  Q_OBJECT
  friend class PDFPageProcessingThread;

public:
  PageProcessingRenderPageRequest(Page *page, QObject *listener, double xres, double yres, QRect render_box = QRect(), bool cache = false) :
    PageProcessingRequest(page, listener),
    xres(xres), yres(yres),
    render_box(render_box),
    cache(cache)
  {}
  Type type() const override { return PageRendering; }

  bool operator==(const PageProcessingRequest & r) const override;
#ifdef DEBUG
  operator QString() const override;
#endif

protected:
  bool execute() override;

  double xres, yres;
  QRect render_box;
  bool cache;
};


class PDFPageRenderedEvent : public QEvent
{

public:
  PDFPageRenderedEvent(double xres, double yres, QRect render_rect, QImage rendered_page):
    QEvent(PageRenderedEvent),
    xres(xres), yres(yres),
    render_rect(render_rect),
    rendered_page(rendered_page)
  {}

  static const QEvent::Type PageRenderedEvent;

  const double xres, yres;
  const QRect render_rect;
  const QImage rendered_page;

};


class PageProcessingLoadLinksRequest : public PageProcessingRequest
{
  Q_OBJECT
  friend class PDFPageProcessingThread;

public:
  PageProcessingLoadLinksRequest(Page *page, QObject *listener) : PageProcessingRequest(page, listener) { }
  Type type() const override { return LoadLinks; }

#ifdef DEBUG
  operator QString() const override;
#endif

protected:
  bool execute() override;
};


class PDFLinksLoadedEvent : public QEvent
{

public:
  PDFLinksLoadedEvent(const QList< QSharedPointer<Annotation::Link> > links):
    QEvent(LinksLoadedEvent),
    links(links)
  {}

  static const QEvent::Type LinksLoadedEvent;

  const QList< QSharedPointer<Annotation::Link> > links;

};


// Class to perform (possibly) lengthy operations on pages in the background
// Modelled after the "Blocking Fortune Client Example" in the Qt docs
// (http://doc.qt.nokia.com/stable/network-blockingfortuneclient.html)

// The `PDFPageProcessingThread` is a thread that processes background jobs.
// Each job is represented by a subclass of `PageProcessingRequest` and
// contains an `execute` method that performs the actual work.
class PDFPageProcessingThread : public QThread
{
  Q_OBJECT

public:
  PDFPageProcessingThread() = default;
  ~PDFPageProcessingThread() override;

  // add a processing request to the work stack
  // Note: request must have been created on the heap and must be in the scope
  // of this thread; use requestRenderPage() and requestLoadLinks() for that
  void addPageProcessingRequest(PageProcessingRequest * request);

  // drop all remaining processing requests
  // WARNING: This function *must not* be called while the calling thread holds
  // any locks that would prevent and work item from finishing. Otherwise, we
  // could run into the following deadlock scenario:
  // clearWorkStack() waits for the currently active work items to finish. The
  // currently active work item waits to acquire a lock necessary for it to
  // finish. However, that lock is held by the caller of clearWorkStack().
  void clearWorkStack();

protected:
  void run() override;

private:
  QStack<PageProcessingRequest*> _workStack;
  QMutex _mutex;
  QWaitCondition _waitCondition;
  bool _idle{true};
  QWaitCondition _idleCondition;
  bool _quit{false};
#ifdef DEBUG
  static void dumpWorkStack(const QStack<PageProcessingRequest*> & ws);
#endif

};

enum SearchFlag { Search_WrapAround = 0x01, Search_CaseInsensitive = 0x02, Search_Backwards = 0x04};
Q_DECLARE_FLAGS(SearchFlags, SearchFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(SearchFlags)

struct SearchRequest
{
  QWeakPointer<Document> doc;
  int pageNum;
  QString searchString;
  SearchFlags flags;
};

struct SearchResult
{
  unsigned int pageNum;
  QRectF bbox;

  bool operator==(const SearchResult & o) const {
    return (pageNum == o.pageNum && bbox == o.bbox);
  }
};


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
  int numPages();
  // Uses doc-read-lock
  QString fileName() const { QReadLocker docLocker(_docLock.data()); return _fileName; }
  // Uses doc-read-lock
  PDFPageProcessingThread& processingThread();
  // Uses doc-read-lock
  PDFPageCache& pageCache();

  // Uses doc-read-lock and may use doc-write-lock
  // NB: no const variant exists as we may need to create a new Page (if it was
  // not cached in _pages), which requires a non-const `this` pointer as parent
  virtual QWeakPointer<Page> page(int at);
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

  // Searches the entire document for the given string and returns a list of
  // boxes that contain that text.
  //
  // TODO:
  //
  //   - Implement as a function that returns a generator object which can
  //     return the search results one at a time rather than all at once.
  //
  //   - See TODO list in `Page::search`
  virtual QList<SearchResult> search(const QString & searchText, const SearchFlags & flags, const int startPage = 0);

protected:
  Document(const QString fileName);

  void clearPages();
  virtual void clearMetaData();

  int _numPages{-1};
  PDFPageProcessingThread _processingThread;
  PDFPageCache _pageCache;
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
  Document *_parent{nullptr};
  const int _n{-1};
  Transition::AbstractTransition * _transition{nullptr};
  QReadWriteLock * _pageLock{new QReadWriteLock(QReadWriteLock::Recursive)};
  const QSharedPointer<QReadWriteLock> _docLock;

  // Getter for derived classes (that are not friends of Document)
  QSharedPointer<QReadWriteLock> docLock() const { return _docLock; }
  // The caller must hold a doc-lock. Uses a page-write-lock.
  virtual void detachFromParent();

  Page(Document *parent, int at, QSharedPointer<QReadWriteLock> docLock);

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

  virtual ~Page() = default;

  Document * document() { QReadLocker pageLocker(_pageLock); return _parent; }
  int pageNum();
  virtual QSizeF pageSizeF() const = 0;
  virtual QRectF getContentBoundingBox() const;
  Transition::AbstractTransition * transition() { QReadLocker pageLocker(_pageLock); return _transition; }

  virtual QList< QSharedPointer<Annotation::Link> > loadLinks() = 0;
  // Uses doc-read-lock and page-read-lock.
  virtual void asyncLoadLinks(QObject *listener);

  // Returns a list of boxes (e.g., for the purpose of selecting text)
  // Box rectangles are in pdf coordinates (i.e., bp)
  // The backend may return big boxes comprised of subboxes (e.g., words made up
  // of characters) to speed up hit calculations. Only one level of subboxes is
  // currently supported. The big box boundingBox must completely encompass all
  // subBoxes' boundingBoxes.
  virtual QList<Box> boxes() { return QList<Box>(); }
  // Return selected text
  // The returned text should contain all characters inside (at least) one of
  // the `selection` polygons.
  // The `selection` polygons must be in pdf coords (i.e., in bp)
  // Optionally, the function can also return wordBoxes and/or charBoxes for
  // each character (i.e., a rect enclosing the word the character is part of
  // and/or a rect enclosing the actual character)
  virtual QString selectedText(const QList<QPolygonF> & selection, QMap<int, QRectF> * wordBoxes = nullptr, QMap<int, QRectF> * charBoxes = nullptr, const bool onlyFullyEnclosed = false) {
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
  virtual QList<SearchResult> search(const QString & searchText, const SearchFlags & flags) = 0;
  static QList<SearchResult> executeSearch(SearchRequest request);
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

