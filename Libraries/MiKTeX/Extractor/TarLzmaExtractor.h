/* TarLzmaExtractor.h:                                  -*- C++ -*-

   Copyright (C) 2001-2016 Christian Schenk

   This file is part of MiKTeX Extractor.

   MiKTeX Extractor is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX Extractor is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Extractor; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(F440C9CE29044CDEB102336B49B6AA17)
#define F440C9CE29044CDEB102336B49B6AA17

#include "TarExtractor.h"

BEGIN_INTERNAL_NAMESPACE;

class TarLzmaExtractor : public TarExtractor
{
public:
  virtual void MIKTEXTHISCALL Extract(const MiKTeX::Core::PathName & tarlzmaPath, const MiKTeX::Core::PathName & destDir, bool makeDirectories, IExtractCallback * pCallback, const std::string & prefix);
};

END_INTERNAL_NAMESPACE;

#endif
