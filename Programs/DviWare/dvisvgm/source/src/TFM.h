/*************************************************************************
** TFM.h                                                                **
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

#ifndef DVISVGM_TFM_H
#define DVISVGM_TFM_H

#include <istream>
#include <string>
#include <vector>
#include "FontMetrics.h"
#include "types.h"
#include "StreamReader.h"

class StreamReader;

class TFM : public FontMetrics
{
	public:
		TFM (std::istream &is);
		double getCharWidth (int c) const;
		double getCharHeight (int c) const;
		double getCharDepth (int c) const;
		double getItalicCorr (int c) const;
		double getDesignSize () const {return _designSize;}
		double getSpace () const;
		double getSpaceStretch () const;
		double getSpaceShrink () const;
		double getQuad () const;
		bool verticalLayout () const  {return false;}
		UInt32 getChecksum () const   {return _checksum;}
		UInt16 firstChar () const     {return _firstChar;}
		UInt16 lastChar () const      {return _lastChar;}

	protected:
		TFM () : _checksum(0), _firstChar(0), _lastChar(0), _designSize(0), _params(7) {}
		void readHeader (StreamReader &reader);
		void readTables (StreamReader &reader, int nw, int nh, int nd, int ni);
		void readParameters (StreamReader &reader, int np);
		virtual int charIndex (int c) const;
		void setCharRange (int firstchar, int lastchar) {_firstChar=firstchar; _lastChar=lastchar;}

	private:
		UInt32 _checksum;
		UInt16 _firstChar, _lastChar;
		double _designSize;  ///< design size of the font in PS points (72bp = 1in)
		std::vector<UInt32>  _charInfoTable;
		std::vector<FixWord> _widthTable;    ///< character widths in design size units
		std::vector<FixWord> _heightTable;   ///< character height in design size units
		std::vector<FixWord> _depthTable;    ///< character depth in design size units
		std::vector<FixWord> _italicTable;   ///< italic corrections in design size units
		std::vector<FixWord> _params;        ///< values of the TFM's param section
};

#endif
