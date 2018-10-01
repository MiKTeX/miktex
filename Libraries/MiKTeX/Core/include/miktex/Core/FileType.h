/* miktex/Core/FileType.h:                              -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#if !defined(AF701BE422BF486BBDFF271AD2E3CCDD)
#define AF701BE422BF486BBDFF271AD2E3CCDD

#include <miktex/Core/config.h>

MIKTEX_CORE_BEGIN_NAMESPACE;

/// File type enum class.
enum class FileType
{
  None = 0,
  /// AFM files (*.afm).
  AFM,
  /// METAFONT base files (*.base).
  BASE,
  /// BibTeX database files (*.bib).
  BIB,
  /// BibTeX style files (*.bst).
  BST,
  CID,
  CLUA,
  CMAP,
  CNF,
  CWEB,
  DB,
  DVI,
  DVIPSCONFIG,
  ENC,
  /// Executable files (*.exe).
  EXE,
  FEA,
  /// TeX format files (*.fmt).
  FMT,
  HBF,
  GLYPHFONT,
  GRAPHICS,
  GF,
  IST,
  LIG,
  LUA,
  MAP,
  MEM, // OBSOLETE
  MF,
  MFPOOL,
  MFT,
  MISCFONT,
  MLBIB,
  MLBST,
  MP,
  MPPOOL,
  MPSUPPORT,
  OCP,
  OFM,
  OPL,
  OTF,
  OTP,
  OVF,
  OVP,
  PDFTEXCONFIG,
  PK,
  PROGRAMBINFILE,
  PROGRAMTEXTFILE,
  PSHEADER,
  SCRIPT,
  SFD,
  TCX,
  /// TeX input files (*.tex).
  TEX,
  TEXPOOL,
  TEXSOURCE,
  /// TeX documentation files (*.dvi;*.pdf).
  TEXSYSDOC,
  /// TeX font metric files (*.tfm).
  TFM,
  TROFF,
  TTF,
  TYPE1,
  TYPE42,
  VF,
  WEB,
  WEB2C,
  E_N_D
};

MIKTEX_CORE_END_NAMESPACE;

#endif
