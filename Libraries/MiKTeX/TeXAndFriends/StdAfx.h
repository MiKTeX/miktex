/* StdAfx.h: pre-compiled header stuff                  -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX TeXMF Library.

   The MiKTeX TeXMF Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX TeXMF Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX TeXMF Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#include <cstdlib>
#include <ctime>

#include <iomanip>
#include <iostream>
#include <set>

#include <miktex/App/Application>

#include <miktex/Core/BufferSizes>
#include <miktex/Core/Cfg>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Directory>
#include <miktex/Core/File>
#include <miktex/Core/FileStream>
#include <miktex/Core/FileType>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Core/Registry>
#include <miktex/Core/StreamReader>

#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>

#include <miktex/Util/StringUtil>
#include <miktex/Util/Tokenizer>
#include <miktex/Util/inliners.h>

extern "C" {
#include <zlib.h>
#include <bzlib.h>
}
