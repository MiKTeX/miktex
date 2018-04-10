/* miktex/asy.cpp:

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

#include "asy-first.h"
#include "asy.h"

namespace gl
{
  void updateHandler(int);
  void exportHandler(int);
}

MIKTEX_BEGIN_NS;

std::atomic_bool exportRequested = false;
std::atomic_bool updateRequested = false;
std::atomic_bool* sigusr1 = nullptr;
std::atomic_bool exitRequested = false;

void RequestHandler()
{
  if (exitRequested)
  {
    throw 0;
  }
  if (updateRequested)
  {
    gl::updateHandler(0);
    updateRequested = false;
  }
  if (exportRequested)
  {
    gl::exportHandler(0);
    exportRequested = false;
  }
}

MIKTEX_END_NS;
