/* miktex/Trace/StopWatch.h:                            -*- C++ -*-

   Copyright (C) 1996-2018 Christian Schenk

   This file is part of the MiKTeX Trace Library.

   The MiKTeX Trace Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Trace Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Trace Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#if !defined(C29AD887CE844234A9E80C64B775E748)
#define C29AD887CE844234A9E80C64B775E748

#include "config.h"

#include <memory>
#include <string>

#include "TraceStream.h"

MIKTEX_TRACE_BEGIN_NAMESPACE;

class MIKTEXNOVTABLE StopWatch
{
public:
  virtual MIKTEXTHISCALL ~StopWatch() noexcept = 0;

public:
  virtual double MIKTEXTHISCALL Stop() = 0;

public:
  static MIKTEXTRACECEEAPI(std::unique_ptr<StopWatch>) Start();

public:
  static MIKTEXTRACECEEAPI(std::unique_ptr<StopWatch>) Start(TraceStream* traceStream, const std::string& facility, const std::string& message);
};

MIKTEX_TRACE_END_NAMESPACE;

#endif
