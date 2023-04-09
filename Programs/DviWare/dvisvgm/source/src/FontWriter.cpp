/*************************************************************************
** FontWriter.cpp                                                       **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2023 Martin Gieseking <martin.gieseking@uos.de>   **
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

#if defined(MIKTEX)
#  include <config.h>
#endif
#include <algorithm>
#include <array>
#include "FontWriter.hpp"
#include "Message.hpp"
#include "utility.hpp"
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/PathNameUtil>
#define EXPATH_(x) MiKTeX::Util::PathNameUtil::ToLengthExtendedPathName(x)
#endif

using namespace std;

bool FontWriter::AUTOHINT_FONTS = false;

const array<FontWriter::FontFormatInfo, 4> FontWriter::_formatInfos {{
	{FontWriter::FontFormat::SVG, "image/svg+xml", "svg", "svg"},
	{FontWriter::FontFormat::TTF, "application/x-font-ttf", "ttf", "truetype"},
	{FontWriter::FontFormat::WOFF, "application/x-font-woff", "woff", "woff"},
	{FontWriter::FontFormat::WOFF2, "application/x-font-woff2", "woff2", "woff2"},
}};


/** Returns the corresponding FontFormat for a given format name (e.g. "svg", "woff" etc.). */
FontWriter::FontFormat FontWriter::toFontFormat (string formatstr) {
	formatstr = util::tolower(formatstr);
	auto it = find_if(_formatInfos.begin(), _formatInfos.end(), [&](const FontFormatInfo &info) {
		return info.formatstr_short == formatstr;
	});
	return it != _formatInfos.end() ? it->format : FontFormat::UNKNOWN;
}


/** Returns the corresponding FontFormatInfo for a given FontFormat. */
const FontWriter::FontFormatInfo* FontWriter::fontFormatInfo (FontFormat format) {
	auto it = find_if(_formatInfos.begin(), _formatInfos.end(), [&](const FontFormatInfo &info) {
		return info.format == format;
	});
	return it != _formatInfos.end() ? &(*it) : nullptr;
}


/** Returns the names of all supported font formats. */
vector<string> FontWriter::supportedFormats () {
	vector<string> formats;
	for (const FontFormatInfo &info : _formatInfos)
		formats.push_back(info.formatstr_short);
	return formats;
}


#include <config.h>

#ifdef DISABLE_WOFF
// dummy functions used if WOFF support is disabled
FontWriter::FontWriter (const PhysicalFont &font) : _currentFont(font) {}
std::string FontWriter::createFontFile (FontFormat format, const set<int> &charcodes, GFGlyphTracer::Callback *cb) const {return "";}
bool FontWriter::writeCSSFontFace (FontFormat format, const set<int> &charcodes, ostream &os, GFGlyphTracer::Callback *cb) const {return false;}
#else
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <woff2/encode.h>
#include "Bezier.hpp"
#include "FileSystem.hpp"
#include "Font.hpp"
#include "Glyph.hpp"
#include "ttf/TTFAutohint.hpp"
#include "ttf/TTFWriter.hpp"

using namespace ttf;

bool FontWriter::createTTFFile (const std::string &ttfname, const PhysicalFont &font, const set<int> &charcodes, GFGlyphTracer::Callback *cb) const {
	TTFWriter ttfWriter(font, charcodes);
	if (cb)
		ttfWriter.setTracerCallback(*cb);
	bool ok = ttfWriter.writeTTF(ttfname);
	if (ok && AUTOHINT_FONTS) {
		TTFAutohint autohinter;
		if (!autohinter.available()) {
			static bool reported=false;
			if (!reported) {
				Message::wstream(true) << "autohint functionality disabled (ttfautohint not found)";
				reported = true;
			}
		}
		else {
			string tmpname = ttfname+"-ah";
			try {
				int errnum = autohinter.autohint(ttfname, tmpname, true);
				if (errnum == 0) {  // success?
					FileSystem::remove(ttfname);
					FileSystem::rename(tmpname, ttfname);
				}
				else {
					Message::wstream(true) << "failed to autohint font '" << _font.name() << "'";
					string msg = autohinter.lastErrorMessage();
					if (!msg.empty())
						Message::wstream() << " (" << msg << ")";
					// keep the unhinted TTF
					FileSystem::remove(tmpname);
				}
			}
			catch (MessageException &e) {
				Message::wstream(true) << e.what() << '\n';
				FileSystem::remove(tmpname);
			}
		}
	}
	return ok;
}


/** Creates a font file containing a given set of glyphs mapped to their Unicode points.
 * @param[in] format target font format
 * @param[in] charcodes character codes of the glyphs to be considered
 * @param[in] cb callback object that allows to react to events triggered by the glyph tracer
 * @return name of the created font file */
string FontWriter::createFontFile (FontFormat format, const set<int> &charcodes, GFGlyphTracer::Callback *cb) const {
	string tmpdir = FileSystem::tmpdir();
	string basename = tmpdir+_font.name()+"-tmp";
	string ttfname = basename+".ttf";
	string targetname = basename+"."+fontFormatInfo(format)->formatstr_short;
	bool ok = createTTFFile(ttfname, _font, charcodes, cb);
	if (ok) {
		if (format == FontFormat::WOFF || format == FontFormat::WOFF2) {
			if (format == FontFormat::WOFF)
				ok = TTFWriter::convertTTFToWOFF(ttfname, targetname);
			else
				TTFWriter::convertTTFToWOFF2(ttfname, targetname);
			if (!PhysicalFont::KEEP_TEMP_FILES)
				FileSystem::remove(ttfname);
		}
	}
	if (!ok)
		throw FontWriterException("failed writing "+string(fontFormatInfo(format)->formatstr_short)+ " file " + targetname);
	return targetname;
}


/** Writes a CSS font-face rule to an output stream that references or contains the WOFF/TTF font data.
 * @param[in] format target font format
 * @param[in] charcodes character codes of the glyphs to be considered
 * @param[in] os stream the CSS data is written to
 * @param[in] cb callback object that allows to react to events triggered by the glyph tracer
 * @return true on success */
bool FontWriter::writeCSSFontFace (FontFormat format, const set<int> &charcodes, ostream &os, GFGlyphTracer::Callback *cb) const {
	if (const FontFormatInfo *info = fontFormatInfo(format)) {
		string filename = createFontFile(format, charcodes, cb);
#if defined(MIKTEX_WINDOWS)
                ifstream ifs(EXPATH_(filename), ios::binary);
#else
		ifstream ifs(filename, ios::binary);
#endif
		if (ifs) {
			os << "@font-face{"
				<< "font-family:" << _font.name() << ';'
				<< "src:url(data:" << info->mimetype << ";base64,";
			util::base64_copy(ifs, os);
			os << ") format('" << info->formatstr_long << "');}\n";
			ifs.close();
			if (!PhysicalFont::KEEP_TEMP_FILES)
				FileSystem::remove(filename);
			return true;
		}
	}
	return false;
}

#endif // !DISABLE_WOFF
