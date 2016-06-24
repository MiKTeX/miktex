/*************************************************************************
** BasicDVIReader.h                                                     **
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

#ifndef DVISVGM_BASICDVIREADER_H
#define DVISVGM_BASICDVIREADER_H

#include "MessageException.h"
#include "StreamReader.h"

struct DVIException : public MessageException
{
	DVIException (const std::string &msg) : MessageException(msg) {}
};


struct InvalidDVIFileException : public DVIException
{
	InvalidDVIFileException(const std::string &msg) : DVIException(msg) {}
};

class Matrix;

class BasicDVIReader : public StreamReader
{
	protected:
		typedef void (BasicDVIReader::*CommandHandler)(int);
		enum DVIFormat {DVI_NONE=0, DVI_STANDARD=2, DVI_PTEX=3, DVI_XDVOLD=5, DVI_XDVNEW=6};

	public:
		BasicDVIReader (std::istream &is);
		virtual ~BasicDVIReader () {}
		virtual void executeAllPages ();
		virtual double getXPos () const      {return 0;}
		virtual double getYPos () const      {return 0;}
		virtual void finishLine ()           {}
		virtual void translateToX (double x) {}
		virtual void translateToY (double y) {}
		virtual int getStackDepth () const   {return 0;}
		virtual void getPageTransformation (Matrix &matrix) const {}
		virtual unsigned getCurrentPageNumber () const {return 0;}

	protected:
		void setDVIFormat (DVIFormat format);
		DVIFormat getDVIFormat () const {return _dviFormat;}
		virtual int evalCommand (CommandHandler &handler, int &param);
		virtual int executeCommand ();
		void executePostPost ();

		// the following methods represent the DVI commands
		// they are called by executeCommand and should not be used directly
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
		virtual void cmdXFontDef (int len);     // XDV only
		virtual void cmdXGlyphArray (int len);  // XDV only
		virtual void cmdXGlyphString (int len); // XDV format 5 only
		virtual void cmdXPic (int len);         // XDV format 5 only

	private:
		DVIFormat _dviFormat;  ///< format of DVI file being processed
};

#endif
