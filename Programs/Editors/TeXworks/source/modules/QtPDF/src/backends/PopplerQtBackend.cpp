/**
 * Copyright (C) 2013-2021  Charlie Sharpsteen, Stefan Löffler
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

// NOTE: `PopplerQtBackend.h` is included via `PDFBackend.h`
#include "PDFBackend.h"

#include <QBitArray>

#if !defined(MIKTEX)
#if defined(HAVE_POPPLER_XPDF_HEADERS) && defined(Q_OS_DARWIN)
#include "poppler-config.h"
#include "GlobalParams.h"
#include <QCoreApplication>
#include <QDir>
#include <memory>
#endif
#endif

// Comparison operator for QSizeF needed to use QSizeF as keys in a QMap
// NB: Must be in the global namespace
inline bool operator<(const QSizeF & a, const QSizeF & b) {
    qreal areaA = a.width() * a.height();
    qreal areaB = b.width() * b.height();
    return (areaA < areaB || (areaA == areaB && a.width() < b.width()));
}

namespace QtPDF {

namespace Backend {

namespace PopplerQt {

// TODO: Find a better place to put this
PDFDestination toPDFDestination(const ::Poppler::Document * doc, const ::Poppler::LinkDestination & dest)
{
  if (!dest.destinationName().isEmpty())
    return PDFDestination(dest.destinationName());

  // Coordinates in LinkDestination are in the range of 0..1, which does not
  // comply with the pdf specs---so we have to convert them back
  qreal w = 1., h = 1.;
  if (doc) {
    ::Poppler::Page * p = doc->page(dest.pageNumber() - 1);
    if (p) {
      w = p->pageSizeF().width();
      h = p->pageSizeF().height();
    }
  }

  PDFDestination retVal(dest.pageNumber() - 1);
  // Note: Poppler seems to give coordinates in a vertically mirrored coordinate
  // system, i.e., y=0 corresponds to the top of the page. This does not
  // comply with the pdf page coordinate system, which has y=0 at the bottom. So
  // we need to compensate for that.
  switch (dest.kind()) {
    case ::Poppler::LinkDestination::destXYZ:
      retVal.setType(PDFDestination::Destination_XYZ);
      retVal.setRect(QRectF((dest.isChangeLeft() ? dest.left() * w : -1), (dest.isChangeTop() ? (1 - dest.top()) * h : -1), -1, -1));
      retVal.setZoom((dest.isChangeZoom() ? dest.zoom() : -1));
      break;
    case ::Poppler::LinkDestination::destFit:
      retVal.setType(PDFDestination::Destination_Fit);
      break;
    case ::Poppler::LinkDestination::destFitH:
      retVal.setType(PDFDestination::Destination_FitH);
      retVal.setRect(QRectF(-1, (dest.isChangeTop() ? (1 - dest.top()) * h : -1), -1, -1));
      break;
    case ::Poppler::LinkDestination::destFitV:
      retVal.setType(PDFDestination::Destination_FitV);
      retVal.setRect(QRectF((dest.isChangeLeft() ? dest.left() * w : -1), -1, -1, -1));
      break;
    case ::Poppler::LinkDestination::destFitR:
      retVal.setType(PDFDestination::Destination_FitR);
      retVal.setRect(QRectF(QPointF(dest.left() * w, (1 - dest.top()) * h), QPointF(dest.right() * w, dest.bottom() * h)));
      break;
    case ::Poppler::LinkDestination::destFitB:
      retVal.setType(PDFDestination::Destination_FitB);
      break;
    case ::Poppler::LinkDestination::destFitBH:
      retVal.setType(PDFDestination::Destination_FitBH);
      retVal.setRect(QRectF(-1, (dest.isChangeTop() ? (1 - dest.top()) * h : -1), -1, -1));
      break;
    case ::Poppler::LinkDestination::destFitBV:
      retVal.setType(PDFDestination::Destination_FitBV);
      retVal.setRect(QRectF((dest.isChangeLeft() ? dest.left() * w : -1), -1, -1, -1));
      break;
  }
  return retVal;
}

void convertAnnotation(Annotation::AbstractAnnotation * dest, const ::Poppler::Annotation * src, const QWeakPointer<Backend::Page> & thePage)
{
  QSharedPointer<Backend::Page> page(thePage.toStrongRef());
  if (!dest || !src || !page)
    return;

  QTransform denormalize = QTransform::fromScale(page->pageSizeF().width(), -page->pageSizeF().height()).translate(0,  -1);

  dest->setRect(denormalize.mapRect(src->boundary()));
  dest->setContents(src->contents());
  dest->setName(src->uniqueName());
  dest->setLastModified(src->modificationDate());
  dest->setPage(thePage);

  // TODO: Does poppler provide the color anywhere?
  // dest->setColor();

  Annotation::AbstractAnnotation::AnnotationFlags& flags = dest->flags();
  flags = Annotation::AbstractAnnotation::AnnotationFlags();
  if (src->flags() & ::Poppler::Annotation::Hidden)
    flags |= Annotation::AbstractAnnotation::Annotation_Hidden;
  if (src->flags() & ::Poppler::Annotation::FixedSize)
    flags |= Annotation::AbstractAnnotation::Annotation_NoZoom;
  if (src->flags() & ::Poppler::Annotation::FixedRotation)
    flags |= Annotation::AbstractAnnotation::Annotation_NoRotate;
  if ((src->flags() & ::Poppler::Annotation::DenyPrint) == 0)
    flags |= Annotation::AbstractAnnotation::Annotation_Print;
  if (src->flags() & ::Poppler::Annotation::DenyWrite)
    flags |= Annotation::AbstractAnnotation::Annotation_ReadOnly;
  if (src->flags() & ::Poppler::Annotation::DenyDelete)
    flags |= Annotation::AbstractAnnotation::Annotation_Locked;
  if (src->flags() & ::Poppler::Annotation::ToggleHidingOnMouse)
    flags |= Annotation::AbstractAnnotation::Annotation_ToggleNoView;

  if (dest->isMarkup()) {
    Annotation::Markup * annot = dynamic_cast<Annotation::Markup*>(dest);
    annot->setAuthor(src->author());
    annot->setCreationDate(src->creationDate());
  }
}


// Document Class
// ==============
Document::Document(const QString & fileName):
  Super(fileName),
  _poppler_doc(::Poppler::Document::load(fileName))
{
#ifdef DEBUG
//  qDebug() << "PopplerQt::Document::Document(" << fileName << ")";
#endif
  parseDocument();
}

Document::~Document()
{
#ifdef DEBUG
//  qDebug() << "PopplerQt::Document::~Document()";
#endif
  clearPages();
  delete _poppler_docLock;
}

void Document::reload()
{
  // Clear the processing thread
  // NB: Do this before acquiring _docLock. See clearWorkStack() documentation.
  // This should not cause any problems as we are supposed to currently be in
  // the main (GUI) thread, and only this thread is supposed to add items to the
  // work stack.
  _processingThread.clearWorkStack();

  QWriteLocker docLocker(_docLock.data());

  clearPages();
  _pageCache.markOutdated();

  {
    QMutexLocker l(_poppler_docLock);
    _poppler_doc = QSharedPointer< ::Poppler::Document >(::Poppler::Document::load(_fileName));
  }

  // TODO: possibly unlock the new document again if it was previously unlocked
  // and the password is still the same

  parseDocument();
}

void Document::parseDocument()
{
  QWriteLocker docLocker(_docLock.data());

  clearMetaData();
  _meta_fileSize = QFileInfo(_fileName).size();
  _numPages = -1;

  if (!_poppler_doc || _isLocked())
    return;

  _numPages = _poppler_doc->numPages();

  // Permissions
  // TODO: Check if this mapping from Poppler flags to our flags is correct
  if (_poppler_doc->okToAddNotes())
    _permissions |= Permission_Annotate;
  if (_poppler_doc->okToAssemble())
    _permissions |= Permission_Assemble;
  if (_poppler_doc->okToChange())
    _permissions |= Permission_Change;
  if (_poppler_doc->okToCopy())
    _permissions |= Permission_Extract;
  if (_poppler_doc->okToCreateFormFields())
    _permissions |= Permission_Annotate;
  if (_poppler_doc->okToExtractForAccessibility())
    _permissions |= Permission_ExtractForAccessibility;
  if (_poppler_doc->okToFillForm())
    _permissions |= Permission_FillForm;
  if (_poppler_doc->okToPrint())
    _permissions |= Permission_Print;
  if (_poppler_doc->okToPrintHighRes())
    _permissions |= Permission_PrintHighRes;

  // **TODO:**
  //
  // _Make these configurable._
  _poppler_doc->setRenderBackend(::Poppler::Document::SplashBackend);
  // Make things look pretty.
  _poppler_doc->setRenderHint(::Poppler::Document::Antialiasing);
  _poppler_doc->setRenderHint(::Poppler::Document::TextAntialiasing);

  // Load meta data
  QStringList metaKeys = _poppler_doc->infoKeys();
  if (metaKeys.contains(QString::fromUtf8("Title"))) {
    _meta_title = _poppler_doc->info(QString::fromUtf8("Title"));
    metaKeys.removeAll(QString::fromUtf8("Title"));
  }
  if (metaKeys.contains(QString::fromUtf8("Author"))) {
    _meta_author = _poppler_doc->info(QString::fromUtf8("Author"));
    metaKeys.removeAll(QString::fromUtf8("Author"));
  }
  if (metaKeys.contains(QString::fromUtf8("Subject"))) {
    _meta_subject = _poppler_doc->info(QString::fromUtf8("Subject"));
    metaKeys.removeAll(QString::fromUtf8("Subject"));
  }
  if (metaKeys.contains(QString::fromUtf8("Keywords"))) {
    _meta_keywords = _poppler_doc->info(QString::fromUtf8("Keywords"));
    metaKeys.removeAll(QString::fromUtf8("Keywords"));
  }
  if (metaKeys.contains(QString::fromUtf8("Creator"))) {
    _meta_creator = _poppler_doc->info(QString::fromUtf8("Creator"));
    metaKeys.removeAll(QString::fromUtf8("Creator"));
  }
  if (metaKeys.contains(QString::fromUtf8("Producer"))) {
    _meta_producer = _poppler_doc->info(QString::fromUtf8("Producer"));
    metaKeys.removeAll(QString::fromUtf8("Producer"));
  }
  if (metaKeys.contains(QString::fromUtf8("CreationDate"))) {
    _meta_creationDate = fromPDFDate(_poppler_doc->info(QString::fromUtf8("CreationDate")));
    metaKeys.removeAll(QString::fromUtf8("CreationDate"));
  }
  if (metaKeys.contains(QString::fromUtf8("ModDate"))) {
    _meta_modDate = fromPDFDate(_poppler_doc->info(QString::fromUtf8("ModDate")));
    metaKeys.removeAll(QString::fromUtf8("ModDate"));
  }

  // Get the most often used page size
  QMap<QSizeF, int> pageSizes;
  for (int i = 0; i < _numPages; ++i) {
    QSizeF ps = _poppler_doc->page(i)->pageSizeF();
    if (pageSizes.contains(ps)) ++pageSizes[ps];
    else pageSizes[ps] = 1;
  }
  int occurrences = -1;
  _meta_pageSize = QSizeF();
  Q_FOREACH(QSizeF ps, pageSizes.keys()) {
      if (occurrences < pageSizes[ps]) {
          _meta_pageSize = ps;
          occurrences = pageSizes[ps];
      }
  }

  // Note: Poppler doesn't handle the meta data key "Trapped" correctly, as that
  // has a value of type `name` (/True, /False, or /Unknown) which doesn't get
  // converted to a string representation properly.
  _meta_trapped = Trapped_Unknown;
    metaKeys.removeAll(QString::fromUtf8("Trapped"));

  foreach (QString key, metaKeys)
    _meta_other[key] = _poppler_doc->info(key);
}

QWeakPointer<Backend::Page> Document::page(int at)
{
  {
    QReadLocker docLocker(_docLock.data());

    if (at < 0 || at >= _numPages)
      return QWeakPointer<Backend::Page>();

    if (at < _pages.size() && !_pages[at].isNull())
      return _pages[at];
  }

  // if we get here, the page is not in the array
  QWriteLocker docLocker(_docLock.data());

  // recheck everything that could have changed before we got the write lock
  if (at >= _numPages)
    return QWeakPointer<Backend::Page>();
  if (at < _pages.size() && !_pages[at].isNull())
    return _pages[at].toWeakRef();

  if( _pages.isEmpty() )
    _pages.resize(_numPages);

  // If we got here, we don't have the page cached. As we need to create a new
  // page, we need to make sure the Poppler document is valid and does not go
  // out of scope
  QMutexLocker popplerLocker(_poppler_docLock);
  if (!_poppler_doc)
    return QWeakPointer<Backend::Page>();

  _pages[at] = QSharedPointer<Backend::Page>(new Page(this, at, _docLock));
  return _pages[at].toWeakRef();
}

PDFDestination Document::resolveDestination(const PDFDestination & namedDestination) const
{
  QReadLocker docLocker(_docLock.data());
  Q_ASSERT(!_poppler_doc.isNull());

  // If namedDestination is not a named destination at all, simply return a copy
  if (namedDestination.isExplicit())
    return namedDestination;

  // If the destination could not be resolved (a nullptr or an invalid page
  // number is returned), return an invalid object
  ::Poppler::LinkDestination * dest = _poppler_doc->linkDestination(namedDestination.destinationName());
  if (dest == nullptr || dest->pageNumber() < 1)
    return PDFDestination();
  return toPDFDestination(_poppler_doc.data(), *dest);
}

#if POPPLER_HAS_OUTLINE
void Document::recursiveConvertToC(QList<PDFToCItem> & items, const QVector<Poppler::OutlineItem> & popplerItems) const
{
  for (const Poppler::OutlineItem & popplerItem : popplerItems) {
    PDFToCItem newItem(popplerItem.name());
    newItem.setOpen(popplerItem.isOpen());
    // Note: color and flags are not supported by poppler

    PDFGotoAction * action = nullptr;
    if (popplerItem.destination())
      action = new PDFGotoAction(toPDFDestination(_poppler_doc.data(), *(popplerItem.destination())));

    if (action && !popplerItem.externalFileName().isEmpty()) {
      // Open external links in new window by default (since poppler doesn't
      // tell us what to do)
      action->setOpenInNewWindow(true);
      action->setRemote();
      action->setFilename(popplerItem.externalFileName());
    }
    newItem.setAction(action);

    recursiveConvertToC(newItem.children(), popplerItem.children());
    items << newItem;
  }
}
#else // POPPLER_HAS_OUTLINE
void Document::recursiveConvertToC(QList<PDFToCItem> & items, QDomNode node) const
{
  while (!node.isNull()) {
    PDFToCItem newItem(node.nodeName());

    QDomNamedNodeMap attributes = node.attributes();
    newItem.setOpen(attributes.namedItem(QString::fromUtf8("Open")).nodeValue() == QString::fromUtf8("true"));
    // Note: color and flags are not supported by poppler

    PDFGotoAction * action = nullptr;
    QString val = attributes.namedItem(QString::fromUtf8("Destination")).nodeValue();
    if (!val.isEmpty())
      action = new PDFGotoAction(toPDFDestination(_poppler_doc.data(), ::Poppler::LinkDestination(val)));
    else {
      val = attributes.namedItem(QString::fromUtf8("DestinationName")).nodeValue();
      if (!val.isEmpty())
        action = new PDFGotoAction(PDFDestination(val));
    }

    val = attributes.namedItem(QString::fromUtf8("ExternalFileName")).nodeValue();
    if (action) {
      if (!val.isEmpty()) {
        // Open external links in new window by default (since poppler doesn't
        // tell us what to do)
        action->setOpenInNewWindow(true);
        action->setRemote();
        action->setFilename(val);
      }
    }
    newItem.setAction(action);

    recursiveConvertToC(newItem.children(), node.firstChild());
    items << newItem;
    node = node.nextSibling();
  }
}
#endif // POPPLER_HAS_OUTLINE

PDFToC Document::toc() const
{
  QReadLocker docLocker(_docLock.data());

  PDFToC retVal;
  if (!_poppler_doc || _isLocked())
    return retVal;

#if POPPLER_HAS_OUTLINE
  recursiveConvertToC(retVal, _poppler_doc->outline());
#else // POPPLER_HAS_OUTLINE
  QDomDocument * toc = _poppler_doc->toc();
  if (!toc)
    return retVal;
  recursiveConvertToC(retVal, toc->firstChild());
  delete toc;
#endif // POPPLER_HAS_OUTLINE
  return retVal;
}

QList<PDFFontInfo> Document::fonts() const
{
  QReadLocker docLocker(_docLock.data());

  if (_fontsLoaded)
    return _fonts;

  if (!_poppler_doc || _isLocked())
    return QList<PDFFontInfo>();

  _fontsLoaded = true;

  foreach(::Poppler::FontInfo popplerFontInfo, _poppler_doc->fonts()) {
    PDFFontInfo fi;
    if (popplerFontInfo.isEmbedded())
      fi.setSource(PDFFontInfo::Source_Embedded);
    else
      fi.setFileName(QFileInfo(popplerFontInfo.file()));
    fi.setDescriptor(PDFFontDescriptor(popplerFontInfo.name()));

    switch (popplerFontInfo.type()) {
      case ::Poppler::FontInfo::Type1:
        fi.setFontType(PDFFontInfo::FontType_Type1);
        fi.setCIDType(PDFFontInfo::CIDFont_None);
        fi.setFontProgramType(PDFFontInfo::ProgramType_Type1);
        break;
      case ::Poppler::FontInfo::Type1C:
        fi.setFontType(PDFFontInfo::FontType_Type1);
        fi.setCIDType(PDFFontInfo::CIDFont_None);
        fi.setFontProgramType(PDFFontInfo::ProgramType_Type1CFF);
        break;
      case ::Poppler::FontInfo::Type1COT:
        fi.setFontType(PDFFontInfo::FontType_Type1);
        fi.setCIDType(PDFFontInfo::CIDFont_None);
        fi.setFontProgramType(PDFFontInfo::ProgramType_OpenType); // speculation
        break;
      case ::Poppler::FontInfo::Type3:
        fi.setFontType(PDFFontInfo::FontType_Type3);
        fi.setCIDType(PDFFontInfo::CIDFont_None);
        fi.setFontProgramType(PDFFontInfo::ProgramType_None); // probably wrong!
        break;
      case ::Poppler::FontInfo::TrueType:
        fi.setFontType(PDFFontInfo::FontType_TrueType);
        fi.setCIDType(PDFFontInfo::CIDFont_None);
        fi.setFontProgramType(PDFFontInfo::ProgramType_TrueType);
        break;
      case ::Poppler::FontInfo::TrueTypeOT:
        fi.setFontType(PDFFontInfo::FontType_TrueType);
        fi.setCIDType(PDFFontInfo::CIDFont_None);
        fi.setFontProgramType(PDFFontInfo::ProgramType_OpenType);
        break;
      case ::Poppler::FontInfo::CIDType0:
        fi.setFontType(PDFFontInfo::FontType_Type0);
        fi.setCIDType(PDFFontInfo::CIDFont_Type0);
        fi.setFontProgramType(PDFFontInfo::ProgramType_None); // probably wrong!
        break;
      case ::Poppler::FontInfo::CIDType0C:
        fi.setFontType(PDFFontInfo::FontType_Type0);
        fi.setCIDType(PDFFontInfo::CIDFont_Type0);
        fi.setFontProgramType(PDFFontInfo::ProgramType_CIDCFF);
        break;
      case ::Poppler::FontInfo::CIDType0COT:
        fi.setFontType(PDFFontInfo::FontType_Type0);
        fi.setCIDType(PDFFontInfo::CIDFont_Type0);
        fi.setFontProgramType(PDFFontInfo::ProgramType_OpenType);
        break;
      case ::Poppler::FontInfo::CIDTrueType:
        fi.setFontType(PDFFontInfo::FontType_Type0);
        fi.setCIDType(PDFFontInfo::CIDFont_Type2); // speculation
        fi.setFontProgramType(PDFFontInfo::ProgramType_TrueType);
        break;
      case ::Poppler::FontInfo::CIDTrueTypeOT:
        fi.setFontType(PDFFontInfo::FontType_Type0);
        fi.setCIDType(PDFFontInfo::CIDFont_Type2); // speculation
        fi.setFontProgramType(PDFFontInfo::ProgramType_OpenType);
        break;
      case ::Poppler::FontInfo::unknown:
      default:
        continue;
    }
    _fonts << fi;
  }
  return _fonts;
}

bool Document::unlock(const QString password)
{
  QWriteLocker docLocker(_docLock.data());

  if (!_poppler_doc)
    return false;
  // Note: we try unlocking regardless of what isLocked() returns as the user
  // might want to unlock a document with the owner's password when user level
  // access is already granted.
  bool success = !_poppler_doc->unlock(password.toLatin1(), password.toLatin1());

  if (success)
    parseDocument();

  // FIXME: Store password for this session in case we need to reload the
  // document later on (e.g., if it has changed on the disk)

  return success;
}


// Page Class
// ==========
Page::Page(Document *parent, int at, QSharedPointer<QReadWriteLock> docLock):
  Super(parent, at, docLock)
{
  _poppler_page = QSharedPointer< ::Poppler::Page >(dynamic_cast<Document *>(_parent)->_poppler_doc->page(at));
  loadTransitionData();
}

Page::~Page()
{
  QWriteLocker pageLocker(_pageLock);
}

// TODO: Does this operation require obtaining the Poppler document mutex? If
// so, it would be better to store the value in a member variable during
// initialization.
QSizeF Page::pageSizeF() const
{
  QReadLocker pageLocker(_pageLock);

  Q_ASSERT(_poppler_page != nullptr);
  return _poppler_page->pageSizeF();
}

QImage Page::renderToImage(double xres, double yres, QRect render_box, bool cache) const
{
  QReadLocker docLocker(_docLock.data());
  QReadLocker pageLocker(_pageLock);
  if (!_parent)
    return QImage();

  QImage renderedPage;

  {
    // Rendering pages is not thread safe.
    QMutexLocker popplerDocLock(dynamic_cast<Backend::PopplerQt::Document *>(_parent)->_poppler_docLock);
    if( render_box.isNull() ) {
      // A null QRect has a width and height of 0 --- we will tell Poppler to render the whole
      // page.
      renderedPage = _poppler_page->renderToImage(xres, yres);
    } else {
      renderedPage = _poppler_page->renderToImage(xres, yres,
          render_box.x(), render_box.y(), render_box.width(), render_box.height());
    }
  }

  if( cache ) {
    PDFPageTile key(xres, yres, render_box, _n);
    QImage * img = new QImage(renderedPage.copy());
    if (img != _parent->pageCache().setImage(key, img, PDFPageCache::CURRENT))
      delete img;
  }

  return renderedPage;
}

QList< QSharedPointer<Annotation::Link> > Page::loadLinks()
{
  {
    QReadLocker pageLocker(_pageLock);

    if (_linksLoaded || !_parent)
      return _links;
  }

  QReadLocker docLocker(_docLock.data());
  QWriteLocker pageLocker(_pageLock);

  // Check if the links were loaded in another thread in the meantime
  if (_linksLoaded || !_parent)
    return _links;


  Q_ASSERT(_poppler_page != nullptr);
  _linksLoaded = true;
  QList< ::Poppler::Link *> popplerLinks;
  QList< ::Poppler::Annotation *> popplerAnnots;
  {
    // Loading links is not thread safe.
    QMutexLocker popplerDocLock(dynamic_cast<Backend::PopplerQt::Document *>(_parent)->_poppler_docLock);
    popplerLinks = _poppler_page->links();
    popplerAnnots = _poppler_page->annotations();
  }

  // Note: Poppler gives the linkArea in normalized coordinates, i.e., in the
  // range of 0..1, with y=0 at the top. We use pdf coordinates internally, so
  // we need to transform things accordingly.
  // Note: Cannot use pageSizeF() here as that tries to acquire a readLock when
  // we already hold a writeLock, which would result in deadlock
  QTransform denormalize = QTransform::fromScale(_poppler_page->pageSizeF().width(), -_poppler_page->pageSizeF().height()).translate(0,  -1);

  // Convert poppler links to PDFLinkAnnotations
  foreach (::Poppler::Link * popplerLink, popplerLinks) {
    QSharedPointer<Annotation::Link> link(new Annotation::Link);

    // Look up the corresponding ::Poppler::LinkAnnotation object. Do this first
    // so the general annotation settings can be overridden by more specific
    // link annotation settings afterwards (if necessary)
    // Note: ::Poppler::LinkAnnotation::linkDestionation() [sic] doesn't reliably
    // return a ::Poppler::Link*. Therefore, we have to find the correct
    // annotation object ourselves. Note, though, that boundary() and rect()
    // don't seem to correspond exactly (i.e., they are neither (necessarily)
    // equal, nor does one (necessarily) contain the other.
    // TODO: Can we have the situation that we get more than one matching
    // annotations out of this?
    foreach (::Poppler::Annotation * popplerAnnot, popplerAnnots) {
      if (!popplerAnnot || popplerAnnot->subType() != ::Poppler::Annotation::ALink || !denormalize.mapRect(popplerAnnot->boundary()).intersects(link->rect()))
        continue;

      ::Poppler::LinkAnnotation * popplerLinkAnnot = dynamic_cast< ::Poppler::LinkAnnotation *>(popplerAnnot);
      convertAnnotation(link.data(), popplerLinkAnnot, _parent->page(_n));
      // TODO: Does Poppler provide an easy interface to all quadPoints?
      // Note: ::Poppler::LinkAnnotation::HighlightMode is identical to PDFLinkAnnotation::HighlightingMode
      link->setHighlightingMode(static_cast<Annotation::Link::HighlightingMode>(popplerLinkAnnot->linkHighlightMode()));
      break;
    }

    link->setRect(denormalize.mapRect(popplerLink->linkArea()));

    switch (popplerLink->linkType()) {
      case ::Poppler::Link::Goto:
        {
          ::Poppler::LinkGoto * popplerGoto = dynamic_cast< ::Poppler::LinkGoto *>(popplerLink);
          PDFGotoAction * action = new PDFGotoAction(toPDFDestination(dynamic_cast<Document *>(_parent)->_poppler_doc.data(), popplerGoto->destination()));
          if (popplerGoto->isExternal()) {
            // TODO: Verify that ::Poppler::LinkGoto only refers to pdf files
            // (for other file types we would need PDFLaunchAction)
            action->setOpenInNewWindow(true);
            action->setRemote();
            action->setFilename(popplerGoto->fileName());
          }
          link->setActionOnActivation(action);
        }
        break;
      case ::Poppler::Link::Execute:
        {
          ::Poppler::LinkExecute * popplerExecute = dynamic_cast< ::Poppler::LinkExecute *>(popplerLink);
          if (popplerExecute->parameters().isEmpty())
            link->setActionOnActivation(new PDFLaunchAction(popplerExecute->fileName()));
          else
            link->setActionOnActivation(new PDFLaunchAction(QString::fromUtf8("%1 %2").arg(popplerExecute->fileName(), popplerExecute->parameters())));
        }
        break;
      case ::Poppler::Link::Browse:
        link->setActionOnActivation(new PDFURIAction(dynamic_cast< ::Poppler::LinkBrowse*>(popplerLink)->url()));
        break;
      /*
      case ::Poppler::Link::Action:
      case ::Poppler::Link::None:
      case ::Poppler::Link::Sound:
      case ::Poppler::Link::Movie:
      case ::Poppler::Link::JavaScript:
      case ::Poppler::Link::Rendition: // Since poppler 0.20
      */
      default:
        // We don't handle these types yet
        link.clear();
        continue;
    }

    _links << link;
  }
  return _links;
}

QList< QSharedPointer<Annotation::AbstractAnnotation> > Page::loadAnnotations()
{
  {
    QReadLocker pageLocker(_pageLock);

    if (_annotationsLoaded)
      return _annotations;
  }

  QReadLocker docLocker(_docLock.data());
  QWriteLocker pageLocker(_pageLock);
  // Check if the annotations were loaded in another thread in the meantime
  if (_annotationsLoaded || !_poppler_page || !_parent)
    return _annotations;

  _annotationsLoaded = true;

  QList< ::Poppler::Annotation *> popplerAnnots;
  {
    // Loading annotations is not thread safe.
    QMutexLocker popplerDocLock(dynamic_cast<Document *>(_parent)->_poppler_docLock);
    popplerAnnots = _poppler_page->annotations();
  }

  // we don't need the docLock anymore
  docLocker.unlock();
  // we don't need the pageLock anymore (until we actually modify _annotations).
  // in fact, convertAnnotation tries to acquire a read lock at some point,
  // which fails while we hold a write lock here
  pageLocker.unlock();

  foreach(::Poppler::Annotation * popplerAnnot, popplerAnnots) {
    if (!popplerAnnot)
      continue;
    switch (popplerAnnot->subType()) {
      case ::Poppler::Annotation::AText:
      {
        Annotation::Text * annot = new Annotation::Text();
        convertAnnotation(annot, popplerAnnot, _parent->page(_n));
        pageLocker.relock();
        _annotations << QSharedPointer<Annotation::AbstractAnnotation>(annot);
        pageLocker.unlock();
        break;
      }
      case ::Poppler::Annotation::ACaret:
      {
        Annotation::Caret * annot = new Annotation::Caret();
        convertAnnotation(annot, popplerAnnot, _parent->page(_n));
        pageLocker.relock();
        _annotations << QSharedPointer<Annotation::AbstractAnnotation>(annot);
        pageLocker.unlock();
        break;
      }
      case ::Poppler::Annotation::AHighlight:
      {
        ::Poppler::HighlightAnnotation * popplerHighlight = dynamic_cast< ::Poppler::HighlightAnnotation*>(popplerAnnot);
        switch (popplerHighlight->highlightType()) {
          case ::Poppler::HighlightAnnotation::Highlight:
          {
            Annotation::Highlight * annot = new Annotation::Highlight();
            convertAnnotation(annot, popplerAnnot, _parent->page(_n));
            pageLocker.relock();
            _annotations << QSharedPointer<Annotation::AbstractAnnotation>(annot);
            pageLocker.unlock();
            break;
          }
          case ::Poppler::HighlightAnnotation::Squiggly:
          {
            Annotation::Squiggly * annot = new Annotation::Squiggly();
            convertAnnotation(annot, popplerAnnot, _parent->page(_n));
            pageLocker.relock();
            _annotations << QSharedPointer<Annotation::AbstractAnnotation>(annot);
            pageLocker.unlock();
            break;
          }
          case ::Poppler::HighlightAnnotation::Underline:
          {
            Annotation::Underline * annot = new Annotation::Underline();
            convertAnnotation(annot, popplerAnnot, _parent->page(_n));
            pageLocker.relock();
            _annotations << QSharedPointer<Annotation::AbstractAnnotation>(annot);
            pageLocker.unlock();
            break;
          }
          case ::Poppler::HighlightAnnotation::StrikeOut:
          {
            Annotation::StrikeOut * annot = new Annotation::StrikeOut();
            convertAnnotation(annot, popplerAnnot, _parent->page(_n));
            pageLocker.relock();
            _annotations << QSharedPointer<Annotation::AbstractAnnotation>(annot);
            pageLocker.unlock();
            break;
          }
        }
        break;
      }
      default:
        break;
    }
  }
  pageLocker.relock();
  return _annotations;
}

QList<SearchResult> Page::search(const QString & searchText, const SearchFlags & flags) const
{
  QList<SearchResult> results;
  SearchResult result;
  double left{0}, right{0}, top{0}, bottom{0};
  ::Poppler::Page::SearchDirection searchDir = (flags.testFlag(Search_Backwards) ? ::Poppler::Page::PreviousResult : ::Poppler::Page::NextResult);
#if POPPLER_HAS_SEARCH_FLAGS
  ::Poppler::Page::SearchFlags searchFlags = (flags.testFlag(Search_CaseInsensitive) ? ::Poppler::Page::IgnoreCase : ::Poppler::Page::SearchFlags());
#else
  ::Poppler::Page::SearchMode searchFlags = (flags.testFlag(Search_CaseInsensitive) ? ::Poppler::Page::CaseInsensitive : ::Poppler::Page::CaseSensitive);
#endif

  QReadLocker docLocker(_docLock.data());
  QReadLocker pageLocker(_pageLock);
  if (!_parent)
    return results;

  result.pageNum = static_cast<unsigned int>(_n);

  QMutexLocker popplerDocLock(dynamic_cast<Document *>(_parent)->_poppler_docLock);

  if (flags & Search_Backwards) {
    left = right = pageSizeF().width();
    top = bottom = pageSizeF().height();
  }

  // The Poppler search function that takes a QRectF has been marked as
  // depreciated---something to do with float <-> double conversion causing
  // infinite loops on some architectures. So, we explicitly use doubles and
  // avoid the depreciated function.
  while ( _poppler_page->search(searchText, left, top, right, bottom, searchDir, searchFlags) ) {
    result.bbox = QRectF(qreal(left), qreal(top), qAbs(qreal(right) - qreal(left)), qAbs(qreal(bottom) - qreal(top)));
    results << result;
  }

  return results;
}

void Page::loadTransitionData()
{
  QWriteLocker pageLocker(_pageLock);
  Q_ASSERT(!_poppler_page.isNull());
  // Transition
  ::Poppler::PageTransition * poppler_trans = _poppler_page->transition();
  if (poppler_trans) {
    switch (poppler_trans->type()) {
    case ::Poppler::PageTransition::Split:
      _transition = new Transition::Split();
      switch (poppler_trans->alignment()) {
        case ::Poppler::PageTransition::Horizontal:
        default:
          _transition->setDirection(0);
          break;
        case ::Poppler::PageTransition::Vertical:
          _transition->setDirection(90);
          break;
      }
      break;
    case ::Poppler::PageTransition::Blinds:
      _transition = new Transition::Blinds();
      switch (poppler_trans->alignment()) {
        case ::Poppler::PageTransition::Horizontal:
        default:
          _transition->setDirection(0);
          break;
        case ::Poppler::PageTransition::Vertical:
          _transition->setDirection(90);
          break;
      }
      break;
    case ::Poppler::PageTransition::Box:
      _transition = new Transition::Box();
      break;
    case ::Poppler::PageTransition::Wipe:
      _transition = new Transition::Wipe();
      _transition->setDirection(poppler_trans->angle());
      break;
    case ::Poppler::PageTransition::Dissolve:
      _transition = new Transition::Dissolve();
      break;
    case ::Poppler::PageTransition::Glitter:
      _transition = new Transition::Glitter();
      _transition->setDirection(poppler_trans->angle());
      break;
    case ::Poppler::PageTransition::Replace:
      _transition = new Transition::Replace();
      break;
    case ::Poppler::PageTransition::Fly:
      _transition = new Transition::Fly();
      _transition->setDirection(poppler_trans->angle());
      break;
    case ::Poppler::PageTransition::Push:
      _transition = new Transition::Push();
      _transition->setDirection(poppler_trans->angle());
      break;
    case ::Poppler::PageTransition::Cover:
      _transition = new Transition::Cover();
      _transition->setDirection(poppler_trans->angle());
      break;
    case ::Poppler::PageTransition::Uncover:
      _transition = new Transition::Uncover();
      _transition->setDirection(poppler_trans->angle());
      break;
    case ::Poppler::PageTransition::Fade:
      _transition = new Transition::Fade();
      break;
    }
    if (_transition) {
#if POPPLER_HAS_DURATION_REAL
      _transition->setDuration(poppler_trans->durationReal());
#else
      _transition->setDuration(poppler_trans->duration());
#endif
      switch (poppler_trans->direction()) {
      case ::Poppler::PageTransition::Inward:
      default:
        _transition->setMotion(Transition::AbstractTransition::Motion_Inward);
        break;
      case ::Poppler::PageTransition::Outward:
        _transition->setMotion(Transition::AbstractTransition::Motion_Outward);
      }
    }
  }
}

QList< Backend::Page::Box > Page::boxes() const
{
  QReadLocker pageLocker(_pageLock);
  Q_ASSERT(_poppler_page != nullptr);
  QList< Backend::Page::Box > retVal;

  foreach (::Poppler::TextBox * popplerTextBox, _poppler_page->textList()) {
    if (!popplerTextBox)
      continue;
    Backend::Page::Box box;
    box.boundingBox = popplerTextBox->boundingBox();
    for (int i = 0; i < popplerTextBox->text().length(); ++i) {
      Backend::Page::Box subBox;
      subBox.boundingBox = popplerTextBox->charBoundingBox(i);
      box.subBoxes << subBox;
    }
    retVal << box;
  }
  return retVal;
}

QString Page::selectedText(const QList<QPolygonF> & selection, QMap<int, QRectF> * wordBoxes /* = nullptr */, QMap<int, QRectF> * charBoxes /* = nullptr */, const bool onlyFullyEnclosed /* = false */) const
{
  QReadLocker pageLocker(_pageLock);
  Q_ASSERT(_poppler_page != nullptr);
  // Using the bounding rects of the selection polygons is almost
  // certainly wrong! However, poppler-qt4 doesn't offer any alternative AFAICS
  // (except for positioning each char in the string manually).
  // Since poppler doesn't add any space glyphs, the selection will contain a
  // list of words. Hence, by iterating over them, we get a list of words with
  // no whitespace inbetween
  QString retVal;
  bool insertSpace = false;

  // Get a list of all boxes
  QList<Poppler::TextBox*> poppler_boxes = _poppler_page->textList();
  Poppler::TextBox * lastPopplerBox = nullptr;

  // Filter boxes by selection
  foreach (Poppler::TextBox * poppler_box, poppler_boxes) {
    if (!poppler_box)
      continue;

    // Determine which characters to include (if any)
    QBitArray include(poppler_box->text().length());
    for (int i = 0; i < poppler_box->text().length(); ++i) {
      QPolygonF remainder(poppler_box->charBoundingBox(i));
      foreach (const QPolygonF & p, selection) {
        // Include characters if they are entirely inside the selection area or
        // onlyFullyEnclosed == false; using "intersection only" can cause
        // problems for overlapping char boxes (if the selection is made of
        // entire char boxes, it would return characters that are not actually
        // inside the selection but are just "edge cases") but is necessary if
        // the selection comes from external sources, such as SyncTeX
        if (p.intersected(poppler_box->charBoundingBox(i)).empty())
          continue;
        if (!onlyFullyEnclosed) {
          include.setBit(i);
          break;
        }
        remainder = remainder.subtracted(p);
        if (remainder.empty()) {
          include.setBit(i);
          break;
        }
      }
    }
    if (include.count(true) == 0) continue;

    // If we get here, we found a box that is at least partially selected, so we
    // append the appropriate text

    // Guess ends of lines: if the new box is mostly below the old box (with the
    // overlap being less than 20% of the height of the larger box), we assume
    // it's a new line. This should work reasonably well for normal text
    // (including RTL text), but may fail in some less common cases (e.g.,
    // subscripts after superscripts, formulas, etc.).
    if (lastPopplerBox && lastPopplerBox->boundingBox().bottom() - poppler_box->boundingBox().top() < 0.2 * qMax(lastPopplerBox->boundingBox().height(), poppler_box->boundingBox().height())) {
      retVal += QString::fromLatin1("\n");

      if (wordBoxes)
        (*wordBoxes)[wordBoxes->count()] = lastPopplerBox->boundingBox();
      if (charBoxes)
        (*charBoxes)[charBoxes->count()] = lastPopplerBox->boundingBox();
      // If we queued a space to be inserted, ignore that as we inserted a
      // newline instead anyway
      insertSpace = false;
    }

    if (insertSpace && lastPopplerBox) {
      retVal += QString::fromLatin1(" ");

      // As word and char Boxes, insert those of the lastPopplerBox since that
      // was the one causing insertSpace to be true
      if (wordBoxes)
        (*wordBoxes)[wordBoxes->count()] = lastPopplerBox->boundingBox();
      if (charBoxes)
        (*charBoxes)[charBoxes->count()] = lastPopplerBox->boundingBox();
    }

    // Default to not inserting a space after this word
    insertSpace = false;

    // Insert the actual characters
    for (int i = 0; i < poppler_box->text().length(); ++i) {
      if (!include.testBit(i)) continue;

      retVal += poppler_box->text()[i];

      if (wordBoxes)
        (*wordBoxes)[wordBoxes->count()] = poppler_box->boundingBox();
      if (charBoxes)
        (*charBoxes)[charBoxes->count()] = poppler_box->charBoundingBox(i);

      // If we reached the end of the word, possibly queue a space to be
      // inserted. By queuing this until the next word is processed, we ensure
      // that spaces are not inserted at the end of the string or before
      // newlines
      if (i == poppler_box->text().length() - 1)
        insertSpace = poppler_box->hasSpaceAfter();
    }
    // Remember the last processed box (required for detecting newlines and
    // inserting spaces)
    lastPopplerBox = poppler_box;
  }

  return retVal;
}

} // namespace PopplerQt

} // namespace Backend

QSharedPointer<Backend::Document> PopplerQtBackend::newDocument(const QString & fileName) {
#if !defined(MIKTEX)
#if defined Q_OS_DARWIN
  // Use bundled fonts.conf (if it exists and the user has not overriden
  // FONTCONFIG_PATH
  if (!qEnvironmentVariableIsSet("FONTCONFIG_PATH")) {
    QDir confPath{QCoreApplication::applicationDirPath()};
    if (confPath.cd("../etc/fonts")) {
      if (confPath.exists("fonts.conf")) {
        qputenv("FONTCONFIG_PATH", confPath.path().toLocal8Bit());
      }
    }
  }
#if defined(HAVE_POPPLER_XPDF_HEADERS)
  static bool globalParamsInitialized = false;
  if (!globalParamsInitialized) {
    globalParamsInitialized = true;
    #if defined(POPPLER_HAS_GLOBALPARAMSINITER)
      QDir dataDir{QCoreApplication::applicationDirPath()};
      if (dataDir.cd(QStringLiteral("../share/poppler"))) {
        GlobalParamsIniter::setCustomDataDir(qPrintable(dataDir.path()));
      }
    #else // defined(POPPLER_HAS_GLOBALPARAMSINITER)
      // for Mac, support "local" poppler-data directory
      // (requires patched poppler-qt lib to be effective,
      // otherwise the GlobalParams gets overwritten when a
      // document is opened)
      QDir dataDir{QCoreApplication::applicationDirPath()};
      if (dataDir.cd(QStringLiteral("../share/poppler"))) {
        #if defined(POPPLER_GLOBALPARAMS_IS_UNIQUE)
          globalParams = std::move(std::unique_ptr<GlobalParams>(new GlobalParams(qPrintable(dataDir.path()))));
        #else
          globalParams = new GlobalParams(qPrintable(dataDir.path()));
        #endif
      }
    #endif // defined(POPPLER_HAS_GLOBALPARAMSINITER)
  }
#endif // defined(HAVE_POPPLER_XPDF_HEADERS)
#endif // defined(Q_OS_DARWIN)
#endif
  return QSharedPointer<Backend::Document>(new Backend::PopplerQt::Document(fileName));
}

} // namespace QtPDF

// vim: set sw=2 ts=2 et
