/* internal.h:                                          -*- C++ -*-

   Copyright (C) 2002-2016 Christian Schenk

   This file is part of MkFntMap.

   MkFntMap is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   MkFntMap is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MkFntMap; if not, write to the Free Software Foundation,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include <cstdarg>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>

#include "mkfntmap-version.h"

#include <miktex/App/Application>

#include <miktex/Core/CSVList>
#include <miktex/Core/Cfg>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Directory>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/FileType>
#include <miktex/Core/Fndb>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/StreamReader>
#include <miktex/Core/StreamWriter>
#include <miktex/Core/Quoter>
#include <miktex/Core/Session>

#include <miktex/Util/Tokenizer>

#include <miktex/Wrappers/PoptWrapper>

#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/rollingfileappender.h>

#define UNUSED_ALWAYS(x)

#define T_(x) MIKTEXTEXT(x)
#define Q_(x) MiKTeX::Core::Quoter<char>(x).Get()

#define BOOLSTR(b) ((b) ? "true" : "false")

#define VA_START(arglist, lpszFormat   )        \
va_start(arglist, lpszFormat);                  \
try                                             \
{

#define VA_END(arglist)                         \
}                                               \
catch(...)                                      \
{                                               \
  va_end(arglist);                              \
  throw;                                        \
}                                               \
va_end(arglist);
