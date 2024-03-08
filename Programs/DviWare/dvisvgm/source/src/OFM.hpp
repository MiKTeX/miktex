/*************************************************************************
** OFM.hpp                                                              **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2024 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef OFM_HPP
#define OFM_HPP

#include <vector>
#include "FixWord.hpp"
#include "FontMetrics.hpp"
#include "StreamReader.hpp"

class OFM : public FontMetrics {
	public:
		virtual int level () const =0;
		bool isOFM () const override           {return true;}
		uint32_t getChecksum () const override {return _checksum;}
		uint32_t firstChar () const override   {return _firstChar;}
		uint32_t lastChar () const override    {return _lastChar;}
		bool verticalLayout () const  override {return _vertical;}
		double getDesignSize () const override {return _designSize;}
		double getAscent () const override     {return double(_ascent)*_designSize;}
		double getDescent () const override    {return double(_descent)*_designSize;}
		double getCharWidth (int c) const override;
		double getCharHeight (int c) const override;
		double getCharDepth (int c) const override;
		double getItalicCorr (int c) const override;
		double getSpace () const override;
		double getSpaceStretch () const override;
		double getSpaceShrink () const override;
		double getQuad () const override;
		using FontMetrics::read;

	protected:
		struct FileDimensions {
			uint32_t lf=0;  // length of file in 4-byte words
			uint32_t lh=0;  // length of header data in 4-byte words
			uint32_t bc=0;  // smallest character code
			uint32_t ec=0;  // largest character code
			uint32_t nw=0;  // number of width entries
			uint32_t nh=0;  // number of height entries
			uint32_t nd=0;  // number of depth entries
			uint32_t ni=0;  // number of italic correction entries
			uint32_t nl=0;  // number of lig-kern entries
			uint32_t nk=0;  // number of kern entries
			uint32_t ne=0;  // number of extensible character entries
			uint32_t np=0;  // number of font parameter words
			uint32_t dir=0; // writing direction of font
			void read (StreamReader &reader);
		};

		FileDimensions read (std::istream &is, uint32_t maxNumWidths);
		void readHeader (StreamReader &reader);
		void readTables (StreamReader &reader, const FileDimensions &dim);
		size_t charIndex (int c) const;
		virtual size_t numCharInfos () const =0;
		virtual size_t widthIndex (size_t n) const =0;
		virtual size_t heightIndex (size_t n) const =0;
		virtual size_t depthIndex (size_t n) const =0;
		virtual size_t italicIndex (size_t n) const =0;

	protected:
		uint32_t _checksum=0;
		uint32_t _firstChar=0, _lastChar=0;
		double _designSize=0;  ///< design size of the font in PS points (72bp = 1in)
		std::vector<FixWord> _widthTable;    ///< character widths in design size units
		std::vector<FixWord> _heightTable;   ///< character height in design size units
		std::vector<FixWord> _depthTable;    ///< character depth in design size units
		std::vector<FixWord> _italicTable;   ///< italic corrections in design size units
		std::vector<FixWord> _params;        ///< values of the OFM's param section
		FixWord _ascent=0, _descent=0;       ///< max. height and depth
		bool _vertical=false;     ///< true if metrics refer to vertical text layout
};


class OFM0 : public OFM {
	public:
		void read (std::istream &is) override;
		int level () const override {return 0;}

	protected:
		void readCharInfos (StreamReader &reader);
		size_t numCharInfos () const override {return _charInfos.size();}
		size_t widthIndex (size_t n) const override  {return (_charInfos[n] >> 48) & 0xFFFF;}
		size_t heightIndex (size_t n) const override {return (_charInfos[n] >> 40) & 0xFF;}
		size_t depthIndex (size_t n) const override  {return (_charInfos[n] >> 32) & 0xFF;}
		size_t italicIndex (size_t n) const override {return (_charInfos[n] >> 24) & 0xFF;}

	private:
		std::vector<uint64_t>  _charInfos;
};


class OFM1 : public OFM {
	struct CharInfo {
		uint16_t widthIndex=0;
		uint8_t heightIndex=0;
		uint8_t depthIndex=0;
		uint8_t italicIndex=0;
	};
	public:
		void read (std::istream &is) override;
		int level () const override {return 1;}

	protected:
		void readCharInfos (StreamReader &reader);
		size_t numCharInfos () const override {return _charInfos.size();}
		size_t widthIndex (size_t n) const override  {return _charInfos[n].widthIndex;}
		size_t heightIndex (size_t n) const override {return _charInfos[n].heightIndex;}
		size_t depthIndex (size_t n) const override  {return _charInfos[n].depthIndex;}
		size_t italicIndex (size_t n) const override {return _charInfos[n].italicIndex;}

	private:
		uint32_t _ncw=0;
		uint32_t _npc=0;
		std::vector<CharInfo> _charInfos;
};

#endif
