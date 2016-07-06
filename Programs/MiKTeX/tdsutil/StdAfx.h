/* StdAfx.h: pre-compiled header stuff                  -*- C++ -*-

   Copyright (C) 2016 Christian Schenk

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
   USA.  */

#if defined(HAVE_CONFIG_H)
#  include <config.h>
#endif

#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

#include <miktex/App/Application>
#include <miktex/Core/Cfg>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>
#include <miktex/Core/Exceptions>
#include <miktex/Core/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Quoter>
#include <miktex/Core/StreamWriter>
#include <miktex/Core/TemporaryDirectory>
#include <miktex/Core/TemporaryFile>
#include <miktex/Util/StringUtil>
#include <miktex/Wrappers/PoptWrapper>

#include "tdsutil-version.h"
