/* miktex.cpp

   Copyright (C) 2015-2018 Christian Schenk

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

#include <cstdio>
#include <miktex/Core/Session>

using namespace MiKTeX::Core;

MIKTEX_BEGIN_EXTERN_C_BLOCK;

int paper_size(const char* paper_name, double* width, double* height)
{
  std::shared_ptr<Session> session = Session::Get();
  PaperSizeInfo paperSize;
  if (paper_name == 0)
  {
    session->GetPaperSizeInfo(0, paperSize);
  }
  else
  {
    paperSize = session->GetPaperSizeInfo(paper_name);
  }
  *width = (double)paperSize.width;
  *height = (double)paperSize.height;
  return 1;
}

void verror(int status, int errnum, const char* format, va_list args)
{
  vfprintf(stderr, format, args);
  fputc('\n', stderr);
  throw status;
}

char* program_name;

void set_program_name(const char* argv0)
{
  std::shared_ptr<Session> session = Session::Get();
  program_name = strdup(argv0);
  session->PushAppName("psutils");
}

MIKTEX_END_EXTERN_C_BLOCK;
