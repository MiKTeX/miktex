/*************************************************************************
** DLLoader.h                                                           **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2016 Martin Gieseking <martin.gieseking@uos.de>   **
**                                                                      **
** This program is free software; you can redistribute it and/or        **
** modify it under the terms of the GNU General Public License as       **
** published by the Free Software Foundation; either version 3 of       **
** the License, or (at your option) any later version.                  **
**                                                                      **
** This program is distributed in the hope that it will be useful, but  **
** WITHOUT ANY WARRANTY; without even the implied warranty of           **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         **
** GNU General Public License for more details.                         **
**                                                                      **
** You should have received a copy of the GNU General Public License    **
** along with this program; if not, see <http://www.gnu.org/licenses/>. **
*************************************************************************/

#ifndef DVISVGM_DLLOADER_H
#define DVISVGM_DLLOADER_H

#ifdef __WIN32__
	#include <windows.h>
#else
	#include <dlfcn.h>
#endif


class DLLoader
{
	public:
		DLLoader (const char *dlname);
		virtual ~DLLoader ();
		bool loaded () const {return _handle != 0;}

	protected:
		DLLoader () : _handle(0) {}
		void* loadSymbol (const char *name);

	private:
#ifdef __WIN32__
		HINSTANCE _handle;
#else
		void *_handle;
#endif
};

#endif
