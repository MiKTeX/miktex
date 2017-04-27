/**
 * Copyright (C) 2012  Stefan Löffler
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
#include <PDFDocumentTools.h>
#include <PDFDocumentView.h>

namespace QtPDF {
namespace DocumentTool {

// AbstractTool
// ========================
//
void AbstractTool::arm() {
  Q_ASSERT(_parent != NULL);
  if (_parent->viewport())
    _parent->viewport()->setCursor(_cursor);
}
void AbstractTool::disarm() {
  Q_ASSERT(_parent != NULL);
  if (_parent->viewport())
    _parent->viewport()->unsetCursor();
}

void AbstractTool::keyPressEvent(QKeyEvent *event)
{
  if (_parent)
    _parent->maybeArmTool(Qt::LeftButton + event->modifiers());
}

void AbstractTool::keyReleaseEvent(QKeyEvent *event)
{
  if (_parent)
    _parent->maybeArmTool(Qt::LeftButton + event->modifiers());
}

void AbstractTool::mousePressEvent(QMouseEvent * event)
{
  if (_parent)
    _parent->maybeArmTool(event->buttons() | event->modifiers());
}

void AbstractTool::mouseReleaseEvent(QMouseEvent * event)
{
  // If the last mouse button was released, we arm the tool corresponding to the
  // left mouse button by default
  Qt::MouseButtons buttons = event->buttons();
  if (buttons == Qt::NoButton)
    buttons |= Qt::LeftButton;

  if (_parent)
    _parent->maybeArmTool(buttons | event->modifiers());
}


// ZoomIn
// ========================
//
ZoomIn::ZoomIn(PDFDocumentView * parent)
: AbstractTool(parent),
  _started(false)
{
  _cursor = QCursor(QPixmap(QString::fromUtf8(":/QtPDF/icons/zoomincursor.png")));
}

void ZoomIn::mousePressEvent(QMouseEvent * event)
{
  Q_ASSERT(_parent != NULL);
  
  if (!event)
    return;
  _started = (event->buttons() == Qt::LeftButton && event->button() == Qt::LeftButton);
  if (_started)
    _startPos = event->pos();
}

void ZoomIn::mouseReleaseEvent(QMouseEvent * event)
{
  Q_ASSERT(_parent != NULL);

  if (!event || !_started)
    return;
  if (event->buttons() == Qt::NoButton && event->button() == Qt::LeftButton) {
    QPoint offset = event->pos() - _startPos;
    if (offset.manhattanLength() <  QApplication::startDragDistance())
      _parent->zoomIn(QGraphicsView::AnchorUnderMouse);
  }
  _started = false;
}

// ZoomOut
// ========================
//
ZoomOut::ZoomOut(PDFDocumentView * parent)
: AbstractTool(parent),
  _started(false)
{
  _cursor = QCursor(QPixmap(QString::fromUtf8(":/QtPDF/icons/zoomoutcursor.png")));
}

void ZoomOut::mousePressEvent(QMouseEvent * event)
{
  if (!event)
    return;
  _started = (event->buttons() == Qt::LeftButton && event->button() == Qt::LeftButton);
  if (_started)
    _startPos = event->pos();
}

void ZoomOut::mouseReleaseEvent(QMouseEvent * event)
{
  Q_ASSERT(_parent != NULL);

  if (!event || !_started)
    return;
  if (event->buttons() == Qt::NoButton && event->button() == Qt::LeftButton) {
    QPoint offset = event->pos() - _startPos;
    if (offset.manhattanLength() <  QApplication::startDragDistance())
      _parent->zoomOut(QGraphicsView::AnchorUnderMouse);
  }
  _started = false;
}


// MagnifyingGlass
// ==============================
//
MagnifyingGlass::MagnifyingGlass(PDFDocumentView * parent) : 
  AbstractTool(parent),
  _started(false)
{
  _magnifier = new PDFDocumentMagnifierView(parent);
  _cursor = QCursor(QPixmap(QString::fromUtf8(":/QtPDF/icons/magnifiercursor.png")));
}

void MagnifyingGlass::setMagnifierShape(const MagnifierShape shape)
{
  Q_ASSERT(_magnifier != NULL);
  _magnifier->setShape(shape);
}

void MagnifyingGlass::setMagnifierSize(const int size)
{
  Q_ASSERT(_magnifier != NULL);
  _magnifier->setSize(size);
}

void MagnifyingGlass::mousePressEvent(QMouseEvent * event)
{
  Q_ASSERT(_magnifier != NULL);
  if (!event)
    return;
  _started = (event->buttons() == Qt::LeftButton && event->button() == Qt::LeftButton);

  if (_started) {
    _magnifier->prepareToShow();
    _magnifier->setPosition(event->pos());
  }
  _magnifier->setVisible(_started);

  // Ensure an update of the viewport so that the drop shadow is painted
  // correctly
  QRect r(QPoint(0, 0), _magnifier->dropShadow().size());
  r.moveCenter(_magnifier->geometry().center());
  _parent->viewport()->update(r);
}

void MagnifyingGlass::mouseMoveEvent(QMouseEvent * event)
{
  Q_ASSERT(_magnifier != NULL);
  Q_ASSERT(_parent != NULL);

  if (!event || !_started)
    return;

  // Only update the portion of the viewport (possibly) obscured by the
  // magnifying glass and its shadow.
  QRect r(QPoint(0, 0), _magnifier->dropShadow().size());
  r.moveCenter(_magnifier->geometry().center());
  _parent->viewport()->update(r);

  _magnifier->setPosition(event->pos());
}

void MagnifyingGlass::mouseReleaseEvent(QMouseEvent * event)
{
  Q_ASSERT(_magnifier != NULL);

  if (!event || !_started)
    return;
  if (event->buttons() == Qt::NoButton && event->button() == Qt::LeftButton) {
    _magnifier->hide();
    _started = false;
    // Force an update of the viewport so that the drop shadow is hidden
    QRect r(QPoint(0, 0), _magnifier->dropShadow().size());
    r.moveCenter(_magnifier->geometry().center());
    _parent->viewport()->update(r);
  }
}

void MagnifyingGlass::paintEvent(QPaintEvent * event)
{
  Q_ASSERT(_magnifier != NULL);
  Q_ASSERT(_parent != NULL);

  if (!_started)
    return;

  // Draw a drop shadow
  QPainter p(_parent->viewport());
  QPixmap& dropShadow(_magnifier->dropShadow());
  QRect r(QPoint(0, 0), dropShadow.size());
  r.moveCenter(_magnifier->geometry().center());
  p.drawPixmap(r.topLeft(), dropShadow);
}


// MarqueeZoom
// ==========================
//
MarqueeZoom::MarqueeZoom(PDFDocumentView * parent) :
  AbstractTool(parent),
  _started(false)
{
  Q_ASSERT(_parent);
  _rubberBand = new QRubberBand(QRubberBand::Rectangle, _parent->viewport());
  _cursor = QCursor(Qt::CrossCursor);
}

void MarqueeZoom::mousePressEvent(QMouseEvent * event)
{
  Q_ASSERT(_parent != NULL);
  Q_ASSERT(_rubberBand != NULL);
  
  if (!event)
    return;
  _started = (event->buttons() == Qt::LeftButton && event->button() == Qt::LeftButton);
  if (_started) {
    _startPos = event->pos();
    _rubberBand->setGeometry(QRect());
    _rubberBand->show();
  }
}

void MarqueeZoom::mouseMoveEvent(QMouseEvent * event)
{
  Q_ASSERT(_rubberBand != NULL);

  QPoint o = _startPos, p = event->pos();

  if (event->buttons() != Qt::LeftButton ) {
    // The user somehow let go of the left button without us recieving an
    // event. Abort the zoom operation.
    _rubberBand->setGeometry(QRect());
    _rubberBand->hide();
  } else if ( (o - p).manhattanLength() > QApplication::startDragDistance() ) {
    // Update rubber band Geometry.
    _rubberBand->setGeometry(QRect(
      QPoint(qMin(o.x(),p.x()), qMin(o.y(), p.y())),
      QPoint(qMax(o.x(),p.x()), qMax(o.y(), p.y()))
    ));
  }
}

void MarqueeZoom::mouseReleaseEvent(QMouseEvent * event)
{
  Q_ASSERT(_parent != NULL);

  if (!event || !_started)
    return;
  if (event->buttons() == Qt::NoButton && event->button() == Qt::LeftButton) {
    QRectF zoomRect = _parent->mapToScene(_rubberBand->geometry()).boundingRect();
    _rubberBand->hide();
    _rubberBand->setGeometry(QRect());
    _parent->zoomToRect(zoomRect);
  }
  _started = false;
}


// Move
// ===================
//
Move::Move(PDFDocumentView * parent) :
  AbstractTool(parent),
  _started(false)
{
  _cursor = QCursor(Qt::OpenHandCursor);
  _closedHandCursor = QCursor(Qt::ClosedHandCursor);
}

void Move::mousePressEvent(QMouseEvent * event)
{
  Q_ASSERT(_parent != NULL);
  
  if (!event)
    return;
  _started = (event->buttons() == Qt::LeftButton && event->button() == Qt::LeftButton);
  if (_started) {
    if (_parent->viewport())
      _parent->viewport()->setCursor(_closedHandCursor);
    _oldPos = event->pos();
  }
}

void Move::mouseMoveEvent(QMouseEvent * event)
{
  Q_ASSERT(_parent != NULL);

  if (!_started || !event)
    return;

  // Adapted from <qt>/src/gui/graphicsview/qgraphicsview.cpp @ QGraphicsView::mouseMoveEvent
  QScrollBar *hBar = _parent->horizontalScrollBar();
  QScrollBar *vBar = _parent->verticalScrollBar();
  QPoint delta = event->pos() - _oldPos;
  hBar->setValue(hBar->value() - delta.x());
  vBar->setValue(vBar->value() - delta.y());
  _oldPos = event->pos();
}

void Move::mouseReleaseEvent(QMouseEvent * event)
{
  Q_ASSERT(_parent != NULL);

  if (!event || !_started)
    return;
  if (event->buttons() == Qt::NoButton && event->button() == Qt::LeftButton)
    if (_parent->viewport())
      _parent->viewport()->setCursor(_cursor);
  _started = false;
}


// ContextClick
// ===========================
//
void ContextClick::mousePressEvent(QMouseEvent * event)
{
  Q_ASSERT(_parent != NULL);
  
  if (!event)
    return;
  _started = (event->buttons() == Qt::LeftButton && event->button() == Qt::LeftButton);
}

void ContextClick::mouseReleaseEvent(QMouseEvent * event)
{
  Q_ASSERT(_parent != NULL);

  if (!event || !_started)
    return;

  _started = false;
  if (event->buttons() == Qt::NoButton && event->button() == Qt::LeftButton) {
    QPointF pos(_parent->mapToScene(event->pos()));

    PDFPageGraphicsItem * pageItem = NULL;
    foreach(QGraphicsItem * item, _parent->scene()->items(pos, Qt::IntersectsItemBoundingRect, Qt::AscendingOrder)) {
      if (item && item->type() == PDFPageGraphicsItem::Type) {
        pageItem = static_cast<PDFPageGraphicsItem*>(item);
        break;
      }
    }
    if (!pageItem)
      return;
    _parent->triggerContextClick(pageItem->pageNum(), pageItem->mapToPage(pageItem->mapFromScene(pos)));
  }
}

// MeasureLineGrip
// ===========================
//
MeasureLineGrip::MeasureLineGrip(MeasureLine * parent, const int pt) :
  QGraphicsRectItem(parent),
  _pt(pt)
{
  setFlag(QGraphicsItem::ItemIgnoresTransformations);
  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::LeftButton);
  setRect(-2, -2, 5, 5);
  setBrush(QBrush(Qt::green));
  setPen(QPen(Qt::black));
}

void MeasureLineGrip::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  setCursor(Qt::CrossCursor);
  QGraphicsRectItem::hoverEnterEvent(event);
}

void MeasureLineGrip::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  unsetCursor();
  QGraphicsRectItem::hoverLeaveEvent(event);
}

void MeasureLineGrip::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  // mousePressEvent must be implemented to receive mouseMoveEvent messages
}

void MeasureLineGrip::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  Q_ASSERT(event != NULL);
  mouseMove(event->scenePos(), event->modifiers());
}

void MeasureLineGrip::mouseMove(const QPointF scenePos, const Qt::KeyboardModifiers modifiers)
{
  MeasureLine * ml = static_cast<MeasureLine*>(parentItem());
  Q_ASSERT(ml != NULL);
  
  switch(_pt) {
  case 1:
  {
    QLineF line(scenePos, ml->line().p2());
    if (modifiers.testFlag(Qt::ControlModifier)) {
      // locked mode; horizontal or vertical line only
      if (line.angle() <= 45 || line.angle() >= 315 ||
          (line.angle() >= 135 && line.angle() <= 225))
        line.setP1(QPointF(line.p1().x(), line.p2().y()));
      else
        line.setP1(QPointF(line.p2().x(), line.p1().y()));
    }
    ml->setLine(line);
    break;
  }
  default:
  {
    QLineF line(ml->line().p1(), scenePos);
    if (modifiers.testFlag(Qt::ControlModifier)) {
      // locked mode; horizontal or vertical line only
      if (line.angle() <= 45 || line.angle() >= 315 ||
          (line.angle() >= 135 && line.angle() <= 225))
        line.setP2(QPointF(line.p2().x(), line.p1().y()));
      else
        line.setP2(QPointF(line.p1().x(), line.p2().y()));
    }
    ml->setLine(line);
    break;
  }
  }
}

// MeasureLine
// ===========================
//
MeasureLine::MeasureLine(QGraphicsView * primaryView, QGraphicsItem * parent /* = NULL */) :
  QGraphicsLineItem(parent),
  _primaryView(primaryView)
{
  _measureBox = new QComboBox();
  _measureBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  _measureBoxProxy = new QGraphicsProxyWidget(this);
  _measureBoxProxy->setWidget(_measureBox);
  _measureBoxProxy->setFlag(QGraphicsItem::ItemIgnoresTransformations);
  _measureBoxProxy->setFlag(QGraphicsItem::ItemSendsGeometryChanges);

  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::LeftButton);

  _grip1 = new MeasureLineGrip(this, 1);
  _grip2 = new MeasureLineGrip(this, 2);
}

void MeasureLine::setLine(QLineF line)
{
  QGraphicsLineItem::setLine(line);
  
  _grip1->setPos(line.p1());
  _grip2->setPos(line.p2());
  updateMeasurement();
}

void MeasureLine::updateMeasurement()
{
  Q_ASSERT(_measureBox != NULL);

  // Note: we use LaTeX units here, i.e., 1 pt = 1/72.27 in (as opposed to the
  // pdf unit 1 pt = 1/72 in, which in this context is called 1 bp); see
  // http://en.wikibooks.org/wiki/LaTeX/Useful_Measurement_Macros
  
  // NOTE: The view internally uses coordinates scaled by DPI/72 (owing to the
  // PDF convention of 1 in = 72 pt). We have to undo that scaling here to get
  // physical units.
  float dx = line().dx() / QApplication::desktop()->physicalDpiX();
  float dy = line().dy() / QApplication::desktop()->physicalDpiY();
  // length: Length of the measurement line in pt (i.e., 1/72.27 inch)
  float length = 72.27 * qSqrt(dx * dx + dy * dy);
  
  int idx = _measureBox->currentIndex();
  _measureBox->clear();
  _measureBox->addItem(QString::fromUtf8("%1 pt").arg(length), QString::fromUtf8("pt"));
  _measureBox->addItem(QString::fromUtf8("%1 mm").arg(length / 2.84), QString::fromUtf8("mm"));
  _measureBox->addItem(QString::fromUtf8("%1 cm").arg(length / 28.4), QString::fromUtf8("cm"));
  _measureBox->addItem(QString::fromUtf8("%1 in").arg(length / 72.27), QString::fromUtf8("in"));
  _measureBox->addItem(QString::fromUtf8("%1 bp").arg(length * 1.00375), QString::fromUtf8("bp"));
  _measureBox->addItem(QString::fromUtf8("%1 pc").arg(length / 12), QString::fromUtf8("pc"));
  _measureBox->addItem(QString::fromUtf8("%1 dd").arg(length / 1.07), QString::fromUtf8("dd"));
  _measureBox->addItem(QString::fromUtf8("%1 cc").arg(length / 12.84), QString::fromUtf8("cc"));
  _measureBox->addItem(QString::fromUtf8("%1 sp").arg(length * 65536), QString::fromUtf8("sp"));
  if (idx < 0 || idx >= _measureBox->count())
    idx = 0;
  _measureBox->setCurrentIndex(idx);
  _measureBox->updateGeometry();
  // Since the box size may have changed, we need to reposition the box
  updateMeasureBoxPos();
}

void MeasureLine::updateMeasureBoxPos()
{
  const float ALMOST_ZERO = 1e-4;
  Q_ASSERT(_primaryView != NULL);
  Q_ASSERT(_measureBoxProxy != NULL);
  Q_ASSERT(_measureBox != NULL);
  
  QPointF center = line().pointAt(0.5);
  // scaling of a unit square
  float scaling = _primaryView->mapToScene(0, 0, 1, 1).boundingRect().width();
  // spacing of 2 pixels (mapped to scene coordinates)
  float spacing = 2 * scaling;
  QPointF offset;
  
  // Get the size of the measurement box in scene coordinates
  QSizeF sceneSize = scaling * _measureBox->size();

  // horizontal line
  if ((line().angle() <= ALMOST_ZERO || line().angle() > 360 - ALMOST_ZERO) ||
      (line().angle() >= 180 - ALMOST_ZERO && line().angle() < 180 + ALMOST_ZERO))
    offset = QPointF(-sceneSize.width() / 2., spacing);
  // vertical line
  else if ((line().angle() >= 90 - ALMOST_ZERO && line().angle() < 90 + ALMOST_ZERO) ||
           (line().angle() >= 270 - ALMOST_ZERO && line().angle() < 270 + ALMOST_ZERO))
    offset = QPointF(spacing, -sceneSize.height() / 2);
  // line pointing up
  else if (line().angle() < 90 || (line().angle() > 180 && line().angle() < 270))
    offset = QPointF(spacing / 1.41421356237, spacing / 1.41421356237);
  // line pointing down
  else
    offset = QPointF(spacing / 1.41421356237, -sceneSize.height() - spacing / 1.41421356237);
  
  _measureBoxProxy->setPos(center + offset);
}

void MeasureLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  QGraphicsLineItem::paint(painter, option, widget);
  // TODO: Possibly change style of pen
  
  // TODO: Only reposition measurement box if zoom level changed
  updateMeasureBoxPos();
}

void MeasureLine::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  setCursor(Qt::SizeAllCursor);
}

void MeasureLine::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  unsetCursor();
}

void MeasureLine::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  // Find the offset of the grab point from handle 1
  _grabOffset = event->scenePos() - line().p1();
}

void MeasureLine::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  // Note: We only receive this event while dragging (i.e., after a
  // mousePressEvent)
  setLine(QLineF(event->scenePos() - _grabOffset, event->scenePos() - _grabOffset + line().p2() - line().p1()));
}


// Measure
// ===========================
//
Measure::Measure(PDFDocumentView * parent) :
  AbstractTool(parent),
  _measureLine(NULL),
  _started(false)
{
}

void Measure::mousePressEvent(QMouseEvent * event)
{
  Q_ASSERT(_parent != NULL);
  if (event && _parent->scene()) {
    _started = (event->buttons() == Qt::LeftButton && event->button() == Qt::LeftButton);
    if (!_started)
      return;
    if (!_measureLine) {
      _measureLine = new MeasureLine(_parent);
      _parent->scene()->addItem(_measureLine);
    }
    _measureLine->setLine(_parent->mapToScene(event->pos()), _parent->mapToScene(event->pos()));
    // Initialize with a hidden measuring line with length 0. This way, simple
    // clicking can be used to remove (i.e., hide) a previously visible line
    _measureLine->hide();
    _started = true;
    _startPos = event->pos();
  }
}

void Measure::mouseMoveEvent(QMouseEvent *event)
{
  if (_started) {
    Q_ASSERT(_measureLine != NULL);
    Q_ASSERT(_measureLine->_grip2 != NULL);
    _measureLine->_grip2->mouseMove(_parent->mapToScene(event->pos()), event->modifiers());
    if ((event->pos() - _startPos).manhattanLength() > QApplication::startDragDistance() && !_measureLine->isVisible())
      _measureLine->show();
  }
}

void Measure::mouseReleaseEvent(QMouseEvent * event)
{
  _started = false;
}

void Measure::keyPressEvent(QKeyEvent *event)
{
  // We need to hijack the Ctrl key modifier here (if the tool is started;
  // otherwise we pass it on (e.g., to a DocumentTool::ContextClick))
  if (event->key() != Qt::Key_Control || !_started)
    AbstractTool::keyPressEvent(event);
}

void Measure::keyReleaseEvent(QKeyEvent *event)
{
  // We need to hijack the Ctrl key modifier here (if the tool is started;
  // otherwise we pass it on (e.g., to a DocumentTool::ContextClick))
  if (event->key() != Qt::Key_Control || !_started)
    AbstractTool::keyReleaseEvent(event);
}

// Select
// ========================
//

// distanceFromRect() computes the Manhatten distance between a point and a
// rectangle. If the point is inside (or on the border of) the rectangle, the
// function returns 0. Otherwise, it returns the smallest Manhatten distance of
// pt to any point on the border of the rectangle.
inline double distanceFromRect(const QPointF & pt, const QRectF & rect) {
  double dx, dy;
  if (pt.x() < rect.left())
    dx = rect.left() - pt.x();
  else if (pt.x() > rect.right())
    dx = pt.x() - rect.right();
  else
    dx = 0;
  if (pt.y() < rect.top())
    dy = rect.top() - pt.y();
  else if (pt.y() > rect.bottom())
    dy = pt.y() - rect.bottom();
  else
    dy = 0;
  return dx + dy;
}

Select::Select(PDFDocumentView * parent) :
  AbstractTool(parent),
  _cursorOverBox(false),
  _highlightPath(NULL),
  _mouseMode(MouseMode_None),
  _rubberBand(NULL),
  _pageNum(-1),
  _startBox(0),
  _startSubbox(0)
{
  // We default to the cross cursor. Only when over a text box we change to the
  // IBeam cursor
  _cursor = QCursor(Qt::CrossCursor);
  // Default to the system highlighting color, with alpha set to 50% (since we
  // simply superimpose the selection on the page images)
  _highlightColor = QApplication::palette().color(QPalette::Highlight);
  _highlightColor.setAlpha(128);
}

Select::~Select()
{
  if (_highlightPath)
    delete _highlightPath;
  if (_rubberBand)
    delete _rubberBand;
}

void Select::disarm()
{
  AbstractTool::disarm();
  resetBoxes();
}

void Select::mousePressEvent(QMouseEvent * event)
{
  Q_ASSERT(_parent != NULL);
  
  // We only handle the left mouse button
  if (event->buttons() != Qt::LeftButton) {
   AbstractTool::mousePressEvent(event);
    return;
  }
  
  PDFDocumentScene * scene = static_cast<PDFDocumentScene*>(_parent->scene());
  Q_ASSERT(scene != NULL);

  // get the number of the page the mouse is currently over; if the mouse is
  // not over any page (e.g., it's between pages), there's nothing left to do
  // here
  int pageNum = scene->pageNumAt(_parent->mapToScene(event->pos()));
  if (pageNum < 0)
    return;

  PDFPageGraphicsItem * pageGraphicsItem = static_cast<PDFPageGraphicsItem*>(scene->pageAt(pageNum));
  Q_ASSERT(pageGraphicsItem != NULL);
  
  // Create the highlight path to visualize selections in the scene
  // Note: it will be parented to the page it belongs to later on
  // FIXME: Maybe use PDFDocumentView::addHighlightPath here instead?
  if (!_highlightPath) {
    _highlightPath = new QGraphicsPathItem(NULL);
    _highlightPath->setBrush(QBrush(_highlightColor));
    _highlightPath->setPen(QPen(Qt::transparent));
  }
  // Clear any previous selection
  _highlightPath->setPath(QPainterPath());

  // Save the starting position (in scene coordinates so we can handle scrolling
  // etc.)
  _startPos = _parent->mapToScene(event->pos());
  // Set the mouse mode. Note that _cursorOverBox is updated dynamically in
  // mouseMoveEvent()
  _mouseMode = (_cursorOverBox ? MouseMode_TextSelect : MouseMode_MarqueeSelect);

  if (_mouseMode == MouseMode_MarqueeSelect) {
    // Create the rubber band widget if it doesn't exist and show it
    if (!_rubberBand)
      _rubberBand = new QRubberBand(QRubberBand::Rectangle, _parent->viewport());
    _rubberBand->setGeometry(QRect(event->pos(), event->pos()));
    _rubberBand->show();
  }
  else if (_mouseMode == MouseMode_TextSelect) {
    // Find the box the mouse cursor is over
    QPointF curPdfCoords = pageGraphicsItem->pointScale().inverted().map(pageGraphicsItem->mapFromScene(_parent->mapToScene(event->pos())));
    for (_startBox = 0; _startBox < _boxes.size() && !_boxes[_startBox].boundingBox.contains(curPdfCoords); ++_startBox) ;
    // If we didn't find the box, something went wrong; bail out
    if (_startBox >= _boxes.size())
      _mouseMode = MouseMode_None;
    else {
      // Find the subbox the cursor is over (if any)
      for (_startSubbox = 0; _startSubbox < _boxes[_startBox].subBoxes.size() && !_boxes[_startBox].subBoxes[_startSubbox].boundingBox.contains(curPdfCoords); ++_startSubbox) ;
      if (_startSubbox >= _boxes[_startBox].subBoxes.size())
        _startSubbox = 0;
    }
  }
}

void Select::mouseMoveEvent(QMouseEvent *event)
{
  Q_ASSERT(_parent != NULL);
  PDFDocumentScene * scene = static_cast<PDFDocumentScene*>(_parent->scene());
  Q_ASSERT(scene != NULL);
  Q_ASSERT(!scene->document().isNull());

  // Check if the mouse cursor is over a page. If not, we bail out and keep the
  // last "valid" state.
  int pageNum = scene->pageNumAt(_parent->mapToScene(event->pos()));
  if (pageNum < 0)
    return;
  
  // If we are not currently selecting and the mouse moved to a different page,
  // reset our boxes data
  // Note: If we are currently selecting, we tick to the original page
  //       regardless where the mouse is
  if (_mouseMode == MouseMode_None && pageNum != _pageNum)
    resetBoxes(pageNum);
  
  PDFPageGraphicsItem * pageGraphicsItem = static_cast<PDFPageGraphicsItem*>(scene->pageAt(pageNum));
  Q_ASSERT(pageGraphicsItem != NULL);

  QTransform toView = pageGraphicsItem->pointScale();

  // Boxes are given in pdf units (bp), whereas screen coordinates are given in
  // (scaled) pixels; here, we transform the screen coordinates, rather than
  // transforming each box
  QPointF curPdfCoords = pageGraphicsItem->pointScale().inverted().map(pageGraphicsItem->mapFromScene(_parent->mapToScene(event->pos())));

  switch (_mouseMode) {
  case MouseMode_None:
  default:
  {
    // Check if the cursor is over a box (in which case we use text select mode)
    // or not (in which case we use marquee select mode)
    _cursorOverBox = false;
    foreach(Backend::Page::Box b, _boxes) {
      if (b.boundingBox.contains(curPdfCoords)) {
        _cursorOverBox = true;
        break;
      }
    }
    _parent->viewport()->setCursor(_cursorOverBox ? Qt::IBeamCursor : Qt::CrossCursor);
    break;
  }
  case MouseMode_MarqueeSelect:
  {
    if (!_highlightPath || _boxes.size() == 0)
      break;
    if (_rubberBand)
      _rubberBand->setGeometry(QRect(_parent->mapFromScene(_startPos), event->pos()));
    // Get the selection rect in pdf coords (bp)
    QPointF startPdfCoords = pageGraphicsItem->pointScale().inverted().map(pageGraphicsItem->mapFromScene(_startPos));
    QRectF marqueeRect(startPdfCoords, curPdfCoords);
    QPainterPath highlightPath;
    // Set WindingFill so overlapping, individual paths are both filled
    // completely.
    highlightPath.setFillRule(Qt::WindingFill);
    foreach(Backend::Page::Box b, _boxes) {
      // Note: If b.boundingBox is fully contained in the marqueeRect, add it
      // without iterating over the subboxes. Otherwise, add all intersected
      // subboxes
      if (marqueeRect.intersects(b.boundingBox)) {
        if (b.subBoxes.isEmpty() || marqueeRect.contains(b.boundingBox))
          highlightPath.addRect(toView.mapRect(b.boundingBox));
        else {
          foreach(Backend::Page::Box sb, b.subBoxes) {
            if (marqueeRect.intersects(sb.boundingBox))
              highlightPath.addRect(toView.mapRect(sb.boundingBox));
          }
        }
      }
    }
    _highlightPath->setPath(highlightPath);
    _highlightPath->setParentItem(pageGraphicsItem);
    break;
  }
  case MouseMode_TextSelect:
  {
    if (!_highlightPath || _boxes.size() == 0)
      break;
    
    // Find the box (and subbox therein) that is closest to the current mouse
    // position
    int i, j, endBox, endSubbox;
    double minDist = -1;
    for (i = 0; i < _boxes.size(); ++i) {
      double dist = distanceFromRect(curPdfCoords, _boxes[i].boundingBox);
      if (minDist < -.5 || dist < minDist) {
        endBox = i;
        minDist = dist;
      }
    }
    minDist = -1;
    endSubbox = 0;
    for (i = 0; i < _boxes[endBox].subBoxes.size(); ++i) {
      double dist = distanceFromRect(curPdfCoords, _boxes[endBox].subBoxes[i].boundingBox);
      if (minDist < -.5 || dist < minDist) {
        endSubbox = i;
        minDist = dist;
      }
    }
    
    // Ensure startBox <= endBox and (startSubbox <= endSubbox in case of
    // equality)
    int startBox = _startBox;
    int startSubbox = _startSubbox;
    if (startBox > endBox) {
      startBox = endBox;
      startSubbox = endSubbox;
      endBox = _startBox;
      endSubbox = _startSubbox;
    }
    else if (startBox == endBox && startSubbox > endSubbox) {
      startSubbox = endSubbox;
      endSubbox = _startSubbox;
    }
    
    QPainterPath highlightPath;
    // Set WindingFill so overlapping, individual paths are both filled
    // completely.
    highlightPath.setFillRule(Qt::WindingFill);
    for (i = startBox; i <= endBox; ++i) {
      // Iterate over subboxes in the case that not the whole box might be
      // selected
      if ((i == startBox || i == endBox) && _boxes[i].subBoxes.size() > 0) {
        for (j = 0; j < _boxes[i].subBoxes.size(); ++j) {
          if ((i == startBox && j < startSubbox) || (i == endBox && j > endSubbox))
            continue;
          highlightPath.addRect(toView.mapRect(_boxes[i].subBoxes[j].boundingBox));
        }
      }
      else
        highlightPath.addRect(toView.mapRect(_boxes[i].boundingBox));
    }
    _highlightPath->setPath(highlightPath);
    _highlightPath->setParentItem(pageGraphicsItem);
    break;
  }
  }
}

void Select::mouseReleaseEvent(QMouseEvent * event)
{
  // We only handle the left mouse button
  if (event->buttons() != Qt::NoButton || event->button() != Qt::LeftButton) {
   AbstractTool::mouseReleaseEvent(event);
    return;
  }
  _mouseMode = MouseMode_None;
  if (_rubberBand)
    _rubberBand->hide();
}

void Select::keyPressEvent(QKeyEvent *event)
{
  Q_ASSERT(event != NULL);

  if (event->matches(QKeySequence::Copy) && _highlightPath) {
    // We only handle "copy" (Ctrl+C) here
    if (!_highlightPath->path().isEmpty()) {
      Q_ASSERT(_parent != NULL);
      PDFDocumentScene * scene = static_cast<PDFDocumentScene*>(_parent->scene());
      Q_ASSERT(scene != NULL);
      QSharedPointer<Backend::Document> doc(scene->document().toStrongRef());
      if (!doc)
        return;
      if (doc->permissions().testFlag(Backend::Document::Permission_Extract)) {
        // We only copy text if we are allowed to do so
          
        QSharedPointer<Backend::Page> page(doc->page(_pageNum).toStrongRef());
        if (page.isNull())
          return;
      
        PDFPageGraphicsItem * pageGraphicsItem = static_cast<PDFPageGraphicsItem*>(scene->pageAt(_pageNum));
        Q_ASSERT(pageGraphicsItem != NULL);
      
        QTransform fromView = pageGraphicsItem->pointScale().inverted();
        QString textToCopy = page->selectedText(_highlightPath->path().toFillPolygons(fromView), NULL, NULL, true);
        // If the text is empty (e.g., there is no valid selection or the backend
        // doesn't (properly) support selectedText()) we don't overwrite the
        // clipboard
        if (!textToCopy.isEmpty()) {
          Q_ASSERT(QApplication::clipboard() != NULL);
          QApplication::clipboard()->setText(textToCopy);
        }
      }
      else {
        // Inform the user that extracting text is not allowed
        // TODO: Add hint to unlock document w/ password, once we allow to
        // provide a password to an unlocked document (i.e., one which we can
        // display, but for which we don't have author's privileges)
        QMessageBox::information(_parent, ::QtPDF::PDFDocumentView::trUtf8("Insufficient permission"), ::QtPDF::PDFDocumentView::trUtf8("Text extraction is not allowed for this document."));
      }
    }
  }
}

void Select::resetBoxes(const int pageNum /* = -1 */)
{
  _pageNum = pageNum;
  _boxes.clear();
#ifdef DEBUG
  // In debug builds, remove any previously shown (selectable) boxes
  foreach(QGraphicsRectItem * rectItem, _displayBoxes) {
    if (!rectItem)
      continue;
    delete rectItem;
  }
  _displayBoxes.clear();
#endif
  
  Q_ASSERT(_parent != NULL);
  PDFDocumentScene * scene = static_cast<PDFDocumentScene*>(_parent->scene());
  Q_ASSERT(scene != NULL);
  QSharedPointer<Backend::Document> doc(scene->document().toStrongRef());
  if (!doc)
    return;
  
  QSharedPointer<Backend::Page> page(doc->page(pageNum).toStrongRef());
  if (page.isNull())
    return;

  PDFPageGraphicsItem * pageGraphicsItem = static_cast<PDFPageGraphicsItem*>(scene->pageAt(pageNum));
  Q_ASSERT(pageGraphicsItem != NULL);
  
  _boxes = page->boxes();
#ifdef DEBUG
  // In debug builds, show all selectable boxes
  QTransform toView = pageGraphicsItem->pointScale();  
  foreach(Backend::Page::Box b, _boxes) {
    QGraphicsRectItem * rectItem;
    if (b.subBoxes.isEmpty()) {
      rectItem = scene->addRect(toView.mapRect(b.boundingBox), QPen(_highlightColor));
      rectItem->setParentItem(pageGraphicsItem);
      _displayBoxes << rectItem;
    }
    else {
      foreach(Backend::Page::Box sb, b.subBoxes) {
        rectItem = scene->addRect(toView.mapRect(sb.boundingBox), QPen(_highlightColor));
        rectItem->setParentItem(pageGraphicsItem);
        _displayBoxes << rectItem;
      }
    }
  }
#endif // DEBUG
}

void Select::pageDestroyed()
{
  _highlightPath = NULL;
#ifdef DEBUG
  _displayBoxes.clear();
#endif
  resetBoxes(-1);
}

void Select::setHighlightColor(const QColor & color)
{
  _highlightColor = color;
  if (_highlightPath)
    _highlightPath->setBrush(color);

#ifdef DEBUG
  // In debug builds, update the display of selectable boxes
  foreach (QGraphicsRectItem * b, _displayBoxes) {
    if (!b)
      continue;
    b->setPen(color);
  }
#endif
}

} // namespace DocumentTool
} // namespace QtPDF

// vim: set sw=2 ts=2 et

