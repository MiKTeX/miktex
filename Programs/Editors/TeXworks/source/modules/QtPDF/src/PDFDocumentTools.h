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
#ifndef PDFDocumentTools_H
#define PDFDocumentTools_H

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QCursor>
#include <QRubberBand>
#include <QComboBox>
#include <QGraphicsView>
#include <QGraphicsLineItem>
#include <QGraphicsProxyWidget>

#include "PDFBackend.h"

#ifdef DEBUG
  #include <QDebug>
#endif

namespace QtPDF {

class PDFDocumentView;
class PDFDocumentMagnifierView;

namespace DocumentTool {

class AbstractTool
{
  friend class QtPDF::PDFDocumentView;
public:
  enum Type { Tool_None, Tool_MagnifyingGlass, Tool_ZoomIn, Tool_ZoomOut, Tool_MarqueeZoom, Tool_Move, Tool_ContextMenu, Tool_ContextClick, Tool_Measure, Tool_Select };
  AbstractTool(PDFDocumentView * parent) : _parent(parent), _cursor(QCursor(Qt::ArrowCursor)) { }
  virtual ~AbstractTool() { }
  
  virtual Type type() const { return Tool_None; }
  virtual bool operator==(const AbstractTool & o) { return (type() == o.type()); }
protected:
  virtual void arm();
  virtual void disarm();

  // By default, key events will call the parent view's maybeArmTool(). Derived
  // classes that rely on key events can override this behavior to handle
  // certain key events itself.
  virtual void keyPressEvent(QKeyEvent *event);
  virtual void keyReleaseEvent(QKeyEvent *event);
  virtual void mousePressEvent(QMouseEvent * event);
  virtual void mouseMoveEvent(QMouseEvent * event) { }
  virtual void mouseReleaseEvent(QMouseEvent * event);
  virtual void paintEvent(QPaintEvent * event) { }

  PDFDocumentView * _parent;
  QCursor _cursor;
};

class ZoomIn : public AbstractTool
{
public:
  ZoomIn(PDFDocumentView * parent);
  virtual Type type() const { return Tool_ZoomIn; }
protected:
  virtual void arm() { AbstractTool::arm(); _started = false; }
  virtual void disarm() { AbstractTool::disarm(); _started = false; }

  virtual void mousePressEvent(QMouseEvent * event);
  virtual void mouseReleaseEvent(QMouseEvent * event);
  QPoint _startPos;
  bool _started;
};

class ZoomOut : public AbstractTool
{
public:
  ZoomOut(PDFDocumentView * parent);
  virtual Type type() const { return Tool_ZoomOut; }
protected:
  virtual void arm() { AbstractTool::arm(); _started = false; }
  virtual void disarm() { AbstractTool::disarm(); _started = false; }

  virtual void mousePressEvent(QMouseEvent * event);
  virtual void mouseReleaseEvent(QMouseEvent * event);
  QPoint _startPos;
  bool _started;
};

class MagnifyingGlass : public AbstractTool
{
public:
  enum MagnifierShape { Magnifier_Rectangle, Magnifier_Circle };
  MagnifyingGlass(PDFDocumentView * parent);
  virtual Type type() const { return Tool_MagnifyingGlass; }
  PDFDocumentMagnifierView * magnifier() { return _magnifier; }

  void setMagnifierShape(const MagnifierShape shape);
  void setMagnifierSize(const int size);

protected:
  virtual void arm() { AbstractTool::arm(); _started = false; }
  virtual void disarm() { AbstractTool::disarm(); _started = false; }

  virtual void mousePressEvent(QMouseEvent * event);
  virtual void mouseMoveEvent(QMouseEvent * event);
  virtual void mouseReleaseEvent(QMouseEvent * event);
  virtual void paintEvent(QPaintEvent * event);

  PDFDocumentMagnifierView * _magnifier;
  bool _started;
};

class MarqueeZoom : public AbstractTool
{
public:
  MarqueeZoom(PDFDocumentView * parent);
  virtual Type type() const { return Tool_MarqueeZoom; }
protected:
  virtual void arm() { AbstractTool::arm(); _started = false; }
  virtual void disarm() { AbstractTool::disarm(); _started = false; }

  virtual void mousePressEvent(QMouseEvent * event);
  virtual void mouseMoveEvent(QMouseEvent * event);
  virtual void mouseReleaseEvent(QMouseEvent * event);

  bool _started;
  QPoint _startPos;
  QRubberBand * _rubberBand;
};

class Move : public AbstractTool
{
public:
  Move(PDFDocumentView * parent);
  virtual Type type() const { return Tool_Move; }
protected:
  virtual void arm() { AbstractTool::arm(); _started = false; }
  virtual void disarm() { AbstractTool::disarm(); _started = false; }

  virtual void mousePressEvent(QMouseEvent * event);
  virtual void mouseMoveEvent(QMouseEvent * event);
  virtual void mouseReleaseEvent(QMouseEvent * event);

  bool _started;
  QPoint _oldPos;
  QCursor _closedHandCursor;
};

class ContextClick : public AbstractTool
{
public:
  ContextClick(PDFDocumentView * parent) : AbstractTool(parent), _started(false) { }
  virtual Type type() const { return Tool_ContextClick; }
protected:
  virtual void mousePressEvent(QMouseEvent * event);
  virtual void mouseReleaseEvent(QMouseEvent * event);
  bool _started;
};

class Measure;
class MeasureLine;

class MeasureLineGrip : public QGraphicsRectItem
{
  friend class Measure;
public:
  // pt should be 1 for the first handle, 2 for the second handle
  MeasureLineGrip(MeasureLine * parent, const int pt);
protected:
  virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
  virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  
  // to be called from friend class Measure during creation of the MeasureLine
  void mouseMove(const QPointF scenePos, const Qt::KeyboardModifiers modifiers);
  
  // 1 for the first handle, 2 for the second handle
  int _pt;
};

class MeasureLine : public QGraphicsLineItem
{
  friend class Measure;
public:
  MeasureLine(QGraphicsView * primaryView, QGraphicsItem * parent = NULL);
  virtual ~MeasureLine() { }

  void setLine(qreal x1, qreal y1, qreal x2, qreal y2) { setLine(QLineF(x1, y1, x2, y2)); }
  void setLine(QPointF p1, QPointF p2) { setLine(QLineF(p1, p2)); }
  void setLine(QLineF line);
  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
protected:
  void updateMeasurement();
  void updateMeasureBoxPos();
  
  virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
  virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  
  QComboBox * _measureBox;
  QGraphicsProxyWidget * _measureBoxProxy;
  MeasureLineGrip * _grip1, * _grip2;
  QMap<QString, float> _measures;
  QGraphicsView * _primaryView;
  QPointF _grabOffset;
};

class Measure : public AbstractTool
{
public:
  Measure(PDFDocumentView * parent);
  virtual Type type() const { return Tool_Measure; }
protected:
  virtual void mousePressEvent(QMouseEvent * event);
  virtual void mouseMoveEvent(QMouseEvent *event);
  virtual void mouseReleaseEvent(QMouseEvent * event);
  virtual void keyPressEvent(QKeyEvent *event);
  virtual void keyReleaseEvent(QKeyEvent *event);
  
  MeasureLine * _measureLine;
  bool _started;
  QPoint _startPos;
};

// Text selection tool
// Supports:
// - "line based" selection (i.e., selects all boxes from the box the mouse
//   press event occured over to the closest box to the mouse when it was
//   released (or the current position if the mouse button is still pressed);
//   relies on the ordering of boxes in the pdf file
// - marquee selection (selects all boxes inside a rectangle drawn by the user
// - Ctrl+C to copy selected text (if supported by backend)
//
// FIXME: When the document (or the current page) is changed, resetBoxes()
//        should be called to ensure we don't work with the wrong boxes
// TODO: Marquee selection is slow for large rectangles
// TODO: Handle selections spanning multiple pages
// TODO: possibly load boxes asynchronously
// TODO: possibly support Ctrl+A to select all, etc.
// TODO: possibly support image selection (like in Adobe Reader), e.g. using a
//       keyboard modifier
// TODO: when the application/widget loses focus, the highlight color should
//       (temporarily) be switched to QPalette::Inactive/QPalette::Highlight
// TOOO: when scrolling (e.g., with the mouse wheel) to a new page but not
//       moving the mouse, the boxes information is not updated
class Select : public AbstractTool
{
public:
  Select(PDFDocumentView * parent);
  virtual ~Select();
  virtual Type type() const { return Tool_Select; }
  
  QColor highlightColor() const { return _highlightColor; }
  void setHighlightColor(const QColor & color);
protected:
  virtual void disarm();
  virtual void mousePressEvent(QMouseEvent * event);
  virtual void mouseMoveEvent(QMouseEvent *event);
  virtual void mouseReleaseEvent(QMouseEvent * event);
  virtual void keyPressEvent(QKeyEvent *event);

  void resetBoxes(const int pageNum = -1);
  
  // The mouse mode depends on whether the LMB is pressed, and where/how it was
  // pressed initially (e.g., over a box, over a free area, etc.)
  enum MouseMode { MouseMode_None, MouseMode_MarqueeSelect, MouseMode_TextSelect, MouseMode_ImageSelect };

  bool _cursorOverBox;
  QPointF _startPos;
  QGraphicsPathItem * _highlightPath;
  MouseMode _mouseMode;
  QRubberBand * _rubberBand;
  QColor _highlightColor;

  int _pageNum;
  QList<Backend::Page::Box> _boxes;
  int _startBox, _startSubbox;
#ifdef DEBUG
  QList<QGraphicsRectItem*> _displayBoxes;
#endif
};

} // namepsace DocumentTool

} // namespace QtPDF

#endif // End header guard
// vim: set sw=2 ts=2 et

