/**
 * Copyright (C) 2022  Stefan Löffler
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
#include "PDFGuideline.h"

#include "GuidelineEditDialog.h"
#include "PDFDocumentView.h"

namespace QtPDF {

PDFGuideline::PDFGuideline(PDFDocumentView * parent, const int posWin, const Qt::Orientation orientation)
  : QWidget(parent), m_parent(parent), m_orientation(orientation)
{
  if (parent) {
    m_pageIdx = parent->currentPage();
  }
  switch (m_orientation) {
    case Qt::Horizontal:
      setCursor(Qt::SplitVCursor);
      break;
    case Qt::Vertical:
      setCursor(Qt::SplitHCursor);
      break;
  }
  setPosWin(posWin);
  connect(m_parent, &PDFDocumentView::updated, this, &PDFGuideline::updatePosition);
  show();
}

PDFGuideline::PDFGuideline(PDFDocumentView * parent, const QPoint posWin, const Qt::Orientation orientation)
  : PDFGuideline(parent, (orientation == Qt::Horizontal ? posWin.y() : posWin.x()), orientation)
{}

void PDFGuideline::updatePosition()
{
  if (!m_parent)
    return;

  setPosPage(m_posPage);
}

void PDFGuideline::dragMove(const QPoint pos)
{
  setPosWin(pos);
}

void PDFGuideline::dragStop(const QPoint pos)
{
  if (!m_parent || !m_parent->viewport())
    return;

  const QRect contentsRect = viewContentRect();

  setPosWin(pos);

  // If the guide line was dragged back to the ruler (or out of the window),
  // delete the guide
  if ((m_orientation == Qt::Horizontal && m_posWin < contentsRect.top()) ||
      (m_orientation == Qt::Vertical && m_posWin < contentsRect.left()))
    deleteLater();
}

void PDFGuideline::setPosWin(const int pos)
{
  m_posWin = pos;
  m_posPage = mapToPage(pos);
  moveAndResize();
}

void PDFGuideline::setPosWin(const QPoint pt)
{
  switch(m_orientation) {
    case Qt::Horizontal: setPosWin(pt.y()); break;
    case Qt::Vertical: setPosWin(pt.x()); break;
  }
}

void PDFGuideline::setPosPage(const qreal pos)
{
  m_posPage = pos;
  m_posWin = mapFromPage(pos);
  moveAndResize();
}

void PDFGuideline::setPosPage(const QPointF pt)
{
  switch(m_orientation) {
    case Qt::Horizontal: setPosPage(pt.y()); break;
    case Qt::Vertical: setPosPage(pt.x()); break;
  }
}

void PDFGuideline::setPage(const int page)
{
  m_pageIdx = page;
  setPosPage(m_posPage);
}

void PDFGuideline::paintEvent(QPaintEvent * event)
{
  Q_UNUSED(event);

  QPainter painter{this};

  painter.setPen(QPen(Qt::blue, 0));
  QRect contentRect = viewContentRect();

  const QTransform bp2px = m_parent->ruler()->pagePx2Bp(page()).inverted();

  // NB: Only draw the line if it is over the viewport, not if it over the ruler
  // (hiding the widget when the line is over the ruler is not an option as
  // hiding it while dragging ends the drag, so the line could not be dragged
  // back down)
  switch (m_orientation) {
    case Qt::Horizontal:
      if (m_posWin >= contentRect.top()) {
        const qreal y = bp2px.map(QPointF(0, m_posPage)).y() - geometry().top();
        painter.drawLine(QPointF(0, y), QPointF(width(), y));
      }
      break;
    case Qt::Vertical:
      if (m_posWin >= contentRect.left()) {
        const qreal x = bp2px.map(QPointF(m_posPage, 0)).x() - geometry().left();
        painter.drawLine(QPointF(x, 0), QPointF(x, height()));
      }
      break;
  }
}

void PDFGuideline::mousePressEvent(QMouseEvent * event)
{
  QWidget::mousePressEvent(event);

  if (event->button() != Qt::LeftButton)
    return;
  m_mouseDownPos = event->pos();
}

void PDFGuideline::mouseMoveEvent(QMouseEvent *event)
{
  QWidget::mouseMoveEvent(event);
  // NB: as mouse tracking is not enabled for this widget, we only receive mouse
  // move events if a mouse button is held

  if (!event->buttons().testFlag(Qt::LeftButton))
    return;
  if (!m_parent)
    return;

  if (!m_isDragging && (m_mouseDownPos - event->pos()).manhattanLength() >= QApplication::startDragDistance())
    m_isDragging = true;

  if (m_isDragging) {
    dragMove(m_parent->mapFromGlobal(event->globalPos()));
    event->accept();
  }
}

void PDFGuideline::mouseReleaseEvent(QMouseEvent *event)
{
  QWidget::mouseReleaseEvent(event);

  if (event->button() != Qt::LeftButton)
    return;

  if (m_isDragging) {
    m_isDragging = false;
    dragStop(m_parent->mapFromGlobal(event->globalPos()));
    event->accept();
  }
}

void PDFGuideline::mouseDoubleClickEvent(QMouseEvent *event)
{
  Q_UNUSED(event)

  GuidelineEditDialog dlg(this);

  QSharedPointer<const Backend::Document> doc = m_parent->document().toStrongRef();

  dlg.setNumPages(doc->numPages());
  dlg.setGuidelinePage(page() + 1);
  dlg.setGuidelinePos(Physical::Length(posPage(), Physical::Length::Bigpoints));

  if (dlg.exec() == QDialog::Accepted) {
    setPage(dlg.guidelinePage() - 1);
    setPosPage(dlg.guidelinePos().val(Physical::Length::Bigpoints));
  }
}

void PDFGuideline::moveAndResize()
{
  const QRect contentsRect = viewContentRect();
  QRect newGeometry;
  switch (m_orientation) {
    case Qt::Horizontal:
      newGeometry = QRect(contentsRect.left(), m_posWin - padding, contentsRect.width(), 2 * padding + 1);
      break;
    case Qt::Vertical:
      newGeometry = QRect(m_posWin - padding, contentsRect.top(), 2 * padding + 1, contentsRect.height());
      break;
  }
  if (newGeometry != geometry())
    setGeometry(newGeometry);
}

int PDFGuideline::mapFromPage(const qreal pos) const
{
  if (!m_parent)
    return {};
  const QWidget * viewport = m_parent->viewport();
  if (!viewport)
    return {};
  const PDFDocumentScene * scene = qobject_cast<PDFDocumentScene*>(m_parent->scene());
  if (!scene)
    return {};
  PDFPageGraphicsItem * page = dynamic_cast<PDFPageGraphicsItem*>(scene->pageAt(m_pageIdx));
  if (!page)
    return {};

  const QTransform upsideDown = QTransform::fromTranslate(0, page->pageSizeF().height()).scale(1, -1);
  const QPointF ptPageItem = upsideDown.map(page->mapFromPage(QPointF(pos, pos)));
  const QPoint ptViewport = m_parent->mapFromScene(page->mapToScene(ptPageItem));
  const QPoint ptWin = m_parent->mapFromGlobal(viewport->mapToGlobal(ptViewport));
  switch (m_orientation) {
    case Qt::Horizontal:
      return ptWin.y();
    case Qt::Vertical:
      return ptWin.x();
  }
  return 0;
}

qreal PDFGuideline::mapToPage(const int pos) const
{
  if (!m_parent)
    return {};
  const QWidget * viewport = m_parent->viewport();
  if (!viewport)
    return {};
  const PDFDocumentScene * scene = qobject_cast<PDFDocumentScene*>(m_parent->scene());
  if (!scene)
    return {};
  PDFPageGraphicsItem * page = dynamic_cast<PDFPageGraphicsItem*>(scene->pageAt(m_pageIdx));
  if (!page)
    return {};

  const QTransform upsideDown = QTransform::fromTranslate(0, page->pageSizeF().height()).scale(1, -1);
  const QPoint ptViewport = viewport->mapFromGlobal(m_parent->mapToGlobal(QPoint(pos, pos)));
  const QPointF ptPageItem = page->mapFromScene(m_parent->mapToScene(ptViewport));
  const QPointF ptPage = page->mapToPage(upsideDown.map(ptPageItem));

  switch (m_orientation) {
    case Qt::Horizontal:
      return ptPage.y();
    case Qt::Vertical:
      return ptPage.x();
  }
  return 0;
}

QRect PDFGuideline::viewContentRect() const
{
  if (!m_parent)
    return {};

  QRect r = m_parent->contentsRect();
  if (m_parent->isRulerVisible())
    return r.marginsRemoved({PDFRuler::rulerSize, PDFRuler::rulerSize, 0, 0});
  else
    return r;
}

} // namespace QtPDF
