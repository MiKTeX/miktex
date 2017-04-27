/**
 * Copyright (C) 2011-2013  Charlie Sharpsteen, Stefan Löffler
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
#include <PDFBackend.h>

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
  float w = 1., h = 1.;
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

void convertAnnotation(Annotation::AbstractAnnotation * dest, const ::Poppler::Annotation * src, QWeakPointer<Backend::Page> thePage)
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
    Annotation::Markup * annot = static_cast<Annotation::Markup*>(dest);
    annot->setAuthor(src->author());
    annot->setCreationDate(src->creationDate());
  }
}


// Document Class
// ==============
Document::Document(QString fileName):
  Super(fileName),
  _poppler_doc(::Poppler::Document::load(fileName)),
  _poppler_docLock(new QMutex()),
  _fontsLoaded(false)
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
  clearMetaData();
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

QWeakPointer<Backend::Page> Document::page(int at) const
{
  QReadLocker docLocker(_docLock.data());

  if (at < 0 || at >= _numPages || at >= _pages.size())
    return QWeakPointer<Backend::Page>();

  return QWeakPointer<Backend::Page>(_pages[at]);
}

PDFDestination Document::resolveDestination(const PDFDestination & namedDestination) const
{
  QReadLocker docLocker(_docLock.data());
  Q_ASSERT(!_poppler_doc.isNull());

  // If namedDestination is not a named destination at all, simply return a copy
  if (namedDestination.isExplicit())
    return namedDestination;

  // If the destination could not be resolved, return an invalid object
  ::Poppler::LinkDestination * dest = _poppler_doc->linkDestination(namedDestination.destinationName());
  if (!dest)
    return PDFDestination();
  return toPDFDestination(_poppler_doc.data(), *dest);
}

void Document::recursiveConvertToC(QList<PDFToCItem> & items, QDomNode node) const
{
  while (!node.isNull()) {
    PDFToCItem newItem(node.nodeName());

    QDomNamedNodeMap attributes = node.attributes();
    newItem.setOpen(attributes.namedItem(QString::fromUtf8("Open")).nodeValue() == QString::fromUtf8("true"));
    // Note: color and flags are not supported by poppler

    PDFGotoAction * action = NULL;
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

PDFToC Document::toc() const
{
  QReadLocker docLocker(_docLock.data());

  PDFToC retVal;
  if (!_poppler_doc || _isLocked())
    return retVal;

  QDomDocument * toc = _poppler_doc->toc();
  if (!toc)
    return retVal;
  recursiveConvertToC(retVal, toc->firstChild());
  delete toc;
  return retVal;
}

QList<PDFFontInfo> Document::fonts() const
{
  QReadLocker docLocker(_docLock.data());

  if (_fontsLoaded)
    return _fonts;

  if (!_poppler_doc || _isLocked())
    return QList<PDFFontInfo>();

  // Since ::Poppler::Document::fonts() is extremely slow, we need to cache the
  // result. Since this function is declared const, we need to const_cast.
  QList<PDFFontInfo> & fonts = const_cast<QList<PDFFontInfo>&>(_fonts);
  bool & fontsLoaded = const_cast<bool&>(_fontsLoaded);
  fontsLoaded = true;

  foreach(::Poppler::FontInfo popplerFontInfo, _poppler_doc->fonts()) {
    PDFFontInfo fi;
    if (popplerFontInfo.isEmbedded())
      fi.setSource(PDFFontInfo::Source_Embedded);
    else
      fi.setFileName(popplerFontInfo.file());
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
    fonts << fi;
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
  Super(parent, at, docLock),
  _annotationsLoaded(false),
  _linksLoaded(false)
{
  _poppler_page = QSharedPointer< ::Poppler::Page >(static_cast<Document *>(_parent)->_poppler_doc->page(at));
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

  Q_ASSERT(_poppler_page != NULL);
  return _poppler_page->pageSizeF();
}

QImage Page::renderToImage(double xres, double yres, QRect render_box, bool cache)
{
  QReadLocker docLocker(_docLock.data());
  QReadLocker pageLocker(_pageLock);
  if (!_parent)
    return QImage();

  QImage renderedPage;

  {
    // Rendering pages is not thread safe.
    QMutexLocker popplerDocLock(static_cast<Backend::PopplerQt::Document *>(_parent)->_poppler_docLock);
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


  Q_ASSERT(_poppler_page != NULL);
  _linksLoaded = true;
  QList< ::Poppler::Link *> popplerLinks;
  QList< ::Poppler::Annotation *> popplerAnnots;
  {
    // Loading links is not thread safe.
    QMutexLocker popplerDocLock(static_cast<Backend::PopplerQt::Document *>(_parent)->_poppler_docLock);
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

      ::Poppler::LinkAnnotation * popplerLinkAnnot = static_cast< ::Poppler::LinkAnnotation *>(popplerAnnot);
      convertAnnotation(link.data(), popplerLinkAnnot, _parent->page(_n));
      // TODO: Does Poppler provide an easy interface to all quadPoints?
      // Note: ::Poppler::LinkAnnotation::HighlightMode is identical to PDFLinkAnnotation::HighlightingMode
      link->setHighlightingMode((Annotation::Link::HighlightingMode)popplerLinkAnnot->linkHighlightMode());
      break;
    }

    link->setRect(denormalize.mapRect(popplerLink->linkArea()));

    switch (popplerLink->linkType()) {
      case ::Poppler::Link::Goto:
        {
          ::Poppler::LinkGoto * popplerGoto = static_cast< ::Poppler::LinkGoto *>(popplerLink);
          PDFGotoAction * action = new PDFGotoAction(toPDFDestination(static_cast<Document *>(_parent)->_poppler_doc.data(), popplerGoto->destination()));
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
          ::Poppler::LinkExecute * popplerExecute = static_cast< ::Poppler::LinkExecute *>(popplerLink);
          if (popplerExecute->parameters().isEmpty())
            link->setActionOnActivation(new PDFLaunchAction(popplerExecute->fileName()));
          else
            link->setActionOnActivation(new PDFLaunchAction(QString::fromUtf8("%1 %2").arg(popplerExecute->fileName()).arg(popplerExecute->parameters())));
        }
        break;
      case ::Poppler::Link::Browse:
        link->setActionOnActivation(new PDFURIAction(static_cast< ::Poppler::LinkBrowse*>(popplerLink)->url()));
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
    QMutexLocker popplerDocLock(static_cast<Document *>(_parent)->_poppler_docLock);
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
        ::Poppler::HighlightAnnotation * popplerHighlight = static_cast< ::Poppler::HighlightAnnotation*>(popplerAnnot);
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

QList<SearchResult> Page::search(QString searchText, SearchFlags flags)
{
  QList<SearchResult> results;
  SearchResult result;
  double left, right, top, bottom;
  ::Poppler::Page::SearchDirection searchDir = (flags & Search_Backwards ? ::Poppler::Page::PreviousResult : ::Poppler::Page::NextResult);
  ::Poppler::Page::SearchMode searchMode = (flags & Search_CaseInsensitive ? ::Poppler::Page::CaseInsensitive : ::Poppler::Page::CaseSensitive);

  QReadLocker docLocker(_docLock.data());
  QReadLocker pageLocker(_pageLock);
  if (!_parent)
    return results;

  result.pageNum = _n;

  QMutexLocker popplerDocLock(static_cast<Document *>(_parent)->_poppler_docLock);

  if (flags & Search_Backwards) {
    left = right = pageSizeF().width();
    top = bottom = pageSizeF().height();
  }
  else {
    left = top = right = bottom = 0;
  }

  // The Poppler search function that takes a QRectF has been marked as
  // depreciated---something to do with float <-> double conversion causing
  // infinite loops on some architectures. So, we explicitly use doubles and
  // avoid the depreciated function.
  while ( _poppler_page->search(searchText, left, top, right, bottom, searchDir, searchMode) ) {
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
      _transition->setDuration(poppler_trans->duration());
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

QList< Backend::Page::Box > Page::boxes()
{
  QReadLocker pageLocker(_pageLock);
  Q_ASSERT(_poppler_page != NULL);
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

QString Page::selectedText(const QList<QPolygonF> & selection, QMap<int, QRectF> * wordBoxes /* = NULL */, QMap<int, QRectF> * charBoxes /* = NULL */, const bool onlyFullyEnclosed /* = false */)
{
  QReadLocker pageLocker(_pageLock);
  Q_ASSERT(_poppler_page != NULL);
  // Using the bounding rects of the selection polygons is almost
  // certainly wrong! However, poppler-qt4 doesn't offer any alternative AFAICS
  // (except for positioning each char in the string manually).
  // Since poppler doesn't add any space glyphs, the selection will contain a
  // list of words. Hence, by iterating over them, we get a list of words with
  // no whitespace inbetween
  QString retVal;

	// Get a list of all boxes
	QList<Poppler::TextBox*> poppler_boxes = _poppler_page->textList();
	Poppler::TextBox * lastPopplerBox = NULL;

	// Filter boxes by selection
	foreach (Poppler::TextBox * poppler_box, poppler_boxes) {
		if (!poppler_box)
			continue;
		bool include = false;
		bool includeEntirely = false;
		foreach (const QPolygonF & p, selection) {
			if (!p.intersected(poppler_box->boundingBox()).empty()) {
				include = true;
				includeEntirely = QPolygonF(poppler_box->boundingBox()).subtracted(p).empty();
				break;
			}
		}
		if (!include)
			continue;
		// If we get here, we found a box in the selection, so we append its text

		// Guess ends of line: if the new box is entirely below the old box, we
		// assume it's a new line. This should work reasonably well for normal text
		// (including RTL text), but may fail in some less common cases (e.g.,
		// subscripts after superscripts, formulas, etc.).
		if (lastPopplerBox && lastPopplerBox->boundingBox().bottom() < poppler_box->boundingBox().top()) {
			retVal += QString::fromLatin1("\n");

			if (wordBoxes)
				(*wordBoxes)[wordBoxes->count()] = lastPopplerBox->boundingBox();
			if (charBoxes)
				(*charBoxes)[charBoxes->count()] = lastPopplerBox->boundingBox();
		}

		bool appendSpace = false;
		if (includeEntirely) {
			retVal += poppler_box->text();
			if (wordBoxes) {
				for (int i = 0; i < poppler_box->text().length(); ++i)
					(*wordBoxes)[wordBoxes->count()] = poppler_box->boundingBox();
			}
			if (charBoxes) {
				for (int i = 0; i < poppler_box->text().length(); ++i)
					(*charBoxes)[charBoxes->count()] = poppler_box->charBoundingBox(i);
			}
			appendSpace = poppler_box->hasSpaceAfter();
		}
		else {
			for (int i = 0; i < poppler_box->text().length(); ++i) {
				foreach (const QPolygonF & p, selection) {
					// Append text for char boxes if they are entirely inside the
					// selection area or onlyFullyEnclosed == false; using "intersection
					// only" can cause problems for overlapping char boxes (if selection
					// is made of entire char boxes, it would return characters that are
					// not actually inside the selection but are just "edge cases") but is
					// necessary if selection comes from external sources, such as SyncTeX
					if (p.intersected(poppler_box->charBoundingBox(i)).empty())
						continue;
					if (!onlyFullyEnclosed || QPolygonF(poppler_box->charBoundingBox(i)).subtracted(p).empty()) {
						retVal += poppler_box->text()[i];

						if (wordBoxes)
							(*wordBoxes)[wordBoxes->count()] = poppler_box->boundingBox();
						if (charBoxes)
							(*charBoxes)[charBoxes->count()] = poppler_box->charBoundingBox(i);

						if (i == poppler_box->text().length() - 1)
							appendSpace = poppler_box->hasSpaceAfter();
						break;
					}
				}
			}
		}
		if (appendSpace) {
			retVal += QString::fromLatin1(" ");

			if (wordBoxes)
				(*wordBoxes)[wordBoxes->count()] = poppler_box->boundingBox();
			if (charBoxes)
				(*charBoxes)[charBoxes->count()] = poppler_box->boundingBox();
		}
		lastPopplerBox = poppler_box;
	}

  return retVal;
}

} // namespace PopplerQt

} // namespace Backend

} // namespace QtPDF

// vim: set sw=2 ts=2 et
