/**
 * Copyright (C) 2023  Stefan Löffler, Charlie Sharpsteen
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
#ifndef PDFFontInfo_H
#define PDFFontInfo_H

#include <QFileInfo>

#include "PDFFontDescriptor.h"

namespace QtPDF {

namespace Backend {

// Note: This is a hack, but since all the information (with the exception of
// the type of font) we use (and that is provided by poppler) is encapsulated in
// PDFFontDescriptor, there is no use right now to completely implement all the
// different font structures
class PDFFontInfo
{
public:
  enum FontType { FontType_Type0, FontType_Type1, FontType_MMType1, \
                  FontType_Type3, FontType_TrueType };
  enum CIDFontType { CIDFont_None, CIDFont_Type0, CIDFont_Type2 };
  enum FontProgramType { ProgramType_None, ProgramType_Type1, \
                         ProgramType_TrueType, ProgramType_Type1CFF, \
                         ProgramType_CIDCFF, ProgramType_OpenType };
  enum FontSource { Source_Embedded, Source_File, Source_Builtin };

  PDFFontInfo() = default;
  virtual ~PDFFontInfo() = default;

  FontType fontType() const { return _fontType; }
  CIDFontType CIDType() const { return _CIDType; }
  FontProgramType fontProgramType() const { return _fontProgramType; }
  PDFFontDescriptor descriptor() const { return _descriptor; }
  // returns the path to the file used for rendering this font, or an invalid
  // QFileInfo for embedded fonts
  QFileInfo fileName() const { return _substitutionFile; }

  bool isSubset() const { return _descriptor.isSubset(); }
  FontSource source() const { return _source; }

  // TODO: Implement some advanced logic; e.g., non-embedded fonts have no font
  // program type
  void setFontType(const FontType fontType) { _fontType = fontType; }
  void setCIDType(const CIDFontType CIDType) { _CIDType = CIDType; }
  void setFontProgramType(const FontProgramType programType) { _fontProgramType = programType; }
  void setDescriptor(const PDFFontDescriptor & descriptor) { _descriptor = descriptor; }
  void setFileName(const QFileInfo & file) { _source = Source_File; _substitutionFile = file; }
  void setSource(const FontSource source) { _source = source; }

  bool operator==(const PDFFontInfo & o) const {
    return (_source == o._source && _descriptor == o._descriptor &&
      _substitutionFile == o._substitutionFile && _fontType == o._fontType &&
      _CIDType == o._CIDType && _fontProgramType == o._fontProgramType);
  }

protected:
  FontSource _source{Source_Builtin};
  PDFFontDescriptor _descriptor;
  QFileInfo _substitutionFile;
  FontType _fontType{FontType_Type1};
  CIDFontType _CIDType{CIDFont_None};
  FontProgramType _fontProgramType{ProgramType_None};
};

} // namespace Backend
} // namespace QtPDF

#endif // PDFFontInfo_H
