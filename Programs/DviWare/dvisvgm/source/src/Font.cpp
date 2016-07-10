/*************************************************************************
** Font.cpp                                                             **
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

#include <config.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include "CMap.h"
#include "FileFinder.h"
#include "FileSystem.h"
#include "Font.h"
#include "FontEngine.h"
#include "Message.h"
#include "MetafontWrapper.h"
#include "SignalHandler.h"
#include "Subfont.h"
#include "Unicode.h"


using namespace std;


UInt32 Font::unicode (UInt32 c) const {
	return Unicode::charToCodepoint(c);
}


/** Returns the encoding object of this font which is asigned in a map file.
 *  If there's no encoding assigned, the function returns 0. */
const FontEncoding* Font::encoding () const {
	if (const FontMap::Entry *entry = fontMapEntry())
		return FontEncoding::encoding(entry->encname);
	return 0;
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

TFMFont::TFMFont (string name, UInt32 cs, double ds, double ss)
	: _metrics(0), _fontname(name), _checksum(cs), _dsize(ds), _ssize(ss)
{
}


TFMFont::~TFMFont () {
	delete _metrics;
}


/** Returns a font metrics object for the current font.
 *  @throw FontException if TFM file can't be found */
const FontMetrics* TFMFont::getMetrics () const {
	if (!_metrics) {
		try {
			_metrics = FontMetrics::read(_fontname.c_str());
			if (!_metrics) {
				_metrics = new NullFontMetric;
				Message::wstream(true) << "can't find "+_fontname+".tfm\n";
			}
		}
		catch (FontMetricException &e) {
			_metrics = new NullFontMetric;
			Message::wstream(true) << e.what() << " in " << _fontname << ".tfm\n";
		}
	}
	return _metrics;
}


double TFMFont::charWidth (int c) const {
	double w = getMetrics() ? getMetrics()->getCharWidth(c) : 0;
	if (style()) {
		w *= style()->extend;
		w += fabs(style()->slant*charHeight(c));  // slant := tan(phi) = dx/height
	}
	return w;
}


double TFMFont::italicCorr (int c) const {
	double w = getMetrics() ? getMetrics()->getItalicCorr(c) : 0;
	if (style())
		w *= style()->extend;
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
const char *PhysicalFont::CACHE_PATH = 0;
double PhysicalFont::METAFONT_MAG = 4;
FontCache PhysicalFont::_cache;


Font* PhysicalFont::create (string name, UInt32 checksum, double dsize, double ssize, PhysicalFont::Type type) {
	return new PhysicalFontImpl(name, 0, checksum, dsize, ssize, type);
}


Font* PhysicalFont::create (string name, int fontindex, UInt32 checksum, double dsize, double ssize) {
	return new PhysicalFontImpl(name, fontindex, checksum, dsize, ssize, PhysicalFont::TTC);
}


const char* PhysicalFont::path () const {
	const char *ext=0;
	switch (type()) {
		case OTF: ext = "otf"; break;
		case PFB: ext = "pfb"; break;
		case TTC: ext = "ttc"; break;
		case TTF: ext = "ttf"; break;
		case MF : ext = "mf";  break;
		default : ext = 0;
	}
	if (ext)
		return FileFinder::lookup(name()+"."+ext);
	return FileFinder::lookup(name());
}


/** Returns true if this font is CID-based. */
bool PhysicalFont::isCIDFont () const {
	if (type() == MF)
		return false;
	FontEngine::instance().setFont(*this);
	return FontEngine::instance().isCIDFont();
}


/** Retrieve the IDs of all charachter maps available in the font file.
 *  @param[out] charMapIDs IDs of the found character maps
 *  @return number of found character maps */
int PhysicalFont::collectCharMapIDs (std::vector<CharMapID> &charMapIDs) const {
	if (type() == MF)
		return 0;
	FontEngine::instance().setFont(*this);
	return FontEngine::instance().getCharMapIDs(charMapIDs);
}


/** Decodes a character code used in the DVI file to the code required to
 *  address the correct character in the font.
 *  @param[in] c DVI character to decode
 *  @return target character code or name */
Character PhysicalFont::decodeChar (UInt32 c) const {
	if (const FontEncoding *enc = encoding())
		return enc->decode(c);
	return Character(Character::CHRCODE, c);
}


/** Returns the number of units per EM. The EM square is the virtual area a glyph is designed on.
 *  All coordinates used to specify portions of the glyph are relative to the origin (0,0) at the
 *  lower left corner of this square, while the upper right corner is located at (m,m), where m
 *  is an integer value defined with the font, and returned by this function. */
int PhysicalFont::unitsPerEm() const {
	if (type() == MF)
		return 1000;
	FontEngine::instance().setFont(*this);
	return FontEngine::instance().getUnitsPerEM();
}


int PhysicalFont::hAdvance () const {
	if (type() == MF)
		return 0;
	FontEngine::instance().setFont(*this);
	return FontEngine::instance().getHAdvance();
}


double PhysicalFont::hAdvance (int c) const {
	if (type() == MF)
		return unitsPerEm()*charWidth(c)/designSize();
	FontEngine::instance().setFont(*this);
	if (const FontMap::Entry *entry = fontMapEntry())
		if (Subfont *sf = entry->subfont)
			c = sf->decode(c);
	return FontEngine::instance().getHAdvance(decodeChar(c));
}


double PhysicalFont::vAdvance (int c) const {
	if (type() == MF)
		return unitsPerEm()*charWidth(c)/designSize();
	FontEngine::instance().setFont(*this);
	if (const FontMap::Entry *entry = fontMapEntry())
		if (Subfont *sf = entry->subfont)
			c = sf->decode(c);
	return FontEngine::instance().getVAdvance(decodeChar(c));
}


string PhysicalFont::glyphName (int c) const {
	if (type() == MF)
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


int PhysicalFont::ascent () const {
	if (type() == MF)
		return 0;
	FontEngine::instance().setFont(*this);
	return FontEngine::instance().getAscender();
}


int PhysicalFont::descent () const {
	if (type() == MF)
		return 0;
	FontEngine::instance().setFont(*this);
	return FontEngine::instance().getDescender();
}


std::string PhysicalFont::familyName () const {
	if (type() == MF)
		return "";
	FontEngine::instance().setFont(*this);
	const char *family = FontEngine::instance().getFamilyName();
	return family ? family : "";
}


std::string PhysicalFont::styleName () const {
	if (type() == MF)
		return "";
	FontEngine::instance().setFont(*this);
	const char *style = FontEngine::instance().getStyleName();
	return style ? style : "";
}



/** Extracts the glyph outlines of a given character.
 *  @param[in]  c character code of requested glyph
 *  @param[out] glyph path segments of the glyph outline
 *  @param[in]  cb optional callback object for tracer class
 *  @return true if outline could be computed */
bool PhysicalFont::getGlyph (int c, GraphicsPath<Int32> &glyph, GFGlyphTracer::Callback *cb) const {
	if (type() == MF) {
		const Glyph *cached_glyph=0;
		if (CACHE_PATH) {
			_cache.write(CACHE_PATH);
			_cache.read(name().c_str(), CACHE_PATH);
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
					GFGlyphTracer tracer(gfname, unitsPerEm()/ds, cb);
					tracer.setGlyph(glyph);
					tracer.executeChar(c);
					glyph.closeOpenSubPaths();
					if (CACHE_PATH)
						_cache.setGlyph(c, glyph);
					return true;
				}
				catch (GFException &e) {
					// @@ print error message
				}
			}
			else {
				Message::wstream(true) << "failed creating " << name() << ".gf\n";
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
 *  @param[out] gfname name of GF font file
 *  @return true on success */
bool PhysicalFont::createGF (string &gfname) const {
	SignalHandler::instance().check();
	gfname = name()+".gf";
	MetafontWrapper mf(name());
	bool ok = mf.make("ljfour", METAFONT_MAG); // call Metafont if necessary
	return ok && mf.success() && getMetrics();
}


/** Traces all glyphs of the current font and stores them in the cache. If caching is disabled, nothing happens.
 *  @param[in] includeCached if true, glyphs already cached are traced again
 *  @param[in] cb optional callback methods called by the tracer
 *  @return number of glyphs traced */
int PhysicalFont::traceAllGlyphs (bool includeCached, GFGlyphTracer::Callback *cb) const {
	int count = 0;
	if (type() == MF && CACHE_PATH) {
		if (const FontMetrics *metrics = getMetrics()) {
			int fchar = metrics->firstChar();
			int lchar = metrics->lastChar();
			string gfname;
			Glyph glyph;
			if (createGF(gfname)) {
				_cache.read(name().c_str(), CACHE_PATH);
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
bool PhysicalFont::getExactGlyphBox(int c, BoundingBox& bbox, GFGlyphTracer::Callback* cb) const {
	Glyph glyph;
	if (getGlyph(c, glyph, cb)) {
		glyph.computeBBox(bbox);
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


Font* VirtualFont::create (string name, UInt32 checksum, double dsize, double ssize) {
	return new VirtualFontImpl(name, checksum, dsize, ssize);
}


//////////////////////////////////////////////////////////////////////////////


PhysicalFontImpl::PhysicalFontImpl (string name, int fontindex, UInt32 cs, double ds, double ss, PhysicalFont::Type type)
	: TFMFont(name, cs, ds, ss),
	_filetype(type), _fontIndex(fontindex), _fontMapEntry(Font::fontMapEntry()), _encodingPair(Font::encoding()), _localCharMap(0)
{
}


PhysicalFontImpl::~PhysicalFontImpl () {
	if (CACHE_PATH)
		_cache.write(CACHE_PATH);
	if (!KEEP_TEMP_FILES)
		tidy();
	delete _localCharMap;
}


const FontEncoding* PhysicalFontImpl::encoding () const {
	if (!_encodingPair.enc1())
		return 0;
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
	else if (type() != MF) {
		FontEngine::instance().setFont(*this);
		if ((_localCharMap = FontEngine::instance().createCustomToUnicodeMap()) != 0)
			_charmapID = FontEngine::instance().setCustomCharMap();
		else
			_charmapID = FontEngine::instance().setUnicodeCharMap();
	}
	return true;
}


/** Returns the Unicode point for a given DVI character. */
UInt32 PhysicalFontImpl::unicode (UInt32 c) const {
	if (type() == MF)
		return Font::unicode(c);
	Character chr = decodeChar(c);
	if (type() == PFB) {
		// try to get the Unicode point from the character name
		string glyphname = glyphName(c);
		UInt32 codepoint;
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
		if (UInt32 mapped_char = _localCharMap->valueAt(chr.number()))
			return mapped_char;
	}
	// No Unicode equivalent found in the font file.
	// Now we should look for a smart alternative but at the moment
	// it's sufficient to simply choose a valid unused codepoint.
	return Unicode::charToCodepoint(chr.number());
}


/** Delete all temporary font files created by Metafont. */
void PhysicalFontImpl::tidy () const {
	if (type() == MF) {
		const char *ext[] = {"gf", "tfm", "log", 0};
		for (const char **p=ext; *p; ++p) {
			if (FileSystem::exists(name()+"."+(*p)))
				FileSystem::remove(name()+"."+(*p));
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

string NativeFont::uniqueName (const string &path, const FontStyle &style) {
	static map<string, int> ids;
	ostringstream oss;
	oss << path << "b" << style.bold << "e" << style.extend << "s" << style.slant;
	map<string, int>::iterator it = ids.find(oss.str());
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
				return PhysicalFont::OTF;
			if (ext == "ttf")
				return PhysicalFont::TTF;
			if (ext == "pfb")
				return PhysicalFont::PFB;
		}
	}
	return PhysicalFont::UNKNOWN;
}


double NativeFont::charWidth (int c) const {
	FontEngine::instance().setFont(*this);
	int upem = FontEngine::instance().getUnitsPerEM();
	double w = upem ? (scaledSize()*FontEngine::instance().getAdvance(c)/upem*_style.extend) : 0;
	w += fabs(_style.slant*charHeight(c));
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
	return upem ? (-scaledSize()*FontEngine::instance().getDescender()/upem) : 0;
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


Character NativeFontImpl::decodeChar (UInt32 c) const {
	return Character(Character::INDEX, c);
}


UInt32 NativeFontImpl::unicode (UInt32 c) const {
	UInt32 ucode = _toUnicodeMap.valueAt(c);
	return Unicode::charToCodepoint(ucode);
}

//////////////////////////////////////////////////////////////////////////////

VirtualFontImpl::VirtualFontImpl (string name, UInt32 cs, double ds, double ss)
	: TFMFont(name, cs, ds, ss)
{
}


VirtualFontImpl::~VirtualFontImpl () {
	// delete dvi vectors received by VFReaderAction
	for (map<UInt32, DVIVector*>::iterator i=_charDefs.begin(); i != _charDefs.end(); ++i)
		delete i->second;
}


const char* VirtualFontImpl::path () const {
	return FileFinder::lookup(name()+".vf");
}


void VirtualFontImpl::assignChar (UInt32 c, DVIVector *dvi) {
	if (dvi) {
		if (_charDefs.find(c) == _charDefs.end())
			_charDefs[c] = dvi;
		else
			delete dvi;
	}
}


const vector<UInt8>* VirtualFontImpl::getDVI (int c) const {
	map<UInt32,DVIVector*>::const_iterator it = _charDefs.find(c);
	return (it == _charDefs.end() ? 0 : it->second);
}

