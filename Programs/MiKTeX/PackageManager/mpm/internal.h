/* internal.h:                                          -*- C++ -*-

   Copyright (C) 2003-2016 Christian Schenk

   This file is part of MpmCon.

   MpmCon is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.
   
   MpmCon is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with MpmCon; if not, write to the Free Software Foundation,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#define Q_(x) MiKTeX::Core::Quoter<char>(x).Get()

#define T_(x) MIKTEXTEXT(x)

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
