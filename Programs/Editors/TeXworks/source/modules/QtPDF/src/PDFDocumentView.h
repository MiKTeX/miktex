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
#ifndef PDFDocumentView_H
#define PDFDocumentView_H

#include "PDFBackend.h"
#include "PDFDocumentTools.h"
#include "PDFRuler.h"
#include "PDFSearcher.h"

#include <QtWidgets>
#include <memory>

namespace QtPDF {

class PDFDocumentScene;

// Forward declare classes defined in this header.
class PDFPageGraphicsItem;
class PDFLinkGraphicsItem;
class PDFDocumentMagnifierView;
class PDFActionEvent;
class PDFDocumentView;


const int TILE_SIZE=1024;

class PDFDocumentView : public QGraphicsView {
  Q_OBJECT
  typedef QGraphicsView Super;

  friend class DocumentTool::AbstractTool;
  friend class DocumentTool::Select;

public:
  enum PageMode { PageMode_SinglePage, PageMode_OneColumnContinuous, PageMode_TwoColumnContinuous, PageMode_Presentation };
  enum MouseMode { MouseMode_MagnifyingGlass, MouseMode_Move, MouseMode_MarqueeZoom, MouseMode_Measure, MouseMode_Select };
  enum Dock { Dock_TableOfContents, Dock_MetaData, Dock_Fonts, Dock_Permissions, Dock_Annotations, Dock_OptionalContent };
  using size_type = QList<QGraphicsItem*>::size_type;

  PDFDocumentView(QWidget *parent = nullptr);
  ~PDFDocumentView() override;
  void setScene(QSharedPointer<PDFDocumentScene> a_scene);
  size_type currentPage();
  size_type lastPage();
  PageMode pageMode() const { return _pageMode; }
  qreal zoomLevel() const { return _zoomLevel; }
  bool useGrayScale() const { return _useGrayScale; }
  void fitInView(const QRectF & rect, Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio);
  const QWeakPointer<QtPDF::Backend::Document> document() const;
  QString selectedText() const;

  // The ownership of the returned pointers is transferred to the caller (i.e.,
  // he has to destroy them, unless the `parent` widget does that automatically)
  // They are fully wired to this PDFDocumentView (e.g., clicking on entries in
  // the table of contents will change this view)
  QDockWidget * dockWidget(const Dock type, QWidget * parent = nullptr);

  DocumentTool::AbstractTool * armedTool() const { return _armedTool; }
  void triggerContextClick(const size_type page, const QPointF pos) { emit contextClick(page, pos); }

  QGraphicsPathItem * addHighlightPath(const size_type page, const QPainterPath & path, const QBrush & brush, const QPen & pen = Qt::NoPen);
  QGraphicsPathItem * addHighlightPath(const size_type page, const QRectF & rect, const QBrush & brush, const QPen & pen = Qt::NoPen) {
    QPainterPath p;
    p.addRect(rect);
    return addHighlightPath(page, p, brush, pen);
  }
  QGraphicsPathItem * addHighlightPath(const size_type page, const QPainterPath & path, const QColor color, const QPen & pen = Qt::NoPen) {
    return addHighlightPath(page, path, QBrush(color), pen);
  }

  QBrush searchResultHighlightBrush() const { return _searchResultHighlightBrush; }
  void setSearchResultHighlightBrush(const QBrush & brush);

  QBrush currentSearchResultHighlightBrush() const { return _currentSearchResultHighlightBrush; }
  void setCurrentSearchResultHighlightBrush(const QBrush & brush);

  bool canGoPrevViewRects() const { return !_oldViewRects.empty(); }

  bool isRulerVisible() const { return _ruler.isVisibleTo(this); }
  PDFRuler * ruler() { return &_ruler; }

public slots:
  void goPrev();
  void goNext();
  void goFirst();
  void goLast();
  void goPrevViewRect();
  // `alignment` can be (a combination of) 0, Qt::AlignLeft, Qt::AlignRight,
  // Qt::AlignHCenter, Qt::AlignTop, Qt::AlignBottom, Qt::AlignVCenter.
  // 0 corresponds to no alignment, i.e., the view will change so that the
  // rectangle of page pageNum closest to the original viewport rect is visible.
  void goToPage(const size_type pageNum, const int alignment = Qt::AlignLeft | Qt::AlignTop);
  // Similar to the one above, but view is aligned at `anchor`. Note that the
  // default alignment is centering here, which is also used if `alignment` == 0.
  // `anchor` must be given in item coordinates
  void goToPage(const size_type pageNum, const QPointF anchor, const int alignment = Qt::AlignHCenter | Qt::AlignVCenter);
  void goToPDFDestination(const PDFDestination & dest, bool saveOldViewRect = true);
  void setPageMode(const PageMode pageMode, const bool forceRelayout = false);
  void setSinglePageMode() { setPageMode(PageMode_SinglePage); }
  void setOneColContPageMode() { setPageMode(PageMode_OneColumnContinuous); }
  void setTwoColContPageMode() { setPageMode(PageMode_TwoColumnContinuous); }
  void setPresentationMode() { setPageMode(PageMode_Presentation); }
  void setMouseMode(const MouseMode newMode);
  void setMouseModeMagnifyingGlass() { setMouseMode(MouseMode_MagnifyingGlass); }
  void setMouseModeMove() { setMouseMode(MouseMode_Move); }
  void setMouseModeMarqueeZoom() { setMouseMode(MouseMode_MarqueeZoom); }
  void setMouseModeMeasure() { setMouseMode(MouseMode_Measure); }
  void setMouseModeSelect() { setMouseMode(MouseMode_Select); }
  void setMagnifierShape(const DocumentTool::MagnifyingGlass::MagnifierShape shape);
  void setMagnifierSize(const int size);
  void setUseGrayScale(const bool grayScale = true) { _useGrayScale = grayScale; }

  void zoomBy(const qreal zoomFactor, const QGraphicsView::ViewportAnchor anchor = QGraphicsView::AnchorViewCenter);
  void zoomIn(const QGraphicsView::ViewportAnchor anchor = QGraphicsView::AnchorViewCenter);
  void zoomOut(const QGraphicsView::ViewportAnchor anchor = QGraphicsView::AnchorViewCenter);
  void zoomToRect(QRectF a_rect);
  void zoomFitWindow();
  void zoomFitWidth();
  void zoomFitContentWidth();
  void zoom100();
  void setZoomLevel(const qreal zoomLevel, const QGraphicsView::ViewportAnchor anchor = QGraphicsView::AnchorViewCenter);

  void search(QString searchText, Backend::SearchFlags flags = Backend::Search_CaseInsensitive);
  void nextSearchResult();
  void previousSearchResult();
  void clearSearchResults();

  void armTool(const DocumentTool::AbstractTool::Type toolType);
  void disarmTool();

  void showRuler(const bool show = true);

signals:
  void changedPage(QtPDF::PDFDocumentView::size_type pageNum);
  void changedZoom(qreal zoomLevel);
  void changedPageMode(QtPDF::PDFDocumentView::PageMode newMode);
  // emitted, e.g., if a new document was loaded, or if the existing document
  // has changed (e.g., if it was unlocked)
  void changedDocument(const QWeakPointer<QtPDF::Backend::Document> newDoc);

  void updated();

  void searchProgressChanged(int percent, QtPDF::PDFDocumentView::size_type occurrences);
  void searchResultHighlighted(const QtPDF::PDFDocumentView::size_type pageNum, const QList<QPolygonF> region);
  void textSelectionChanged(const bool isTextSelected);

  void requestOpenUrl(const QUrl url);
  void requestExecuteCommand(QString command);
  void requestOpenPdf(QString filename, QtPDF::PDFDestination destination, bool newWindow);
  void contextClick(const QtPDF::PDFDocumentView::size_type page, const QPointF pos);

protected:
  // Keep track of the current page by overloading the widget paint event.
  void paintEvent(QPaintEvent * event) override;
  void keyPressEvent(QKeyEvent * event) override;
  void keyReleaseEvent(QKeyEvent * event) override;
  void mousePressEvent(QMouseEvent * event) override;
  void mouseMoveEvent(QMouseEvent * event) override;
  void mouseReleaseEvent(QMouseEvent * event) override;
  void wheelEvent(QWheelEvent * event) override;
  void changeEvent(QEvent * event) override;
  void resizeEvent(QResizeEvent * event) override;

  // Maybe this will become public later on
  // Ownership of tool is transferred to PDFDocumentView
  void registerTool(std::unique_ptr<DocumentTool::AbstractTool> tool);

  DocumentTool::AbstractTool * getToolByType(const DocumentTool::AbstractTool::Type type);

  void armTool(DocumentTool::AbstractTool * tool);

protected slots:
  void maybeUpdateSceneRect();
  void maybeArmTool(uint modifiers);
  void pdfActionTriggered(const QtPDF::PDFAction * action);
  // Note: view specifies which part of the page should be visible and must
  // therefore be given in page coordinates
  void goToPage(const QtPDF::PDFPageGraphicsItem * page, const QRectF view, const bool mayZoom = false);
  void goToPage(const QtPDF::PDFPageGraphicsItem * page, const int alignment = Qt::AlignLeft | Qt::AlignTop);
  void goToPage(const QtPDF::PDFPageGraphicsItem * page, const QPointF anchor, const int alignment = Qt::AlignHCenter | Qt::AlignVCenter);
  void searchResultReady(PDFSearcher::size_type pageIndex);
  void searchProgressValueChanged(PDFSearcher::size_type progressValue);
  void reinitializeFromScene();
  void notifyTextSelectionChanged();

private:
  PageMode _pageMode{PageMode_OneColumnContinuous};
  MouseMode _mouseMode{MouseMode_Move};
  QCursor _hiddenCursor;
  // Use std::vector instead of QVector as the latter can't handle non-copyable
  // types
  std::vector< std::unique_ptr<DocumentTool::AbstractTool> > _tools;
  DocumentTool::AbstractTool * _armedTool{nullptr};
  QMap<uint, DocumentTool::AbstractTool::Type> _toolAccessors;

  QStack<PDFDestination> _oldViewRects;

  QSharedPointer<PDFDocumentScene> _pdf_scene;

  qreal _zoomLevel{1.0};
  size_type _currentPage{-1}, _lastPage{-1};

  PDFSearcher _searcher;
  QList<QGraphicsItem *> _searchResults;
  size_type _currentSearchResult{-1};
  QBrush _searchResultHighlightBrush;
  QBrush _currentSearchResultHighlightBrush;
  PDFRuler _ruler{this};
  bool _useGrayScale{false};

  // Never try to set a vanilla QGraphicsScene, always use a PDFGraphicsScene.
  void setScene(QGraphicsScene *scene);
  // Parent class has no copy constructor.
  Q_DISABLE_COPY(PDFDocumentView)
};

class PDFDocumentMagnifierView : public QGraphicsView {
  Q_OBJECT
  typedef QGraphicsView Super;

  PDFDocumentView * _parent_view;
  qreal _zoomLevel{1.0}, _zoomFactor{2.0};

  DocumentTool::MagnifyingGlass::MagnifierShape _shape{DocumentTool::MagnifyingGlass::Magnifier_Circle};
  int _size{300};

public:
  PDFDocumentMagnifierView(PDFDocumentView *parent = nullptr);
  // the zoom factor multiplies the parent view's _zoomLevel
  void setZoomFactor(const qreal zoomFactor);
  void setPosition(const QPoint pos);
  void setShape(const DocumentTool::MagnifyingGlass::MagnifierShape shape) { setSizeAndShape(_size, shape); }
  void setSize(const int size) { setSizeAndShape(size, _shape); }
  void setSizeAndShape(const int size, const DocumentTool::MagnifyingGlass::MagnifierShape shape);
  // ensures all settings are in sync with the parent view
  // make sure you call it before calling show()!
  // Note: we cannot override show() because prepareToShow() usually needs to be
  // called before setPosition as well (as it adjusts the region accessible in
  // setPosition())
  void prepareToShow();

  QPixmap& dropShadow();

protected:
  void wheelEvent(QWheelEvent * event) override { event->ignore(); }
  void paintEvent(QPaintEvent * event) override;

  QPixmap _dropShadow;
};

// Inherits from `QGraphicsOject` instead of `QGraphicsItem` in order to
// support SIGNALS/SLOTS used by threaded rendering.
//
// NOTE: __`QGraphicsObject` was added in Qt 4.6__
class PDFPageGraphicsItem : public QGraphicsObject
{
  Q_OBJECT
  typedef QGraphicsObject Super;
  using size_type = PDFDocumentView::size_type;

  QWeakPointer<Backend::Page> _page;

  double _dpiX;
  double _dpiY;
  // the nominal (i.e., unmagnified) page size in pixel
  QSizeF _pageSize;
  size_type _pageNum;

  bool _linksLoaded;
  bool _annotationsLoaded;

  QTransform _pageScale, _pointScale;
  qreal _zoomLevel;

  friend class PageProcessingRenderPageRequest;
  friend class PageProcessingLoadLinksRequest;
//  friend class PDFPageLayout;

  static void imageToGrayScale(QImage & img);

public:
  PDFPageGraphicsItem(QWeakPointer<Backend::Page> a_page, const double dpiX, const double dpiY, QGraphicsItem *parent = nullptr);

  // This seems fragile as it assumes no other code declaring a custom graphics
  // item will choose the same ID for it's object types. Unfortunately, there
  // appears to be no equivalent of `registerEventType` for `QGraphicsItem`
  // subclasses.
  enum { Type = UserType + 1 };
  int type() const override;

  void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

  QRectF boundingRect() const override;

  QWeakPointer<Backend::Page> page() const { return _page; }

  // Maps the point _point_ from the page's coordinate system (in pt) to this
  // item's coordinate system - chain with mapToScene and related methods to get
  // coordinates in other systems
  QPointF mapFromPage(const QPointF & point) const;
  // Maps the point _point_ from the item's coordinate system to the page's
  // coordinate system (in pt) - chain with mapFromScene and related methods to
  // convert from coordinates in other systems
  QPointF mapToPage(const QPointF & point) const;

  QTransform pageScale() { return _pageScale; }
  QTransform pointScale() { return _pointScale; }

  // get the nominal (i.e., unmagnified) page size in pixel
  QSizeF pageSizeF() const { return _pageSize; }
  size_type pageNum() const { return _pageNum; }

protected:
  bool event(QEvent * event) override;

private:
  // Parent has no copy constructor.
  Q_DISABLE_COPY(PDFPageGraphicsItem)

private slots:
  void addLinks(QList< QSharedPointer<Annotation::Link> > links);
  void addAnnotations(QList< QSharedPointer<Annotation::AbstractAnnotation> > annotations);
};

// TODO: Should be turned into a QGraphicsPolygonItem
class PDFLinkGraphicsItem : public QGraphicsRectItem {
  typedef QGraphicsRectItem Super;

  QSharedPointer<Annotation::Link> _link;
  bool _activated;

public:
  PDFLinkGraphicsItem(QSharedPointer<Annotation::Link> a_link, QGraphicsItem *parent = nullptr);
  // See concerns in `PDFPageGraphicsItem` for why this feels fragile.
  enum { Type = UserType + 2 };
  int type() const override;
  void retranslateUi();

protected:
  void hoverEnterEvent(QGraphicsSceneHoverEvent * event) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent * event) override;

  void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent * event) override;

private:
  // Parent class has no copy constructor.
  Q_DISABLE_COPY(PDFLinkGraphicsItem)
};


// TODO: Should be turned into a QGraphicsPolygonItem
class PDFMarkupAnnotationGraphicsItem : public QGraphicsRectItem {
  typedef QGraphicsRectItem Super;

  QSharedPointer<Annotation::Markup> _annot;
  bool _activated;
  QWidget * _popup;

public:
  PDFMarkupAnnotationGraphicsItem(QSharedPointer<Annotation::Markup> annot, QGraphicsItem *parent = nullptr);
  // See concerns in `PDFPageGraphicsItem` for why this feels fragile.
  enum { Type = UserType + 3 };
  int type() const override;

protected:
  void hoverEnterEvent(QGraphicsSceneHoverEvent * event) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent * event) override;

  void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent * event) override;

private:
  // Parent class has no copy constructor.
  Q_DISABLE_COPY(PDFMarkupAnnotationGraphicsItem)
};

class PDFActionEvent : public QEvent {
  typedef QEvent Super;

public:
  PDFActionEvent(const PDFAction * action);
  static QEvent::Type ActionEvent;
  const PDFAction * action;
};

} // namespace QtPDF

// Note: Q_DECLARE_METATYPE must be specified _outside_ any namespace
// declaration (according to Qt docs)

// We need to declare a QList<PDFLinkGraphicsItem *> meta-type so we can
// pass it through inter-thread (i.e., queued) connections
Q_DECLARE_METATYPE(QList<QtPDF::PDFLinkGraphicsItem *>)
// We need to declare a QList<PDFMarkupAnnotationGraphicsItem *> meta-type so we can
// pass it through inter-thread (i.e., queued) connections
Q_DECLARE_METATYPE(QList<QtPDF::PDFMarkupAnnotationGraphicsItem *>)


#endif // End header include guard

// vim: set sw=2 ts=2 et
