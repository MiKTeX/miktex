/**
 * Copyright (C) 2020  Charlie Sharpsteen, Stefan Löffler
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
#ifndef PDFToC_H
#define PDFToC_H

#include "PDFActions.h"

#include <QColor>
#include <QString>

namespace QtPDF {

namespace Backend {

class PDFToCItem
{
public:
  enum PDFToCItemFlag { Flag_Italic = 0x1, Flag_Bold = 0x2 };
  Q_DECLARE_FLAGS(PDFToCItemFlags, PDFToCItemFlag)

  PDFToCItem(const QString label = QString()) : _label(label) { }
  PDFToCItem(const PDFToCItem & o);
  virtual ~PDFToCItem();
  PDFToCItem & operator=(const PDFToCItem & o);

  QString label() const { return _label; }
  bool isOpen() const { return _isOpen; }
  PDFAction * action() const { return _action; }
  QColor color() const { return _color; }
  const QList<PDFToCItem> & children() const { return _children; }
  QList<PDFToCItem> & children() { return _children; }
  PDFToCItemFlags flags() const { return _flags; }
  PDFToCItemFlags & flags() { return _flags; }

  void setLabel(const QString label) { _label = label; }
  void setOpen(const bool isOpen = true) { _isOpen = isOpen; }
  void setAction(PDFAction * action);
  void setColor(const QColor color) { _color = color; }

  bool operator==(const PDFToCItem & o) const;

protected:
  QString _label;
  bool _isOpen{false}; // derived from the sign of the `Count` member of the outline item dictionary
  PDFAction * _action{nullptr}; // if the `Dest` member of the outline item dictionary is set, it must be converted to a PDFGotoAction
  QColor _color;
  QList<PDFToCItem> _children;
  PDFToCItemFlags _flags;
};

using PDFToC = QList<PDFToCItem>;

} // namespace Backend

} // namespace QtPDF

#endif // !defined(PDFToC_H)
