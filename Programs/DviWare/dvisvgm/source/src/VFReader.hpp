/*************************************************************************
** VFReader.hpp                                                         **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2020 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef VFREADER_HPP
#define VFREADER_HPP

#include "MessageException.hpp"
#include "StreamReader.hpp"


struct VFException : public MessageException {
	explicit VFException (const std::string &msg) : MessageException(msg) {}
};

struct VFActions;

class VFReader : public StreamReader {
	using ApproveAction = bool (*)(int);
	public:
		explicit VFReader (std::istream &is) : StreamReader(is) {}
		VFActions* replaceActions (VFActions *a);
		bool executeAll ();
		bool executePreambleAndFontDefs ();
		bool executeCharDefs ();

	protected:
		int executeCommand (ApproveAction approve=nullptr);

		// the following methods represent the VF commands
		// they are called by executeCommand and should not be used directly
		void cmdPre ();
		void cmdPost ();
		void cmdShortChar (int pl);
		void cmdLongChar ();
		void cmdFontDef (int len);

	private:
		VFActions *_actions=nullptr; ///< actions to execute when reading a VF command
		double _designSize=0;  ///< design size of currently read VF in PS points
};

#endif
