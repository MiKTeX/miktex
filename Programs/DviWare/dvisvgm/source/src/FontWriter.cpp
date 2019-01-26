/*************************************************************************
** FontWriter.cpp                                                       **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2019 Martin Gieseking <martin.gieseking@uos.de>   **
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
	for (const FontFormatInfo &info : _formatInfos) {
		if (formatstr == info.formatstr_short)
			return info.format;
	}
	return FontFormat::UNKNOWN;
}


/** Returns the corresponding FontFormatInfo for a given FontFormat. */
const FontWriter::FontFormatInfo* FontWriter::fontFormatInfo (FontFormat format) {
	for (const FontFormatInfo &info : _formatInfos) {
		if (format == info.format)
			return &info;
	}
	return nullptr;
}


/** Returns the names of all supported font formats. */
vector<string> FontWriter::supportedFormats () {
	vector<string> formats;
	for (const FontFormatInfo &info : _formatInfos)
		formats.emplace_back(info.formatstr_short);
	return formats;
}


#include <config.h>

#ifdef DISABLE_WOFF
// dummy functions used if WOFF support is disabled
FontWriter::FontWriter (const PhysicalFont &font) : _font(font) {}
std::string FontWriter::createFontFile (FontFormat format, const set<int> &charcodes, GFGlyphTracer::Callback *cb) const {return "";}
bool FontWriter::writeCSSFontFace (FontFormat format, const set<int> &charcodes, ostream &os, GFGlyphTracer::Callback *cb) const {return false;}
#else
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <woff2/encode.h>
#include "ffwrapper.h"
#include "Bezier.hpp"
#include "FileSystem.hpp"
#include "Font.hpp"
#include "Glyph.hpp"
#include "TTFAutohint.hpp"
#include "TrueTypeFont.hpp"


FontWriter::FontWriter (const PhysicalFont &font) : _font(font) {
	static bool initialized=false;
	if (!initialized) {
		ff_init();
		initialized = true;
	}
}


struct SFDActions : Glyph::Actions {
	SFDActions (ostream &os) : _os(os) {}
	void draw (char cmd, const Glyph::Point *points, int n) override {
		if (cmd == 'Q') {
			// convert quadratic Bézier curve to cubic one
			DPair p0(_currentPoint.x(), _currentPoint.y());
			DPair p1(points[0].x(), points[0].y());
			DPair p2(points[1].x(), points[1].y());
			Bezier bezier(p0, p1, p2);
			for (int i=1; i < 4; i++)
				_os << lround(bezier.point(i).x()) << ' ' << lround(bezier.point(i).y()) << ' ';
			_os << 'c';
		}
		else {
			for (int i=0; i < n; i++)
				_os << points[i].x() << ' ' << points[i].y() << ' ';
			switch (cmd) {
				case 'M': _os << 'm'; _startPoint = points[0]; break;
				case 'L': _os << 'l'; break;
				case 'C': _os << 'c'; break;
				case 'Z': _os << _startPoint.x() << ' ' << _startPoint.y() << " l"; _currentPoint = _startPoint; break;
			}
		}
		if (n > 0)
			_currentPoint = points[n-1];
		_os << " 0\n";
	}
	ostream &_os;
   Glyph::Point _startPoint, _currentPoint;
};


/** Creates a Spline Font Database (SFD) file describing the font and its glyphs.
 *  https://fontforge.github.io/sfdformat.html */
static void writeSFD (const string &sfdname, const PhysicalFont &font, const set<int> &charcodes, GFGlyphTracer::Callback *cb) {
	ofstream sfd(sfdname);
	if (!sfd)
		throw FontWriterException("failed writing SFD file "+sfdname);

	sfd <<
		"SplineFontDB: 3.0\n"
		"FontName: " << font.name() << '\n';

	// ensure that the sum of the SFD's Ascent and Descent values equals the font's units per EM
	double yext = font.ascent()+font.descent();
	double scale = double(font.unitsPerEm())/(yext != 0 ? yext : fabs(font.ascent()));
	sfd <<
		"Ascent: " << font.ascent()*scale << "\n"
		"Descent: " << font.descent()*scale << "\n"
		"LayerCount: 2\n"           // number of layers must be 2 at least
		"Layer: 0 0 \"Back\" 1\n"   // layer 0: background layer with cubic splines
		"Layer: 1 0 \"Fore\" 0\n"   // layer 1: foreground layer with cubic splines
		"Encoding: UnicodeFull\n"   // character codes can use the full Unicode range
		"BeginChars: 1114112 " << charcodes.size() << '\n';

	double extend = font.style() ? font.style()->extend : 1;
	for (int c : charcodes) {
		string name = font.glyphName(c);
		if (name.empty()) {
			// if the font doesn't provide glyph names, use AGL name uFOO
			ostringstream oss;
			oss << 'u' << hex << uppercase << setw(4) << setfill('0') << c;
			name = oss.str();
		}
		uint32_t codepoint = font.unicode(c);
		sfd <<
			"StartChar: " << name << "\n"
			"Encoding: "  << codepoint << ' ' << codepoint << " 0\n"
			"Width: "     << font.hAdvance(c)*extend << "\n"
			"VWidth: "    << font.vAdvance(c) << "\n"
			"Fore\n"
			"SplineSet\n";
		Glyph glyph;
		if (font.getGlyph(c, glyph, cb)) {
			SFDActions actions(sfd);
			glyph.iterate(actions, false);
		}
		sfd <<
			"EndSplineSet\n"
			"EndChar\n";
	}
	sfd.flush();
	sfd.close();
	if (sfd.fail())
		throw FontWriterException("failed writing SFD file "+sfdname);
}


bool FontWriter::createTTFFile (const string &sfdname, const string &ttfname) const {
	TTFAutohint autohinter;
	if (!autohinter.available())
		return ff_sfd_to_ttf(sfdname.c_str(), ttfname.c_str(), AUTOHINT_FONTS);

	bool ok = ff_sfd_to_ttf(sfdname.c_str(), ttfname.c_str(), false);
	if (ok && AUTOHINT_FONTS) {
		string tmpname = ttfname+"-ah";
		int errnum = autohinter.autohint(ttfname, tmpname, true);
		if (errnum) {
			Message::wstream(true) << "failed to autohint font '" << _font.name() << "'";
			string msg = autohinter.lastErrorMessage();
			if (!msg.empty())
				Message::wstream() << " (" << msg << ")";
			// keep the unhinted TTF
			FileSystem::remove(tmpname);
		}
		else {
			FileSystem::remove(ttfname);
			FileSystem::rename(tmpname, ttfname);
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
	string sfdname = basename+".sfd";
	writeSFD(sfdname, _font, charcodes, cb);
	string ttfname = basename+".ttf";
	string targetname = basename+"."+fontFormatInfo(format)->formatstr_short;
	bool ok = createTTFFile(sfdname, ttfname);
	if (ok) {
		if (format == FontFormat::WOFF || format == FontFormat::WOFF2) {
			TrueTypeFont ttf(ttfname);
			if (format == FontFormat::WOFF)
				ttf.writeWOFF(targetname);
			else
				ok = ttf.writeWOFF2(targetname);
			if (!PhysicalFont::KEEP_TEMP_FILES)
				FileSystem::remove(ttfname);
		}
	}
	if (!PhysicalFont::KEEP_TEMP_FILES)
		FileSystem::remove(sfdname);
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
		ifstream ifs(filename, ios::binary);
		if (ifs) {
			os << "@font-face{"
				<< "font-family:" << _font.name() << ';'
				<< "src:url(data:" << info->mimetype << ";base64,";
			util::base64_copy(istreambuf_iterator<char>(ifs), istreambuf_iterator<char>(), ostreambuf_iterator<char>(os));
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
