/* vfchar.cpp:

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX DVI Library.

   The MiKTeX DVI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2, or (at your option) any later version.

   The MiKTeX DVI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the MiKTeX DVI Library; if not, write to the
   Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
   USA.  */

#include "StdAfx.h"

#include "internal.h"

class DviFont;

VfChar::VfChar(DviFont * pFont) :
  DviChar(pFont),
  trace_vfchar(TraceStream::Open(MIKTEX_TRACE_DVIVFCHAR))
{
}

VfChar::~VfChar()
{
  try
  {
    if (pPacket != nullptr)
    {
      if (packetSize > sizeof(smallPacket))
      {
        delete[] pPacket;
      }
      pPacket = nullptr;
      packetSize = 0;
    }
    if (trace_vfchar != nullptr)
    {
      trace_vfchar->Close();
      trace_vfchar = nullptr;
    }
  }
  catch (const exception &)
  {
  }
}

const short long_char = 242;

void VfChar::Read(InputStream & inputstream, int size, double conv)
{
  if (size == long_char)
  {
    packetSize = inputstream.ReadSignedQuad();
    charCode = inputstream.ReadSignedQuad();
    tfm = inputstream.ReadSignedQuad();
  }
  else
  {
    packetSize = size;
    charCode = inputstream.ReadByte();
    tfm = inputstream.ReadTrio();
  }

  trace_vfchar->WriteLine("libdvi", T_("going to read vf character packet"));
  trace_vfchar->WriteFormattedLine("libdvi", "pl: %d", packetSize);
  trace_vfchar->WriteFormattedLine("libdvi", "cc: %d", charCode);
  trace_vfchar->WriteFormattedLine("libdvi", "tfm: %d", tfm);

  tfm = ScaleFix(tfm, pDviFont->GetScaledAt());
  cx = static_cast<int>(tfm * conv + 0.5);

  trace_vfchar->WriteFormattedLine("libdvi", "dx: %d", cx);

  if (packetSize <= sizeof(smallPacket))
  {
    pPacket = smallPacket;
  }
  else
  {
    pPacket = new BYTE[packetSize];
  }

  inputstream.Read(pPacket, packetSize);
}
