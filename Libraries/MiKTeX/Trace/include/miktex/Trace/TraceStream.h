/* miktex/Trace/TraceStream.h:                           -*- C++ -*-

   Copyright (C) 1996-2019 Christian Schenk

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

#if !defined(A727BE8FD70B4A9A8E03971A80D27A11)
#define A727BE8FD70B4A9A8E03971A80D27A11

#include "config.h"

#include <memory>
#include <string>
#include <vector>

#include "TraceCallback.h"

/// @namespace MiKTeX::Trace
/// @brief Tracing utilities.
MIKTEX_TRACE_BEGIN_NAMESPACE;

class MIKTEXNOVTABLE TraceStream
{
public:
  virtual MIKTEXTHISCALL ~TraceStream() noexcept = 0;

public:
  virtual void MIKTEXTHISCALL Close() = 0;

public:
  virtual bool MIKTEXTHISCALL IsEnabled(const std::string& facility) = 0;

public:
  static MIKTEXTRACECEEAPI(std::unique_ptr<TraceStream>) Open(const std::string& name, TraceCallback* callback);

public:
  static std::unique_ptr<TraceStream> Open(const std::string& name)
  {
    return Open(name, nullptr);
  }

public:
  static MIKTEXTRACECEEAPI(void) SetOptions(const std::vector<std::string>& options);

public:
  static MIKTEXTRACECEEAPI(void) SetOptions(const std::string& options);

public:
  // DEPRECATED
  static void SetTraceFlags(const std::string& options)
  {
    SetOptions(options);
  }

public:
  virtual void MIKTEXTHISCALL Write(const std::string& facility, const std::string& text) = 0;

public:
  virtual void MIKTEXTHISCALL WriteLine(const std::string& facility, const std::string& text) = 0;

public:
  virtual void MIKTEXCEECALL WriteFormattedLine(const std::string& facility, const char* format, ...) = 0;

public:
  virtual void MIKTEXTHISCALL VTrace(const std::string& facility, const std::string& format, va_list arglist) = 0;
};

MIKTEX_TRACE_END_NAMESPACE;

#endif
