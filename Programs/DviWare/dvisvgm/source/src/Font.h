/*************************************************************************
** Font.h                                                               **
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

#ifndef DVISVGM_FONT_H
#define DVISVGM_FONT_H

#include <map>
#include <string>
#include <vector>
#include "Character.h"
#include "CharMapID.h"
#include "Color.h"
#include "FontCache.h"
#include "FontEncoding.h"
#include "FontMap.h"
#include "FontMetrics.h"
#include "GFGlyphTracer.h"
#include "Glyph.h"
#include "GraphicsPath.h"
#include "MessageException.h"
#include "RangeMap.h"
#include "ToUnicodeMap.h"
#include "VFActions.h"
#include "VFReader.h"
#include "types.h"


struct FontStyle;


struct GlyphMetrics
{
	GlyphMetrics () : wl(0), wr(0), h(0), d(0) {}
	GlyphMetrics (double wwl, double wwr, double hh, double dd) : wl(wwl), wr(wwr), h(hh), d(dd) {}
	double wl, wr, h, d;
};


/** Abstract base for all font classes. */
struct Font {
	virtual ~Font () {}
	virtual Font* clone (double ds, double sc) const =0;
	virtual const Font* uniqueFont () const =0;
	virtual std::string name () const =0;
	virtual double designSize () const =0;
	virtual double scaledSize () const =0;
	virtual double scaleFactor () const     {return scaledSize()/designSize();}
	virtual double charWidth (int c) const =0;
	virtual double charDepth (int c) const =0;
	virtual double charHeight (int c) const =0;
	virtual double italicCorr (int c) const =0;
	virtual const FontMetrics* getMetrics () const =0;
	virtual const char* path () const =0;
	virtual const char* filename () const;
	virtual const FontEncoding* encoding () const;
	virtual bool getGlyph (int c, Glyph &glyph, GFGlyphTracer::Callback *cb=0) const =0;
	virtual void getGlyphMetrics (int c, bool vertical, GlyphMetrics &metrics) const;
	virtual UInt32 unicode (UInt32 c) const;
	virtual void tidy () const {}
	virtual bool findAndAssignBaseFontMap () {return true;}
	virtual bool verticalLayout () const     {return getMetrics() ? getMetrics()->verticalLayout() : false;}
	virtual bool verifyChecksums () const    {return true;}
	virtual int fontIndex () const           {return 0;}
	virtual const FontStyle* style () const  {return 0;}
	virtual Color color () const             {return Color::BLACK;}
	virtual const FontMap::Entry* fontMapEntry () const;
};


/** Empty font without any glyphs. Instances of this class are used
 *  if no physical or virtual font file can be found.
 *  The metric values returned by the member functions are based on cmr10. */
struct EmptyFont : public Font
{
	public:
		EmptyFont (std::string name) : _fontname(name) {}
		Font* clone (double ds, double sc) const  {return new EmptyFont(*this);}
		const Font* uniqueFont () const           {return this;}
		std::string name () const                 {return _fontname;}
		double designSize () const                {return 10;}    // cmr10 design size in pt
		double scaledSize () const                {return 10;}    // cmr10 scaled size in pt
		double charWidth (int c) const            {return 9.164;} // width of cmr10's 'M' in pt
		double charHeight (int c) const           {return 6.833;} // height of cmr10's 'M' in pt
		double charDepth (int c) const            {return 0;}
		double italicCorr (int c) const           {return 0;}
		const FontMetrics* getMetrics () const    {return 0;}
		const char* path () const                 {return 0;}
		bool getGlyph (int c, Glyph &glyph, GFGlyphTracer::Callback *cb=0) const {return false;}

	private:
		std::string _fontname;
};


/** Interface for all physical fonts. */
class PhysicalFont : public virtual Font
{
	public:
		enum Type {MF, OTF, PFB, TTC, TTF, UNKNOWN};

		static Font* create (std::string name, UInt32 checksum, double dsize, double ssize, PhysicalFont::Type type);
		static Font* create (std::string name, int fontindex, UInt32 checksum, double dsize, double ssize);
		virtual Type type () const =0;
		virtual bool getGlyph (int c, Glyph &glyph, GFGlyphTracer::Callback *cb=0) const;
		virtual bool getExactGlyphBox (int c, BoundingBox &bbox, GFGlyphTracer::Callback *cb=0) const;
		virtual bool getExactGlyphBox (int c, GlyphMetrics &metrics, bool vertical, GFGlyphTracer::Callback *cb=0) const;
		virtual bool isCIDFont () const;
		virtual int hAdvance () const;
		virtual std::string familyName () const;
		virtual std::string styleName () const;
		virtual double hAdvance (int c) const;
		virtual double vAdvance (int c) const;
		std::string glyphName (int c) const;
		virtual int unitsPerEm () const;
		virtual double scaledAscent () const;
		virtual int ascent () const;
		virtual int descent () const;
		virtual int traceAllGlyphs (bool includeCached, GFGlyphTracer::Callback *cb=0) const;
		virtual int collectCharMapIDs (std::vector<CharMapID> &charmapIDs) const;
		virtual CharMapID getCharMapID () const =0;
		virtual void setCharMapID (const CharMapID &id) {}
		virtual Character decodeChar (UInt32 c) const;
		const char* path () const;

	protected:
		bool createGF (std::string &gfname) const;

	public:
		static bool EXACT_BBOX;
		static bool KEEP_TEMP_FILES;
		static const char *CACHE_PATH; ///< path to cache directory (0 if caching is disabled)
		static double METAFONT_MAG;    ///< magnification factor for Metafont calls

	protected:
		static FontCache _cache;
};


/** Interface for all virtual fonts. */
class VirtualFont : public virtual Font
{
	friend class FontManager;
	public:
		typedef std::vector<UInt8> DVIVector;

	public:
		static Font* create (std::string name, UInt32 checksum, double dsize, double ssize);
		virtual const DVIVector* getDVI (int c) const =0;
		bool getGlyph (int c, Glyph &glyph, GFGlyphTracer::Callback *cb=0) const {return false;}

	protected:
		virtual void assignChar (UInt32 c, DVIVector *dvi) =0;
};


class TFMFont : public virtual Font
{
	public:
		TFMFont (std::string name, UInt32 checksum, double dsize, double ssize);
		~TFMFont ();
		const FontMetrics* getMetrics () const;
		std::string name () const   {return _fontname;}
		double designSize () const  {return _dsize;}
		double scaledSize () const  {return _ssize;}
		double charWidth (int c) const;
		double charDepth (int c) const;
		double charHeight (int c) const;
		double italicCorr (int c) const;
		bool verifyChecksums () const;

	private:
		mutable FontMetrics *_metrics;
		std::string _fontname;
		UInt32 _checksum; ///< cheksum to be compared with TFM checksum
		double _dsize;    ///< design size in PS point units
		double _ssize;    ///< scaled size in PS point units
};


class PhysicalFontProxy : public PhysicalFont
{
	friend class PhysicalFontImpl;
	public:
		Font* clone (double ds, double sc) const    {return new PhysicalFontProxy(*this, ds, sc);}
		const Font* uniqueFont () const             {return _pf;}
		std::string name () const                   {return _pf->name();}
		double designSize () const                  {return _dsize;}
		double scaledSize () const                  {return _ssize;}
		double charWidth (int c) const              {return _pf->charWidth(c);}
		double charDepth (int c) const              {return _pf->charDepth(c);}
		double charHeight (int c) const             {return _pf->charHeight(c);}
		double italicCorr (int c) const             {return _pf->italicCorr(c);}
		const FontMetrics* getMetrics () const      {return _pf->getMetrics();}
		Type type () const                          {return _pf->type();}
		UInt32 unicode (UInt32 c) const             {return _pf->unicode(c);}
		int fontIndex () const                      {return _pf->fontIndex();}
		const FontStyle* style () const             {return _pf->style();}
		const FontMap::Entry* fontMapEntry () const {return _pf->fontMapEntry();}
		const FontEncoding* encoding () const       {return _pf->encoding();}
		CharMapID getCharMapID () const             {return _pf->getCharMapID();}
		int collectCharMapIDs (std::vector<CharMapID> &charmapIDs) const {return _pf->collectCharMapIDs(charmapIDs);}

	protected:
		PhysicalFontProxy (const PhysicalFont *font, double ds, double ss) : _pf(font), _dsize(ds), _ssize(ss) {}
		PhysicalFontProxy (const PhysicalFontProxy &proxy, double ds, double ss) : _pf(proxy._pf), _dsize(ds), _ssize(ss) {}

	private:
		const PhysicalFont *_pf;
		double _dsize;  ///< design size in PS point units
		double _ssize;  ///< scaled size in PS point units
};


class PhysicalFontImpl : public PhysicalFont, public TFMFont
{
	friend class PhysicalFont;
	public:
		~PhysicalFontImpl();
		Font* clone (double ds, double ss) const    {return new PhysicalFontProxy(this, ds, ss);}
		const Font* uniqueFont () const             {return this;}
		Type type () const                          {return _filetype;}
		int fontIndex() const                       {return _fontIndex;}
		const FontStyle* style () const             {return _fontMapEntry ? &_fontMapEntry->style : 0;}
		const FontMap::Entry* fontMapEntry () const {return _fontMapEntry;}
		const FontEncoding* encoding () const;
		UInt32 unicode (UInt32 c) const;
		bool findAndAssignBaseFontMap ();
		void tidy () const;
		CharMapID getCharMapID () const             {return _charmapID;}

	protected:
		PhysicalFontImpl (std::string name, int fontindex, UInt32 checksum, double dsize, double ssize, PhysicalFont::Type type);

	private:
		Type _filetype;
		int _fontIndex;
		const FontMap::Entry *_fontMapEntry;
		FontEncodingPair _encodingPair;
		CharMapID _charmapID;  ///< ID of the font's charmap to use
		const RangeMap *_localCharMap;
};


class NativeFont : public PhysicalFont
{
	public:
		virtual NativeFont* clone (double ptsize, const FontStyle &style, Color color) const =0;
		virtual Font* clone (double ds, double sc) const =0;
		std::string name () const;
		Type type () const;
		double designSize () const               {return _ptsize;}
		double scaledSize () const               {return _ptsize;}
		double charWidth (int c) const;
		double charDepth (int c) const;
		double charHeight (int c) const;
		double italicCorr (int c) const          {return 0;}
		const FontMetrics* getMetrics () const   {return 0;}
		const FontStyle* style () const          {return &_style;}
		Color color () const                     {return _color;}
		const FontMap::Entry* fontMapEntry () const {return 0;}
		static std::string uniqueName (const std::string &path, const FontStyle &style);

	protected:
		NativeFont (double ptsize, const FontStyle &style, Color color) : _ptsize(ptsize), _style(style), _color(color) {}

	private:
		double _ptsize;    ///< font size in PS point units
		FontStyle _style;
		Color _color;
};


class NativeFontProxy : public NativeFont
{
	friend class NativeFontImpl;
	public:
		NativeFont* clone (double ptsize, const FontStyle &style, Color color) const {
			return new NativeFontProxy(this, ptsize, style, color);
		}

		Font* clone (double ds, double sc) const {return new NativeFontProxy(this , sc, *style(), color());}
		const Font* uniqueFont () const          {return _nfont;}
		const char* path () const                {return _nfont->path();}
		int fontIndex () const                   {return _nfont->fontIndex();}
		Character decodeChar (UInt32 c) const    {return _nfont->decodeChar(c);}
		UInt32 unicode (UInt32 c) const          {return _nfont->unicode(c);}
		CharMapID getCharMapID () const          {return _nfont->getCharMapID();}

	protected:
		NativeFontProxy (const NativeFont *nfont, double ptsize, const FontStyle &style, Color color)
			: NativeFont(ptsize, style, color), _nfont(nfont) {}

	private:
		const NativeFont *_nfont;
};


class NativeFontImpl : public NativeFont
{
	public:
		NativeFontImpl (const std::string &fname, int fontIndex, double ptsize, const FontStyle &style, Color color)
			: NativeFont(ptsize, style, color), _path(fname), _fontIndex(fontIndex) {}

		NativeFont* clone (double ptsize, const FontStyle &style, Color color) const {
			return new NativeFontProxy(this, ptsize, style, color);
		}

		Font* clone (double ds, double sc) const {return new NativeFontProxy(this , sc, *style(), color());}
		const Font* uniqueFont () const          {return this;}
		const char* path () const                {return _path.c_str();}
		int fontIndex() const                    {return _fontIndex;}
		std::string fontFamily () const;
		bool findAndAssignBaseFontMap ();
		CharMapID getCharMapID () const          {return CharMapID::NONE;}
		Character decodeChar (UInt32 c) const;
		UInt32 unicode (UInt32 c) const;

	private:
		std::string _path;
		int _fontIndex;
		ToUnicodeMap _toUnicodeMap; ///< maps from char indexes to unicode points
};


class VirtualFontProxy : public VirtualFont
{
	friend class VirtualFontImpl;
	public:
		Font* clone (double ds, double ss) const {return new VirtualFontProxy(*this, ds, ss);}
		const Font* uniqueFont () const          {return _vf;}
		std::string name () const                {return _vf->name();}
		const DVIVector* getDVI (int c) const    {return _vf->getDVI(c);}
		double designSize () const               {return _dsize;}
		double scaledSize () const               {return _ssize;}
		double charWidth (int c) const           {return _vf->charWidth(c);}
		double charDepth (int c) const           {return _vf->charDepth(c);}
		double charHeight (int c) const          {return _vf->charHeight(c);}
		double italicCorr (int c) const          {return _vf->italicCorr(c);}
		const FontMetrics* getMetrics () const   {return _vf->getMetrics();}
		const char* path () const                {return _vf->path();}

	protected:
		VirtualFontProxy (const VirtualFont *font, double ds, double ss) : _vf(font), _dsize(ds), _ssize(ss) {}
		VirtualFontProxy (const VirtualFontProxy &proxy, double ds, double ss) : _vf(proxy._vf), _dsize(ds), _ssize(ss) {}
		void assignChar (UInt32 c, DVIVector *dvi) {delete dvi;}

	private:
		const VirtualFont *_vf;
		double _dsize;  ///< design size in PS point units
		double _ssize;  ///< scaled size in PS point units
};


class VirtualFontImpl : public VirtualFont, public TFMFont
{
	friend class VirtualFont;
	public:
		~VirtualFontImpl ();
		Font* clone (double ds, double ss) const {return new VirtualFontProxy(this, ds, ss);}
		const Font* uniqueFont () const   {return this;}
		const DVIVector* getDVI (int c) const;
		const char* path () const;

	protected:
		VirtualFontImpl (std::string name, UInt32 checksum, double dsize, double ssize);
		void assignChar (UInt32 c, DVIVector *dvi);

	private:
		std::map<UInt32, DVIVector*> _charDefs; ///< dvi subroutines defining the characters
};


struct FontException : public MessageException
{
	FontException (std::string msg) : MessageException(msg) {}
};

#endif
