/* omega-miktex.cpp:
   
   Copyright (C) 2007-2017 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include "omega-miktex.h"

using namespace MiKTeX::Core;

// special case: Web2C likes to add 1 to the nameoffile base address
inline char* GetNameOfFileForWeb2C()
{
  return &OMEGAPROG.nameoffile[-1];
}

C4P::C4P_signed32& first = OMEGAPROG.first;
OMEGAPROGCLASS::memoryword**& fonttables = OMEGAPROG.fonttables;
OMEGAPROGCLASS::memoryword**& fontsorttables = OMEGAPROG.fontsorttables;
C4P::C4P_signed32& last = OMEGAPROG.last;
C4P::C4P_signed16& namelength = OMEGAPROG.namelength;
C4P::C4P_integer& ocpbufsize = OMEGAPROG.ocpbufsize;
C4P::C4P_integer**& ocptables = OMEGAPROG.ocptables;
OMEGAPROGCLASS::quarterword* otpinputbuf = OMEGAPROG.otpinputbuf;
OMEGAPROGCLASS::halfword& otpinputend = OMEGAPROG.otpinputend;
OMEGAPROGCLASS::quarterword* otpoutputbuf = OMEGAPROG.otpoutputbuf;
OMEGAPROGCLASS::halfword& otpoutputend = OMEGAPROG.otpoutputend;

char* nameoffile = GetNameOfFileForWeb2C();
