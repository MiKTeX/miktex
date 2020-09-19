/*************************************************************************
** FontWriter.hpp                                                       **
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

#ifndef FONTWRITER_HPP
#define FONTWRITER_HPP

#include <ostream>
#include <set>
#include <string>
#include <vector>
#include "GFGlyphTracer.hpp"
#include "MessageException.hpp"

class PhysicalFont;

class FontWriter {
	public:
		enum class FontFormat {UNKNOWN, SVG, TTF, WOFF, WOFF2};
		static bool AUTOHINT_FONTS;

	public:
		explicit FontWriter (const PhysicalFont &font);
		std::string createFontFile (FontFormat format, const std::set<int> &charcodes, GFGlyphTracer::Callback *cb=nullptr) const;
		bool writeCSSFontFace (FontFormat format, const std::set<int> &charcodes, std::ostream &os, GFGlyphTracer::Callback *cb=nullptr) const;
		static FontFormat toFontFormat (std::string formatstr);
		static std::vector<std::string> supportedFormats ();

	protected:
		struct FontFormatInfo {
			FontWriter::FontFormat format;
			const char *mimetype;
			const char *formatstr_short;
			const char *formatstr_long;
		};
		static const FontFormatInfo* fontFormatInfo (FontFormat format);
		bool createTTFFile (const std::string &sfdname, const std::string &ttfname) const;

	private:
		const PhysicalFont &_font;
		static const std::array<FontFormatInfo, 4> _formatInfos;
};


struct FontWriterException : MessageException {
	explicit FontWriterException (const std::string &msg) : MessageException(msg) {}
};

#endif

