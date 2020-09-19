/*************************************************************************
** BasicDVIReader.hpp                                                   **
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

#ifndef BASICDVIREADER_HPP
#define BASICDVIREADER_HPP

#include "Matrix.hpp"
#include "MessageException.hpp"
#include "StreamReader.hpp"

struct DVIException : public MessageException {
	explicit DVIException (const std::string &msg) : MessageException(msg) {}
};


struct InvalidDVIFileException : public DVIException {
	explicit InvalidDVIFileException (const std::string &msg) : DVIException(msg) {}
};


/** This class provides the basic functionality to read a DVI file.
 *  It just skips all DVI commands and apply any semantic to it. The latter must
 *  be realized by deriving a separate class that implements the cmdXXX template
 *  methods. These are low-level functions that represent the DVI commands and
 *  require to read and evaluate the correct portion of data from the DVI stream.
 *  Since the DVI commands are almost skipped by advancing the file pointer,
 *  running through a DVI file is pretty fast. */
class BasicDVIReader : public StreamReader {
	protected:
		using CommandHandler = void (BasicDVIReader::*)(int);
		enum DVIVersion {DVI_NONE=0, DVI_STANDARD=2, DVI_PTEX=3, DVI_XDV5=5, DVI_XDV6=6, DVI_XDV7=7};
		const uint8_t OP_SETCHAR0=0, OP_SETCHAR127=127, OP_SET1=128, OP_BOP=139, OP_EOP=140, OP_FNTNUM0=171,
			OP_FNTNUM63=234, OP_PRE=247, OP_POST=248, OP_POSTPOST=249, OP_DIR=255, DVI_FILL=223;

	public:
		explicit BasicDVIReader (std::istream &is);
		virtual void executeAllPages ();
		virtual double getXPos () const      {return 0;}
		virtual double getYPos () const      {return 0;}
		virtual void finishLine ()           {}
		virtual void translateToX (double x) {}
		virtual void translateToY (double y) {}
		virtual int stackDepth () const   {return 0;}
		virtual Matrix getPageTransformation () const {return Matrix(1);}
		virtual unsigned currentPageNumber () const {return 0;}

	protected:
		void setDVIVersion (DVIVersion version);
		DVIVersion getDVIVersion () const {return _dviVersion;}
		virtual int evalCommand (CommandHandler &handler, int &param);
		virtual int executeCommand ();
		void executePostPost ();
		bool evalXDVOpcode (int op, CommandHandler &handler) const;

		// The following template methods represent the single DVI commands. They
		// must read the correct chunk of data from the input stream in order to
		// process the DVI file correctly.
		virtual void cmdSetChar0 (int c);
		virtual void cmdSetChar (int len);
		virtual void cmdPutChar (int len);
		virtual void cmdSetRule (int len);
		virtual void cmdPutRule (int len);
		virtual void cmdNop (int len);
		virtual void cmdBop (int len);
		virtual void cmdEop (int len);
		virtual void cmdPush (int len);
		virtual void cmdPop (int len);
		virtual void cmdDir (int len);
		virtual void cmdRight (int len);
		virtual void cmdDown (int len);
		virtual void cmdX0 (int len);
		virtual void cmdY0 (int len);
		virtual void cmdW0 (int len);
		virtual void cmdZ0 (int len);
		virtual void cmdX (int len);
		virtual void cmdY (int len);
		virtual void cmdW (int len);
		virtual void cmdZ (int len);
		virtual void cmdFontDef (int len);
		virtual void cmdFontNum0 (int n);
		virtual void cmdFontNum (int len);
		virtual void cmdXXX (int len);
		virtual void cmdPre (int len);
		virtual void cmdPost (int len);
		virtual void cmdPostPost (int len);
		virtual void cmdXFontDef (int len);       // XDV only
		virtual void cmdXGlyphArray (int len);    // XDV only
		virtual void cmdXGlyphString (int len);   // XDV version 5 only
		virtual void cmdXPic (int len);           // XDV version 5 only
		virtual void cmdXTextAndGlyphs (int len); // XDV version 7 only

	private:
		DVIVersion _dviVersion;  ///< DVI version of file being processed
};

#endif
