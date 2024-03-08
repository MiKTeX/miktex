/**
 * Copyright (C) 2023  Stefan Löffler
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
#ifndef PDFPageLayout_H
#define PDFPageLayout_H

#include <QList>
#include <QObject>

namespace QtPDF {

class PDFPageGraphicsItem;

// Cannot use QGraphicsGridLayout and similar classes for pages because it only
// works for QGraphicsLayoutItem (i.e., QGraphicsWidget)
class PDFPageLayout : public QObject {
  Q_OBJECT
  struct LayoutItem {
    PDFPageGraphicsItem * page;
    int row;
    int col;
  };

  QList<LayoutItem> _layoutItems;
  int _numCols{1};
  int _firstCol{0};
  qreal _xSpacing{10}; // spacing in pixel @ zoom=1
  qreal _ySpacing{10};
  bool _isContinuous{true};

public:
  PDFPageLayout() = default;
  ~PDFPageLayout() override = default;
  int columnCount() const { return _numCols; }
  int firstColumn() const { return _firstCol; }
  qreal xSpacing() const { return _xSpacing; }
  qreal ySpacing() const { return _ySpacing; }
  bool isContinuous() const { return _isContinuous; }
  void setContinuous(const bool continuous = true);

  void setColumnCount(const int numCols);
  void setColumnCount(const int numCols, const int firstCol);
  void setFirstColumn(const int firstCol);
  void setXSpacing(const qreal xSpacing);
  void setYSpacing(const qreal ySpacing);
  int rowCount() const;

  void addPage(PDFPageGraphicsItem * page);
  void removePage(PDFPageGraphicsItem * page);
  void insertPage(PDFPageGraphicsItem * page, PDFPageGraphicsItem * before = nullptr);
  void clearPages() { _layoutItems.clear(); }

public slots:
  void relayout();

signals:
  void layoutChanged(const QRectF sceneRect);

private:
  void rearrange();
  void continuousModeRelayout();
  void singlePageModeRelayout();
};

} // namespace QtPDF

#endif // !defined(PDFPageLayout_H)
