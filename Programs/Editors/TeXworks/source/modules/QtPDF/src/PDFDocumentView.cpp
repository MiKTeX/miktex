/**
 * Copyright (C) 2013-2024  Charlie Sharpsteen, Stefan Löffler
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

#include "PDFDocumentView.h"


#include "InfoWidgets.h"
#include "PDFDocumentScene.h"
#include "PDFGuideline.h"

// This has to be outside the namespace (according to Qt docs)
static void initResources()
{
  Q_INIT_RESOURCE(QtPDF_trans);
  Q_INIT_RESOURCE(QtPDF_icons);
}

namespace QtPDF {

void trStrings() {
  // The language and translator are currently not used but are accessed here so
  // they show up in the .ts files.
  Q_UNUSED(QT_TRANSLATE_NOOP("QtPDF", "[language name]"))
  Q_UNUSED(QT_TRANSLATE_NOOP("QtPDF", "[translator's name/email]"))
}

// In static builds, we need to explicitly initialize the resources
// (translations).
// NOTE: In shared builds, this doesn't seem to hurt.
class ResourceInitializer
{
public:
  // Call out-of-namespace function in constructor
  ResourceInitializer() { ::initResources(); }
};
static ResourceInitializer _resourceInitializer;

#ifdef DEBUG
#include <QDebug>
#endif

// PDFDocumentView
// ===============

// This class descends from `QGraphicsView` and is responsible for controlling
// and displaying the contents of a `Document` using a `QGraphicsScene`.
PDFDocumentView::PDFDocumentView(QWidget *parent /* = nullptr */):
  Super(parent)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  qRegisterMetaType<QtPDF::PDFSearcher::size_type>("QtPDF::PDFSearcher::size_type");
#endif
  initResources();
  setBackgroundRole(QPalette::Dark);
  setAlignment(Qt::AlignCenter);
  setFocusPolicy(Qt::StrongFocus);

  QColor fillColor(Qt::darkYellow);
  fillColor.setAlphaF(0.3f);
  _searchResultHighlightBrush = QBrush(fillColor);

  fillColor = QColor(Qt::yellow);
  fillColor.setAlphaF(0.6f);
  _currentSearchResultHighlightBrush = QBrush(fillColor);

  // If _currentPage is not set to -1, the compiler may default to 0. In that
  // case, `goFirst()` or `goToPage(0)` will fail because the view will think
  // it is already looking at page 0.
  _currentPage = -1;

  registerTool(std::unique_ptr<DocumentTool::AbstractTool>(new DocumentTool::ZoomIn(this)));
  registerTool(std::unique_ptr<DocumentTool::AbstractTool>(new DocumentTool::ZoomOut(this)));
  registerTool(std::unique_ptr<DocumentTool::AbstractTool>(new DocumentTool::MagnifyingGlass(this)));
  registerTool(std::unique_ptr<DocumentTool::AbstractTool>(new DocumentTool::MarqueeZoom(this)));
  registerTool(std::unique_ptr<DocumentTool::AbstractTool>(new DocumentTool::Move(this)));
  registerTool(std::unique_ptr<DocumentTool::AbstractTool>(new DocumentTool::ContextClick(this)));
  registerTool(std::unique_ptr<DocumentTool::AbstractTool>(new DocumentTool::Measure(this)));
  registerTool(std::unique_ptr<DocumentTool::AbstractTool>(new DocumentTool::Select(this)));

  // Some tools (e.g., the Select tool) may need to be informed of mouse events
  // (e.g., mouseMoveEvent) when armed, even before a mouse button is pressed
  viewport()->setMouseTracking(true);

  // We deliberately set the mouse mode to a different value above so we can
  // call setMouseMode (which bails out if the mouse mode is not changed), which
  // in turn sets up other variables such as _toolAccessors
  setMouseMode(MouseMode_MagnifyingGlass);

  connect(&_searcher, &PDFSearcher::resultReady, this, &PDFDocumentView::searchResultReady);
  connect(&_searcher, &PDFSearcher::progressValueChanged, this, &PDFDocumentView::searchProgressValueChanged);

  showRuler(false);
  connect(&_ruler, &PDFRuler::dragStart, this, [this](QPoint pos, Qt::Edge origin) {
    const Qt::Orientation orientation = [](Qt::Edge origin) {
      switch (origin) {
        case Qt::TopEdge:
        case Qt::BottomEdge:
          return Qt::Horizontal;
        case Qt::LeftEdge:
        case Qt::RightEdge:
          return Qt::Vertical;
      }
      return Qt::Horizontal;
    }(origin);
    PDFGuideline * line = new PDFGuideline(this, pos, orientation);
    connect(&_ruler, &PDFRuler::dragMove, line, &PDFGuideline::dragMove);
    connect(&_ruler, &PDFRuler::dragStop, line, [this,line](const QPoint p){
      disconnect(&_ruler, nullptr, line, nullptr);
      line->dragStop(p);
    });
  });
}

PDFDocumentView::~PDFDocumentView()
{
  _searcher.ensureStopped();
}

// Accessors
// ---------
void PDFDocumentView::setScene(QSharedPointer<PDFDocumentScene> a_scene)
{
  Super::setScene(a_scene.data());

  // disconnect us from the old scene (if any)
  if (_pdf_scene) {
    disconnect(_pdf_scene.data(), nullptr, this, nullptr);
    _pdf_scene.clear();
  }

  _pdf_scene = a_scene;

  // Reinitialize all scene-related data
  // NB: This also resets any text selection and search results as it clears the
  // page graphic items.
  reinitializeFromScene();

  if (a_scene) {
    // Respond to page jumps requested by the `PDFDocumentScene`.
    //
    // **TODO:**
    // _May want to consider not doing this by default. It is conceivable to have
    // a View that would ignore page jumps that other scenes would respond to._
    connect(_pdf_scene.data(), &PDFDocumentScene::pageChangeRequested, this, [this](int pageNum){ this->goToPage(pageNum); });
    connect(_pdf_scene.data(), &PDFDocumentScene::pdfActionTriggered, this, &PDFDocumentView::pdfActionTriggered);
    connect(_pdf_scene.data(), &PDFDocumentScene::documentChanged, this, &PDFDocumentView::reinitializeFromScene);
    // The connection PDFDocumentScene::documentChanged > PDFDocumentView::changedDocument
    // must be last in this list to ensure all internal states are updated (e.g.
    // in _lastPage in reinitializeFromScene()) before the signal is
    // communicated on to the "outside world".
    connect(_pdf_scene.data(), &PDFDocumentScene::documentChanged, this, &PDFDocumentView::changedDocument);
  }

  // ensure the zoom is reset if we load a new document
  zoom100();

  // Ensure we're at the top left corner (we need to set _currentPage to -1 to
  // ensure goToPage() actually does anything.
  size_type page = _currentPage;
  if (page >= 0) {
    _currentPage = -1;
    goToPage(page);
  }

  // Ensure proper layout
  setPageMode(_pageMode, true);

  if (_pdf_scene)
    emit changedDocument(_pdf_scene->document());
  else
    emit changedDocument(QSharedPointer<Backend::Document>());
}
PDFDocumentView::size_type PDFDocumentView::currentPage() { return _currentPage; }
PDFDocumentView::size_type PDFDocumentView::lastPage()    { return _lastPage; }

void PDFDocumentView::setPageMode(const PageMode pageMode, const bool forceRelayout /* = false */)
{
  if (_pageMode == pageMode && !forceRelayout)
    return;
  if (!_pdf_scene) {
    // If we don't have a scene (yet), save the setting for future use and return
    _pageMode = pageMode;
    emit changedPageMode(pageMode);
    return;
  }

  QGraphicsItem *currentPage = _pdf_scene->pageAt(_currentPage);
  if (!currentPage)
    return;

  // Save the current view relative to the current page so we can restore it
  // after changing the mode
  // **TODO:** Safeguard
  QRectF viewRect(mapToScene(viewport()->rect()).boundingRect());
  viewRect.translate(-currentPage->pos());

  // **TODO:** Avoid relayouting everything twice when switching from SinglePage
  // to TwoColumnContinuous (once by setContinuous(), and a second time by
  // setColumnCount() below)
  switch (pageMode) {
    case PageMode_SinglePage:
    case PageMode_Presentation:
      _pdf_scene->showOnePage(_currentPage);
      _pdf_scene->pageLayout().setContinuous(false);
      break;
    case PageMode_OneColumnContinuous:
      if (_pageMode == PageMode_SinglePage) {
        _pdf_scene->pageLayout().setContinuous(true);
        _pdf_scene->showAllPages();
        // Reset the scene rect; causes it the encompass the whole scene
        setSceneRect(QRectF());
      }
      _pdf_scene->pageLayout().setColumnCount(1, 0);
      break;
    case PageMode_TwoColumnContinuous:
      if (_pageMode == PageMode_SinglePage) {
        _pdf_scene->pageLayout().setContinuous(true);
        _pdf_scene->showAllPages();
        // Reset the scene rect; causes it the encompass the whole scene
        setSceneRect(QRectF());
      }
      _pdf_scene->pageLayout().setColumnCount(2, 1);
      break;
  }

  // Ensure the background is black during presentation (independent of the
  // current palette and background role)
  if (pageMode == PageMode_Presentation)
    setBackgroundBrush(QBrush(Qt::black));
  else
    setBackgroundBrush(Qt::NoBrush);

  _pageMode = pageMode;
  _pdf_scene->pageLayout().relayout();

  // We might need to update the scene rect (when switching to single page mode)
  maybeUpdateSceneRect();

  if (pageMode == PageMode_Presentation)
    zoomFitWindow();
  else {
    // Restore the view from before as good as possible
    viewRect.translate(_pdf_scene->pageAt(_currentPage)->pos());
    ensureVisible(viewRect, 0, 0);
  }

  emit changedPageMode(pageMode);
}

QDockWidget * PDFDocumentView::dockWidget(const Dock type, QWidget * parent /* = nullptr */)
{
  QDockWidget * dock = new QDockWidget(QString(), parent);
  Q_ASSERT(dock != nullptr);

  PDFDocumentInfoWidget * infoWidget{nullptr};
  switch (type) {
    case Dock_TableOfContents:
    {
      PDFToCInfoWidget * tocWidget = new PDFToCInfoWidget(dock);
      connect(tocWidget, &PDFToCInfoWidget::actionTriggered, this, &PDFDocumentView::pdfActionTriggered);
      infoWidget = tocWidget;
      break;
    }
    case Dock_MetaData:
      infoWidget = new PDFMetaDataInfoWidget(dock);
      break;
    case Dock_Fonts:
      infoWidget = new PDFFontsInfoWidget(dock);
      break;
    case Dock_Permissions:
      infoWidget = new PDFPermissionsInfoWidget(dock);
      break;
    case Dock_Annotations:
      infoWidget = new PDFAnnotationsInfoWidget(dock);
      // TODO: possibility to jump to selected/activated annotation
      break;
    case Dock_OptionalContent:
      infoWidget = new PDFOptionalContentInfoWidget(dock);
      break;
  }
  if (!infoWidget) {
    dock->deleteLater();
    return nullptr;
  }
  if (_pdf_scene && _pdf_scene->document())
      infoWidget->initFromDocument(_pdf_scene->document());
  connect(this, &PDFDocumentView::changedDocument, infoWidget, &PDFDocumentInfoWidget::initFromDocument);

  dock->setWindowTitle(infoWidget->windowTitle());
  dock->setObjectName(infoWidget->objectName() + QString::fromLatin1(".DockWidget"));
  connect(infoWidget, &PDFDocumentInfoWidget::windowTitleChanged, dock, &QDockWidget::setWindowTitle);

  // We don't want docks to (need to) take up a lot of space. If the infoWidget
  // can't shrink, we thus put it into a scroll area that can
  if (!(static_cast<int>(infoWidget->sizePolicy().horizontalPolicy()) & QSizePolicy::ShrinkFlag) || \
      !(static_cast<int>(infoWidget->sizePolicy().verticalPolicy()) & QSizePolicy::ShrinkFlag)) {
    QScrollArea * scrollArea = new QScrollArea(dock);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidget(infoWidget);
    dock->setWidget(scrollArea);
  }
  else
    dock->setWidget(infoWidget);
  return dock;
}

// path in PDF coordinates
QGraphicsPathItem * PDFDocumentView::addHighlightPath(const size_type page, const QPainterPath & path, const QBrush & brush, const QPen & pen /* = Qt::NoPen */)
{
  if (!_pdf_scene)
    return nullptr;

  PDFPageGraphicsItem * pageItem = dynamic_cast<PDFPageGraphicsItem*>(_pdf_scene->pageAt(static_cast<int>(page)));
  if (!pageItem || !PDFDocumentScene::isPageItem(pageItem))
    return nullptr;

  QGraphicsPathItem * highlightItem = new QGraphicsPathItem(path, pageItem);
  highlightItem->setBrush(brush);
  highlightItem->setPen(pen);
  highlightItem->setTransform(pageItem->pointScale());
  return highlightItem;
}

void PDFDocumentView::fitInView(const QRectF & rect, Qt::AspectRatioMode aspectRatioMode /* = Qt::IgnoreAspectRatio */)
{
  const unsigned int ScaleDataSize = 4;
  struct ScaleData {
    qreal xratio, yratio;
    bool horizontalScrollbar, verticalScrollbar;
  } _scaleDat[ScaleDataSize];
  qreal xratio = -1, yratio = -1;
  bool horizontalScrollbar = true, verticalScrollbar = true;
  QRectF viewRect;
  QRectF sceneRect;

  // This method is modeled closely after QGraphicsView::fitInView(), with two
  // notable exceptions: 1) no arbitrary (hard-coded) margin is added, thus
  // allowing page-scrolling without (slight) shifts. 2) we calculate zoom
  // factors with and without scroll bars and pick the most suitable one; this
  // way, cases where we would zoom out so much that, e.g., a horizontal scroll
  // bar is no longer needed are handled properly

  // Ensure that we have a valid rect to fit into the view
  if (rect.isNull())
    return;

  // Save the current scroll bar policies so we can restore them later;
  // NB: this method repeatedly changes the scroll bar policies to "simulate"
  // cases without scroll bars as needed
  Qt::ScrollBarPolicy oldHorizontalPolicy = horizontalScrollBarPolicy();
  Qt::ScrollBarPolicy oldVerticalPolicy = verticalScrollBarPolicy();

  // Reset the view scale to 1:1.
  QRectF unity = transform().mapRect(QRectF(0, 0, 1, 1));
  if (unity.isEmpty())
    return;
  scale(1 / unity.width(), 1 / unity.height());

  // 1) determine the potential scaling ratios for "only" fitting the given
  // rect into the current viewport; the scroll bars remain as they are
  _scaleDat[0].horizontalScrollbar = (horizontalScrollBar() ? horizontalScrollBar()->isVisible() : false);
  _scaleDat[0].verticalScrollbar = (verticalScrollBar() ? verticalScrollBar()->isVisible() : false);
  viewRect = viewport()->rect();
  sceneRect = transform().mapRect(rect);
  if (!viewRect.isEmpty() && !sceneRect.isEmpty()) {
    _scaleDat[0].xratio = viewRect.width() / sceneRect.width();
    _scaleDat[0].yratio = viewRect.height() / sceneRect.height();
  }
  else {
    _scaleDat[0].xratio = -1;
    _scaleDat[0].yratio = -1;
  }

  // 2) determine potential scaling ratios for fitting the whole scene width
  // into the viewport (with disabled horizontal scroll bars if possible!)
  setHorizontalScrollBarPolicy(oldHorizontalPolicy == Qt::ScrollBarAsNeeded ? Qt::ScrollBarAlwaysOff : oldHorizontalPolicy);
  setVerticalScrollBarPolicy(oldVerticalPolicy);
  _scaleDat[1].horizontalScrollbar = (horizontalScrollBar() ? horizontalScrollBar()->isVisible() : false);
  _scaleDat[1].verticalScrollbar = (verticalScrollBar() ? verticalScrollBar()->isVisible() : false);

  viewRect = viewport()->rect();
  setHorizontalScrollBarPolicy(oldHorizontalPolicy);
  sceneRect.setLeft(transform().mapRect(_pdf_scene->sceneRect()).left());
  sceneRect.setRight(transform().mapRect(_pdf_scene->sceneRect()).right());

  if (!viewRect.isEmpty() && !sceneRect.isEmpty()) {
    _scaleDat[1].xratio = viewRect.width() / sceneRect.width();
    _scaleDat[1].yratio = viewRect.height() / sceneRect.height();
  }
  else {
    _scaleDat[1].xratio = -1;
    _scaleDat[1].yratio = -1;
  }

  // 3) determine potential scaling ratios for fitting the whole scene height
  // into the viewport (with disabled vertical scroll bars if possible!)
  setHorizontalScrollBarPolicy(oldHorizontalPolicy);
  setVerticalScrollBarPolicy(oldVerticalPolicy == Qt::ScrollBarAsNeeded ? Qt::ScrollBarAlwaysOff : oldVerticalPolicy);
  _scaleDat[2].horizontalScrollbar = (horizontalScrollBar() ? horizontalScrollBar()->isVisible() : false);
  _scaleDat[2].verticalScrollbar = (verticalScrollBar() ? verticalScrollBar()->isVisible() : false);

  viewRect = viewport()->rect();
  setVerticalScrollBarPolicy(oldVerticalPolicy);
  sceneRect.setTop(transform().mapRect(_pdf_scene->sceneRect()).top());
  sceneRect.setBottom(transform().mapRect(_pdf_scene->sceneRect()).bottom());

  if (!viewRect.isEmpty() && !sceneRect.isEmpty()) {
    _scaleDat[2].xratio = viewRect.width() / sceneRect.width();
    _scaleDat[2].yratio = viewRect.height() / sceneRect.height();
  }
  else {
    _scaleDat[2].xratio = -1;
    _scaleDat[2].yratio = -1;
  }

  // 4) determine potential scaling ratios for fitting the whole scene
  // into the viewport (with disabled scroll bars if possible!)
  setHorizontalScrollBarPolicy(oldHorizontalPolicy == Qt::ScrollBarAsNeeded ? Qt::ScrollBarAlwaysOff : oldHorizontalPolicy);
  setVerticalScrollBarPolicy(oldVerticalPolicy == Qt::ScrollBarAsNeeded ? Qt::ScrollBarAlwaysOff : oldVerticalPolicy);
  _scaleDat[3].horizontalScrollbar = (horizontalScrollBar() ? horizontalScrollBar()->isVisible() : false);
  _scaleDat[3].verticalScrollbar = (verticalScrollBar() ? verticalScrollBar()->isVisible() : false);

  viewRect = viewport()->rect();
  setHorizontalScrollBarPolicy(oldHorizontalPolicy);
  setVerticalScrollBarPolicy(oldVerticalPolicy);
  sceneRect = transform().mapRect(_pdf_scene->sceneRect());

  if (!viewRect.isEmpty() && !sceneRect.isEmpty()) {
    _scaleDat[3].xratio = viewRect.width() / sceneRect.width();
    _scaleDat[3].yratio = viewRect.height() / sceneRect.height();
  }
  else {
    _scaleDat[3].xratio = -1;
    _scaleDat[3].yratio = -1;
  }

  // Determine the optimal (i.e., maximum) zoom factor
  // Respect the aspect ratio mode.
  switch (aspectRatioMode) {
    case Qt::KeepAspectRatio:
      xratio = yratio = -1;
      for (unsigned int i = 0; i < ScaleDataSize; ++i) {
        qreal r = qMin(_scaleDat[i].xratio, _scaleDat[i].yratio);
        if (xratio < r) {
          xratio = yratio = r;
          horizontalScrollbar = _scaleDat[i].horizontalScrollbar;
          verticalScrollbar = _scaleDat[i].verticalScrollbar;
        }
      }
      break;
    case Qt::KeepAspectRatioByExpanding:
      xratio = yratio = -1;
      for (unsigned int i = 0; i < ScaleDataSize; ++i) {
        qreal r = qMax(_scaleDat[i].xratio, _scaleDat[i].yratio);
        if (xratio < r) {
          xratio = yratio = r;
          horizontalScrollbar = _scaleDat[i].horizontalScrollbar;
          verticalScrollbar = _scaleDat[i].verticalScrollbar;
        }
      }
      break;
    case Qt::IgnoreAspectRatio:
      xratio = yratio = -1;
      for (unsigned int i = 0; i < ScaleDataSize; ++i) {
        if (xratio < _scaleDat[i].xratio) {
          xratio = _scaleDat[i].xratio;
          horizontalScrollbar = _scaleDat[i].horizontalScrollbar;
        }
        if (yratio < _scaleDat[i].yratio) {
          yratio = _scaleDat[i].yratio;
          verticalScrollbar = _scaleDat[i].verticalScrollbar;
        }
      }
      break;
  }

  if (xratio <= 0 || yratio <= 0)
    return;

  // Set the scroll bar policy to what it will be in the end so `centerOn` works
  // with the correct viewport
  setHorizontalScrollBarPolicy(horizontalScrollbar ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(verticalScrollbar ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);

  // Scale and center on the center of \a rect.
  scale(xratio, yratio);
  centerOn(rect.center());

  // Reset the scroll bar policy to what it was before
  setHorizontalScrollBarPolicy(oldHorizontalPolicy);
  setVerticalScrollBarPolicy(oldVerticalPolicy);
}

const QWeakPointer<QtPDF::Backend::Document> PDFDocumentView::document() const
{
  return (_pdf_scene ? _pdf_scene->document() : QWeakPointer<QtPDF::Backend::Document>());
}

QString PDFDocumentView::selectedText() const
{
  QSharedPointer<Backend::Document> doc = document().toStrongRef();
  if(!doc || !doc->permissions().testFlag(Backend::Document::Permission_Extract))
    return QString();
  if (!_armedTool || _armedTool->type() != DocumentTool::AbstractTool::Tool_Select)
    return QString();
  return dynamic_cast<DocumentTool::Select*>(_armedTool)->selectedText();
}

// Public Slots
// ------------

void PDFDocumentView::goPrev()  { goToPage(_currentPage - 1, Qt::AlignBottom); }
void PDFDocumentView::goNext()  { goToPage(_currentPage + 1, Qt::AlignTop); }
void PDFDocumentView::goFirst() { goToPage(static_cast<size_type>(0)); }
void PDFDocumentView::goLast()  { goToPage(_lastPage - 1); }

void PDFDocumentView::goPrevViewRect() {
  if (_oldViewRects.empty())
    return;
  goToPDFDestination(_oldViewRects.pop(), false);
}


// `goToPage` will shift the view to a different page. If the `alignment`
// parameter is `Qt::AlignLeft | Qt::AlignTop` (the default), the view will
// ensure the top left corner of the page is visible and aligned with the top
// left corner of the viewport (if possible). Other alignments can be used in
// the same way. If `alignment` for a direction is not set the view will
// show the same portion of the new page as it did before with the old page.
void PDFDocumentView::goToPage(const size_type pageNum, const int alignment /* = Qt::AlignLeft | Qt::AlignTop */)
{
  // We silently ignore any invalid page numbers.
  if (!_pdf_scene || pageNum < 0 || pageNum >= _lastPage)
    return;
  if (pageNum == _currentPage)
    return;

  goToPage(dynamic_cast<const PDFPageGraphicsItem*>(_pdf_scene->pageAt(pageNum)), alignment);
}

void PDFDocumentView::goToPage(const size_type pageNum, const QPointF anchor, const int alignment /* = Qt::AlignHCenter | Qt::AlignVCenter */)
{
  // We silently ignore any invalid page numbers.
  if (!_pdf_scene || pageNum < 0 || pageNum >= _lastPage)
    return;
  if (pageNum == _currentPage)
    return;

  goToPage(dynamic_cast<const PDFPageGraphicsItem*>(_pdf_scene->pageAt(pageNum)), anchor, alignment);
}

void PDFDocumentView::goToPDFDestination(const PDFDestination & dest, bool saveOldViewRect /* = true */)
{
  if (!dest.isValid())
    return;

  Q_ASSERT(_pdf_scene != nullptr);
  Q_ASSERT(!_pdf_scene->document().isNull());
  QSharedPointer<Backend::Document> doc(_pdf_scene->document().toStrongRef());
  if (!doc)
    return;

  PDFDestination finalDest;
  if (!dest.isExplicit()) {
    finalDest = doc->resolveDestination(dest);
    if (!finalDest.isValid())
      return;
  }
  else
    finalDest = dest;

  Q_ASSERT(PDFDocumentScene::isPageItem(_pdf_scene->pageAt(_currentPage)));
  PDFPageGraphicsItem * pageItem = dynamic_cast<PDFPageGraphicsItem*>(_pdf_scene->pageAt(_currentPage));
  Q_ASSERT(pageItem != nullptr);

  // Get the current (=old) viewport in the current (=old) page's
  // coordinate system
  QRectF oldViewport = pageItem->mapRectFromScene(mapToScene(viewport()->rect()).boundingRect());
  oldViewport = QRectF(pageItem->mapToPage(oldViewport.topLeft()), \
                       pageItem->mapToPage(oldViewport.bottomRight()));
  // Calculate the new viewport (in page coordinates)
  QRectF view(finalDest.viewport(doc.data(), oldViewport, _zoomLevel));

  if (saveOldViewRect) {
    PDFDestination origin(_currentPage);
    origin.setType(PDFDestination::Destination_FitR);
    origin.setRect(oldViewport);
    _oldViewRects.push(origin);
  }

  goToPage(dynamic_cast<PDFPageGraphicsItem*>(_pdf_scene->pageAt(finalDest.page())), view, true);
}

void PDFDocumentView::zoomBy(const qreal zoomFactor, const QGraphicsView::ViewportAnchor anchor /* = QGraphicsView::AnchorViewCenter */)
{
  if (zoomFactor <= 0)
    return;

  _zoomLevel *= zoomFactor;
  // Set the transformation anchor to AnchorViewCenter so we always zoom out of
  // the center of the view (rather than out of the upper left corner)
  QGraphicsView::ViewportAnchor oldAnchor = transformationAnchor();
  setTransformationAnchor(anchor);
  this->scale(zoomFactor, zoomFactor);
  setTransformationAnchor(oldAnchor);

  emit changedZoom(_zoomLevel);
}

void PDFDocumentView::setZoomLevel(const qreal zoomLevel, const QGraphicsView::ViewportAnchor anchor /* = QGraphicsView::AnchorViewCenter */)
{
  if (zoomLevel <= 0)
    return;
  zoomBy(zoomLevel / _zoomLevel, anchor);
}

void PDFDocumentView::zoomIn(const QGraphicsView::ViewportAnchor anchor /* = QGraphicsView::AnchorViewCenter */) { zoomBy(3.0/2.0, anchor); }
void PDFDocumentView::zoomOut(const QGraphicsView::ViewportAnchor anchor /* = QGraphicsView::AnchorViewCenter */) { zoomBy(2.0/3.0, anchor); }

void PDFDocumentView::zoomToRect(QRectF a_rect)
{
  // NOTE: The argument, `a_rect`, is assumed to be in _scene coordinates_.
  fitInView(a_rect, Qt::KeepAspectRatio);

  // Since we passed `Qt::KeepAspectRatio` to `fitInView` both x and y scaling
  // factors were changed by the same amount. So we'll just take the x scale to
  // be the new `_zoomLevel`.
  _zoomLevel = transform().m11();
  emit changedZoom(_zoomLevel);
}

void PDFDocumentView::zoomFitWindow()
{
  if (!scene())
    return;

  QGraphicsItem *currentPage = _pdf_scene->pageAt(_currentPage);
  if (!currentPage)
    return;

  QRectF rect(currentPage->sceneBoundingRect());
  // Add a margin of half the inter-page spacing around the page rect so
  // scrolling by one such rect will take us to exactly the same area on the
  // next page
  qreal dx = _pdf_scene->pageLayout().xSpacing();
  qreal dy = _pdf_scene->pageLayout().ySpacing();
  rect.adjust(-dx / 2, -dy / 2, dx / 2, dy / 2);

  zoomToRect(rect);
}


void PDFDocumentView::zoomFitWidth()
{
  if (!_pdf_scene)
    return;

  QGraphicsItem *currentPage = _pdf_scene->pageAt(_currentPage);
  if (!currentPage)
    return;

  QRectF rect(currentPage->sceneBoundingRect());

  // Add a margin of half the inter-page spacing around the page rect so
  // scrolling by one such rect will take us to exactly the same area on the
  // next page
  qreal dx = _pdf_scene->pageLayout().xSpacing();
  rect.adjust(-dx / 2, 0, dx / 2, 0);

  // Store current y position so we can center on it later.
  qreal ypos = mapToScene(viewport()->rect()).boundingRect().center().y();

  // Squash the rect to minimal height so its width will be limitting the zoom
  // factor
  rect.setTop(ypos - 1e-5);
  rect.setBottom(ypos + 1e-5);

  zoomToRect(rect);
}

void PDFDocumentView::zoomFitContentWidth()
{
  if (!_pdf_scene)
    return;

  PDFPageGraphicsItem *currentPage = dynamic_cast<PDFPageGraphicsItem*>(_pdf_scene->pageAt(_currentPage));
  if (!currentPage)
    return;

  QSharedPointer<Backend::Page> page = currentPage->page().toStrongRef();
  if (!page)
    return;

  QRectF rect(page->getContentBoundingBox());
  rect = currentPage->mapRectToScene(QRectF(currentPage->mapFromPage(rect.topLeft()), currentPage->mapFromPage(rect.bottomRight())));

  // Store current y position so we can center on it later.
  qreal ypos = mapToScene(viewport()->rect()).boundingRect().center().y();

  // Squash the rect to minimal height so its width will be limitting the zoom
  // factor
  rect.setTop(ypos - 1e-5);
  rect.setBottom(ypos + 1e-5);

  zoomToRect(rect);
}


void PDFDocumentView::zoom100()
{
  // Reset zoom level to 100%

  // Reset the view scale to 1:1.
  QRectF unity = transform().mapRect(QRectF(0, 0, 1, 1));
  if (unity.isEmpty())
      return;

  // Set the transformation anchor to AnchorViewCenter so we always zoom out of
  // the center of the view (rather than out of the upper left corner)
  QGraphicsView::ViewportAnchor anchor = transformationAnchor();
  setTransformationAnchor(QGraphicsView::AnchorViewCenter);
  scale(1 / unity.width(), 1 / unity.height());
  setTransformationAnchor(anchor);

  _zoomLevel = transform().m11();
  emit changedZoom(_zoomLevel);
}

void PDFDocumentView::setMouseMode(const MouseMode newMode)
{
  if (_mouseMode == newMode)
    return;

  // TODO: eventually make _toolAccessors configurable
  _toolAccessors.clear();
  _toolAccessors[Qt::KeyboardModifiers(Qt::ControlModifier) | Qt::MouseButtons(Qt::LeftButton)] = DocumentTool::AbstractTool::Tool_ContextClick;
  _toolAccessors[Qt::KeyboardModifiers(Qt::NoModifier) | Qt::MouseButtons(Qt::RightButton)] = DocumentTool::AbstractTool::Tool_ContextMenu;
  _toolAccessors[Qt::KeyboardModifiers(Qt::NoModifier) | Qt::MouseButtons(Qt::MiddleButton)] = DocumentTool::AbstractTool::Tool_Move;
  _toolAccessors[Qt::KeyboardModifiers(Qt::ShiftModifier) | Qt::MouseButtons(Qt::LeftButton)] = DocumentTool::AbstractTool::Tool_ZoomIn;
  _toolAccessors[Qt::KeyboardModifiers(Qt::AltModifier) | Qt::MouseButtons(Qt::LeftButton)] = DocumentTool::AbstractTool::Tool_ZoomOut;
  // Other tools: Tool_MagnifyingGlass, Tool_MarqueeZoom, Tool_Move

  disarmTool();

  switch (newMode) {
    case MouseMode_Move:
      armTool(DocumentTool::AbstractTool::Tool_Move);
      _toolAccessors[Qt::KeyboardModifiers(Qt::NoModifier) | Qt::MouseButtons(Qt::LeftButton)] = DocumentTool::AbstractTool::Tool_Move;
      break;

    case MouseMode_MarqueeZoom:
      armTool(DocumentTool::AbstractTool::Tool_MarqueeZoom);
      _toolAccessors[Qt::KeyboardModifiers(Qt::NoModifier) | Qt::MouseButtons(Qt::LeftButton)] = DocumentTool::AbstractTool::Tool_MarqueeZoom;
      break;

    case MouseMode_MagnifyingGlass:
      armTool(DocumentTool::AbstractTool::Tool_MagnifyingGlass);
      _toolAccessors[Qt::KeyboardModifiers(Qt::NoModifier) | Qt::MouseButtons(Qt::LeftButton)] = DocumentTool::AbstractTool::Tool_MagnifyingGlass;
      break;

    case MouseMode_Measure:
      armTool(DocumentTool::AbstractTool::Tool_Measure);
      _toolAccessors[Qt::KeyboardModifiers(Qt::NoModifier) | Qt::MouseButtons(Qt::LeftButton)] = DocumentTool::AbstractTool::Tool_Measure;
      break;

    case MouseMode_Select:
      armTool(DocumentTool::AbstractTool::Tool_Select);
      _toolAccessors[Qt::KeyboardModifiers(Qt::NoModifier) | Qt::MouseButtons(Qt::LeftButton)] = DocumentTool::AbstractTool::Tool_Select;
      break;
  }

  _mouseMode = newMode;
}

void PDFDocumentView::setMagnifierShape(const DocumentTool::MagnifyingGlass::MagnifierShape shape)
{
  DocumentTool::MagnifyingGlass * magnifier = dynamic_cast<DocumentTool::MagnifyingGlass*>(getToolByType(DocumentTool::AbstractTool::Tool_MagnifyingGlass));
  if (magnifier)
    magnifier->setMagnifierShape(shape);
}

void PDFDocumentView::setMagnifierSize(const int size)
{
  DocumentTool::MagnifyingGlass * magnifier = dynamic_cast<DocumentTool::MagnifyingGlass*>(getToolByType(DocumentTool::AbstractTool::Tool_MagnifyingGlass));
  if (magnifier)
    magnifier->setMagnifierSize(size);
}

void PDFDocumentView::search(QString searchText, Backend::SearchFlags flags /* = Backend::Search_CaseInsensitive */)
{
  if (!_pdf_scene)
    return;

  // If `searchText` is the same as for the last search, focus on the next
  // search result.
  // Note: The primary use case for this is hitting `Enter` several times in the
  // search box to go to the next result.
  // On the other hand, with this there is no easy way to abort a long-running
  // search (e.g., in a large document) and restarting it in another part by
  // simply going there and hitting `Enter` again. As a workaround, one can
  // change the search text in that case (e.g., to something meaningless and
  // then back again to abort the previous search and restart at the new
  // location).
  if (searchText == _searcher.searchString() && flags == _searcher.searchFlags()) {
    nextSearchResult();
    return;
  }

  _searcher.ensureStopped();

  clearSearchResults();
  _currentSearchResult = -1;

  _searcher.setSearchString(searchText);
  _searcher.setSearchFlags(flags);
  _searcher.setDocument(document());
  _searcher.setStartPage(currentPage());
  _searcher.start();
}

void PDFDocumentView::nextSearchResult()
{
  if (!_pdf_scene || _searchResults.empty())
    return;

  // Note: _currentSearchResult is initially -1 if no result is selected
  if (_currentSearchResult >= 0 && _searchResults[_currentSearchResult])
    dynamic_cast<QGraphicsPathItem*>(_searchResults[_currentSearchResult])->setBrush(_searchResultHighlightBrush);

  if ( (_currentSearchResult + 1) >= _searchResults.size() )
    _currentSearchResult = 0;
  else
    ++_currentSearchResult;

  // FIXME: The rest of the code in this method is the same as in previousSearchResult()
  // We should move this into its own private method

  QGraphicsPathItem* highlightPath = dynamic_cast<QGraphicsPathItem*>(_searchResults[_currentSearchResult]);

  if (!highlightPath)
    return;

  highlightPath->setBrush(_currentSearchResultHighlightBrush);

  PDFPageGraphicsItem * pageItem = dynamic_cast<PDFPageGraphicsItem *>(highlightPath->parentItem());
  if (pageItem) {
    const QRectF bb = pageItem->mapRectFromItem(highlightPath, highlightPath->boundingRect());
    const QRectF pdfRect = QRectF(pageItem->mapToPage(bb.topLeft()), pageItem->mapToPage(bb.bottomRight()));
    goToPage(pageItem, pdfRect, false);

    QSharedPointer<Backend::Page> page = pageItem->page().toStrongRef();
    // FIXME: shape subpath coordinates seem to be in upside down pdf coordinates. We should find a better place to construct the proper transform (e.g., in PDFPageGraphicsItem)
    if (page)
      emit searchResultHighlighted(pageItem->pageNum(), highlightPath->shape().toSubpathPolygons(QTransform::fromTranslate(0, page->pageSizeF().height()).scale(1, -1)));
  }
}

void PDFDocumentView::previousSearchResult()
{
  if (!_pdf_scene || _searchResults.empty())
    return;

  if (_currentSearchResult >= 0 && _searchResults[_currentSearchResult])
    dynamic_cast<QGraphicsPathItem*>(_searchResults[_currentSearchResult])->setBrush(_searchResultHighlightBrush);

  if ( (_currentSearchResult - 1) < 0 )
    _currentSearchResult = _searchResults.size() - 1;
  else
    --_currentSearchResult;

  // FIXME: The rest of the code in this method is the same as in previousSearchResult()
  // We should move this into its own private method

  QGraphicsPathItem* highlightPath = dynamic_cast<QGraphicsPathItem*>(_searchResults[_currentSearchResult]);

  if (!highlightPath)
    return;

  highlightPath->setBrush(_currentSearchResultHighlightBrush);

  PDFPageGraphicsItem * pageItem = dynamic_cast<PDFPageGraphicsItem *>(highlightPath->parentItem());
  if (pageItem) {
    const QRectF bb = pageItem->mapRectFromItem(highlightPath, highlightPath->boundingRect());
    const QRectF pdfRect = QRectF(pageItem->mapToPage(bb.topLeft()), pageItem->mapToPage(bb.bottomRight()));
    goToPage(pageItem, pdfRect, false);

    QSharedPointer<Backend::Page> page = pageItem->page().toStrongRef();
    // FIXME: shape subpath coordinates seem to be in upside down pdf coordinates. We should find a better place to construct the proper transform (e.g., in PDFPageGraphicsItem)
    if (page)
      emit searchResultHighlighted(pageItem->pageNum(), highlightPath->shape().toSubpathPolygons(QTransform::fromTranslate(0, page->pageSizeF().height()).scale(1, -1)));
  }
}

void PDFDocumentView::clearSearchResults()
{
  if (!_pdf_scene || _searchResults.empty())
    return;

  foreach( QGraphicsItem *item, _searchResults )
    _pdf_scene->removeItem(item);

  _searchResults.clear();
}

void PDFDocumentView::setSearchResultHighlightBrush(const QBrush & brush)
{
  _searchResultHighlightBrush = brush;
  for (int i = 0; i < _searchResults.size(); ++i) {
    if (i == _currentSearchResult || !_searchResults[i])
      continue;
    dynamic_cast<QGraphicsPathItem*>(_searchResults[i])->setBrush(brush);
  }
}

void PDFDocumentView::setCurrentSearchResultHighlightBrush(const QBrush & brush)
{
  _currentSearchResultHighlightBrush = brush;
  if (_currentSearchResult >= 0 && _currentSearchResult < _searchResults.size() && _searchResults[_currentSearchResult])
    dynamic_cast<QGraphicsPathItem*>(_searchResults[_currentSearchResult])->setBrush(brush);
}


// Protected Slots
// --------------
void PDFDocumentView::searchResultReady(PDFSearcher::size_type pageIndex)
{
  const auto & results = _searcher.resultAt(pageIndex);
  // Convert the search result to highlight boxes
  for(const Backend::SearchResult & result : results) {
    _searchResults << addHighlightPath(result.pageNum, result.bbox, _searchResultHighlightBrush);
  }

  // If this is the first result that becomes available in a new search, center
  // on the first result
  if (_currentSearchResult == -1)
    nextSearchResult();

  // Inform the rest of the world of our progress (in %, and how many
  // occurrences were found so far).
  emit searchProgressChanged(static_cast<int>(100 * (_searcher.progressValue() - _searcher.progressMinimum()) / (_searcher.progressMaximum() - _searcher.progressMinimum())), _searchResults.count());
}

void PDFDocumentView::searchProgressValueChanged(PDFSearcher::size_type progressValue)
{
  // Inform the rest of the world of our progress (in %, and how many
  // occurrences were found so far)
  // NOTE: the searchProgressValueChanged slot is not necessarily synchronized
  // with the searchResultReady slot. I.e., it can happen that
  // searchProgressValueChanged reports 100% with 0 search results (before
  // searchResultReady is called for the first time). Thus, searchResultReady
  // is also set up to emit searchProgressChanged. In summary,
  // searchProgressValueChanged is intended primarily for informing the user
  // of the progress when no matches are found, whereas searchResultReady is
  // primarily intended for informing the user of the progress when matches are
  // found.
  if (_searcher.progressMaximum() == _searcher.progressMinimum())
    emit searchProgressChanged(100, _searchResults.count());
  else
    emit searchProgressChanged(static_cast<int>(100 * (progressValue - _searcher.progressMinimum()) / (_searcher.progressMaximum() - _searcher.progressMinimum())), _searchResults.count());
}

void PDFDocumentView::maybeUpdateSceneRect() {
  if (!_pdf_scene || (_pageMode != PageMode_SinglePage && _pageMode != PageMode_Presentation))
    return;

  // Set the scene rect of the view, i.e., the rect accessible via the scroll
  // bars. In single page mode, this must be the rect of the current page
  PDFPageGraphicsItem * pageItem = dynamic_cast<PDFPageGraphicsItem *>(_pdf_scene->pageAt(_currentPage));
  if (pageItem)
    setSceneRect(pageItem->sceneBoundingRect());
}

void PDFDocumentView::maybeArmTool(uint modifiers)
{
  // Arms the tool corresponding to `modifiers` if one is available.
  DocumentTool::AbstractTool * t = getToolByType(_toolAccessors.value(modifiers, DocumentTool::AbstractTool::Tool_None));
  if (t != _armedTool) {
    disarmTool();
    armTool(t);
  }
}

void PDFDocumentView::goToPage(const PDFPageGraphicsItem * page, const int alignment /* = Qt::AlignLeft | Qt::AlignTop */)
{
  if (!_pdf_scene || !page || !PDFDocumentScene::isPageItem(page))
    return;
  size_type pageNum = _pdf_scene->pageNumFor(page);
  if (pageNum == _currentPage)
    return;

  PDFPageGraphicsItem *oldPage = dynamic_cast<PDFPageGraphicsItem*>(_pdf_scene->pageAt(_currentPage));

  if (_pageMode != PageMode_Presentation) {
    QRectF viewRect(mapToScene(QRect(QPoint(0, 0), viewport()->size())).boundingRect());

    // Note: This function must work if oldPage == nullptr (e.g., during start up)
    if (oldPage && PDFDocumentScene::isPageItem(oldPage))
      viewRect = oldPage->mapRectFromScene(viewRect);
    else {
      // If we don't have an oldPage for whatever reason (e.g., during start up)
      // we default to the top left corner of newPage instead
      viewRect = page->mapRectFromScene(viewRect);
      viewRect.moveTopLeft(QPointF(0, 0));
    }

    switch (alignment & Qt::AlignHorizontal_Mask) {
      case Qt::AlignLeft:
        viewRect.moveLeft(page->boundingRect().left());
        break;
      case Qt::AlignRight:
        viewRect.moveRight(page->boundingRect().right());
        break;
      case Qt::AlignHCenter:
        viewRect.moveCenter(QPointF(page->boundingRect().center().x(), viewRect.center().y()));
        break;
      default:
        // without (valid) alignment, we don't do anything
        break;
    }
    switch (alignment & Qt::AlignVertical_Mask) {
      case Qt::AlignTop:
        viewRect.moveTop(page->boundingRect().top());
        break;
      case Qt::AlignBottom:
        viewRect.moveBottom(page->boundingRect().bottom());
        break;
      case Qt::AlignVCenter:
        viewRect.moveCenter(QPointF(viewRect.center().x(), page->boundingRect().center().y()));
        break;
      default:
        // without (valid) alignment, we don't do anything
        break;
    }

    if (_pageMode == PageMode_SinglePage) {
      _pdf_scene->showOnePage(page);
      maybeUpdateSceneRect();
    }

    viewRect = page->mapRectToScene(viewRect);
    // Note: ensureVisible seems to have a small glitch. Even if the passed
    // `viewRect` is identical, the result may depend on the view's previous state
    // if the margins are not -1. However, -1 margins don't work during the
    // initialization when the viewport doesn't have its final size yet (for
    // whatever reasons, the end result is a view centered on the scene).
    // So we use centerOn for now which should give the same result since
    // viewRect has the same size as the viewport.
  //  ensureVisible(viewRect, -1, -1);
    centerOn(viewRect.center());
    _currentPage = pageNum;
  }
  else { // _pageMode != PageMode_Presentation
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    double oldXres = QApplication::desktop()->physicalDpiX() * _zoomLevel;
    double oldYres = QApplication::desktop()->physicalDpiY() * _zoomLevel;
#else
    double oldXres = screen()->physicalDotsPerInchX() * _zoomLevel;
    double oldYres = screen()->physicalDotsPerInchY() * _zoomLevel;
#endif
    _pdf_scene->showOnePage(page);
    _currentPage = pageNum;
    maybeUpdateSceneRect();
    zoomFitWindow();
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    double xres = QApplication::desktop()->physicalDpiX() * _zoomLevel;
    double yres = QApplication::desktop()->physicalDpiY() * _zoomLevel;
#else
    double xres = screen()->physicalDotsPerInchX() * _zoomLevel;
    double yres = screen()->physicalDotsPerInchY() * _zoomLevel;
#endif
    QSharedPointer<Backend::Page> backendPage(page->page().toStrongRef());

    if (backendPage && backendPage->transition()) {
      backendPage->transition()->reset();
      // Setting listener = nullptr in calls to getTileImage to force synchronous
      // rendering
      if (oldPage) {
        QSharedPointer<Backend::Page> oldBackendPage(oldPage->page().toStrongRef());
        backendPage->transition()->start(*(oldBackendPage->getTileImage(nullptr, oldXres, oldYres)), *(backendPage->getTileImage(nullptr, xres, yres)));
      }
    }
  }
  emit changedPage(_currentPage);
}

// TODO: Test
// deprecated/unused/unmaintained
// missing: oldPage handling/checks, presentation mode
void PDFDocumentView::goToPage(const PDFPageGraphicsItem * page, const QPointF anchor, const int alignment /* = Qt::AlignHCenter | Qt::AlignVCenter */)
{
  if (!_pdf_scene || !page || !PDFDocumentScene::isPageItem(page))
    return;
  size_type pageNum = _pdf_scene->pageNumFor(page);
  if (pageNum == _currentPage)
    return;

  QRectF viewRect(mapToScene(QRect(QPoint(0, 0), viewport()->size())).boundingRect());

  // Transform to item coordinates
  viewRect = page->mapRectFromScene(viewRect);

  switch (alignment & Qt::AlignHorizontal_Mask) {
    case Qt::AlignLeft:
      viewRect.moveLeft(anchor.x());
      break;
    case Qt::AlignRight:
      viewRect.moveRight(anchor.x());
      break;
    case Qt::AlignHCenter:
    default:
      viewRect.moveCenter(QPointF(anchor.x(), viewRect.center().y()));
      break;
  }
  switch (alignment & Qt::AlignVertical_Mask) {
    case Qt::AlignTop:
      viewRect.moveTop(anchor.y());
      break;
    case Qt::AlignBottom:
      viewRect.moveBottom(anchor.y());
      break;
    case Qt::AlignVCenter:
    default:
      viewRect.moveCenter(QPointF(viewRect.center().x(), anchor.y()));
      break;
  }

  if (_pageMode == PageMode_SinglePage) {
    _pdf_scene->showOnePage(page);
    maybeUpdateSceneRect();
  }

  viewRect = page->mapRectToScene(viewRect);
  // Note: ensureVisible seems to have a small glitch. Even if the passed
  // `viewRect` is identical, the result may depend on the view's previous state
  // if the margins are not -1. However, -1 margins don't work during the
  // initialization when the viewport doesn't have its final size yet (for
  // whatever reasons, the end result is a view centered on the scene).
  // So we use centerOn for now which should give the same result since
  // viewRect has the same size as the viewport.
//  ensureVisible(viewRect, -1, -1);
  centerOn(viewRect.center());

  _currentPage = pageNum;
  emit changedPage(_currentPage);
}

void PDFDocumentView::goToPage(const PDFPageGraphicsItem * page, const QRectF view, const bool mayZoom /* = false */)
{
  if (!page || page->page().isNull())
    return;

  // We must check if rect is valid, not view, as the latter usually has
  // negative height due to the inverted pdf coordinate system (y axis is up,
  // not down)
  QRectF rect(page->mapRectToScene(QRectF(page->mapFromPage(view.topLeft()), \
                                          page->mapFromPage(view.bottomRight()))));
  if (!rect.isValid())
    return;

  if (_pageMode == PageMode_Presentation) {
    _pdf_scene->showOnePage(page);
    maybeUpdateSceneRect();
    zoomFitWindow();
    // view is ignored in presentation mode as we always zoom to fit the window
  }
  else {
    if (_pageMode == PageMode_SinglePage) {
      _pdf_scene->showOnePage(page);
      maybeUpdateSceneRect();
    }

    if (mayZoom) {
      fitInView(rect, Qt::KeepAspectRatio);
      _zoomLevel = transform().m11();
      emit changedZoom(_zoomLevel);
    }
    else
      centerOn(rect.center());
  }

  if (_currentPage != page->pageNum()) {
    _currentPage = page->pageNum();
    emit changedPage(_currentPage);
  }
}

void PDFDocumentView::pdfActionTriggered(const PDFAction * action)
{
  if (!action)
    return;

  // Propagate link signals so that the outside world doesn't have to care about
  // our internal implementation (document/view structure, etc.)
  switch (action->type()) {
    case PDFAction::ActionTypeGoTo:
      {
        const PDFGotoAction * actionGoto = dynamic_cast<const PDFGotoAction*>(action);
        // TODO: Possibly handle other properties of destination() (e.g.,
        // viewport settings, zoom level, etc.)
        // Note: if this action requires us to open other files (possible
        // security issue) or to create a new window, we need to propagate this
        // up the hierarchy. Otherwise we can handle it ourselves here.
        if (actionGoto->isRemote() || actionGoto->openInNewWindow())
          emit requestOpenPdf(actionGoto->filename(), actionGoto->destination(), actionGoto->openInNewWindow());
        else {
          Q_ASSERT(_pdf_scene != nullptr);
          Q_ASSERT(!_pdf_scene->document().isNull());
          QSharedPointer<Backend::Document> doc(_pdf_scene->document().toStrongRef());
          if (!doc)
            break;
          PDFDestination dest = doc->resolveDestination(actionGoto->destination());
          goToPDFDestination(dest);
        }
      }
      break;
    case PDFAction::ActionTypeURI:
      {
        const PDFURIAction * actionURI = dynamic_cast<const PDFURIAction*>(action);
        emit requestOpenUrl(actionURI->url());
      }
      break;
    case PDFAction::ActionTypeLaunch:
      {
        const PDFLaunchAction * actionLaunch = dynamic_cast<const PDFLaunchAction*>(action);
        emit requestExecuteCommand(actionLaunch->command());
      }
      break;
    case PDFAction::ActionTypeSetOCGState:
      {
        const PDFOCGAction * actionOCG = dynamic_cast<const PDFOCGAction*>(action);
        QSharedPointer<Backend::Document> doc(_pdf_scene->document().toStrongRef());
        if (!actionOCG || !doc) {
          break;
        }
        QAbstractItemModel * ocgModel = doc->optionalContentModel();
        if (!ocgModel) {
          break;
        }
        for (const auto & kv : actionOCG->changes()) {
          const int & row = kv.first;
          const PDFOCGAction::OCGStateChange & type = kv.second;
          const QModelIndex idx = ocgModel->index(row, 0);
          switch (type) {
            case PDFOCGAction::OCGStateChange::NoChange:
              break;
            case PDFOCGAction::OCGStateChange::Show:
              ocgModel->setData(idx, Qt::Checked, Qt::CheckStateRole);
              break;
            case PDFOCGAction::OCGStateChange::Hide:
              ocgModel->setData(idx, Qt::Unchecked, Qt::CheckStateRole);
              break;
            case PDFOCGAction::OCGStateChange::Toggle:
              if (ocgModel->data(idx, Qt::CheckStateRole) == QVariant(Qt::Checked)) {
                ocgModel->setData(idx, Qt::Unchecked, Qt::CheckStateRole);
              }
              else {
                ocgModel->setData(idx, Qt::Checked, Qt::CheckStateRole);
              }
              break;
          }
        }
      }
      break;
    // **TODO:**
    // We don't handle other actions yet, but the ActionTypes Quit, Presentation,
    // EndPresentation, Find, GoToPage, Close, and Print should be propagated to
    // the outside world
    default:
      // All other link types are currently not supported
      break;
  }
}

void PDFDocumentView::reinitializeFromScene()
{
  if (_pdf_scene) {
    _lastPage = _pdf_scene->lastPage();
    if (_lastPage <= 0)
      _currentPage = -1;
    else {
      if (_currentPage < 0)
        _currentPage = 0;
      if (_currentPage >= _lastPage)
        _currentPage = _lastPage - 1;
    }
  }
  else {
    _lastPage = -1;
    _currentPage = -1;
  }
  // Ensure the text selection marker is reset (if any) as it holds pointers to
  // page items (highlight path, boxes) that are now changed and/or destroyed.
  DocumentTool::Select * selectTool = dynamic_cast<DocumentTool::Select*>(getToolByType(DocumentTool::AbstractTool::Tool_Select));
  if (selectTool)
    selectTool->pageDestroyed();
  // Ensure (old) search data is destroyed as well
  _searcher.ensureStopped();
  // Also reset _searchString. Otherwise the next search for the same string
  // will assume the search has already been run (without results as
  // _searchResults is empty) and won't run it again on the new scene data.
  _searcher.setSearchString(QString());
  _searchResults.clear();
  _currentSearchResult = -1;

  QSharedPointer<Backend::Document> doc{document().toStrongRef()};
  if (doc) {
    QAbstractItemModel * ocgModel = doc.data()->optionalContentModel();
    if (ocgModel) {
      connect(ocgModel, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> & roles) {
        Q_UNUSED(topLeft)
        Q_UNUSED(bottomRight)
        // Only repaint if the check state is modified (or all states are
        // modified, i.e., roles is empty)
        if (!roles.empty() && !roles.contains(Qt::CheckStateRole)) {
          return;
        }
        QSharedPointer<Backend::Document> doc{document().toStrongRef()};
        if (doc) {
          // Invalidate all document tiles
          QtPDF::Backend::Document::pageCache().removeDocumentTiles(doc.data());
          // Update the view after returning to the event loop (in case other
          // views also display this same document --- and consequently call
          // pageCache().removeDocumentTiles() --- we don't want to recreate and
          // re-remove tiles multiple times)
#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
          QTimer::singleShot(1, viewport(), SLOT(update()));
#else
          QTimer::singleShot(1, viewport(), static_cast<void (QWidget::*)()>(&QWidget::update));
#endif
        }
      });
    }
  }
}

void PDFDocumentView::notifyTextSelectionChanged()
{
  DocumentTool::Select * tool = dynamic_cast<DocumentTool::Select *>(getToolByType(DocumentTool::AbstractTool::Tool_Select));
  if (!tool) return;
  emit textSelectionChanged(tool->isTextSelected());
}

void PDFDocumentView::registerTool(std::unique_ptr<DocumentTool::AbstractTool> tool)
{
  if (!tool)
    return;

  // Remove any identical tools
  for (auto it = _tools.begin(); it != _tools.end(); ) {
    const std::unique_ptr<DocumentTool::AbstractTool> & t = *it;
    if (t && *t == *tool) {
      it = _tools.erase(it);
    }
    else {
      ++it;
    }
  }
  // Add the new tool
  _tools.push_back(std::move(tool));
}

DocumentTool::AbstractTool* PDFDocumentView::getToolByType(const DocumentTool::AbstractTool::Type type)
{
  for(const std::unique_ptr<DocumentTool::AbstractTool> & tool : _tools) {
    if (tool && tool->type() == type)
      return tool.get();
  }
  return nullptr;
}



// Event Handlers
// --------------

// Keep track of the current page by overloading the widget paint event.
void PDFDocumentView::paintEvent(QPaintEvent *event)
{
  Super::paintEvent(event);

  // After `QGraphicsView` has taken care of updates to this widget, find the
  // currently displayed page. We do this by grabbing all items that are
  // currently within the bounds of the viewport's top half. We take the
  // first item found to be the "current page".
  if (_pdf_scene) {
    QRect pageBbox = viewport()->rect();
    pageBbox.setHeight(pageBbox.height() / 2);
    size_type nextCurrentPage = _pdf_scene->pageNumAt(mapToScene(pageBbox));

    if ( nextCurrentPage != _currentPage && nextCurrentPage >= 0 && nextCurrentPage < _lastPage )
    {
      _currentPage = nextCurrentPage;
      emit changedPage(_currentPage);
    }
  }

  if (_armedTool)
    _armedTool->paintEvent(event);

  emit updated();
}

void PDFDocumentView::keyPressEvent(QKeyEvent *event)
{
  // FIXME: No moving while tools are active?
  switch ( event->key() )
  {
    case Qt::Key_Home:
      goFirst();
      event->accept();
      break;

    case Qt::Key_End:
      goLast();
      event->accept();
      break;

    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Left:
    case Qt::Key_Right:
      // Check to see if we need to jump to the next page in single page mode.
      if ( pageMode() == PageMode_SinglePage || pageMode() == PageMode_Presentation ) {
        int scrollStep = (event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown) ? verticalScrollBar()->pageStep() : verticalScrollBar()->singleStep();
        int scrollPos = verticalScrollBar()->value();

        // Take no action on the first and last page so that PageUp/Down can
        // move the view right up to the page boundary.
        if (
          (event->key() == Qt::Key_PageUp || event->key() == Qt::Key_Up) &&
          (scrollPos - scrollStep) <= verticalScrollBar()->minimum() &&
          _currentPage > 0
        ) {
          goPrev();
          event->accept();
          break;
        }
        if (
          (event->key() == Qt::Key_PageDown || event->key() == Qt::Key_Down) &&
          (scrollPos + scrollStep) >= verticalScrollBar()->maximum() &&
          _currentPage < _lastPage
        ) {
          goNext();
          event->accept();
          break;
        }
      }

      // Deliberate fall-through; we only override the movement keys if a tool is
      // currently in use or the view is in single page mode and the movement
      // would cross a page boundary.
      // falls through
    default:
      Super::keyPressEvent(event);
      break;
  }
  // If we have an armed tool, pass the event on to it
  // Note: by default, PDFDocumentTool::keyPressEvent() calls maybeArmTool() if
  // it doesn't handle the event
  if (_armedTool)
    _armedTool->keyPressEvent(event);
  // If there is no currently armed tool, maybe we can arm one now
  else
    maybeArmTool(Qt::MouseButtons(Qt::LeftButton) | event->modifiers());
}

void PDFDocumentView::keyReleaseEvent(QKeyEvent *event)
{
  // If we have an armed tool, pass the event on to it
  // Note: by default, PDFDocumentTool::keyReleaseEvent() calls maybeArmTool() if
  // it doesn't handle the event
  if(_armedTool)
    _armedTool->keyReleaseEvent(event);
  else
    maybeArmTool(Qt::MouseButtons(Qt::LeftButton) | event->modifiers());
}

void PDFDocumentView::mousePressEvent(QMouseEvent * event)
{
  Super::mousePressEvent(event);

  // Don't do anything if the event was handled elsewhere (e.g., by a
  // PDFLinkGraphicsItem)
  if (event->isAccepted())
    return;

  // Maybe arm a new tool, depending on the mouse buttons and keyboard modifiers.
  // This is particularly relevant if the current widget is not the focussed
  // widget. In that case, mousePressEvent() will focus this widget, but any
  // keyboard modifiers may require a new tool to fire.
  // In the typical case, the correct tool will already be armed, so the call to
  // maybeArmTool will effectively be a no-op.
  maybeArmTool(event->buttons() | event->modifiers());

  DocumentTool::AbstractTool * oldArmed = _armedTool;

  if(_armedTool)
    _armedTool->mousePressEvent(event);

  // This mouse event may have armed a new tool (either explicitly, or because
  // the previously armed tool passed it on to maybeArmTool). In that case, we
  // need to pass it on to the newly armed tool
  if (_armedTool && _armedTool != oldArmed)
    _armedTool->mousePressEvent(event);
}

void PDFDocumentView::mouseMoveEvent(QMouseEvent * event)
{
  if(_armedTool)
    _armedTool->mouseMoveEvent(event);
  // We only use the event for information purposes and don't actually "handle"
  // it, therefore we ignore it so it can also be passed on to parent widgets
  event->ignore();
  Super::mouseMoveEvent(event);
}

void PDFDocumentView::mouseReleaseEvent(QMouseEvent * event)
{
  Super::mouseReleaseEvent(event);

  if(_armedTool)
    _armedTool->mouseReleaseEvent(event);
  else
    maybeArmTool(event->buttons() | event->modifiers());
}

void PDFDocumentView::wheelEvent(QWheelEvent * event)
{
  int deltaY = event->angleDelta().y();

  if (deltaY != 0 && event->buttons() == Qt::NoButton && event->modifiers() == Qt::ControlModifier) {
    // TODO: Possibly make the Ctrl modifier configurable?
    // According to Qt docs, the resolution of delta() is not necessarily the
    // same for all mice. delta() returns the rotation in 1/8 degrees. Here, we
    // use a zoom factor of 1.5 every 15 degrees (= delta() == 120, which seems
    // to be a widespread default resolution).
    // TODO: for high-resolution mice, this may trigger many small zooms,
    // resulting in the rendering of many intermediate resolutions. This can
    // cause a lagging display and can potentially fill the pdf cache.
    zoomBy(pow(1.5, deltaY / 120.), QGraphicsView::AnchorUnderMouse);
    event->accept();
    return;
  }
  if (event->modifiers() == Qt::ShiftModifier) {
    // If "Shift" (and only that modifier) is pressed, swap orientations
    // (e.g., to allow horizontal scrolling)
    // TODO: Possibly make the Shift modifier configurable?
    event->accept();
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QWheelEvent newEvent(event->pos(), event->delta(), event->buttons(), Qt::NoModifier, (event->orientation() == Qt::Vertical ? Qt::Horizontal : Qt::Vertical));
#else
    QWheelEvent newEvent(event->position(), event->globalPosition(),
                         QPoint{event->pixelDelta().y(), event->pixelDelta().x()},
                         QPoint{event->angleDelta().y(), event->angleDelta().x()},
                         event->buttons(), Qt::NoModifier, event->phase(),
                         event->inverted(), event->source());
#endif
    wheelEvent(&newEvent);
    return;
  }
  if (deltaY != 0 && (pageMode() == PageMode_SinglePage || pageMode() == PageMode_Presentation)) {
    // In single page mode we need to flip to the next page if the scroll bar
    // is a the top or bottom of it's range.`
    int scrollPos = verticalScrollBar()->value();
    if (deltaY < 0 && scrollPos == verticalScrollBar()->maximum()) {
      goNext();

      event->accept();
      return;
    }
    if (deltaY > 0 && scrollPos == verticalScrollBar()->minimum()) {
      goPrev();

      event->accept();
      return;
    }
  }

  Super::wheelEvent(event);
}

void PDFDocumentView::changeEvent(QEvent * event)
{
  if (event && event->type() == QEvent::LanguageChange) {
    if (_pdf_scene)
      _pdf_scene->retranslateUi();
  }
  Super::changeEvent(event);
}

void PDFDocumentView::resizeEvent(QResizeEvent * event)
{
  _ruler.resize(size());
  Super::resizeEvent(event);
}

void PDFDocumentView::armTool(const DocumentTool::AbstractTool::Type toolType)
{
  armTool(getToolByType(toolType));
}

void PDFDocumentView::armTool(DocumentTool::AbstractTool * tool)
{
  if (_armedTool == tool)
    return;
  if (_armedTool)
    disarmTool();
  if (tool)
    tool->arm();
  _armedTool = tool;
}

void PDFDocumentView::disarmTool()
{
  if (!_armedTool)
    return;
  _armedTool->disarm();
  _armedTool = nullptr;
}

void PDFDocumentView::showRuler(const bool show)
{
  if (show) {
    _ruler.show();
    setViewportMargins(PDFRuler::rulerSize, PDFRuler::rulerSize, 0, 0);
  }
  else {
    _ruler.hide();
    setViewportMargins(0, 0, 0, 0);
  }
}


// PDFDocumentMagnifierView
// ========================
//
PDFDocumentMagnifierView::PDFDocumentMagnifierView(PDFDocumentView *parent /* = nullptr */) :
  Super(parent),
  _parent_view(parent)
{
  // the magnifier should initially be hidden
  hide();

  // suppress scrollbars
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  // suppress any border styling (which doesn't work with a mask, e.g., for a
  // circular magnifier)
  setFrameShape(QFrame::NoFrame);

  if (parent) {
    // transfer some settings from the parent view
    setBackgroundRole(parent->backgroundRole());
    setAlignment(parent->alignment());
  }

  setShape(_shape);
}

void PDFDocumentMagnifierView::prepareToShow()
{
  if (!_parent_view)
    return;

  // Ensure we have the same scene
  if (_parent_view->scene() != scene())
    setScene(_parent_view->scene());
  // Fix the zoom
  qreal zoomLevel = _parent_view->zoomLevel() * _zoomFactor;
  if (zoomLevel != _zoomLevel)
    scale(zoomLevel / _zoomLevel, zoomLevel / _zoomLevel);
  _zoomLevel = zoomLevel;
  // Ensure we have enough padding at the border that we can display the
  // magnifier even beyond the edge
  setSceneRect(_parent_view->sceneRect().adjusted(-width() / _zoomLevel, -height() / _zoomLevel, width() / _zoomLevel, height() / _zoomLevel));
  raise();
}

void PDFDocumentMagnifierView::setZoomFactor(const qreal zoomFactor)
{
  _zoomFactor = zoomFactor;
  // Actual handling of zoom levels happens in prepareToShow, as the zoom level
  // of the parent cannot change while the magnifier is shown
}

void PDFDocumentMagnifierView::setPosition(const QPoint pos)
{
  move(pos.x() - width() / 2, pos.y() - height() / 2);
  centerOn(_parent_view->mapToScene(pos));
}

void PDFDocumentMagnifierView::setSizeAndShape(const int size, const DocumentTool::MagnifyingGlass::MagnifierShape shape)
{
  _size = size;
  _shape = shape;

  switch (shape) {
    case DocumentTool::MagnifyingGlass::Magnifier_Rectangle:
      setFixedSize(size * 4 / 3, size);
      clearMask();
      // There is a bug that affects masking of QAbstractScrollArea and its
      // subclasses:
      //
      //   https://bugreports.qt.nokia.com/browse/QTBUG-7150
      //
      // The workaround is to explicitly mask the viewport. As of Qt 4.7.4, this
      // bug is still present. As of Qt 5, it also seems to affect other
      // platforms
      viewport()->clearMask();
      break;
    case DocumentTool::MagnifyingGlass::Magnifier_Circle:
      setFixedSize(size, size);
      setMask(QRegion(rect(), QRegion::Ellipse));
      // Hack to fix QTBUG-7150
      viewport()->setMask(QRegion(rect(), QRegion::Ellipse));
      break;
  }
  _dropShadow = QPixmap();
}

void PDFDocumentMagnifierView::paintEvent(QPaintEvent * event)
{
  Super::paintEvent(event);

  // Draw our custom border
  // Note that QGraphicsView is derived from QAbstractScrollArea, but we are not
  // asked to paint on that but on the widget it contains. Therefore, we can't
  // just say QPainter(this)
  QPainter painter(viewport());

  painter.setRenderHint(QPainter::Antialiasing);

  QPen pen(Qt::gray);
  pen.setWidth(2);

  QRect rect(this->rect());

  painter.setPen(pen);
  switch(_shape) {
    case DocumentTool::MagnifyingGlass::Magnifier_Rectangle:
      painter.drawRect(rect);
      break;
    case DocumentTool::MagnifyingGlass::Magnifier_Circle:
      // Ensure we're drawing where we should, regardless how the window system
      // handles masks
      painter.setClipRegion(mask());
      // **TODO:** It seems to be necessary to adjust the window rect by one pixel
      // to draw an evenly wide border; is there a better way?
      rect.adjust(1, 1, 0, 0);
      painter.drawEllipse(rect);
      break;
  }

  // **Note:** We don't/can't draw the drop-shadow here. The reason is that we
  // rely on Super::paintEvent to do the actual rendering, which constructs its
  // own QPainter so we can't do clipping. Resetting the mask is no option,
  // either, as that may trigger an update (recursion!).
  // Alternatively, we could fill the border with the background from the
  // underlying window. But _parent_view->render() no option, because it
  // requires QWidget::DrawChildren (apparently the QGraphicsItems are
  // implemented as child widgets) which would cause a recursion again (the
  // magnifier is also a child widget!). Calling scene()->render() is no option,
  // either, because then render requests for unmagnified images would originate
  // from here, which would break the current implementation of
  // PDFPageGraphicsItem::paint().
  // Instead, drop-shadows are drawn in PDFDocumentView::paintEvent(), invoking
  // PDFDocumentMagnifierView::dropShadow().
}

// Modelled after http://labs.qt.nokia.com/2009/10/07/magnifying-glass
QPixmap& PDFDocumentMagnifierView::dropShadow()
{
  if (!_dropShadow.isNull())
    return _dropShadow;

  int padding = 10;
  _dropShadow = QPixmap(width() + 2 * padding, height() + 2 * padding);

  _dropShadow.fill(Qt::transparent);

  switch(_shape) {
    case DocumentTool::MagnifyingGlass::Magnifier_Rectangle:
      {
        QPainterPath path;
        QRectF boundingRect(_dropShadow.rect().adjusted(0, 0, -1, -1));
        QLinearGradient gradient(boundingRect.center(), QPointF(0.0, boundingRect.center().y()));
        gradient.setSpread(QGradient::ReflectSpread);
        QGradientStops stops;
        QColor color(Qt::black);
        color.setAlpha(64);
        stops.append(QGradientStop(1.0 - padding * 2.0 / _dropShadow.width(), color));
        color.setAlpha(0);
        stops.append(QGradientStop(1.0, color));

        QPainter shadow(&_dropShadow);
        shadow.setRenderHint(QPainter::Antialiasing);

        // paint horizontal gradient
        gradient.setStops(stops);

        path = QPainterPath();
        path.moveTo(boundingRect.topLeft());
        path.lineTo(boundingRect.topLeft() + QPointF(padding, padding));
        path.lineTo(boundingRect.bottomRight() + QPointF(-padding, -padding));
        path.lineTo(boundingRect.bottomRight());
        path.lineTo(boundingRect.topRight());
        path.lineTo(boundingRect.topRight() + QPointF(-padding, padding));
        path.lineTo(boundingRect.bottomLeft() + QPointF(padding, -padding));
        path.lineTo(boundingRect.bottomLeft());
        path.closeSubpath();

        shadow.fillPath(path, gradient);

        // paint vertical gradient
        stops[0].first = 1.0 - padding * 2.0 / _dropShadow.height();
        gradient.setStops(stops);

        path = QPainterPath();
        path.moveTo(boundingRect.topLeft());
        path.lineTo(boundingRect.topLeft() + QPointF(padding, padding));
        path.lineTo(boundingRect.bottomRight() + QPointF(-padding, -padding));
        path.lineTo(boundingRect.bottomRight());
        path.lineTo(boundingRect.bottomLeft());
        path.lineTo(boundingRect.bottomLeft() + QPointF(padding, -padding));
        path.lineTo(boundingRect.topRight() + QPointF(-padding, padding));
        path.lineTo(boundingRect.topRight());
        path.closeSubpath();

        gradient.setFinalStop(QPointF(QRectF(_dropShadow.rect()).center().x(), 0.0));
        shadow.fillPath(path, gradient);
      }
      break;
    case DocumentTool::MagnifyingGlass::Magnifier_Circle:
      {
        QRadialGradient gradient(QRectF(_dropShadow.rect()).center(), _dropShadow.width() / 2.0, QRectF(_dropShadow.rect()).center());
        QColor color(Qt::black);
        color.setAlpha(0);
        gradient.setColorAt(1.0, color);
        color.setAlpha(64);
        gradient.setColorAt(1.0 - padding * 2.0 / _dropShadow.width(), color);

        QPainter shadow(&_dropShadow);
        shadow.setRenderHint(QPainter::Antialiasing);
        shadow.fillRect(_dropShadow.rect(), gradient);
      }
      break;
  }
  return _dropShadow;
}





// PDFPageGraphicsItem
// ===================

// This class descends from `QGraphicsObject` and implements the on-screen
// representation of `Page` objects.
PDFPageGraphicsItem::PDFPageGraphicsItem(QWeakPointer<Backend::Page> a_page, const double dpiX, const double dpiY, QGraphicsItem *parent /* = nullptr */):
  Super(parent),
  _page(a_page),
  // FIXME: The QGraphicsObject should be independent of the hardware it is
  // shown on
  _dpiX(dpiX),
  _dpiY(dpiY),
  _pageNum(-1),
  _linksLoaded(false),
  _annotationsLoaded(false),
  _zoomLevel(0.0)
{
  // So we get information during paint events about what portion of the page
  // is visible.
  //
  // NOTE: This flag needs Qt 4.6 or newer.
  setFlags(QGraphicsItem::ItemUsesExtendedStyleOption);

  QSharedPointer<Backend::Page> page(_page.toStrongRef());
  if (page) {
    _pageNum = page->pageNum();
    // Create an empty pixmap that is the same size as the PDF page. This
    // allows us to delay the rendering of pages until they actually come into
    // view yet still know what the page size is.
    _pageSize = page->pageSizeF();
    _pageSize.setWidth(_pageSize.width() * _dpiX / 72.0);
    _pageSize.setHeight(_pageSize.height() * _dpiY / 72.0);

    // `_pageScale` holds a transformation matrix that can map between normalized
    // page coordinates (in the range 0...1) and the coordinate system for this
    // graphics item. `_pointScale` is similar, except it maps from coordinates
    // expressed in pixels at a resolution of 72 dpi.
    _pageScale = QTransform::fromScale(_pageSize.width(), _pageSize.height());
    _pointScale = QTransform::fromScale(_dpiX / 72.0, _dpiY / 72.0);
  }
}

QRectF PDFPageGraphicsItem::boundingRect() const { return QRectF(QPointF(0.0, 0.0), _pageSize); }
int PDFPageGraphicsItem::type() const { return Type; }

QPointF PDFPageGraphicsItem::mapFromPage(const QPointF & point) const
{
  QSharedPointer<Backend::Page> page(_page.toStrongRef());
  if (!page)
    return QPointF();
  // item coordinates are in pixels
  return QPointF(_pageSize.width() * point.x() / page->pageSizeF().width(), \
    _pageSize.height() * (1.0 - point.y() / page->pageSizeF().height()));
}

QPointF PDFPageGraphicsItem::mapToPage(const QPointF & point) const
{
  QSharedPointer<Backend::Page> page(_page.toStrongRef());
  if (!page)
    return QPointF();
  // item coordinates are in pixels
  return QPointF(page->pageSizeF().width() * point.x() / _pageSize.width(), \
    page->pageSizeF().height() * (1.0 - point.y() / _pageSize.height()));
}

// An overloaded paint method allows us to handle rendering via asynchronous
// calls to backend functions.
void PDFPageGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  // Really, there is an X scaling factor and a Y scaling factor, but we assume
  // that the X scaling factor is equal to the Y scaling factor.
  qreal scaleFactor = painter->transform().m11();
  QTransform scaleT = QTransform::fromScale(scaleFactor, scaleFactor);
  QRect pageRect = scaleT.mapRect(boundingRect()).toAlignedRect();
  QSharedPointer<Backend::Page> page(_page.toStrongRef());
  QSharedPointer<QImage> renderedPage;

  if (!page)
    return;

  // If this is the first time this `PDFPageGraphicsItem` has come into view,
  // `_linksLoaded` will be `false`. We then load all of the links on the page.
  if (!_linksLoaded)
  {
    page->asyncLoadLinks(this);
    _linksLoaded = true;
  }

  if (!_annotationsLoaded) {
    // FIXME: Load annotations asynchronously?
    addAnnotations(page->loadAnnotations());
    _annotationsLoaded = true;
  }

  if ( _zoomLevel != scaleFactor )
    _zoomLevel = scaleFactor;

  // get a pointer to the parent view (if any)
  PDFDocumentView * view = (widget ? qobject_cast<PDFDocumentView*>(widget->parent()) : nullptr);

  painter->save();

  if (view && view->pageMode() == PDFDocumentView::PageMode_Presentation) {
    // NOTE: There is no point in clipping here as we always display the whole
    // page, anyway. Hence, the images all have the correct size (no tiling) and
    // are usually completely visible.

    // The transformation matrix of the `painter` object contains information
    // such as the current zoom level of the widget viewing this PDF page. We
    // throw away the scaling information because that has already been
    // applied during page rendering. (Note: we don't support rotation/skewing,
    // so we only care about the translational part)
    QTransform pageT = painter->transform();
    painter->setTransform(QTransform::fromTranslate(pageT.dx(), pageT.dy()));
    if (page->transition() && page->transition()->isRunning()) {
      // Get and draw the current frame of the transition
      // NOTE: In the (unlikely) case that the two pages we are transitioning
      // between are not the same size, the frame image will be padded to
      // encompass both pages. In that case, we (may) need to paint the image
      // outside the boundaries of this graphics item (unfortunately, that can't
      // be helped, but it should not cause too many problems as there should be
      // no other items visible below it).
      // NOTE: Don't use QRect::center() here to align the respective centers as
      // round-off errors can introduce a shift of +-1px.
      QImage img(page->transition()->getImage());
      QPoint offset((pageRect.width() - img.width()) / 2, (pageRect.height() - img.height()) / 2);
      painter->drawImage(offset, img);
      // Trigger an update as soon as possible (without recursion) to proceed
      // with the animation.
      if (widget) {
#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
        QTimer::singleShot(1, widget, SLOT(update()));
#else
        QTimer::singleShot(1, widget, static_cast<void (QWidget::*)()>(&QWidget::update));
#endif
      }
    }
    else {
      // render the whole page synchronously (we don't want "rendering" to show
      // up during presentations, and we don't need tiles as we always display
      // the full page, anyway).
      renderedPage = page->getTileImage(nullptr, _dpiX * scaleFactor, _dpiY * scaleFactor);
      if (renderedPage)
        painter->drawImage(QPoint(0, 0), *renderedPage);
    }
  }
  else { // presentation mode
    // Clip to the exposed rectangle to prevent unnecessary drawing operations.
    // This can provide up to a 50% speedup depending on the size of the tile.
    painter->setClipRect(option->exposedRect);

    // The transformation matrix of the `painter` object contains information
    // such as the current zoom level of the widget viewing this PDF page. We
    // throw away the scaling information because that has already been
    // applied during page rendering. (Note: we don't support rotation/skewing,
    // so we only care about the translational part)
    QTransform pageT = painter->transform();
    painter->setTransform(QTransform::fromTranslate(pageT.dx(), pageT.dy()));
#ifdef DEBUG
    // Pen style used to draw the outline of each tile for debugging purposes.
    QPen tilePen(Qt::darkGray);
    tilePen.setStyle(Qt::DashDotLine);
    painter->setPen(tilePen);
#endif

    QRect visibleRect = scaleT.mapRect(option->exposedRect).toAlignedRect();

    // Each tile is rendered at TILE_SIZE pixels, which may be scaled (e.g. on
    // high-dpi screens) and displayed at an effective size
    int effectiveTileSize = static_cast<int>(TILE_SIZE / painter->device()->devicePixelRatio());

    int imin = (visibleRect.left() - pageRect.left()) / effectiveTileSize;
    int imax = (visibleRect.right() - pageRect.left());
    if (imax % effectiveTileSize == 0)
      imax /= effectiveTileSize;
    else
      imax = imax / effectiveTileSize + 1;

    int jmin = (visibleRect.top() - pageRect.top()) / effectiveTileSize;
    int jmax = (visibleRect.bottom() - pageRect.top());
    if (jmax % effectiveTileSize == 0)
      jmax /= effectiveTileSize;
    else
      jmax = jmax / effectiveTileSize + 1;

    for (int j = jmin; j < jmax; ++j) {
      for (int i = imin; i < imax; ++i) {
        // renderTile is the rect used for rendering/retrieving tiles. It is
        // agnostic of the painter (e.g., its devicePixelRatio)
        QRect renderTile(i * TILE_SIZE, j * TILE_SIZE, TILE_SIZE, TILE_SIZE);
        // displayTile is the rect used for displaying. It takes the painter's
        // settings into account (e.g. its devicePixelRatio)
        QRect displayTile(i * effectiveTileSize, j * effectiveTileSize, effectiveTileSize, effectiveTileSize);

        bool useGrayScale = false;
        // If we are rendering a PDFDocumentView that has `useGrayScale` set
        // respect that setting.
        if (view && view->useGrayScale())
          useGrayScale = true;
        // If we are rendering a PDFDocumentMagnifierView who's parent
        // PDFDocumentView has `useGrayScale` set respect that setting.
        else if (widget && widget->parent() && widget->parent()->parent()) {
          PDFDocumentView * parentView = (widget ? qobject_cast<PDFDocumentView*>(widget->parent()->parent()) : nullptr);
          if (parentView && parentView->useGrayScale())
            useGrayScale = true;
        }

        renderedPage = page->getTileImage(this, _dpiX * scaleFactor * painter->device()->devicePixelRatio(), _dpiY * scaleFactor * painter->device()->devicePixelRatio(), renderTile);
        // renderedPage as returned from getTileImage _should_ always be valid
        if ( renderedPage ) {
          if (useGrayScale) {
            // In gray scale mode, we need to obtain a deep copy of the rendered
            // page image to avoid altering the cached (color) image
            QImage postProcessed = renderedPage->copy();
            imageToGrayScale(postProcessed);
            postProcessed.setDevicePixelRatio(painter->device()->devicePixelRatio());
            painter->drawImage(displayTile.topLeft(), postProcessed);
          }
          else {
            QImage img = *renderedPage;
            img.setDevicePixelRatio(painter->device()->devicePixelRatio());
            painter->drawImage(displayTile.topLeft(), img);
          }
        }
#ifdef DEBUG
        painter->drawRect(displayTile);
#endif
      }
    }
  }
  painter->restore();
}

//static
void PDFPageGraphicsItem::imageToGrayScale(QImage & img)
{
  // Casting to QRgb* only works for 32bit images
  Q_ASSERT(img.depth() == 32);
  QRgb * data = reinterpret_cast<QRgb*>(img.scanLine(0));
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
  for (int i = 0; i < img.byteCount() / 4; ++i) {
#else
  for (qsizetype i = 0; i < img.sizeInBytes() / 4; ++i) {
#endif
    // Qt formula (qGray()): 0.34375 * r + 0.5 * g + 0.15625 * b
    // MuPDF formula (rgb_to_gray()): r * 0.3f + g * 0.59f + b * 0.11f;
    int gray = qGray(data[i]);
    data[i] = qRgba(gray, gray, gray, qAlpha(data[i]));
  }
}

// Event Handlers
// --------------
bool PDFPageGraphicsItem::event(QEvent *event)
{
  // Look for callbacks from asynchronous page operations.
  if( event->type() == Backend::PDFLinksLoadedEvent::LinksLoadedEvent ) {
    event->accept();

    // Cast to a `PDFLinksLoaded` event so we can access the links.
    const Backend::PDFLinksLoadedEvent *links_loaded_event = dynamic_cast<const Backend::PDFLinksLoadedEvent*>(event);
    addLinks(links_loaded_event->links);

    return true;

  }
  if( event->type() == Backend::PDFPageRenderedEvent::PageRenderedEvent ) {
    event->accept();

    // FIXME: We're sort of misusing the render event here---it contains a copy
    // of the image data that we never touch. The assumption is that the page
    // cache now has new data, so we call `update` to trigger a repaint which
    // fetches stuff from the cache.
    //
    // Perhaps there should be a separate event for when the cache is updated.
    update();

    return true;
  }

  // Otherwise, pass event to default handler.
  return Super::event(event);
}

// This method causes the `PDFPageGraphicsItem` to create `PDFLinkGraphicsItem`
// objects for a list of asynchronously generated `PDFLinkAnnotation` objects.
// The page item also takes ownership the objects created.  Calling
// `setParentItem` causes the link objects to be added to the scene that owns
// the page object. `update` is then called to ensure all links are drawn at
// once.
void PDFPageGraphicsItem::addLinks(QList< QSharedPointer<Annotation::Link> > links)
{
#ifdef DEBUG
  QElapsedTimer stopwatch;
  stopwatch.start();
#endif
  foreach( QSharedPointer<Annotation::Link> link, links ){
    PDFLinkGraphicsItem * linkItem = new PDFLinkGraphicsItem(link);
    // Map the link from pdf coordinates to scene coordinates
    linkItem->setTransform(QTransform::fromTranslate(0, _pageSize.height()).scale(_dpiX / 72., -_dpiY / 72.));
    linkItem->setParentItem(this);
  }
#ifdef DEBUG
  qDebug() << "Added links in: " << stopwatch.elapsed() << " milliseconds";
#endif

  update();
}

void PDFPageGraphicsItem::addAnnotations(QList< QSharedPointer<Annotation::AbstractAnnotation> > annotations)
{
#ifdef DEBUG
  QElapsedTimer stopwatch;
  stopwatch.start();
#endif
  foreach( QSharedPointer<Annotation::AbstractAnnotation> annot, annotations ){
    // We currently only handle popups
    if (!annot->isMarkup())
      continue;
    QSharedPointer<Annotation::Markup> markupAnnot = annot.staticCast<Annotation::Markup>();
    PDFMarkupAnnotationGraphicsItem * markupAnnotItem = new PDFMarkupAnnotationGraphicsItem(markupAnnot);
    // Map the link from pdf coordinates to scene coordinates
    markupAnnotItem->setTransform(QTransform::fromTranslate(0, _pageSize.height()).scale(_dpiX / 72., -_dpiY / 72.));
    markupAnnotItem->setParentItem(this);
  }
#ifdef DEBUG
  qDebug() << "Added annotations in: " << stopwatch.elapsed() << " milliseconds";
#endif

  update();
}


// PDFLinkGraphicsItem
// ===================

// This class descends from `QGraphicsRectItem` and serves the following
// functions:
//
//    * Provides easy access to the on-screen geometry of a hyperlink area.
//
//    * Handles tasks such as cursor changes on mouse hover and link activation
//      on mouse clicks.
PDFLinkGraphicsItem::PDFLinkGraphicsItem(QSharedPointer<Annotation::Link> a_link, QGraphicsItem *parent /* = nullptr */):
  Super(parent),
  _link(a_link),
  _activated(false)
{
  // The link area is expressed in "normalized page coordinates", i.e.  values
  // in the range [0, 1]. The transformation matrix of this item will have to
  // be adjusted so that links will show up correctly in a graphics view.
  setRect(_link->rect());

  // Allows links to provide a context-specific cursor when the mouse is
  // hovering over them.
  //
  // **NOTE:** _Requires Qt 4.4 or newer._
  setAcceptHoverEvents(true);

  // Only left-clicks will trigger the link.
  setAcceptedMouseButtons(Qt::LeftButton);

#ifdef DEBUG
  // **TODO:**
  // _Currently for debugging purposes only so that the link area can be
  // determined visually, but might make a nice option._
  setPen(QPen(Qt::red));
#else
  // Perhaps there is a way to not draw the outline at all? Might be more
  // efficient...
  setPen(QPen(Qt::transparent));
#endif

  retranslateUi();
}

int PDFLinkGraphicsItem::type() const { return Type; }

void PDFLinkGraphicsItem::retranslateUi()
{
  PDFAction * action = _link->actionOnActivation();
  if (action) {
    // Set some meaningful tooltip to inform the user what the link does
    // Using <p>...</p> ensures the tooltip text is interpreted as rich text
    // and thus is wrapping sensibly to avoid over-long lines.
    // Using PDFDocumentView::tr avoids having to explicitly derive
    // PDFLinkGraphicsItem explicily from QObject and puts all translatable
    // strings into the same context.
    switch(action->type()) {
      case PDFAction::ActionTypeGoTo:
        {
          PDFGotoAction * actionGoto = dynamic_cast<PDFGotoAction*>(action);
          if (actionGoto->isRemote())
            setToolTip(QString::fromUtf8("<p>%1</p>").arg(actionGoto->filename()));
            // FIXME: Possibly include page as well after the filename
          else
            setToolTip(QString::fromUtf8("<p>") + PDFDocumentView::tr("Goto page %1").arg(actionGoto->destination().page() + 1) + QString::fromUtf8("</p>"));
        }
        break;
      case PDFAction::ActionTypeURI:
        {
          PDFURIAction * actionURI = dynamic_cast<PDFURIAction*>(action);
          setToolTip(QString::fromUtf8("<p>%1</p>").arg(actionURI->url().toString()));
        }
        break;
      case PDFAction::ActionTypeLaunch:
        {
          PDFLaunchAction * actionLaunch = dynamic_cast<PDFLaunchAction*>(action);
          setToolTip(QString::fromUtf8("<p>") + PDFDocumentView::tr("Execute `%1`").arg(actionLaunch->command()) + QString::fromUtf8("</p>"));
        }
        break;
      default:
        // All other link types are currently not supported
        break;
    }
  }
}

// Event Handlers
// --------------

// Swap cursor during hover events.
void PDFLinkGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  Q_UNUSED(event)
  setCursor(Qt::PointingHandCursor);
}

void PDFLinkGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  Q_UNUSED(event)
  unsetCursor();
}

// Respond to clicks. Limited to left-clicks by `setAcceptedMouseButtons` in
// this object's constructor.
void PDFLinkGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  // Actually opening the link is handled during a `mouseReleaseEvent` --- but
  // only if the `_activated` flag is `true`.
  // Only set _activated if no keyboard modifiers are currently pressed (which
  // most likely indicates some tool or other is active)
  if (event->modifiers() == Qt::NoModifier)
    _activated = true;
  else {
    _activated = false;
    Super::mousePressEvent(event);
  }
}

// The real nitty-gritty of link activation happens in here.
void PDFLinkGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  // Check that this link was "activated" (mouse press occurred within the link
  // bounding box) and that the mouse release also occurred within the bounding
  // box.
  if (!_activated || !contains(event->pos()))
  {
    _activated = false;
    Super::mouseReleaseEvent(event);
    return;
  }

  // Post an event to the parent scene. The scene then takes care of processing
  // it further, notifying objects, such as `PDFDocumentView`, that may want to
  // take action via a `SIGNAL`.
  // **TODO:** Wouldn't a direct call be more efficient?
  if (_link && _link->actionOnActivation())
    QCoreApplication::postEvent(scene(), new PDFActionEvent(_link->actionOnActivation()));
  _activated = false;
}


// PDFMarkupAnnotationGraphicsItem
// ===============================

// This class descends from `QGraphicsRectItem` and serves the following
// functions:
//
//    * Provides easy access to the on-screen geometry of a markup annotation.
//
//    * Handles tasks such as cursor changes on mouse hover and link activation
//      on mouse clicks.
//
//    * Displays note popups if necessary
PDFMarkupAnnotationGraphicsItem::PDFMarkupAnnotationGraphicsItem(QSharedPointer<Annotation::Markup> annot, QGraphicsItem *parent /* = nullptr */):
  Super(parent),
  _annot(annot),
  _activated(false),
  _popup(nullptr)
{
  // The area is expressed in "normalized page coordinates", i.e.  values
  // in the range [0, 1]. The transformation matrix of this item will have to
  // be adjusted so that links will show up correctly in a graphics view.
  setRect(_annot->rect());

  // Allows annotations to provide a context-specific cursor when the mouse is
  // hovering over them.
  //
  // **NOTE:** _Requires Qt 4.4 or newer._
  setAcceptHoverEvents(true);

  // Only left-clicks will trigger the popup (if any).
  setAcceptedMouseButtons(annot->popup() ? Qt::LeftButton : Qt::NoButton);

#ifdef DEBUG
  // **TODO:**
  // _Currently for debugging purposes only so that the annotation area can be
  // determined visually, but might make a nice option._
  setPen(QPen(Qt::blue));
#else
  // Perhaps there is a way to not draw the outline at all? Might be more
  // efficient...
  setPen(QPen(Qt::transparent));
#endif

  QString tooltip(annot->richContents());
  // If the text is not already split into paragraphs, we do that here to ensure
  // proper line folding in the tooltip and hence to avoid very wide tooltips.
  if (tooltip.indexOf(QString::fromLatin1("<p>")) < 0)
    tooltip = QString::fromLatin1("<p>%1</p>").arg(tooltip.replace(QChar::fromLatin1('\n'), QString::fromLatin1("</p>\n<p>")));
  setToolTip(tooltip);
}

int PDFMarkupAnnotationGraphicsItem::type() const { return Type; }

// Event Handlers
// --------------

// Swap cursor during hover events.
void PDFMarkupAnnotationGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  Q_UNUSED(event)
  if (_annot->popup())
    setCursor(Qt::PointingHandCursor);
}

void PDFMarkupAnnotationGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  Q_UNUSED(event)
  if (_annot->popup())
    unsetCursor();
}

// Respond to clicks. Limited to left-clicks by `setAcceptedMouseButtons` in
// this object's constructor.
void PDFMarkupAnnotationGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  // Actually opening the popup is handled during a `mouseReleaseEvent` --- but
  // only if the `_activated` flag is `true`.
  // Only set _activated if no keyboard modifiers are currently pressed (which
  // most likely indicates some tool or other is active)
  if (event->modifiers() == Qt::NoModifier)
    _activated = true;
  else {
    _activated = false;
    Super::mousePressEvent(event);
  }
}

void PDFMarkupAnnotationGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  Q_ASSERT(event != nullptr);

  if (!_activated) {
    Super::mouseReleaseEvent(event);
    return;
  }
  _activated = false;

  if (!contains(event->pos()) || !_annot)
    return;

  // Find widget that received this mouse event in the first place
  // Note: according to the Qt docs, QApplication::widgetAt() can be slow. But
  // we don't care here, as this is called only once.
  QWidget * sender = QApplication::widgetAt(event->screenPos());

  if (!sender || !qobject_cast<PDFDocumentView*>(sender->parent()))
    return;

  if (_popup) {
    if (_popup->isVisible())
      _popup->hide();
    else {
      _popup->move(sender->mapFromGlobal(event->screenPos()));
      _popup->show();
      _popup->raise();
      _popup->setFocus();
    }
    return;
  }

  _popup = new QWidget(sender);

  QStringList styles;
  if (_annot->color().isValid()) {
    QColor c(_annot->color());
    styles << QString::fromUtf8(".QWidget { background-color: %1; }").arg(c.name());
    if (qGray(c.rgb()) >= 100)
      styles << QString::fromUtf8(".QWidget, .QLabel { color: black; }");
    else
      styles << QString::fromUtf8(".QWidget, .QLabel { color: white; }");
  }
  else {
    styles << QString::fromUtf8(".QWidget { background-color: %1; }").arg(QApplication::palette().color(QPalette::Window).name());
      styles << QString::fromUtf8(".QWidget, .QLabel { color: %1; }").arg(QApplication::palette().color(QPalette::Text).name());
  }
  _popup->setStyleSheet(styles.join(QString::fromLatin1("\n")));
  QGridLayout * layout = new QGridLayout(_popup);
  layout->setContentsMargins(2, 2, 2, 5);

  QLabel * subject = new QLabel(QString::fromUtf8("<b>%1</b>").arg(_annot->subject()), _popup);
  layout->addWidget(subject, 0, 0, 1, -1);
  QLabel * author = new QLabel(_annot->author(), _popup);
  layout->addWidget(author, 1, 0, 1, 1);
  QLabel * date = new QLabel(QLocale().toString(_annot->creationDate(), QLocale::LongFormat), _popup);
  layout->addWidget(date, 1, 1, 1, 1, Qt::AlignRight);
  QTextEdit * content = new QTextEdit(_annot->richContents(), _popup);
  content->setEnabled(false);
  layout->addWidget(content, 2, 0, 1, -1);

  _popup->setLayout(layout);
  _popup->move(sender->mapFromGlobal(event->screenPos()));
  _popup->show();
  // TODO: Make popup closable, movable; position it properly (also upon
  // zooming!), give some visible indication to which annotation it belongs.
  // (Probably turn it into a subclass of QWidget, too).
}


// PDFActionEvent
// ============

// A PDF Link event is generated when a link is clicked and contains the page
// number of the link target.
PDFActionEvent::PDFActionEvent(const PDFAction * action) : Super(ActionEvent), action(action) {}

// Obtain a unique ID for `PDFActionEvent` that can be used by event handlers to
// filter out these events.
QEvent::Type PDFActionEvent::ActionEvent = static_cast<QEvent::Type>( QEvent::registerEventType() );



PDFOptionalContentInfoWidget::PDFOptionalContentInfoWidget(QWidget *parent)
  : PDFDocumentInfoWidget(parent)
{
  QVBoxLayout * layout{new QVBoxLayout(this)};
  layout->setContentsMargins(0, 0, 0, 0);
  _list = new QListView(this);
  layout->addWidget(_list);
  setLayout(layout);
  retranslateUi();
}

void PDFOptionalContentInfoWidget::initFromDocument(const QWeakPointer<Backend::Document> newDoc)
{
  _doc = newDoc;
  QSharedPointer<Backend::Document> doc{newDoc.toStrongRef()};
  if (!doc) {
    return;
  }
  _list->setModel(doc->optionalContentModel());
}

void PDFOptionalContentInfoWidget::clear()
{
  _list->disconnect(this);
  _list->setModel(nullptr);
}

void PDFOptionalContentInfoWidget::retranslateUi()
{
  setWindowTitle(PDFDocumentView::tr("Layers"));
}

} // namespace QtPDF

// vim: set sw=2 ts=2 et

