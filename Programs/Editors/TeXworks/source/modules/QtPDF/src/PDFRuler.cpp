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

#include "PDFRuler.h"

#include "PDFDocumentView.h"

#include <QPainter>
#include <QTransform>

Q_DECLARE_METATYPE(QtPDF::Physical::Length::Unit)

namespace QtPDF {

PDFRuler::PDFRuler(PDFDocumentView *parent)
  : QWidget(parent)
{
  connect(parent, &PDFDocumentView::updated, this, static_cast<void (PDFRuler::*)()>(&PDFRuler::update));

  auto createUnitAction = [&](const Physical::Length::Unit u) {
    QAction * action = new QAction(QString::fromUtf8(Physical::Length::unitSymbol(u)), this);
    action->setData(u);
    action->setCheckable(true);
    action->setChecked(u == unit());
    connect(action, &QAction::triggered, this, [this, u]() { setUnit(u); });
    m_contextMenu.addAction(action);
    m_contextMenuActionGroup->addAction(action);
    return action;
  };
  m_unitActions.append(createUnitAction(Physical::Length::Centimeters));
  m_unitActions.append(createUnitAction(Physical::Length::Inches));
  m_unitActions.append(createUnitAction(Physical::Length::Bigpoints));
}

void PDFRuler::setUnit(const Physical::Length::Unit & newUnit)
{
  if (m_Unit == newUnit)
    return;
  m_Unit = newUnit;
  Q_FOREACH(QAction * const a, m_unitActions) {
    Physical::Length::Unit u = a->data().value<Physical::Length::Unit>();
    a->setChecked(u == newUnit);
  }
  update();
}

void PDFRuler::contextMenuEvent(QContextMenuEvent * event)
{
  m_contextMenu.popup(event->globalPos());
}

void PDFRuler::paintEvent(QPaintEvent * event)
{
  Q_UNUSED(event)
  QPainter painter{this};

  // Clear background
  painter.setPen(Qt::black);
  painter.fillRect(rect(), Qt::lightGray);
  painter.drawLine(QPoint(rulerSize, rulerSize), QPointF(width(), rulerSize));
  painter.drawLine(QPoint(rulerSize, rulerSize), QPointF(rulerSize, height()));

  // Obtain pointer to the view and the page object
  PDFDocumentView * docView = qobject_cast<PDFDocumentView*>(parent());
  if (!docView)
    return;

  // Get rect in pixels
  const QRectF pageRect = pageRectPx(docView->currentPage());

  // Calculate transforms from px to physical units and back
  const QTransform px2pt = pagePx2Bp(docView->currentPage());
  const double pt2physFactor = Physical::Length::convert(1, Physical::Length::Bigpoints, unit());
  const QTransform px2phys = px2pt * QTransform::fromScale(pt2physFactor, pt2physFactor);
  const QTransform phys2px = px2phys.inverted();

  // Get the viewing rect in physical coordinates
  QRectF physRect = px2phys.mapRect(QRectF(QPointF(rulerSize, rulerSize), rect().bottomRight()));

  QFont font = QGuiApplication::font();
  font.setPixelSize(qFloor(rulerSize * 0.6));
  painter.setFont(font);

  // Draw unit label
  const QString unitLabel = QString::fromUtf8(Physical::Length::unitSymbol(unit()));
  painter.drawText(QRectF(0, 0, rulerSize, rulerSize), Qt::AlignCenter, unitLabel);

  const auto calcMajorInterval = [](const qreal physSpacing) {
    const qreal magnitude = qPow(10, qFloor(qLn(physSpacing) / qLn(10)));
    const qreal mantissa = physSpacing / magnitude; // in the interval [1, 10)
    if (mantissa < 2)
      return 1. * magnitude;
    else if (mantissa < 5)
      return 2 * magnitude;
    else
      return 5 * magnitude;
  };

  // TODO: make constexpr once we switch to C++17
  const auto roundZero = [](const qreal x) {
    // Round (very) small numbers to avoid 0 being displayed as something like
    // 1.23e-16
    constexpr double threshold = 8 * std::numeric_limits<qreal>::epsilon();
    if (qAbs(x) < threshold)
      return 0.;
    return x;
  };

  {
    // Horizontal ruler
    painter.save();
    painter.setClipRect(QRectF(QPointF(rulerSize, 0), QPointF(rect().right(), rulerSize)));
    painter.fillRect(QRectF(QPointF(pageRect.left(), 0), QPointF(pageRect.right(), rulerSize)), Qt::white);
    const qreal dxMajor = calcMajorInterval(px2phys.mapRect(QRectF(QPointF(0, 0), QSizeF(100, 100))).width());
    const int nMinor = 10;
    const qreal dxMinor = dxMajor / nMinor;
    const qreal xMin = qFloor(physRect.left() / dxMajor) * dxMajor;
    for (int i = 0; xMin + i * dxMinor <= physRect.right(); ++i) {
      const Qt::Alignment alignment = Qt::AlignHCenter | Qt::AlignTop;
      const qreal xPhys = roundZero(xMin + i * dxMinor);
      const qreal x = phys2px.map(QPointF(xPhys, 0)).x();
      if (i % nMinor == 0) {
        const QString label = QStringLiteral("%L1").arg(xPhys);
        painter.drawLine(QPointF(x, .7 * rulerSize), QPointF(x, rulerSize));
        QRectF boundingRect = painter.boundingRect(QRectF(QPointF(x, 0), QSizeF(0, rulerSize / 2)), alignment, label);
        painter.drawText(boundingRect, alignment, label);
      }
      else {
        painter.drawLine(QPointF(x, .85 * rulerSize), QPointF(x, rulerSize));
      }
    }
    painter.restore();
  }

  {
    // Vertical ruler
    painter.save();
    painter.setClipRect(QRectF(QPointF(0, rulerSize), QPointF(rulerSize, rect().bottom())));
    painter.fillRect(QRectF(QPointF(0, pageRect.top()), QPointF(rulerSize, pageRect.bottom())), Qt::white);
    painter.rotate(-90);
    const qreal dyMajor = calcMajorInterval(px2phys.mapRect(QRectF(QPointF(0, 0), QSizeF(100, 100))).height());
    const int nMinor = 10;
    const qreal dyMinor = dyMajor / nMinor;
    const qreal yMin = qFloor(physRect.top() / dyMajor) * dyMajor;
    for (int i = 0; yMin + i * dyMinor <= physRect.bottom(); ++i) {
      const Qt::Alignment alignment = Qt::AlignHCenter | Qt::AlignTop;
      const qreal yPhys = roundZero(yMin + i * dyMinor);
      const qreal y = phys2px.map(QPointF(0, yPhys)).y();
      if (y < rulerSize)
        continue;
      if (i % nMinor == 0) {
        const QString label = QStringLiteral("%L1").arg(yPhys);
        painter.drawLine(QPointF(-y, .7 * rulerSize), QPointF(-y, rulerSize));
        QRectF boundingRect = painter.boundingRect(QRectF(QPointF(-y, 0), QSizeF(0, rulerSize / 2)), alignment, label);
        painter.drawText(boundingRect, alignment, label);
      }
      else {
        painter.drawLine(QPointF(-y, .85 * rulerSize), QPointF(-y, rulerSize));
      }
    }
    painter.resetTransform();
    painter.restore();
  }
}

void PDFRuler::resizeEvent(QResizeEvent * event)
{
  QRegion mask;
  mask += QRect(QPoint(0, 0), QSize(width(), rulerSize + 1));
  mask += QRect(QPoint(0, 0), QSize(rulerSize + 1, height()));
  setMask(mask);
  QWidget::resizeEvent(event);
}

void PDFRuler::mousePressEvent(QMouseEvent * event)
{
  QWidget::mousePressEvent(event);
  if (event->isAccepted())
    return;
  if (event->buttons() == Qt::LeftButton) {
    m_mouseDownPt = event->pos();
    event->accept();
  }
}

void PDFRuler::mouseMoveEvent(QMouseEvent * event)
{
  QWidget::mouseMoveEvent(event);
  if (event->isAccepted())
    return;
  if (event->buttons() == Qt::LeftButton) {
    if (!m_isDragging) {
      const bool draggedFarEnough = (event->pos() - m_mouseDownPt).manhattanLength() >= QApplication::startDragDistance();
      if (draggedFarEnough) {
        const bool startedAtTop = QRect(QPoint(rulerSize, 0), QPoint(width(), rulerSize)).contains(m_mouseDownPt);
        const bool startedAtLeft = QRect(QPoint(0, rulerSize), QPoint(rulerSize, height())).contains(m_mouseDownPt);

        if (startedAtTop || startedAtLeft) {
          m_isDragging = true;
          emit dragStart(event->pos(), (startedAtTop ? Qt::TopEdge : Qt::LeftEdge));
        }
      }
    }
    else {
      emit dragMove(event->pos());
    }
    event->accept();
  }
}

void PDFRuler::mouseReleaseEvent(QMouseEvent * event)
{
  QWidget::mouseReleaseEvent(event);
  if (event->isAccepted())
    return;
  if (m_isDragging && event->button() == Qt::LeftButton) {
    m_isDragging = false;
    emit dragStop(event->pos());
    event->accept();
  }
}

QRectF PDFRuler::pageRectPx(const int pageIdx) const
{
  // Obtain pointer to the view and the page object
  PDFDocumentView * docView = qobject_cast<PDFDocumentView*>(parent());
  if (!docView)
    return {};
  PDFDocumentScene * scene = qobject_cast<PDFDocumentScene*>(docView->scene());
  if (!scene)
    return {};
  PDFPageGraphicsItem * page = dynamic_cast<PDFPageGraphicsItem*>(scene->pageAt(pageIdx));
  if (!page)
    return {};

  const QRectF rv = docView->mapFromScene(page->mapToScene(QRectF(QPointF(0, 0), page->pageSizeF()))).boundingRect();
  if (!isVisible())
    return rv;
  return rv.translated(rulerSize + 1, rulerSize + 1);
}

QRectF PDFRuler::pageRectBp(const int pageIdx) const
{
  // Obtain pointer to the view and the page object
  PDFDocumentView * docView = qobject_cast<PDFDocumentView*>(parent());
  if (!docView)
    return {};
  PDFDocumentScene * scene = qobject_cast<PDFDocumentScene*>(docView->scene());
  if (!scene)
    return {};
  PDFPageGraphicsItem * page = dynamic_cast<PDFPageGraphicsItem*>(scene->pageAt(pageIdx));
  if (!page)
    return {};

  return page->pointScale().inverted().mapRect(QRectF(QPointF(0, 0), page->pageSizeF()));
}

QTransform PDFRuler::pagePx2Bp(const int pageIdx) const
{
  const QRectF pageRect = pageRectPx(pageIdx);
  const QRectF pdfPageRect = pageRectBp(pageIdx);
  return QTransform::fromTranslate(-pageRect.left(), -pageRect.top()) * \
      QTransform::fromScale(pdfPageRect.width() / pageRect.width(), pdfPageRect.height() / pageRect.height()) * \
      QTransform::fromTranslate(pdfPageRect.left(), pdfPageRect.top());
}

} // namespace QtPDF
