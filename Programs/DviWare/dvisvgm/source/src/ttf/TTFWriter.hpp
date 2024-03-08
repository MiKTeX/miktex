/*************************************************************************
** TTFWriter.hpp                                                        **
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

#pragma once

#include <set>
#include <vector>
#include "CmapTable.hpp"
#include "GlyfTable.hpp"
#include "HeadTable.hpp"
#include "HheaTable.hpp"
#include "HmtxTable.hpp"
#include "LocaTable.hpp"
#include "MaxpTable.hpp"
#include "NameTable.hpp"
#include "OS2Table.hpp"
#include "PostTable.hpp"
#include "VheaTable.hpp"
#include "VmtxTable.hpp"
#include "../GFGlyphTracer.hpp"
#include "../RangeMap.hpp"

class PhysicalFont;

namespace ttf {

class TTFWriter {
	public:
		TTFWriter (const PhysicalFont &font, const std::set<int> &chars);
		bool writeTTF (std::ostream &os);
		bool writeWOFF (std::ostream &os);
		bool writeWOFF2 (std::ostream &os);
		bool writeTTF (const std::string &ttfname);
		bool writeWOFF (const std::string &woffname);
		bool writeWOFF2 (const std::string &woff2name);
		static bool convertTTFToWOFF (std::istream &is, std::ostream &os);
		static bool convertTTFToWOFF2 (std::istream &is, std::ostream &os);
		static bool convertTTFToWOFF (const std::string &ttfname, const std::string &woffname);
		static bool convertTTFToWOFF2 (const std::string &ttfname, const std::string &woff2name);
		void setTracerCallback (GFGlyphTracer::Callback &callback) {_tracerCallback = &callback;}
		GFGlyphTracer::Callback* getTracerCallback () const {return _tracerCallback;}
		const PhysicalFont& getFont () const {return _font;}
		const RangeMap& getUnicodeCharMap () const {return _unicodeCharMap;}
		int targetUnitsPerEm () const;
		double unitsPerEmFactor () const;
		int16_t hAdvance (uint16_t c) const;
		void updateGlobalBbox (uint32_t c, int16_t xmin, int16_t ymin, int16_t xmax, int16_t ymax);
		bool isFixedPitch () const {return _hmtx.isFixedPitch();}

		void updateContourInfo (uint16_t maxContours, uint16_t maxPoints) {
			_maxp.updateContourInfo(maxPoints, maxContours);
		}

		void updateGlobalAdvanceWidth (uint16_t w, uint16_t numHMetrics) {_hhea.updateAdvanceWidth(w, numHMetrics);}
		void updateGlobalAdvanceHeight (uint16_t h, uint16_t numVMetrics) {
			if (_vhea.active())
				_vhea.updateAdvanceHeight(h, numVMetrics);
		}

		void addGlyphOffset (uint32_t offset) {
			_loca.addOffset(offset);
			if (offset > 0xFFFF)
				_head.setLongOffsetFormat();
		}

#ifdef TTFDEBUG
		static bool CREATE_PS_GLYPH_OUTLINES;
#endif

	protected:
		std::list<TableBuffer> createTableBuffers ();

	private:
		const PhysicalFont &_font;
		RangeMap _unicodeCharMap;   ///< Unicode -> character code
		GFGlyphTracer::Callback *_tracerCallback;
		CmapTable _cmap;  ///< glyph index to character code mapping table
		GlyfTable _glyf;  ///< glyph data table
		HeadTable _head;  ///< font header table
		HheaTable _hhea;  ///< horizontal header table
		HmtxTable _hmtx;  ///< horizontal metrics table
		LocaTable _loca;  ///< glyph index to location/offset table
		MaxpTable _maxp;  ///< maximum profile table
		NameTable _name;  ///< naming table
		OS2Table  _os2;   ///< OS/2 and Windows specific metrics table
		PostTable _post;  ///< PostScript information table
		VheaTable _vhea;  ///< vertical header table
		VmtxTable _vmtx;  ///< vertical metrics table
		// The following vector contains pointers to the TTF table objects declared above.
		// If a table t1 contains data that computation depends on data of table t2,
		// t1 must be inserted after t2. For example, the loca and head tables require data
		// from the glyf table to compute their data, so that glyf must precede loca and head.
		std::vector<TTFTable*> _tables;
};

} // namespace ttf