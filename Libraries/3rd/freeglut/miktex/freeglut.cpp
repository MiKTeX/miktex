/* freeglut.cpp:

   Copyright (C) 2018 Christian Schenk

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

#include <miktex/Util/StringUtil>

#include "freeglut.h"

using namespace MiKTeX::Util;

wchar_t* miktex_uw_strdup(const char* s)
{
  return _wcsdup(StringUtil::UTF8ToWideChar(s).c_str());
}
