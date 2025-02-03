/**
 * Copyright (C) 2022-2023  Stefan Löffler
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

#ifndef PDFRuler_H
#define PDFRuler_H

#include "PhysicalUnits.h"

#include <QActionGroup>
#include <QMenu>
#include <QVector>
#include <QWidget>

namespace QtPDF {

class PDFDocumentView;

class PDFRuler : public QWidget
{
  Q_OBJECT
  friend class PDFGuideline;
public:
  constexpr static unsigned int rulerSize = 20;
  using size_type = QVector<int>::size_type;

  PDFRuler(PDFDocumentView * parent);

  Physical::Length::Unit unit() const { return m_Unit; }
  void setUnit(const Physical::Length::Unit & newUnit);

signals:
  void dragStart(QPoint point, Qt::Edge origin);
  void dragMove(QPoint point);
  void dragStop(QPoint point);

protected:
  void contextMenuEvent(QContextMenuEvent * event) override;
  void paintEvent(QPaintEvent * event) override;
  void resizeEvent(QResizeEvent * event) override;
  void mousePressEvent(QMouseEvent * event) override;
  void mouseMoveEvent(QMouseEvent * event) override;
  void mouseReleaseEvent(QMouseEvent * event) override;

  QRectF pageRectPx(const size_type pageIdx) const;
  QRectF pageRectBp(const size_type pageIdx) const;
  QTransform pagePx2Bp(const size_type pageIdx) const;

private:
  Physical::Length::Unit m_Unit{Physical::Length::Centimeters};
  QMenu m_contextMenu{this};
  QActionGroup * m_contextMenuActionGroup{new QActionGroup(this)};
  QVector<QAction*> m_unitActions;

  QPoint m_mouseDownPt;
  bool m_isDragging{false};
};

} // namespace QtPDF

#endif // defined(PDFRuler_H)
