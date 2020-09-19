/**
 * Copyright (C) 2017-2020  Stefan Löffler
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
#ifndef __PAPER_SIZES_H
#define __PAPER_SIZES_H

#include <QString>
#include <QSizeF>
#include <QCoreApplication>

class PaperSize
{
  Q_DECLARE_TR_FUNCTIONS(PaperSize)


public:
  enum LengthUnit { Millimeters, Inches };
  PaperSize(const QString & name, const QSizeF & size, const LengthUnit lengthUnit = Millimeters, const bool rotatable = true, const QSizeF & tolerances = QSizeF());

  QString label() const;
  bool landscape() const { return _landscape; }
  void setLandscape(const bool landscape = true) { if (_rotatable) _landscape = landscape; }

  static PaperSize findForMillimeter(const QSizeF & paperSize);
  static PaperSize findForPDFSize(const QSizeF & paperSize) { return findForMillimeter(paperSize * (25.4 / 72.)); }
  static PaperSize findForInch(const QSizeF & paperSize) { return findForMillimeter(paperSize * 25.4); }

  bool operator ==(const PaperSize & other) const;
protected:
  QString _name;
  QSizeF _size;
  QSizeF _tolerances;
  bool _rotatable;
  bool _landscape;
  LengthUnit _lengthUnit;
};

#endif // !defined(__PAPER_SIZES_H)
