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
#ifndef PDFGUIDELINE_H
#define PDFGUIDELINE_H

#include <QWidget>

namespace QtPDF {

class PDFDocumentView;
class PDFPageGraphicsItem;

class PDFGuideline : public QWidget
{
  Q_OBJECT
  constexpr static int padding = 2;

  PDFDocumentView * m_parent{nullptr};
  int m_pageIdx{-1};
  qreal m_posPage;
  int m_posWin;
  Qt::Orientation m_orientation{Qt::Horizontal};

  bool m_isDragging{false};
  QPoint m_mouseDownPos;

public:
  PDFGuideline(PDFDocumentView * parent, const int posWin, const Qt::Orientation orientation);
  PDFGuideline(PDFDocumentView * parent, const QPoint posWin, const Qt::Orientation orientation);

  void updatePosition();

  void dragMove(const QPoint pos);
  void dragStop(const QPoint pos);

  int posWin() const { return m_posWin; }
  qreal posPage() const { return m_posPage; }
  void setPosWin(const int pos);
  void setPosWin(const QPoint pt);
  void setPosPage(const qreal pos);
  void setPosPage(const QPointF pt);

  int page() const { return m_pageIdx; }
  void setPage(const int page);

protected:
  void paintEvent(QPaintEvent * event) override;
  void mousePressEvent(QMouseEvent * event) override;
  void mouseMoveEvent(QMouseEvent * event) override;
  void mouseReleaseEvent(QMouseEvent * event) override;
  void mouseDoubleClickEvent(QMouseEvent * event) override;

  void moveAndResize();

  int mapFromPage(const qreal pos) const;
  qreal mapToPage(const int pos) const;
  QRect viewContentRect() const;
};

} // namespace QtPDF

#endif // PDFGUIDELINE_H
