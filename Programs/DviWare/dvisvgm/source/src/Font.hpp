/*************************************************************************
** Font.hpp                                                             **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2025 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef FONT_HPP
#define FONT_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "Character.hpp"
#include "CharMapID.hpp"
#include "Color.hpp"
#include "FontCache.hpp"
#include "FontEncoding.hpp"
#include "FontMap.hpp"
#include "FontMetrics.hpp"
#include "GFGlyphTracer.hpp"
#include "Glyph.hpp"
#include "GraphicsPath.hpp"
#include "MessageException.hpp"
#include "RangeMap.hpp"
#include "ToUnicodeMap.hpp"
#include "VFActions.hpp"
#include "VFReader.hpp"
#include "utility.hpp"

struct FontStyle;

struct GlyphMetrics {
	GlyphMetrics () : wl(0), wr(0), h(0), d(0) {}
	GlyphMetrics (double wwl, double wwr, double hh, double dd) : wl(wwl), wr(wwr), h(hh), d(dd) {}
	double wl, wr, h, d;
};

struct FontVisitor;

/** Abstract base for all font classes. */
class Font {
	public:
		virtual ~Font () =default;
		virtual std::unique_ptr<Font> clone (double ds, double sc) const =0;
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
		virtual bool getGlyph (int c, Glyph &glyph, GFGlyphTracer::Callback *callback) const =0;
		virtual void getGlyphMetrics (int c, bool vertical, GlyphMetrics &metrics) const;
		virtual uint32_t unicode (uint32_t c) const;
		virtual void tidy () const {}
		virtual bool findAndAssignBaseFontMap () {return true;}
		virtual bool verticalLayout () const     {return getMetrics() != nullptr && getMetrics()->verticalLayout();}
		virtual bool verifyChecksums () const    {return true;}
		virtual int fontIndex () const           {return 0;}
		virtual const FontStyle* style () const  {return nullptr;}
		virtual Color color () const             {return Color::BLACK;}
		virtual const FontMap::Entry* fontMapEntry () const;
		virtual void visit (FontVisitor &visitor) =0;
		virtual void visit (FontVisitor &visitor) const =0;
};


/** Empty font without any glyphs. Instances of this class are used
 *  if no physical or virtual font file can be found.
 *  The metric values returned by the member functions are based on cmr10. */
class EmptyFont : public Font {
	public:
		explicit EmptyFont (std::string name) : _fontname(std::move(name)) {}
		std::unique_ptr<Font> clone (double ds, double sc) const override  {return util::make_unique<EmptyFont>(*this);}
		const Font* uniqueFont () const override           {return this;}
		std::string name () const override                 {return _fontname;}
		double designSize () const override                {return 10;}    // cmr10 design size in pt
		double scaledSize () const override                {return 10;}    // cmr10 scaled size in pt
		double charWidth (int c) const override            {return 9.164;} // width of cmr10's 'M' in pt
		double charHeight (int c) const override           {return 6.833;} // height of cmr10's 'M' in pt
		double charDepth (int c) const override            {return 0;}
		double italicCorr (int c) const override           {return 0;}
		const FontMetrics* getMetrics () const override    {return nullptr;}
		const char* path () const override                 {return nullptr;}
		bool getGlyph (int c, Glyph &glyph, GFGlyphTracer::Callback*) const override {return false;}
		void visit (FontVisitor &visitor) override {}
		void visit (FontVisitor &visitor) const override {}

	private:
		std::string _fontname;
};


/** Interface for all physical fonts. */
class PhysicalFont : public virtual Font {
	public:
		enum class Type {MF, OTF, PFB, TTC, TTF, UNKNOWN};

		static std::unique_ptr<Font> create (const std::string &name, uint32_t checksum, double dsize, double ssize, PhysicalFont::Type type);
		static std::unique_ptr<Font> create (const std::string &name, int fontindex, uint32_t checksum, double dsize, double ssize);
		virtual Type type () const =0;
		bool getGlyph (int c, Glyph &glyph, GFGlyphTracer::Callback *cb) const override;
		virtual bool getExactGlyphBox (int c, BoundingBox &bbox, GFGlyphTracer::Callback *cb) const;
		virtual bool getExactGlyphBox (int c, GlyphMetrics &metrics, bool vertical, GFGlyphTracer::Callback *cb) const;
		virtual bool isCIDFont () const;
		virtual int hAverageAdvance () const;
		virtual std::string familyName () const;
		virtual std::string styleName () const;
		virtual double hAdvance (int c) const;
		virtual double vAdvance (int c) const;
		std::string glyphName (int c) const;
		virtual int unitsPerEm () const;
		virtual double scaledAscent () const;
		virtual int ascent () const;
		virtual int descent () const;
		virtual int traceAllGlyphs (bool includeCached, GFGlyphTracer::Callback *cb) const;
		virtual int collectCharMapIDs (std::vector<CharMapID> &charmapIDs) const;
		virtual CharMapID getCharMapID () const =0;
		virtual void setCharMapID (const CharMapID &id) {}
		virtual Character decodeChar (uint32_t c) const;
		virtual int charIndexByName (const std::string &charname) const;
		const char* path () const override;
		void visit (FontVisitor &visitor) override;
		void visit (FontVisitor &visitor) const override;

	protected:
		bool createGF (std::string &gfname) const;

	public:
		static bool EXACT_BBOX;
		static bool KEEP_TEMP_FILES;
		static std::string CACHE_PATH; ///< path to cache directory ("" if caching is disabled)
		static double METAFONT_MAG;    ///< magnification factor for Metafont calls

	protected:
		static FontCache _cache;
};


/** Interface for all virtual fonts. */
class VirtualFont : public virtual Font {
	friend class FontManager;
	public:
		using DVIVector = std::vector<char>;

	public:
		static std::unique_ptr<Font> create (const std::string &name, uint32_t checksum, double dsize, double ssize);
		virtual const DVIVector* getDVI (int c) const =0;
		bool getGlyph (int c, Glyph &glyph, GFGlyphTracer::Callback*) const override {return false;}
		void visit (FontVisitor &visitor) override;
		void visit (FontVisitor &visitor) const override;

	protected:
		virtual void assignChar (uint32_t c, DVIVector &&dvi) =0;
};


class TFMFont : public virtual Font {
	public:
		TFMFont (std::string name, uint32_t cs, double ds, double ss);
		const FontMetrics* getMetrics () const override;
		std::string name () const override  {return _fontname;}
		double designSize () const override {return _dsize;}
		double scaledSize () const override {return _ssize;}
		double charWidth (int c) const override;
		double charDepth (int c) const override;
		double charHeight (int c) const override;
		double italicCorr (int c) const override;
		bool verifyChecksums () const override;

	private:
		mutable std::unique_ptr<FontMetrics> _metrics;
		std::string _fontname;
		uint32_t _checksum; ///< cheksum to be compared with TFM checksum
		double _dsize;    ///< design size in PS point units
		double _ssize;    ///< scaled size in PS point units
};


class PhysicalFontRef : public PhysicalFont {
	friend class PhysicalFontImpl;
	public:
		std::unique_ptr<Font> clone (double ds, double sc) const override {
			return std::unique_ptr<PhysicalFontRef>(new PhysicalFontRef(*this, ds, sc));
		}

		const Font* uniqueFont () const override             {return _pf;}
		std::string name () const override                   {return _pf->name();}
		double designSize () const override                  {return _dsize;}
		double scaledSize () const override                  {return _ssize;}
		double charWidth (int c) const override              {return _pf->charWidth(c);}
		double charDepth (int c) const override              {return _pf->charDepth(c);}
		double charHeight (int c) const override             {return _pf->charHeight(c);}
		double italicCorr (int c) const override             {return _pf->italicCorr(c);}
		const FontMetrics* getMetrics () const override      {return _pf->getMetrics();}
		Type type () const override                          {return _pf->type();}
		uint32_t unicode (uint32_t c) const override         {return _pf->unicode(c);}
		int fontIndex () const override                      {return _pf->fontIndex();}
		const FontStyle* style () const override             {return _pf->style();}
		const FontMap::Entry* fontMapEntry () const override {return _pf->fontMapEntry();}
		const FontEncoding* encoding () const override       {return _pf->encoding();}
		CharMapID getCharMapID () const override             {return _pf->getCharMapID();}
		int collectCharMapIDs (std::vector<CharMapID> &charmapIDs) const override {return _pf->collectCharMapIDs(charmapIDs);}

	protected:
		PhysicalFontRef (const PhysicalFont *font, double ds, double ss) : _pf(font), _dsize(ds), _ssize(ss) {}
		PhysicalFontRef (const PhysicalFontRef &ref, double ds, double ss) : _pf(ref._pf), _dsize(ds), _ssize(ss) {}

	private:
		const PhysicalFont *_pf;
		double _dsize;  ///< design size in PS point units
		double _ssize;  ///< scaled size in PS point units
};


class PhysicalFontImpl : public PhysicalFont, public TFMFont {
	friend class PhysicalFont;
	public:
		~PhysicalFontImpl () override;

		std::unique_ptr<Font> clone (double ds, double ss) const override {
			return std::unique_ptr<PhysicalFontRef>(new PhysicalFontRef(this, ds, ss));
		}

		const Font* uniqueFont () const override             {return this;}
		Type type () const override                          {return _filetype;}
		int fontIndex() const override                       {return _fontIndex;}
		const FontStyle* style () const override;
		const FontEncoding* encoding () const override;
		uint32_t unicode (uint32_t c) const override;
		bool findAndAssignBaseFontMap () override;
		void tidy () const override;
		CharMapID getCharMapID () const override             {return _charmapID;}

	protected:
		PhysicalFontImpl (const std::string &name, int fontindex, uint32_t checksum, double dsize, double ssize, PhysicalFont::Type type);

	private:
		Type _filetype;
		int _fontIndex;
		FontEncodingPair _encodingPair;
		CharMapID _charmapID;  ///< ID of the font's charmap to use
		std::unique_ptr<const RangeMap> _localCharMap;
};


class NativeFont : public PhysicalFont {
	public:
		virtual std::unique_ptr<NativeFont> clone (double ptsize, const FontStyle &style, Color color) const =0;
		std::unique_ptr<Font> clone (double ds, double sc) const override =0;
		std::string name () const override;
		Type type () const override;
		const NativeFont* uniqueFont () const override   {return this;}
		double designSize () const override              {return _ptsize;}
		double scaledSize () const override              {return _ptsize;}
		double charWidth (int c) const override;
		double charDepth (int c) const override;
		double charHeight (int c) const override;
		double italicCorr (int c) const override;
		virtual double hAdvance (Character c) const;
		virtual double vAdvance (Character c) const;
		const FontMetrics* getMetrics () const override  {return nullptr;}
		const FontStyle* style () const override         {return &_style;}
		Color color () const override                    {return _color;}
		const FontMap::Entry* fontMapEntry () const override {return nullptr;}
		virtual void mapCharToUnicode (uint32_t c, uint32_t codepoint) =0;
		static std::string uniqueName (const std::string &path, const FontStyle &style);
		void visit (FontVisitor &visitor) override;
		void visit (FontVisitor &visitor) const override;

	protected:
		NativeFont (double ptsize, const FontStyle &style, Color color) : _ptsize(ptsize), _style(style), _color(color) {}
		using PhysicalFont::hAdvance;
		using PhysicalFont::vAdvance;

	private:
		double _ptsize;    ///< font size in PS point units
		FontStyle _style;
		Color _color;
};


class NativeFontRef : public NativeFont {
	friend class NativeFontImpl;
	public:
		std::unique_ptr<NativeFont> clone (double ptsize, const FontStyle &style, Color color) const override {
			return std::unique_ptr<NativeFontRef>(new NativeFontRef(this, ptsize, style, color));
		}

		std::unique_ptr<Font> clone (double ds, double sc) const override {
			return std::unique_ptr<NativeFontRef>(new NativeFontRef(this , sc, *style(), color()));
		}

		const NativeFont* uniqueFont () const override    {return _nfont;}
		const char* path () const override                {return _nfont->path();}
		int fontIndex () const override                   {return _nfont->fontIndex();}
		Character decodeChar (uint32_t c) const override  {return _nfont->decodeChar(c);}
		uint32_t unicode (uint32_t c) const override      {return _nfont->unicode(c);}
		CharMapID getCharMapID () const override          {return _nfont->getCharMapID();}
		bool verticalLayout() const override              {return _nfont->verticalLayout();}
		std::string name () const override                {return _nfont->name();}

		void mapCharToUnicode (uint32_t c, uint32_t codepoint) override {
			const_cast<NativeFont*>(_nfont)->mapCharToUnicode(c, codepoint);
		}

	protected:
		NativeFontRef (const NativeFont *nfont, double ptsize, const FontStyle &style, Color color)
			: NativeFont(ptsize, style, color), _nfont(nfont) {}

	private:
		const NativeFont *_nfont;
};


class NativeFontImpl : public NativeFont {
	public:
		NativeFontImpl (std::string fname, int fontIndex, double ptsize, const FontStyle &style, Color color)
			: NativeFont(ptsize, style, color), _path(std::move(fname)), _fontIndex(fontIndex) {}

		NativeFontImpl (std::string fname, std::string fontname, double ptsize);

		std::unique_ptr<NativeFont> clone (double ptsize, const FontStyle &style, Color color) const override {
			return std::unique_ptr<NativeFontRef>(new NativeFontRef(this, ptsize, style, color));
		}

		std::unique_ptr<Font> clone (double ds, double sc) const override {
			return std::unique_ptr<NativeFontRef>(new NativeFontRef(this , sc, *style(), color()));
		}

		const char* path () const override                {return _path.c_str();}
		int fontIndex () const override                   {return _fontIndex;}
		std::string name () const override                {return _name.empty() ? NativeFont::name() : _name;}
		bool findAndAssignBaseFontMap () override;
		CharMapID getCharMapID () const override          {return CharMapID::NONE;}
		Character decodeChar (uint32_t c) const override;
		uint32_t unicode (uint32_t c) const override;
		bool verticalLayout() const override;
		void mapCharToUnicode (uint32_t c, uint32_t codepoint) override;

	private:
		std::string _path;
		std::string _name;
		int _fontIndex = 0;
		ToUnicodeMap _toUnicodeMap; ///< maps from char indexes to unicode points
};


class VirtualFontRef : public VirtualFont {
	friend class VirtualFontImpl;
	public:
		std::unique_ptr<Font> clone (double ds, double ss) const override {
			return std::unique_ptr<VirtualFontRef>(new VirtualFontRef(*this, ds, ss));
		}

		const Font* uniqueFont () const override          {return _vf;}
		std::string name () const override                {return _vf->name();}
		const DVIVector* getDVI (int c) const override    {return _vf->getDVI(c);}
		double designSize () const override               {return _dsize;}
		double scaledSize () const override               {return _ssize;}
		double charWidth (int c) const override           {return _vf->charWidth(c);}
		double charDepth (int c) const override           {return _vf->charDepth(c);}
		double charHeight (int c) const override          {return _vf->charHeight(c);}
		double italicCorr (int c) const override          {return _vf->italicCorr(c);}
		const FontMetrics* getMetrics () const override   {return _vf->getMetrics();}
		const char* path () const override                {return _vf->path();}

	protected:
		VirtualFontRef (const VirtualFont *font, double ds, double ss) : _vf(font), _dsize(ds), _ssize(ss) {}
		VirtualFontRef (const VirtualFontRef &ref, double ds, double ss) : _vf(ref._vf), _dsize(ds), _ssize(ss) {}
		void assignChar (uint32_t c, DVIVector &&dvi) override {}

	private:
		const VirtualFont *_vf;
		double _dsize;  ///< design size in PS point units
		double _ssize;  ///< scaled size in PS point units
};


class VirtualFontImpl : public VirtualFont, public TFMFont {
	friend class VirtualFont;
	public:
		std::unique_ptr<Font> clone (double ds, double ss) const override {
			return std::unique_ptr<VirtualFontRef>(new VirtualFontRef(this, ds, ss));
		}

		const Font* uniqueFont () const override {return this;}
		const DVIVector* getDVI (int c) const override;
		const char* path () const override;

	protected:
		VirtualFontImpl (const std::string &name, uint32_t checksum, double dsize, double ssize);
		void assignChar (uint32_t c, DVIVector &&dvi) override;

	private:
		std::unordered_map<uint32_t, DVIVector> _charDefs; ///< dvi subroutines defining the characters
};


struct FontVisitor {
	virtual ~FontVisitor () =default;
	virtual void visited (const PhysicalFont *font) {}
	virtual void visited (const VirtualFont *font) {}
	virtual void visited (const NativeFont *font) {visited(static_cast<const PhysicalFont*>(font));}
	virtual void visited (PhysicalFont *font) {}
	virtual void visited (VirtualFont *font) {}
	virtual void visited (NativeFont *font) {visited(static_cast<PhysicalFont*>(font));}
};


/** This function works similar to dynamic_cast but only on pointers to Font classes.
 *  It uses double dispatch instead of RTTI and should therefore be faster.
 *  @param[in] font font pointer to be cast
 *  @return cast pointer on success, nullptr otherwise */
template <typename C>
C font_cast (typename util::set_const_of<Font>::by<typename std::remove_pointer<C>::type>::type *font) {
	struct : FontVisitor {
		void visited (C font) override {result = font;}
		C result = nullptr;
	} visitor;
	if (font)
		font->visit(visitor);
	return visitor.result;
}


struct FontException : public MessageException {
	explicit FontException (const std::string &msg) : MessageException(msg) {}
};

#endif
