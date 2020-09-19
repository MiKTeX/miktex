/* vfchar.cpp:

   Copyright (C) 1996-2020 Christian Schenk

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

#include "config.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "internal.h"

class DviFont;

VfChar::VfChar(DviFont* dviFont) :
  DviChar(dviFont),
  trace_vfchar(TraceStream::Open(MIKTEX_TRACE_DVIVFCHAR, dviFont->GetTraceCallback()))
{
}

VfChar::~VfChar()
{
  try
  {
    if (packet != nullptr)
    {
      if (packetSize > sizeof(smallPacket))
      {
        delete[] packet;
      }
      packet = nullptr;
      packetSize = 0;
    }
    if (trace_vfchar != nullptr)
    {
      trace_vfchar->Close();
      trace_vfchar = nullptr;
    }
  }
  catch (const exception&)
  {
  }
}

const short long_char = 242;

void VfChar::Read(InputStream& inputstream, int size, double conv)
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
  trace_vfchar->WriteLine("libdvi", fmt::format("pl: {0}", packetSize));
  trace_vfchar->WriteLine("libdvi", fmt::format("cc: {0}", charCode));
  trace_vfchar->WriteLine("libdvi", fmt::format("tfm: {0}", tfm));

  tfm = ScaleFix(tfm, dviFont->GetScaledAt());
  cx = static_cast<int>(tfm * conv + 0.5);

  trace_vfchar->WriteLine("libdvi", fmt::format("dx: {0}", cx));

  if (packetSize <= sizeof(smallPacket))
  {
    packet = smallPacket;
  }
  else
  {
    packet = new BYTE[packetSize];
  }

  inputstream.Read(packet, packetSize);
}
