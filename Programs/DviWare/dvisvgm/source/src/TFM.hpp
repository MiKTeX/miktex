/*************************************************************************
** TFM.hpp                                                              **
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

#ifndef TFM_HPP
#define TFM_HPP

#include <istream>
#include <string>
#include <vector>
#include "FixWord.hpp"
#include "FontMetrics.hpp"
#include "StreamReader.hpp"

class StreamReader;

class TFM : public FontMetrics {
	public:
		explicit TFM (std::istream &is);
		double getCharWidth (int c) const override;
		double getCharHeight (int c) const override;
		double getCharDepth (int c) const override;
		double getItalicCorr (int c) const override;
		double getDesignSize () const override {return _designSize;}
		double getSpace () const override;
		double getSpaceStretch () const override;
		double getSpaceShrink () const override;
		double getQuad () const override;
		double getAscent () const override     {return double(_ascent)*_designSize;}
		double getDescent () const override    {return double(_descent)*_designSize;}
		bool verticalLayout () const override  {return false;}
		uint32_t getChecksum () const override {return _checksum;}
		uint16_t firstChar () const override   {return _firstChar;}
		uint16_t lastChar () const override    {return _lastChar;}

	protected:
		TFM () : _params(7) {}
		void readHeader (StreamReader &reader);
		void readTables (StreamReader &reader, int nw, int nh, int nd, int ni);
		void readParameters (StreamReader &reader, int np);
		virtual int charIndex (int c) const;
		void setCharRange (int firstchar, int lastchar) {_firstChar=firstchar; _lastChar=lastchar;}

	private:
		uint32_t _checksum=0;
		uint16_t _firstChar=0, _lastChar=0;
		double _designSize=0;  ///< design size of the font in PS points (72bp = 1in)
		std::vector<uint32_t>  _charInfoTable;
		std::vector<FixWord> _widthTable;    ///< character widths in design size units
		std::vector<FixWord> _heightTable;   ///< character height in design size units
		std::vector<FixWord> _depthTable;    ///< character depth in design size units
		std::vector<FixWord> _italicTable;   ///< italic corrections in design size units
		std::vector<FixWord> _params;        ///< values of the TFM's param section
		FixWord _ascent, _descent;           ///< max. height and depth
};

#endif
