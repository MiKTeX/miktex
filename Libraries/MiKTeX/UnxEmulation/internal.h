/* internal.h: internal definitions			-*- C++ -*-

   Copyright (C) 2007-2016 Christian Schenk

   This file is part of the MiKTeX UNXEMU Library.

   The MiKTeX UNXEMU Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX UNXEMU Library is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX UNXEMU Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#define MIKTEXUNXEXPORT MIKTEXDLLEXPORT

#define D2A2BA842ACE40C6A8A17A9358F2147E
#include "miktex/unxemu.h"

#include <miktex/Core/Exceptions>

#define T_(x) MIKTEXTEXT(x)

#define MIKTEXSTATICFUNC(type) static type
#define MIKTEXINTERNALFUNC(type) type
#define MIKTEXINTERNALVAR(type) type

#define C_FUNC_BEGIN()				\
  {						\
    try						\
      {

#define C_FUNC_END()					\
      }							\
    catch (const MiKTeX::Core::MiKTeXException & e)	\
      {							\
        if (stderr != 0)				\
	  {						\
	    Utils::PrintException (e);			\
	  }						\
        exit (1);					\
      }							\
    catch (const std::exception & e)			\
      {							\
        if (stderr != 0)				\
	  {						\
	    Utils::PrintException (e);			\
	  }						\
        exit (1);					\
      }							\
  }
