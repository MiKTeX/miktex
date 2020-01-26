/* secondary.{cc,hh} -- code for generating fake glyphs
 *
 * Copyright (c) 2003-2019 Eddie Kohler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 */

#include <config.h>
#include "secondary.hh"
#include "metrics.hh"
#include "automatic.hh"
#include "otftotfm.hh"
#include "util.hh"
#include <efont/t1bounds.hh>
#include <efont/t1font.hh>
#include <efont/t1rw.hh>
#include <efont/otfname.hh>
#include <efont/otfos2.hh>
#include <efont/otfpost.hh>
#include <efont/t1csgen.hh>
#include <efont/t1unparser.hh>
#include <efont/ttfcs.hh>
#include <lcdf/straccum.hh>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <algorithm>

enum {
    U_EXCLAMDOWN = 0x00A1,      // U+00A1 INVERTED EXCLAMATION MARK
    U_DEGREE = 0x00B0,          // U+00B0 DEGREE SIGN
    U_QUESTIONDOWN = 0x00BF,    // U+00BF INVERTED QUESTION MARK
    U_IJ = 0x0132,              // U+0132 LATIN CAPITAL LIGATURE IJ
    U_ij = 0x0133,              // U+0133 LATIN SMALL LIGATURE IJ
    U_DOTLESSJ = 0x0237,        // U+0237 LATIN SMALL LETTER DOTLESS J
    U_RINGABOVE = 0x02DA,       // U+02DA RING ABOVE
    U_COMBININGRINGABOVE = 0x030A, // U+030A COMBINING RING ABOVE
    U_CWM = 0x200C,             // U+200C ZERO WIDTH NON-JOINER
    U_ENDASH = 0x2013,          // U+2013 EN DASH
    U_PERTENTHOUSAND = 0x2031,  // U+2031 PER TEN THOUSAND SIGN
    U_INTERROBANG = 0x203D,     // U+203D INTERROBANG
    U_FRACTION = 0x2044,        // U+2044 FRACTION SLASH
    U_CENTIGRADE = 0x2103,      // U+2103 DEGREE CELSIUS
    U_ASTERISKMATH = 0x2217,    // U+2217 ASTERISK OPERATOR
    U_BARDBL = 0x2225,          // U+2225 PARALLEL TO
    U_VISIBLESPACE = 0x2423,    // U+2423 OPEN BOX
    U_DBLBRACKETLEFT = 0x27E6,  // U+27E6 MATHEMATICAL LEFT WHITE SQUARE BRACKET
    U_DBLBRACKETRIGHT = 0x27E7, // U+27E7 MATHEMATICAL RIGHT WHITE SQUARE BRACKET
    U_INTERROBANGDOWN = 0x2E18, // U+2E18 INVERTED INTERROBANG
    U_ALTSELECTOR = 0xD802,     // invalid Unicode
    U_CAPITALCWM = 0xD809,      // invalid Unicode
    U_ASCENDERCWM = 0xD80A,     // invalid Unicode
    U_TWELVEUDASH = 0xD80C,     // invalid Unicode
    U_RINGFITTED = 0xD80D,      // invalid Unicode

    // BEGIN BACKWARDS COMPATIBILITY -- newer texglyphlist.txt does not include
    // these code points
    U_SS = 0xD800,              // invalid Unicode
    U_SSSMALL = 0xD803,         // invalid Unicode
    U_FFSMALL = 0xD804,         // invalid Unicode
    U_FISMALL = 0xD805,         // invalid Unicode
    U_FLSMALL = 0xD806,         // invalid Unicode
    U_FFISMALL = 0xD807,        // invalid Unicode
    U_FFLSMALL = 0xD808,        // invalid Unicode
    // END BACKWARDS COMPATIBILITY

    U_VS1 = 0xFE00,
    U_VS16 = 0xFE0F,
    U_VS17 = 0xE0100,
    U_VS256 = 0xE01FF,
    U_DOTLESSJ_2 = 0xF6BE,
    U_THREEQUARTERSEMDASH = 0xF6DE,
    U_FSMALL = 0xF766,
    U_ISMALL = 0xF769,
    U_LSMALL = 0xF76C,
    U_SSMALL = 0xF773,
    U_MATHDOTLESSJ = 0x1D6A5    // U+1D6A5 MATHEMATICAL ITALIC SMALL DOTLESS J
};


FontInfo::FontInfo(const Efont::OpenType::Font *otf_, ErrorHandler *errh)
    : otf(otf_), cmap(0), cff_file(0), cff(0), post(0), name(0), _nglyphs(-1),
      _got_glyph_names(false), _ttb_program(0), _override_is_fixed_pitch(false),
      _override_italic_angle(false), _override_x_height(x_height_auto)
{
    cmap = new Efont::OpenType::Cmap(otf->table("cmap"), errh);
    assert(cmap->ok());

    if (String cff_string = otf->table("CFF")) {
        cff_file = new Efont::Cff(cff_string, otf->units_per_em(), errh);
        if (!cff_file->ok())
            return;
        Efont::Cff::FontParent *fp = cff_file->font(PermString(), errh);
        if (!fp || !fp->ok())
            return;
        if (!(cff = dynamic_cast<Efont::Cff::Font *>(fp))) {
            errh->error("CID-keyed fonts not supported");
            return;
        }
        _nglyphs = cff->nglyphs();
    }

    if (!cff) {
        post = new Efont::OpenType::Post(otf->table("post"), errh);
        // read number of glyphs from 'maxp' -- should probably be elsewhere
        if (Efont::OpenType::Data maxp = otf->table("maxp"))
            if (maxp.length() >= 6)
                _nglyphs = maxp.u16(4);
        if (_nglyphs < 0 && post->ok())
            _nglyphs = post->nglyphs();
    }

    name = new Efont::OpenType::Name(otf->table("name"), errh);
}

FontInfo::~FontInfo()
{
    delete cmap;
    delete cff_file;
    delete post;
    delete name;
    delete _ttb_program;
}

bool
FontInfo::ok() const
{
    if (cff)
        return cmap->ok() && cff->ok();
    else
        return post && post->ok() && name && name->ok();
}

bool
FontInfo::glyph_names(Vector<PermString> &glyph_names) const
{
    program()->glyph_names(glyph_names);
    return true;
}

int
FontInfo::glyphid(PermString name) const
{
    if (cff)
        return cff->glyphid(name);
    else {
        if (!_got_glyph_names) {
            glyph_names(_glyph_names);
            _got_glyph_names = true;
        }
        PermString *found = std::find(_glyph_names.begin(), _glyph_names.end(), name);
        if (found == _glyph_names.end())
            return 0;
        return found - _glyph_names.begin();
    }
}

String
FontInfo::family_name() const
{
    if (cff)
        return cff->dict_string(Efont::Cff::oFamilyName);
    else
        return name->english_name(Efont::OpenType::Name::N_FAMILY);
}

String
FontInfo::postscript_name() const
{
    if (cff)
        return cff->font_name();
    else
        return name->english_name(Efont::OpenType::Name::N_POSTSCRIPT);
}

const Efont::CharstringProgram *
FontInfo::program() const
{
    if (cff)
        return cff;
    else {
        if (!_ttb_program)
            _ttb_program = new Efont::TrueTypeBoundsCharstringProgram(otf);
        return _ttb_program;
    }
}

bool
FontInfo::is_fixed_pitch() const
{
    if (_override_is_fixed_pitch)
        return _is_fixed_pitch;
    else if (cff) {
        double d;
        return (cff->dict_value(Efont::Cff::oIsFixedPitch, &d) && d);
    } else
        return post->is_fixed_pitch();
}

double
FontInfo::italic_angle() const
{
    if (_override_italic_angle)
        return _italic_angle;
    else if (cff) {
        double d;
        (void) cff->dict_value(Efont::Cff::oItalicAngle, &d);
        return d;
    } else
        return post->italic_angle();
}

double FontInfo::x_height(const Transform& font_xform) const {
    if (_override_x_height == x_height_explicit)
        return _x_height;
    double x1 = -1, x2 = -1;
    if (_override_x_height != x_height_os2)
        // XXX what if 'x', 'm', 'z' were subject to substitution?
        x1 = char_one_bound(*this, font_xform, 3, false, units_per_em(),
                            (int) 'x', (int) 'm', (int) 'z', 0);
    if (_override_x_height != x_height_x)
        try {
            Efont::OpenType::Os2 os2(otf->table("OS/2"));
            x2 = (Point(0, os2.x_height()) * font_xform).y;
        } catch (Efont::OpenType::Bounds) {
        }
    static bool warned = false;
    if (_override_x_height == x_height_auto
        && x1 >= 0
        && x2 >= 0
        && fabs(x1 - x2) > units_per_em() / 50.) {
        if (!warned) {
            ErrorHandler* errh = ErrorHandler::default_handler();
            errh->warning("font x-height and height of %<x%> differ by %d%%", (int) (fabs(x1 - x2) * 100 / units_per_em()));
            errh->message("(The height of %<x%> is usually more reliable than the x-height, so I%,m\nusing that. Or try --use-x-height or --no-use-x-height.)\n");
            warned = true;
        }
        return x1;
    } else
        return x2 >= 0 ? x2 : x1;
}


/* */

Secondary::~Secondary()
{
}

bool
Secondary::encode_uni(int code, PermString name,
                      const uint32_t* uni_begin, const uint32_t* uni_end,
                      Metrics &metrics, ErrorHandler *errh)
{
    uint32_t uni = 0;
    if (uni_begin + 1 == uni_end)
        uni = *uni_begin;

    SettingSet set(this, metrics);
    int max_s = 0;
    while (uni_begin != uni_end) {
        int s = setting(*uni_begin, set, errh);
        if (s == 0)
            return false;
        max_s = (max_s > s ? max_s : s);
        ++uni_begin;
        set.checkpoint();
    }

    if (uni == U_ALTSELECTOR
        || (uni >= U_VS1 && uni <= U_VS16)
        || (uni >= U_VS17 && uni <= U_VS256)) {
        int selector = 0;
        if (uni >= U_VS1 && uni <= U_VS16)
            selector = uni - U_VS1 + 1;
        else if (uni >= U_VS17 && uni <= U_VS256)
            selector = uni - U_VS17 + 17;
        metrics.add_altselector_code(code, selector);
        name = selector ? permprintf("<vs%d>", selector) : PermString("<altselector>");
    }

    metrics.encode_virtual(code, name, 0, set.settings(), max_s > 1);
    return true;
}

T1Secondary::T1Secondary(const FontInfo &finfo, const String &font_name,
                         const String &otf_file_name)
    : Secondary(finfo), _font_name(font_name), _otf_file_name(otf_file_name),
      _units_per_em(finfo.units_per_em()),
      _xheight((int) ceil(finfo.x_height(Transform()))),
      _spacewidth(_units_per_em)
{
    double bounds[4], width;
    if (char_bounds(bounds, width, finfo, Transform(), ' '))
        _spacewidth = (int) ceil(width);
}

int
Secondary::setting(uint32_t uni, SettingSet& set, ErrorHandler *errh)
{
    if (_next)
        return _next->setting(uni, set, errh);
    else
        return 0;
}

SettingSet& SettingSet::show(int uni) {
    if (!ok_)
        return *this;
    int code = metrics_.unicode_encoding(uni);
    if (code < 0) {
        Glyph glyph = s_->_finfo.cmap->map_uni(uni);
        if (glyph != 0)
            code = metrics_.force_encoding(glyph);
    }
    if (code < 0) {
        ok_ = false;
        while (v_.size() > original_size_)
            v_.pop_back();
    } else {
        if (!v_.empty()
            && v_.back().op == Setting::SHOW
            && kern_type_)
            v_.push_back(Setting(kern_type_));
        v_.push_back(Setting(Setting::SHOW, code, metrics_.base_glyph(code)));
    }
    return *this;
}


static String dotlessj_file_name;

static String
dotlessj_dvips_include(const String &, const FontInfo &, ErrorHandler *)
{
    return "<" + pathname_filename(dotlessj_file_name);
}

int
T1Secondary::dotlessj_font(Metrics &metrics, ErrorHandler *errh, Glyph &dj_glyph)
{
    if (!_font_name || !_finfo.otf || !_finfo.cff)
        return -1;

    String dj_name;
    bool install_metrics;
    // XXX make sure dotlessj is for the main font?
    if ((dj_name = installed_metrics_font_name(_font_name, "dotlessj")))
        install_metrics = false;
    else {
        dj_name = suffix_font_name(_font_name, "--lcdfj");
        install_metrics = true;
    }

    // is dotlessj already mapped?
    for (int i = 0; i < metrics.n_mapped_fonts(); i++)
        if (metrics.mapped_font_name(i) == dj_name)
            return i;

    if (String filename = installed_type1_dotlessj(_otf_file_name, _finfo.cff->font_name(), (output_flags & G_DOTLESSJ), errh)) {

        // check for special case: "\0" means the font's "j" is already
        // dotless
        if (filename == String("\0", 1))
            return J_NODOT;

        // open dotless-j font file
        FILE *f = fopen(filename.c_str(), "rb");
        if (!f) {
            errh->error("%s: %s", filename.c_str(), strerror(errno));
            return -1;
        }

        // read font
        Efont::Type1Reader *reader;
        int c = getc(f);
        ungetc(c, f);
        if (c == 128)
            reader = new Efont::Type1PFBReader(f);
        else
            reader = new Efont::Type1PFAReader(f);
        Efont::Type1Font *font = new Efont::Type1Font(*reader);
        delete reader;

        if (!font->ok()) {
            errh->error("%s: no glyphs in dotless-J font", filename.c_str());
            delete font;
            return -1;
        }

        // find dotless-J character
        Vector<PermString> glyph_names;
        font->glyph_names(glyph_names);
        Vector<PermString>::iterator g = std::find(glyph_names.begin(), glyph_names.end(), "uni0237");
        if (g == glyph_names.end()) {
            errh->error("%s: dotless-J font has no %<uni0237%> glyph", filename.c_str());
            delete font;
            return -1;
        }
        dj_glyph = g - glyph_names.begin();

        // create metrics for dotless-J
        if (install_metrics) {
            Metrics dj_metrics(font, 256);
            dj_metrics.encode('j', U_DOTLESSJ, dj_glyph);
            ::dotlessj_file_name = filename;
            output_metrics(dj_metrics, font->font_name(), -1, _finfo, String(), String(), dj_name, dotlessj_dvips_include, errh);
        } else if (verbose)
            errh->message("using %<%s%> for dotless-J font metrics", dj_name.c_str());

        // add font to metrics
        return metrics.add_mapped_font(font, dj_name);

    } else
        return -1;
}

int
T1Secondary::setting(uint32_t uni, SettingSet& set, ErrorHandler *errh)
{
    Transform xform;
    extern int letterspace;

    if (set.show(uni).check())
        return 1;

    switch (uni) {

      case U_CWM:
      case U_ALTSELECTOR:
        set.push_back(Setting::RULE, 0, _xheight);
        return 1;

      case U_CAPITALCWM:
        set.push_back(Setting::RULE, 0, font_cap_height(_finfo, xform));
        return 1;

      case U_ASCENDERCWM:
        set.push_back(Setting::RULE, 0, font_ascender(_finfo, xform));
        return 1;

    case U_VISIBLESPACE: {
        int sb = (int) (0.050 * _units_per_em), h = (int) (0.150 * _units_per_em),
            lw = (int) (0.040 * _units_per_em);
        set.move(sb, -h);
        set.push_back(Setting::RULE, lw, h);
        set.push_back(Setting::RULE, _spacewidth, lw);
        set.push_back(Setting::RULE, lw, h);
        set.move(sb, h);
        return 2;
    }

    case U_SS:
        if (set.show('S').show('S').check())
            return 1;
        break;

    case U_SSSMALL:
        if (set.show(U_SSMALL).show(U_SSMALL).check()
            || set.show('s').show('s').check())
            return 1;
        break;

      case U_FFSMALL:
        if (set.show(U_FSMALL).show(U_FSMALL).check()
            || set.show('f').show('f').check())
            return 1;
        break;

      case U_FISMALL:
        if (set.show(U_FSMALL).show(U_ISMALL).check()
            || set.show('f').show('i').check())
            return 1;
        break;

      case U_FLSMALL:
        if (set.show(U_FSMALL).show(U_LSMALL).check()
            || set.show('f').show('l').check())
            return 1;
        break;

      case U_FFISMALL:
        if (set.show(U_FSMALL).show(U_FSMALL).show(U_ISMALL).check()
            || set.show('f').show('f').show('i').check())
            return 1;
        break;

      case U_FFLSMALL:
        if (set.show(U_FSMALL).show(U_FSMALL).show(U_LSMALL).check()
            || set.show('f').show('f').show('l').check())
            return 1;
        break;

      case U_IJ:
        if (set.show('I').show('J').check())
            return 1;
        break;

      case U_ij:
        if (set.show('i').show('j').check())
            return 1;
        break;

      case U_DOTLESSJ:
      case U_DOTLESSJ_2:
      case U_MATHDOTLESSJ: {
          Glyph dj_glyph;
          int which = dotlessj_font(set.metrics(), errh, dj_glyph);
          if (which >= 0) {
              set.push_back(Setting::FONT, which);
              set.push_back(Setting::SHOW, 'j', dj_glyph);
              return 2;
          } else if (which == J_NODOT && set.show('j').check())
              return 1;
          break;
      }

      case U_DBLBRACKETLEFT:
        if (set.show('[').check()) {
            if (!_finfo.is_fixed_pitch()) {
                double d = char_one_bound(_finfo, xform, 4, true, 0, '[', 0);
                set.move((int) (-0.666 * d - letterspace));
            }
            set.show('[');
            return 1;
        }
        break;

      case U_DBLBRACKETRIGHT:
        if (set.show(']').check()) {
            if (!_finfo.is_fixed_pitch()) {
                double d = char_one_bound(_finfo, xform, 4, true, 0, ']', 0);
                set.move((int) (-0.666 * d - letterspace));
            }
            set.show(']');
            return 1;
        }
        break;

      case U_BARDBL:
        if (set.show('|').check()) {
            if (!_finfo.is_fixed_pitch()) {
                double d = char_one_bound(_finfo, Transform(), 4, true, 0, '|', 0);
                set.move((int) (-0.333 * d - letterspace));
            }
            set.show('|');
            return 1;
        }
        break;

      case U_ASTERISKMATH: {
          double bounds[5];
          double dropdown = 0;
          if (char_bounds(bounds, bounds[4], _finfo, xform, '*'))
              dropdown += std::max(bounds[3], 0.) + std::min(bounds[1], 0.);
          if (char_bounds(bounds, bounds[4], _finfo, xform, '('))
              dropdown -= std::max(bounds[3], 0.) + std::min(bounds[1], 0.);
          int dy = (int) (-dropdown / 2);
          if (set.move(0, dy).show('*').move(0, -dy).check())
              return 1;
          break;
      }

      case U_TWELVEUDASH:
        if (set.show(U_ENDASH).check()) {
            if (!_finfo.is_fixed_pitch()) {
                double d = char_one_bound(_finfo, xform, 4, true, 0, U_ENDASH, 0);
                set.move((int) (_units_per_em * 0.667 - 2 * d - letterspace));
            }
            set.show(U_ENDASH);
            return 1;
        }
        break;

      case U_THREEQUARTERSEMDASH:
        if (set.show(U_ENDASH).check()) {
            if (!_finfo.is_fixed_pitch()) {
                double d = char_one_bound(_finfo, xform, 4, true, 0, U_ENDASH, 0);
                set.move((int) (_units_per_em * 0.750 - 2 * d - letterspace));
            }
            set.show(U_ENDASH);
            return 1;
        }
        break;

      case U_CENTIGRADE:
        // TODO: set italic correction to that of a 'C'
        if (set.show(U_DEGREE).kernx(true).show('C').kernx(false).check())
            return 1;
        break;

      case U_INTERROBANG: {
          double exclam_offset =
              (char_one_bound(_finfo, xform, 4, true, 0, '?', 0)
               - char_one_bound(_finfo, xform, 4, true, 0, '!', 0)) * 0.5
              + 0.050 * _units_per_em;
          if (set.push_back(Setting::PUSH).move((int) exclam_offset)
              .show('!').push_back(Setting::POP).show('?').check())
              return 1;
          break;
      }

      case U_INTERROBANGDOWN: {
          double exclam_offset =
              (char_one_bound(_finfo, xform, 4, true, 0, U_QUESTIONDOWN, 0)
               - char_one_bound(_finfo, xform, 4, true, 0, U_EXCLAMDOWN, 0)) * 0.5
              + 0.050 * _units_per_em;
          if (set.push_back(Setting::PUSH).move((int) exclam_offset)
              .show(U_EXCLAMDOWN).push_back(Setting::POP).show(U_QUESTIONDOWN).check())
              return 1;
          break;
      }

      case U_PERTENTHOUSAND:
        if (set.show(0xF661).kernx(true).show(U_FRACTION)
            .show(0xF655).show(0xF655).show(0xF655).kernx(false).check())
            return 1;
        break;

      case U_RINGFITTED: {
          int A_width = char_one_bound(_finfo, xform, 4, true, -_units_per_em, 'A', 0);
          uint32_t ring_char = U_RINGABOVE;
          int ring_width = char_one_bound(_finfo, xform, 4, true, -_units_per_em, ring_char, 0);
          if (ring_width <= -_units_per_em) {
              ring_char = U_COMBININGRINGABOVE;
              ring_width = char_one_bound(_finfo, xform, 4, true, -_units_per_em, ring_char, 0);
          }
          if (A_width > -_units_per_em && ring_width > -_units_per_em) {
              int offset = (A_width - ring_width) / 2;
              if (set.move(offset).show(ring_char).move(A_width - ring_width - offset).check()) {
                  return 1;
              }
          }
          break;
      }

    }

    // variant selectors get the same setting as ALTSELECTOR
    if ((uni >= U_VS1 && uni <= U_VS16) || (uni >= U_VS17 && uni <= U_VS256))
        return setting(U_ALTSELECTOR, set, errh);

    // otherwise, try other secondaries
    return Secondary::setting(uni, set, errh);
}


bool
char_bounds(double bounds[4], double& width, const FontInfo &finfo,
            const Transform &transform, uint32_t uni)
{
    if (Efont::OpenType::Glyph g = finfo.cmap->map_uni(uni))
        return Efont::CharstringBounds::bounds(transform, finfo.program()->glyph_context(g), bounds, width);
    else
        return false;
}

double
char_one_bound(const FontInfo &finfo, const Transform &transform,
               int dimen, bool max, double best, int uni, ...)
{
    double bounds[5];
    va_list val;
    va_start(val, uni);
    while (uni != 0) {
        if (char_bounds(bounds, bounds[4], finfo, transform, uni))
            if (max ? bounds[dimen] > best : bounds[dimen] < best)
                best = bounds[dimen];
        uni = va_arg(val, int);
    }
    va_end(val);
    return best;
}

double
font_cap_height(const FontInfo &finfo, const Transform &font_xform)
{
    try {
        Efont::OpenType::Os2 os2(finfo.otf->table("OS/2"));
        return os2.cap_height();
    } catch (Efont::OpenType::Bounds) {
        // XXX what if 'H', 'O', 'B' were subject to substitution?
        return char_one_bound(finfo, font_xform, 3, false, finfo.units_per_em(),
                              (int) 'H', (int) 'O', (int) 'B', 0);
    }
}

double
font_ascender(const FontInfo &finfo, const Transform &font_xform)
{
    try {
        Efont::OpenType::Os2 os2(finfo.otf->table("OS/2"));
        return os2.typo_ascender();
    } catch (Efont::OpenType::Bounds) {
        // XXX what if 'd', 'l' were subject to substitution?
        return char_one_bound(finfo, font_xform, 3, true,
                              finfo.x_height(font_xform),
                              (int) 'd', (int) 'l', 0);
    }
}
