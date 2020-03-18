/*************************************************************************
** Font.cpp                                                             **
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

#if defined(MIKTEX)
#  include <config.h>
#endif
#include <cstdlib>
#include <fstream>
#include <set>
#include <sstream>
#include "CMap.hpp"
#include "FileFinder.hpp"
#include "FileSystem.hpp"
#include "Font.hpp"
#include "FontEngine.hpp"
#include "Message.hpp"
#include "MetafontWrapper.hpp"
#include "SignalHandler.hpp"
#include "Subfont.hpp"
#include "Unicode.hpp"
#include "utility.hpp"


using namespace std;


uint32_t Font::unicode (uint32_t c) const {
	return Unicode::charToCodepoint(c);
}


/** Returns the encoding object of this font which is asigned in a map file.
 *  If there's no encoding assigned, the function returns 0. */
const FontEncoding* Font::encoding () const {
	if (const FontMap::Entry *entry = fontMapEntry())
		return FontEncoding::encoding(entry->encname);
	return nullptr;
}


const FontMap::Entry* Font::fontMapEntry () const {
	string fontname = name();
	size_t pos = fontname.rfind('.');
	if (pos != string::npos)
		fontname = fontname.substr(0, pos); // strip extension
	return FontMap::instance().lookup(fontname);
}


/** Compute the extents of a given glyph.
 *  @param[in] c character code of glyph
 *  @param[in] vertical true if is glyph is part of vertical aligned text
 *  @param[out] metrics the resulting extents */
void Font::getGlyphMetrics (int c, bool vertical, GlyphMetrics &metrics) const {
	double s = scaleFactor();
	if (vertical) {  // get metrics for vertical text flow?
		if (verticalLayout()) {  // is the font designed for vertical texts?
			metrics.wl = s*charDepth(c);
			metrics.wr = s*charHeight(c);
			metrics.h  = 0;
			metrics.d  = s*charWidth(c);
		}
		else {  // rotate box by 90 degrees for alphabetic text
			metrics.wl = s*charDepth(c);
			metrics.wr = s*charHeight(c);
			metrics.h  = 0;
			metrics.d  = s*(charWidth(c)+italicCorr(c));
		}
	}
	else {
		metrics.wl = 0;
		metrics.wr = s*(charWidth(c)+italicCorr(c));
		metrics.h  = s*charHeight(c);
		metrics.d  = s*charDepth(c);
	}
}


const char* Font::filename () const {
	const char *fname = strrchr(path(), '/');
	if (fname)
		return fname+1;
	return path();
}

///////////////////////////////////////////////////////////////////////////////////////

TFMFont::TFMFont (string name, uint32_t cs, double ds, double ss)
	: _fontname(std::move(name)), _checksum(cs), _dsize(ds), _ssize(ss)
{
}


/** Returns a font metrics object for the current font.
 *  @throw FontException if TFM file can't be found */
const FontMetrics* TFMFont::getMetrics () const {
	if (!_metrics) {
		try {
			_metrics = FontMetrics::read(_fontname);
			if (!_metrics) {
				_metrics = util::make_unique<NullFontMetric>();
				Message::wstream(true) << "can't find "+_fontname+".tfm\n";
			}
		}
		catch (FontMetricException &e) {
			_metrics = util::make_unique<NullFontMetric>();
			Message::wstream(true) << e.what() << " in " << _fontname << ".tfm\n";
		}
	}
	return _metrics.get();
}


double TFMFont::charWidth (int c) const {
	double w = getMetrics() ? getMetrics()->getCharWidth(c) : 0;
	if (style())
		w *= style()->extend;
	return w;
}


double TFMFont::italicCorr (int c) const {
	double w = getMetrics() ? getMetrics()->getItalicCorr(c) : 0;
	if (style()) {
		w += abs(style()->slant*charHeight(c));  // slant := tan(phi) = dx/height
		w *= style()->extend;
	}
	return w;
}


double TFMFont::charDepth (int c) const  {return getMetrics() ? getMetrics()->getCharDepth(c) : 0;}
double TFMFont::charHeight (int c) const {return getMetrics() ? getMetrics()->getCharHeight(c) : 0;}


/** Tests if the checksum of the font matches that of the corresponding TFM file. */
bool TFMFont::verifyChecksums () const {
	if (_checksum != 0 && getMetrics() && getMetrics()->getChecksum() != 0)
		return _checksum == getMetrics()->getChecksum();
	return true;
}

//////////////////////////////////////////////////////////////////////////////

// static class variables
bool PhysicalFont::EXACT_BBOX = false;
bool PhysicalFont::KEEP_TEMP_FILES = false;
string PhysicalFont::CACHE_PATH;
double PhysicalFont::METAFONT_MAG = 4;
FontCache PhysicalFont::_cache;


unique_ptr<Font> PhysicalFont::create (const string &name, uint32_t checksum, double dsize, double ssize, PhysicalFont::Type type) {
	return unique_ptr<PhysicalFontImpl>(new PhysicalFontImpl(name, 0, checksum, dsize, ssize, type));
}


unique_ptr<Font> PhysicalFont::create (const string &name, int fontindex, uint32_t checksum, double dsize, double ssize) {
	return unique_ptr<PhysicalFontImpl>(new PhysicalFontImpl(name, fontindex, checksum, dsize, ssize, PhysicalFont::Type::TTC));
}


const char* PhysicalFont::path () const {
	const char *ext=nullptr;
	switch (type()) {
		case Type::OTF: ext = "otf"; break;
		case Type::PFB: ext = "pfb"; break;
		case Type::TTC: ext = "ttc"; break;
		case Type::TTF: ext = "ttf"; break;
		case Type::MF : ext = "mf";  break;
		default : ext = nullptr;
	}
	if (ext)
		return FileFinder::instance().lookup(name()+"."+ext);
	return FileFinder::instance().lookup(name());
}


/** Returns true if this font is CID-based. */
bool PhysicalFont::isCIDFont () const {
	if (type() == Type::MF)
		return false;
	FontEngine::instance().setFont(*this);
	return FontEngine::instance().isCIDFont();
}


/** Retrieve the IDs of all charachter maps available in the font file.
 *  @param[out] charMapIDs IDs of the found character maps
 *  @return number of found character maps */
int PhysicalFont::collectCharMapIDs (std::vector<CharMapID> &charMapIDs) const {
	if (type() == Type::MF)
		return 0;
	FontEngine::instance().setFont(*this);
	return FontEngine::instance().getCharMapIDs(charMapIDs);
}


/** Decodes a character code used in the DVI file to the code required to
 *  address the correct character in the font.
 *  @param[in] c DVI character to decode
 *  @return target character code or name */
Character PhysicalFont::decodeChar (uint32_t c) const {
	if (const FontEncoding *enc = encoding())
		return enc->decode(c);
	return Character(Character::CHRCODE, c);
}


/** Returns the number of units per EM. The EM square is the virtual area a glyph is designed on.
 *  All coordinates used to specify portions of the glyph are relative to the origin (0,0) at the
 *  lower left corner of this square, while the upper right corner is located at (m,m), where m
 *  is an integer value defined with the font, and returned by this function. */
int PhysicalFont::unitsPerEm() const {
	if (type() == Type::MF)
		return 1000;
	FontEngine::instance().setFont(*this);
	return FontEngine::instance().getUnitsPerEM();
}


int PhysicalFont::hAdvance () const {
	if (type() == Type::MF)
		return 0;
	FontEngine::instance().setFont(*this);
	return FontEngine::instance().getHAdvance();
}


double PhysicalFont::hAdvance (int c) const {
	if (type() == Type::MF)
		return unitsPerEm()*charWidth(c)/designSize();
	FontEngine::instance().setFont(*this);
	if (const FontMap::Entry *entry = fontMapEntry())
		if (Subfont *sf = entry->subfont)
			c = sf->decode(c);
	return FontEngine::instance().getHAdvance(decodeChar(c));
}


double PhysicalFont::vAdvance (int c) const {
	if (type() == Type::MF)
		return unitsPerEm()*charWidth(c)/designSize();
	FontEngine::instance().setFont(*this);
	if (const FontMap::Entry *entry = fontMapEntry())
		if (Subfont *sf = entry->subfont)
			c = sf->decode(c);
	return FontEngine::instance().getVAdvance(decodeChar(c));
}


string PhysicalFont::glyphName (int c) const {
	if (type() == Type::MF)
		return "";
	FontEngine::instance().setFont(*this);
	if (const FontMap::Entry *entry = fontMapEntry())
		if (Subfont *sf = entry->subfont)
			c = sf->decode(c);
	return FontEngine::instance().getGlyphName(decodeChar(c));
}


double PhysicalFont::scaledAscent() const {
	return ascent()*scaledSize()/unitsPerEm();
}


/** Returns the unscaled ascender of the font in design units. */
int PhysicalFont::ascent () const {
	if (type() == Type::MF)
		return getMetrics() ? getMetrics()->getAscent()*unitsPerEm()/getMetrics()->getQuad() : 0;
	FontEngine::instance().setFont(*this);
	return FontEngine::instance().getAscender();
}


/** Returns the unscaled descender of the font in design units. */
int PhysicalFont::descent () const {
	if (type() == Type::MF)
		return getMetrics() ? getMetrics()->getDescent()*unitsPerEm()/getMetrics()->getQuad() : 0;
	FontEngine::instance().setFont(*this);
	return FontEngine::instance().getDescender();
}


std::string PhysicalFont::familyName () const {
	if (type() == Type::MF)
		return "";
	FontEngine::instance().setFont(*this);
	const char *family = FontEngine::instance().getFamilyName();
	return family ? family : "";
}


std::string PhysicalFont::styleName () const {
	if (type() == Type::MF)
		return "";
	FontEngine::instance().setFont(*this);
	const char *style = FontEngine::instance().getStyleName();
	return style ? style : "";
}



/** Extracts the glyph outlines of a given character.
 *  @param[in]  c character code of requested glyph
 *  @param[out] glyph path segments of the glyph outline
 *  @param[in]  callback optional callback object for tracer class
 *  @return true if outline could be computed */
bool PhysicalFont::getGlyph (int c, GraphicsPath<int32_t> &glyph, GFGlyphTracer::Callback *callback) const {
	if (type() == Type::MF) {
		const Glyph *cached_glyph=nullptr;
		if (!CACHE_PATH.empty()) {
			_cache.write(CACHE_PATH);
			_cache.read(name(), CACHE_PATH);
			cached_glyph = _cache.getGlyph(c);
		}
		if (cached_glyph) {
			glyph = *cached_glyph;
			return true;
		}
		else {
			string gfname;
			if (createGF(gfname)) {
				try {
					double ds = getMetrics() ? getMetrics()->getDesignSize() : 1;
					GFGlyphTracer tracer(gfname, unitsPerEm()/ds, callback);
					tracer.setGlyph(glyph);
					tracer.executeChar(c);
					glyph.closeOpenSubPaths();
					if (!CACHE_PATH.empty())
						_cache.setGlyph(c, glyph);
					return true;
				}
				catch (GFException &e) {
					// @@ print error message
				}
			}
		}
	}
	else { // vector fonts (OTF, PFB, TTF, TTC)
		bool ok=true;
		FontEngine::instance().setFont(*this);
		if (const FontMap::Entry *entry = fontMapEntry())
			if (Subfont *sf = entry->subfont)
				c = sf->decode(c);
		ok = FontEngine::instance().traceOutline(decodeChar(c), glyph, false);
		glyph.closeOpenSubPaths();
		return ok;
	}
	return false;
}


/** Creates a GF file for this font object.
 *  @param[out] gfname name of the generated GF font file
 *  @return true on success */
bool PhysicalFont::createGF (string &gfname) const {
	static set<string> failed_fonts;
	if (failed_fonts.find(name()) == failed_fonts.end()) {
		SignalHandler::instance().check();
		gfname = FileSystem::tmpdir()+name()+".gf";
		MetafontWrapper mf(name(), FileSystem::tmpdir());
		bool ok = mf.make("ljfour", METAFONT_MAG); // call Metafont if necessary
		if (ok && mf.success() && getMetrics())
			return true;
		// report failure only once
		failed_fonts.insert(name());
		Message::wstream(true) << "failed to create " << name() << ".gf\n";
	}
	return false;
}


/** Traces all glyphs of the current font and stores them in the cache. If caching is disabled, nothing happens.
 *  @param[in] includeCached if true, glyphs already cached are traced again
 *  @param[in] cb optional callback methods called by the tracer
 *  @return number of glyphs traced */
int PhysicalFont::traceAllGlyphs (bool includeCached, GFGlyphTracer::Callback *cb) const {
	int count = 0;
	if (type() == Type::MF && !CACHE_PATH.empty()) {
		if (const FontMetrics *metrics = getMetrics()) {
			int fchar = metrics->firstChar();
			int lchar = metrics->lastChar();
			string gfname;
			Glyph glyph;
			if (createGF(gfname)) {
				_cache.read(name(), CACHE_PATH);
				double ds = getMetrics() ? getMetrics()->getDesignSize() : 1;
				GFGlyphTracer tracer(gfname, unitsPerEm()/ds, cb);
				tracer.setGlyph(glyph);
				for (int i=fchar; i <= lchar; i++) {
					if (includeCached || !_cache.getGlyph(i)) {
						glyph.clear();
						tracer.executeChar(i);
						glyph.closeOpenSubPaths();
						_cache.setGlyph(i, glyph);
						++count;
					}
				}
				_cache.write(CACHE_PATH);
			}
		}
	}
	return count;
}


/** Computes the exact bounding box of a glyph.
 *  @param[in]  c character code of the glyph
 *  @param[out] bbox the computed bounding box
 *  @param[in]  cb optional calback object forwarded to the tracer
 *  @return true if the box could be computed successfully */
bool PhysicalFont::getExactGlyphBox (int c, BoundingBox& bbox, GFGlyphTracer::Callback* cb) const {
	Glyph glyph;
	if (getGlyph(c, glyph, cb)) {
		bbox = glyph.computeBBox();
		double s = scaledSize()/unitsPerEm();
		bbox.scale(s, s);
		return true;
	}
	return false;
}


bool PhysicalFont::getExactGlyphBox (int c, GlyphMetrics &metrics, bool vertical, GFGlyphTracer::Callback *cb) const {
	BoundingBox charbox;
	if (!getExactGlyphBox(c, charbox, cb))
		return false;
	if ((metrics.wl = -charbox.minX()) < 0) metrics.wl=0;
	if ((metrics.wr = charbox.maxX()) < 0)  metrics.wr=0;
	if ((metrics.h = charbox.maxY()) < 0)   metrics.h=0;
	if ((metrics.d = -charbox.minY()) < 0)  metrics.d=0;
	if (vertical) {  // vertical text orientation
		if (verticalLayout()) {  // font designed for vertical layout?
			metrics.wl = metrics.wr = (metrics.wl+metrics.wr)/2;
			metrics.d += metrics.h;
			metrics.h = 0;
		}
		else {
			double depth = metrics.d;
			metrics.d = metrics.wr;
			metrics.wr = metrics.h;
			metrics.h = metrics.wl;
			metrics.wl = depth;
		}
	}
	return true;
}


unique_ptr<Font> VirtualFont::create (const string &name, uint32_t checksum, double dsize, double ssize) {
	return unique_ptr<VirtualFontImpl>(new VirtualFontImpl(name, checksum, dsize, ssize));
}


//////////////////////////////////////////////////////////////////////////////


PhysicalFontImpl::PhysicalFontImpl (const string &name, int fontindex, uint32_t cs, double ds, double ss, PhysicalFont::Type type)
	: TFMFont(name, cs, ds, ss),
	_filetype(type), _fontIndex(fontindex), _encodingPair(Font::encoding())
{
}


PhysicalFontImpl::~PhysicalFontImpl () {
	if (!CACHE_PATH.empty())
		_cache.write(CACHE_PATH);
	if (!KEEP_TEMP_FILES)
		tidy();
}


const FontEncoding* PhysicalFontImpl::encoding () const {
	if (!_encodingPair.enc1())
		return nullptr;
	return &_encodingPair;
}


bool PhysicalFontImpl::findAndAssignBaseFontMap () {
	const FontEncoding *enc = encoding();
	if (enc && enc->mapsToCharIndex()) {
		// try to find a base font map that maps from character indexes to a suitable
		// target encoding supported by the font file
		if (const FontEncoding *bfmap = enc->findCompatibleBaseFontMap(this, _charmapID))
			_encodingPair.assign(bfmap);
		else
			return false;
	}
	else if (type() != Type::MF) {
		FontEngine::instance().setFont(*this);
		_localCharMap = FontEngine::instance().createCustomToUnicodeMap();
		if (_localCharMap)
			_charmapID = FontEngine::instance().setCustomCharMap();
		else
			_charmapID = FontEngine::instance().setUnicodeCharMap();
	}
	return true;
}


/** Returns the Unicode point for a given DVI character. */
uint32_t PhysicalFontImpl::unicode (uint32_t c) const {
	if (type() == Type::MF)
		return Font::unicode(c);
	Character chr = decodeChar(c);
	if (type() == Type::PFB) {
		// try to get the Unicode point from the character name
		string glyphname = glyphName(c);
		uint32_t codepoint;
		if (!glyphname.empty() && (codepoint = Unicode::aglNameToCodepoint(glyphname)) != 0)
			return codepoint;
		if (c <= 0x1900)  // does character code c fit into Private Use Zone U+E000?
			return 0xe000+c;
//		Message::wstream() << "can't properly map PS character '" << glyphname << "' (0x" << hex << c << ") to Unicode\n";
		// If we get here, there is no easy mapping. As for now, we use the character code as Unicode point.
		// Although quite unlikely, it might collide with properly mapped characters.
		return Unicode::charToCodepoint(c);
	}
	if (chr.type() == Character::NAME || chr.number() == 0)
		return Unicode::charToCodepoint(chr.number());

	if (_localCharMap) {
		if (uint32_t mapped_char = _localCharMap->valueAt(chr.number()))
			return mapped_char;
	}
	// No Unicode equivalent found in the font file.
	// Now we should look for a smart alternative but at the moment
	// it's sufficient to simply choose a valid unused codepoint.
	return Unicode::charToCodepoint(chr.number());
}


const FontStyle* PhysicalFontImpl::style () const {
	if (auto entry = fontMapEntry())
		return &entry->style;
	return nullptr;
}


/** Delete all temporary font files created by Metafont. */
void PhysicalFontImpl::tidy () const {
	if (type() == Type::MF) {
		string fname = FileSystem::tmpdir()+name();
		for (const char *ext : {"gf", "tfm", "log"}) {
			if (FileSystem::exists(fname+"."+ext))
				FileSystem::remove(fname+"."+ext);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

string NativeFont::uniqueName (const string &path, const FontStyle &style) {
	static unordered_map<string, int> ids;
	ostringstream oss;
	oss << path << "b" << style.bold << "e" << style.extend << "s" << style.slant;
	auto it = ids.find(oss.str());
	int id = ids.size();
	if (it == ids.end())
		ids[oss.str()] = id;
	else
		id = it->second;
	oss.str("");
	oss << "nf" << id;
	return oss.str();
}


string NativeFont::name () const {
	return uniqueName(path(), _style);
}


PhysicalFont::Type NativeFont::type () const {
	if (const char *filepath = path()) {
		if (const char *p =strrchr(filepath, '.')) {
			string ext = p+1;
			if (ext == "otf")
				return PhysicalFont::Type::OTF;
			if (ext == "ttf")
				return PhysicalFont::Type::TTF;
			if (ext == "pfb")
				return PhysicalFont::Type::PFB;
		}
	}
	return PhysicalFont::Type::UNKNOWN;
}


double NativeFont::charWidth (int c) const {
	FontEngine::instance().setFont(*this);
	int upem = FontEngine::instance().getUnitsPerEM();
	double w = upem ? (scaledSize()*FontEngine::instance().getAdvance(c)/upem*_style.extend) : 0;
	w += abs(_style.slant*charHeight(c));
	return w;
}


double NativeFont::charHeight (int c) const {
	FontEngine::instance().setFont(*this);
	int upem = FontEngine::instance().getUnitsPerEM();
	return upem ? (scaledSize()*FontEngine::instance().getAscender()/upem) : 0;
}


double NativeFont::charDepth (int c) const {
	FontEngine::instance().setFont(*this);
	int upem = FontEngine::instance().getUnitsPerEM();
	return upem ? (scaledSize()*FontEngine::instance().getDescender()/upem) : 0;
}


bool NativeFontImpl::findAndAssignBaseFontMap () {
	FontEngine &fe = FontEngine::instance();
	fe.setFont(*this);
	fe.setUnicodeCharMap();
	fe.buildCharMap(_toUnicodeMap);
	if (!_toUnicodeMap.addMissingMappings(fe.getNumGlyphs()))
		Message::wstream(true) << "incomplete Unicode mapping for native font " << name() << " (" << filename() << ")\n";
	return true;
}


Character NativeFontImpl::decodeChar (uint32_t c) const {
	return Character(Character::INDEX, c);
}


uint32_t NativeFontImpl::unicode (uint32_t c) const {
	uint32_t ucode = _toUnicodeMap.valueAt(c);
	return Unicode::charToCodepoint(ucode);
}

//////////////////////////////////////////////////////////////////////////////

VirtualFontImpl::VirtualFontImpl (const string &name, uint32_t cs, double ds, double ss)
	: TFMFont(name, cs, ds, ss)
{
}


const char* VirtualFontImpl::path () const {
	return FileFinder::instance().lookup(name()+".vf");
}


void VirtualFontImpl::assignChar (uint32_t c, DVIVector &&dvi) {
	_charDefs.emplace(c, std::move(dvi));
}


/** Returns the DVI sippet that describes a given character of the virtual font.
 *  @param[in] c character code
 *  @return pointer to vector of DVI commands, or 0 if character doesn't exist */
const vector<uint8_t>* VirtualFontImpl::getDVI (int c) const {
	auto it = _charDefs.find(c);
	return (it == _charDefs.end() ? nullptr : &it->second);
}

