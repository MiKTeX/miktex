/* config.h (created from config.h.cmake)               -*- C++ -*-

   Copyright (C) 2017-2019 Christian Schenk

   This file is part of IniTeXMF.

   IniTeXMF is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   IniTeXMF is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with IniTeXMF; if not, write to the Free Software Foundation,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#cmakedefine WITH_BOOTSTRAPPING 1

#cmakedefine WITH_KPSEWHICH 1
#cmakedefine WITH_MKTEXLSR 1
#cmakedefine WITH_TEXHASH 1
#cmakedefine WITH_TEXLINKS 1
#cmakedefine WITH_TEXDOC 1
#cmakedefine WITH_UPDMAP 1

#define MIKTEX_BOOTSTRAPPING_PACKAGES "${MIKTEX_BOOTSTRAPPING_PACKAGES}"

#if defined(MIKTEX_MACOS_BUNDLE)
#  define MIKTEX_MACOS_BUNDLE_NAME "${MIKTEX_MACOS_BUNDLE_NAME}"
#endif
