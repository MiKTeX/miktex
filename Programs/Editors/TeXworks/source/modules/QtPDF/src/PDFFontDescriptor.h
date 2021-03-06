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

#ifndef PDFFontDescriptor_H
#define PDFFontDescriptor_H

#include <QFlags>
#include <QRectF>
#include <QString>

namespace QtPDF {

namespace Backend {

class PDFFontDescriptor
{
public:
  enum FontStretch { FontStretch_UltraCondensed, FontStretch_ExtraCondensed, \
                     FontStretch_Condensed, FontStretch_SemiCondensed, \
                     FontStretch_Normal, FontStretch_SemiExpanded, \
                     FontStretch_Expanded, FontStretch_ExtraExpanded, \
                     FontStretch_UltraExpanded };
  enum Flag { Flag_FixedPitch = 0x01, Flag_Serif = 0x02, Flag_Symbolic = 0x04, \
              Flag_Script = 0x08, Flag_Nonsymbolic = 0x20, Flag_Italic = 0x40, \
              Flag_AllCap = 0x10000, Flag_SmallCap = 0x20000, \
              Flag_ForceBold = 0x40000 };
  Q_DECLARE_FLAGS(Flags, Flag)

  PDFFontDescriptor(const QString & fontName = QString());
  virtual ~PDFFontDescriptor() = default;

  bool isSubset() const;

  QString name() const { return _name; }
  // pureName() removes the subset tag
  QString pureName() const;

  void setName(const QString name) { _name = name; }
  // TODO: Accessor methods for all other properties

  bool operator==(const PDFFontDescriptor & o) const;

protected:
  // From pdf specs
  QString _name;
  QString _family;
  enum FontStretch _stretch{FontStretch_Normal};
  int _weight{400};
  Flags _flags;
  QRectF _bbox;
  float _italicAngle{0};
  float _ascent{0};
  float _descent{0};
  float _leading{0};
  float _capHeight{0};
  float _xHeight{0};
  float _stemV{0};
  float _stemH{0};
  float _avgWidth{0};
  float _maxWidth{0};
  float _missingWidth{0};
  QString _charSet;

  // From pdf specs for CID fonts only
  // _style
  // _lang
  // _fD
  // _CIDSet
};

} // namespace Backend

} // namespace QtPDF

#endif // !defined(PDFFontDescriptor_H)
