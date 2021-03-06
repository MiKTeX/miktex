/**
 * Copyright (C) 2013-2020  Stefan Löffler
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

#include "PDFBackend.h"

#ifdef DEBUG
  #include <QDebug>
#endif
#include <QComboBox>
#include <QCursor>
#include <QGraphicsLineItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPointer>
#include <QRubberBand>

// Class representing an owning pointer to a QObject
// When an OwningQObjectPointer is destroyed, the QObject it points to is
// destroyed as well. Unlike std::unique_ptr, OwningQObjectPointer gracefully
// handles the case that the managed QObject is destroyed externally (e.g. by
// calling QObject::deleteLater()).
template <class T>
class OwningQObjectPointer
{
  bool m_deleteLater;
  QPointer<T> m_p;

  OwningQObjectPointer(const OwningQObjectPointer &) = delete;
  OwningQObjectPointer<T> & operator=(const OwningQObjectPointer &) = delete;
public:
  OwningQObjectPointer(T * p, const bool deleteLater = false) : m_deleteLater(deleteLater), m_p(p) {}
  ~OwningQObjectPointer() noexcept {
    if (!m_deleteLater) {
      delete m_p;
    }
    else if(!m_p.isNull()) {
      m_p->deleteLater();
    }
  }
  OwningQObjectPointer(OwningQObjectPointer &&) = default;
  OwningQObjectPointer<T> & operator=(OwningQObjectPointer &&) = default;
  OwningQObjectPointer<T> & operator=(T * p) {
    QPointer<T> oldP = m_p;
    m_p = p;
    if (!m_deleteLater) {
      delete oldP;
    }
    else if (!oldP.isNull()) {
      oldP->deleteLater();
    }
    return *this;
  }

  T* data() const { return m_p.data(); }
  T* operator->() const { return data(); }
  T& operator*() const { return *data(); }
  operator T*() const { return data(); }
  bool isNull() const { return m_p.isNull(); }
  void clear() { m_p.clear(); }

  bool usesDeleteLater() const { return m_deleteLater; }
  void setDeleteLater(const bool deleteLater = true) { m_deleteLater = deleteLater; }
};


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
  virtual ~AbstractTool() = default;

  virtual Type type() const { return Tool_None; }
  virtual bool operator==(const AbstractTool & o) const { return (type() == o.type()); }
protected:
  // Copy/move c'tor and assignment operators should be protected to avoid
  // use outside the inheritance tree (which could result in slicing)
  AbstractTool(const AbstractTool &) = default;
  AbstractTool(AbstractTool &&) = default;
  AbstractTool & operator=(AbstractTool &) = default;
  AbstractTool & operator=(AbstractTool &&) = default;

  virtual void arm();
  virtual void disarm();

  // By default, key events will call the parent view's maybeArmTool(). Derived
  // classes that rely on key events can override this behavior to handle
  // certain key events itself.
  virtual void keyPressEvent(QKeyEvent * event);
  virtual void keyReleaseEvent(QKeyEvent * event);
  virtual void mousePressEvent(QMouseEvent * event);
  virtual void mouseMoveEvent(QMouseEvent * event) { Q_UNUSED(event) }
  virtual void mouseReleaseEvent(QMouseEvent * event);
  virtual void paintEvent(QPaintEvent * event) { Q_UNUSED(event) }

  QPointer<PDFDocumentView> _parent;
  QCursor _cursor;
};

class ZoomIn : public AbstractTool
{
public:
  ZoomIn(PDFDocumentView * parent);
  Type type() const override { return Tool_ZoomIn; }
protected:
  void arm() override { AbstractTool::arm(); _started = false; }
  void disarm() override { AbstractTool::disarm(); _started = false; }

  void mousePressEvent(QMouseEvent * event) override;
  void mouseReleaseEvent(QMouseEvent * event) override;
  QPoint _startPos;
  bool _started;
};

class ZoomOut : public AbstractTool
{
public:
  ZoomOut(PDFDocumentView * parent);
  Type type() const override { return Tool_ZoomOut; }
protected:
  void arm() override { AbstractTool::arm(); _started = false; }
  void disarm() override { AbstractTool::disarm(); _started = false; }

  void mousePressEvent(QMouseEvent * event) override;
  void mouseReleaseEvent(QMouseEvent * event) override;
  QPoint _startPos;
  bool _started;
};

class MagnifyingGlass : public AbstractTool
{
public:
  enum MagnifierShape { Magnifier_Rectangle, Magnifier_Circle };
  MagnifyingGlass(PDFDocumentView * parent);
  Type type() const override { return Tool_MagnifyingGlass; }
  PDFDocumentMagnifierView * magnifier() { return _magnifier; }

  void setMagnifierShape(const MagnifierShape shape);
  void setMagnifierSize(const int size);

protected:
  void arm() override { AbstractTool::arm(); _started = false; }
  void disarm() override { AbstractTool::disarm(); hide(); }

  void mousePressEvent(QMouseEvent * event) override;
  void mouseMoveEvent(QMouseEvent * event) override;
  void mouseReleaseEvent(QMouseEvent * event) override;
  void paintEvent(QPaintEvent * event) override;

  virtual void hide();

  OwningQObjectPointer<PDFDocumentMagnifierView> _magnifier;
  bool _started;
};

class MarqueeZoom : public AbstractTool
{
public:
  MarqueeZoom(PDFDocumentView * parent);
  Type type() const override { return Tool_MarqueeZoom; }
protected:
  void arm() override { AbstractTool::arm(); _started = false; }
  void disarm() override { AbstractTool::disarm(); _started = false; }

  void mousePressEvent(QMouseEvent * event) override;
  void mouseMoveEvent(QMouseEvent * event) override;
  void mouseReleaseEvent(QMouseEvent * event) override;

  bool _started;
  QPoint _startPos;
  OwningQObjectPointer<QRubberBand> _rubberBand;
};

class Move : public AbstractTool
{
public:
  Move(PDFDocumentView * parent);
  Type type() const override { return Tool_Move; }
protected:
  void arm() override { AbstractTool::arm(); _started = false; }
  void disarm() override { AbstractTool::disarm(); _started = false; }

  void mousePressEvent(QMouseEvent * event) override;
  void mouseMoveEvent(QMouseEvent * event) override;
  void mouseReleaseEvent(QMouseEvent * event) override;

  bool _started;
  QPoint _oldPos;
  QCursor _closedHandCursor;
};

class ContextClick : public AbstractTool
{
public:
  ContextClick(PDFDocumentView * parent) : AbstractTool(parent), _started(false) { }
  Type type() const override { return Tool_ContextClick; }
protected:
  void mousePressEvent(QMouseEvent * event) override;
  void mouseReleaseEvent(QMouseEvent * event) override;
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
  void hoverEnterEvent(QGraphicsSceneHoverEvent * event) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent * event) override;
  void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;

  // to be called from friend class Measure during creation of the MeasureLine
  void mouseMove(const QPointF scenePos, const Qt::KeyboardModifiers modifiers);

  // 1 for the first handle, 2 for the second handle
  int _pt;
};

class MeasureLine : public QGraphicsLineItem
{
  friend class Measure;
public:
  MeasureLine(QGraphicsView * primaryView, QGraphicsItem * parent = nullptr);

  void setLine(qreal x1, qreal y1, qreal x2, qreal y2) { setLine(QLineF(x1, y1, x2, y2)); }
  void setLine(QPointF p1, QPointF p2) { setLine(QLineF(p1, p2)); }
  void setLine(QLineF line);
  void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) override;
protected:
  void updateMeasurement();
  void updateMeasureBoxPos();

  void hoverEnterEvent(QGraphicsSceneHoverEvent * event) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent * event) override;
  void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;

  // _measureBox will be embedded in _measureBoxProxy, which in turn will be
  // parented to this object; so they will all be destroyed automatically by
  // ~QGraphicsItem()
  QComboBox * _measureBox;
  QGraphicsProxyWidget * _measureBoxProxy;
  // _grip1 & _grip2 are raw pointers, but they are parented to this object, so
  // they will automatically be destroyed by ~QGraphicsItem()
  MeasureLineGrip * _grip1, * _grip2;
  QMap<QString, float> _measures;
  QGraphicsView * _primaryView;
  QPointF _grabOffset;
};

class Measure : public AbstractTool
{
public:
  Measure(PDFDocumentView * parent);
  Type type() const override { return Tool_Measure; }
protected:
  void mousePressEvent(QMouseEvent * event) override;
  void mouseMoveEvent(QMouseEvent * event) override;
  void mouseReleaseEvent(QMouseEvent * event) override;
  void keyPressEvent(QKeyEvent * event) override;
  void keyReleaseEvent(QKeyEvent * event) override;

  // _measureLine will immediately be added to a QGraphicsScene, which takes
  // ownership. So we don't need a std::unqiue_ptr or similar
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
  friend class QtPDF::PDFDocumentView;
public:
  Select(PDFDocumentView * parent);
  Type type() const override { return Tool_Select; }

  QColor highlightColor() const { return _highlightColor; }
  void setHighlightColor(const QColor & color);
  bool isTextSelected() const { return (_highlightPath ? !_highlightPath->path().isEmpty() : false); }
  QString selectedText() const;

protected:
  void disarm() override;
  void mousePressEvent(QMouseEvent * event) override;
  void mouseMoveEvent(QMouseEvent * event) override;
  void mouseReleaseEvent(QMouseEvent * event) override;
  void keyPressEvent(QKeyEvent * event) override;

  void resetBoxes(const int pageNum = -1);
  // Call this to notify Select that the page graphics item it has been working
  // on has been destroyed so all pointers to graphics items should be
  // invalidated
  void pageDestroyed();

  // The mouse mode depends on whether the LMB is pressed, and where/how it was
  // pressed initially (e.g., over a box, over a free area, etc.)
  enum MouseMode { MouseMode_None, MouseMode_MarqueeSelect, MouseMode_TextSelect, MouseMode_ImageSelect };

  bool _cursorOverBox;
  QPointF _startPos;
  // _highlightPath will immediately be added to a QGraphicsScene, which takes
  // ownership. So we don't need a std::unqiue_ptr or similar
  QGraphicsPathItem * _highlightPath;
  MouseMode _mouseMode;
  OwningQObjectPointer<QRubberBand> _rubberBand;
  QColor _highlightColor;

  int _pageNum;
  QList<Backend::Page::Box> _boxes;
  int _startBox, _startSubbox;
#ifdef DEBUG
  // All elements of _displayBoxes are automatically added to a QGraphicsScene,
  // which takes ownership. So we don't need a std::unqiue_ptr or similar
  QList<QGraphicsRectItem*> _displayBoxes;
#endif
};

} // namepsace DocumentTool

} // namespace QtPDF

#endif // End header guard
// vim: set sw=2 ts=2 et

