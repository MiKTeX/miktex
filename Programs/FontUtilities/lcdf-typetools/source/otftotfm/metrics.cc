/* metrics.{cc,hh} -- an encoding during and after OpenType features
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "metrics.hh"
#include "dvipsencoding.hh"
#include "util.hh"
#include "glyphfilter.hh"
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include <lcdf/straccum.hh>

Metrics::Metrics(const Efont::CharstringProgram *font, int nglyphs)
    : _boundary_glyph(nglyphs), _emptyslot_glyph(nglyphs + 1),
      _design_units(1000), _units_per_em(font->units_per_em()),
      _liveness_marked(false)
{
    _encoding.assign(256, Char());
    add_mapped_font(font, String());
}

Metrics::~Metrics()
{
    for (Char *c = _encoding.begin(); c != _encoding.end(); c++)
        delete c->virtual_char;
}

int
Metrics::add_mapped_font(const Efont::CharstringProgram *font, const String &name)
{
    _mapped_fonts.push_back(font);
    _mapped_font_names.push_back(name);
    return _mapped_fonts.size() - 1;
}

void
Metrics::check() const
{
    // check invariants
    // 1. all 'ligatures' entries refer to valid characters
    // 2. all 'ligatures' entries with 'in1 == c' are in '_encoding[c].ligs'
    // 3. 'virtual_char' SHOW operations point to valid non-virtual chars
    for (int code = 0; code < _encoding.size(); code++) {
        const Char *ch = &_encoding[code];
        assert((ch->virtual_char != 0) == (ch->glyph == VIRTUAL_GLYPH));
        for (const Ligature *l = ch->ligatures.begin(); l != ch->ligatures.end(); l++)
            assert(valid_code(l->in2) && valid_code(l->out));
        for (const Kern *k = ch->kerns.begin(); k != ch->kerns.end(); k++)
            assert(valid_code(k->in2));
        if (const VirtualChar *vc = ch->virtual_char) {
            assert(vc->name);
            int font_number = 0;
            for (const Setting *s = vc->setting.begin(); s != vc->setting.end(); s++) {
                assert(s->valid_op());
                if (s->op == Setting::SHOW && font_number == 0)
                    assert(nonvirtual_code(s->x));
                else if (s->op == Setting::FONT)
                    font_number = s->x;
            }
        }
        assert(ch->built_in1 < 0 || valid_code(ch->built_in1));
        assert(ch->built_in2 < 0 || valid_code(ch->built_in2));
        assert((ch->built_in1 >= 0) == (ch->built_in2 >= 0));
        assert(ch->base_code < 0 || valid_code(ch->base_code));
        if (valid_code(ch->base_code)) {
            const Char *ch2 = &_encoding[ch->base_code];
            assert((!ch->virtual_char && ch->glyph)
                   || (!ch2->virtual_char && ch2->glyph));
        }
        if (ch->flag(Char::CONTEXT_ONLY))
            assert(ch->virtual_char && ch->built_in1 >= 0 && ch->built_in2 >= 0);
        if (ch->flag(Char::CONTEXT_ONLY))
            assert(ch->flag(Char::LIVE));
    }
}

PermString
Metrics::code_name(Code code) const
{
    if (code < 0 || code >= _encoding.size())
        return permprintf("<badcode%d>", code);
    else {
        const Char &ch = _encoding[code];
        if (ch.virtual_char)
            return ch.virtual_char->name;
        else if (ch.glyph == _boundary_glyph)
            return "<boundary>";
        else if (ch.glyph == _emptyslot_glyph)
            return "<emptyslot>";
        else if (ch.glyph >= 0 && ch.glyph < _mapped_fonts[0]->nglyphs())
            return _mapped_fonts[0]->glyph_name(ch.glyph);
        else
            return permprintf("<glyph%d>", ch.glyph);
    }
}


/*****************************************************************************/
/* encoding                                                                  */

Metrics::Code
Metrics::unicode_encoding(uint32_t uni) const
{
    for (const Char *ch = _encoding.begin(); ch < _encoding.end(); ch++)
        if (ch->unicode == uni)
            return ch - _encoding.begin();
    return -1;
}

Metrics::Code
Metrics::hard_encoding(Glyph g, Code after) const
{
    if (g < 0)
        return -1;
    int answer = -1, n = 0;
    for (int i = _encoding.size() - 1; i >= after; i--)
        if (_encoding[i].glyph == g)
            answer = i, n++;
    if (n < 2 && after == 0) {
        if (g >= _emap.size())
            _emap.resize(g + 1, -2);
        _emap[g] = answer;
    }
    return answer;
}

Metrics::Code
Metrics::force_encoding(Glyph g, int lookup_source)
{
    assert(g >= 0);
    int e = encoding(g, 0);
    if (e >= 0)
        return e;
    else {
        Char ch;
        ch.glyph = g;
        ch.base_code = _encoding.size();
        ch.lookup_source = lookup_source;
        _encoding.push_back(ch);
        assign_emap(g, ch.base_code);
        return ch.base_code;
    }
}

void
Metrics::encode(Code code, uint32_t uni, Glyph g)
{
    assert(code >= 0 && g >= 0 && g != VIRTUAL_GLYPH);
    if (code >= _encoding.size())
        _encoding.resize(code + 1, Char());
    _encoding[code].unicode = uni;
    _encoding[code].glyph = g;
    if (g > 0)
        _encoding[code].base_code = code;
    assert(!_encoding[code].virtual_char);
    assign_emap(g, code);
}

void
Metrics::encode_virtual(Code code, PermString name, uint32_t uni, const Vector<Setting> &v, bool base_char)
{
    assert(code >= 0 && v.size() > 0);
    if (code >= _encoding.size())
        _encoding.resize(code + 1, Char());
    _encoding[code].unicode = uni;
    _encoding[code].glyph = VIRTUAL_GLYPH;
    if (base_char)
        _encoding[code].flags |= Char::BASE_REP;
    assert(!_encoding[code].virtual_char);
    VirtualChar *vc = _encoding[code].virtual_char = new VirtualChar;
    vc->name = name;
    vc->setting = v;
    int font_number = 0;
    for (Setting *s = vc->setting.begin(); s != vc->setting.end(); s++) {
        assert(s->valid_op() && (s->op != Setting::SHOW || font_number != 0 || nonvirtual_code(s->x)));
        if (s->op == Setting::FONT)
            font_number = s->x;
    }
}

void
Metrics::apply_base_encoding(const String &font_name, const DvipsEncoding &dvipsenc, const Vector<int> &mapping)
{
    int font_number = -1;
    for (Char *c = _encoding.begin(); c != _encoding.end(); c++)
        if (c->glyph > 0 && !c->virtual_char && c->glyph < mapping.size()
            && mapping[c->glyph] >= 0) {
            if (font_number < 0)
                font_number = add_mapped_font(mapped_font(0), font_name);
            VirtualChar *vc = c->virtual_char = new VirtualChar;
            vc->name = dvipsenc.encoding(mapping[c->glyph]);
            vc->setting.push_back(Setting(Setting::FONT, font_number));
            vc->setting.push_back(Setting(Setting::SHOW, mapping[c->glyph], c->glyph));
            c->glyph = VIRTUAL_GLYPH;
            c->base_code = -1;
            c->flags = (c->flags & ~Char::BASE_LIVE) | Char::BASE_REP;
        }
}

void
Metrics::add_altselector_code(Code code, int altselector_type)
{
    for (Kern *k = _altselectors.begin(); k != _altselectors.end(); k++)
        if (k->in2 == code) {
            k->kern = altselector_type;
            return;
        }
    _altselectors.push_back(Kern(code, altselector_type));
}

bool
Metrics::base_glyphs(Vector<Glyph> &v, int size) const
{
    bool any = false;
    v.assign(_encoding.size(), 0);
    for (const Char *ch = _encoding.begin(); ch != _encoding.end(); ch++)
        if (ch->base_code >= 0 && ch->base_code < size) {
            v[ch->base_code] = ch->glyph;
            any = true;
        }
    return any;
}


/*****************************************************************************/
/* Char methods                                                              */

void
Metrics::Char::clear()
{
    glyph = 0;
    base_code = -1;
    unicode = 0;
    ligatures.clear();
    kerns.clear();
    delete virtual_char;
    virtual_char = 0;
    pdx = pdy = adx = 0;
    built_in1 = built_in2 = -1;
    lookup_source = -1;
    flags = 0;
}

void
Metrics::Char::swap(Char &c)
{
    std::swap(glyph, c.glyph);
    // NB: only a partial switch of base_code!!
    if (base_code < 0)
        base_code = c.base_code;
    c.base_code = -1;
    std::swap(unicode, c.unicode);
    ligatures.swap(c.ligatures);
    kerns.swap(c.kerns);
    std::swap(virtual_char, c.virtual_char);
    std::swap(pdx, c.pdx);
    std::swap(pdy, c.pdy);
    std::swap(adx, c.adx);
    std::swap(built_in1, c.built_in1);
    std::swap(built_in2, c.built_in2);
    std::swap(lookup_source, c.lookup_source);
    std::swap(flags, c.flags);
}


/*****************************************************************************/
/* manipulating ligature lists                                               */

Metrics::Ligature *
Metrics::ligature_obj(Code code1, Code code2)
{
    assert(valid_code(code1) && valid_code(code2));
    Char &ch = _encoding[code1];
    for (Ligature *l = ch.ligatures.begin(); l != ch.ligatures.end(); l++)
        if (l->in2 == code2)
            return l;
    return 0;
}

inline void
Metrics::new_ligature(Code in1, Code in2, Code out)
{
    assert(valid_code(in1) && valid_code(in2) && valid_code(out));
    _encoding[in1].ligatures.push_back(Ligature(in2, out));
}

inline void
Metrics::repoint_ligature(Code, Ligature *l, Code out)
{
    l->out = out;
}

void
Metrics::add_ligature(Code in1, Code in2, Code out)
{
    if (Ligature *l = ligature_obj(in1, in2)) {
        Char &ch = _encoding[l->out];
        if (ch.flags & Char::BUILT) {
            // move old ligatures to point to the true ligature
            for (Ligature *ll = ch.ligatures.begin(); ll != ch.ligatures.end(); ll++)
                add_ligature(out, ll->in2, ll->out);
            repoint_ligature(in1, l, out);
        }
    } else
        new_ligature(in1, in2, out);
}

Metrics::Code
Metrics::pair_code(Code in1, Code in2, int lookup_source)
{
    if (const Ligature *l = ligature_obj(in1, in2)) {
        if (lookup_source < 0)
            _encoding[l->out].flags &= ~Char::INTERMEDIATE;
        return l->out;
    } else {
        Char ch;
        ch.glyph = VIRTUAL_GLYPH;
        ch.flags = Char::BUILT | (lookup_source >= 0 ? Char::INTERMEDIATE : 0);
        VirtualChar *vc = ch.virtual_char = new VirtualChar;
        vc->name = permprintf("%s__%s", code_str(in1), code_str(in2));
        setting(in1, vc->setting, SET_INTERMEDIATE);
        vc->setting.push_back(Setting(Setting::KERN));
        setting(in2, vc->setting, SET_INTERMEDIATE);
        ch.built_in1 = in1;
        ch.built_in2 = in2;
        ch.lookup_source = lookup_source;
        _encoding.push_back(ch);
        new_ligature(in1, in2, _encoding.size() - 1);
        return _encoding.size() - 1;
    }
}

void
Metrics::remove_ligatures(Code in1, Code in2)
{
    if (in1 == CODE_ALL) {
        for (in1 = 0; in1 < _encoding.size(); in1++)
            remove_ligatures(in1, in2);
    } else {
        Char &ch = _encoding[in1];
        if (in2 == CODE_ALL)
            ch.ligatures.clear();
        else if (Ligature *l = ligature_obj(in1, in2)) {
            *l = ch.ligatures.back();
            ch.ligatures.pop_back();
        }
    }
}


/*****************************************************************************/
/* manipulating kern lists                                                   */

Metrics::Kern *
Metrics::kern_obj(Code in1, Code in2)
{
    assert(valid_code(in1) && valid_code(in2));
    Char &ch = _encoding[in1];
    for (Kern *k = ch.kerns.begin(); k != ch.kerns.end(); k++)
        if (k->in2 == in2)
            return k;
    return 0;
}

int
Metrics::kern(Code in1, Code in2) const
{
    assert(valid_code(in1) && valid_code(in2));
    const Char &ch = _encoding[in1];
    for (const Kern *k = ch.kerns.begin(); k != ch.kerns.end(); k++)
        if (k->in2 == in2)
            return k->kern;
    return 0;
}

void
Metrics::add_kern(Code in1, Code in2, int kern)
{
    if (Kern *k = kern_obj(in1, in2))
        k->kern += kern;
    else
        _encoding[in1].kerns.push_back(Kern(in2, kern));
}

void
Metrics::set_kern(Code in1, Code in2, int kern)
{
    if (in1 == CODE_ALL) {
        for (in1 = 0; in1 < _encoding.size(); in1++)
            set_kern(in1, in2, kern);
    } else {
        Char &ch = _encoding[in1];
        if (in2 == CODE_ALL) {
            assert(kern == 0);
            ch.kerns.clear();
        } else if (Kern *k = kern_obj(in1, in2)) {
            if (kern == 0) {
                *k = ch.kerns.back();
                ch.kerns.pop_back();
            } else
                k->kern = kern;
        } else if (kern != 0)
            ch.kerns.push_back(Kern(in2, kern));
    }
}

int
Metrics::reencode_right_ligkern(Code old_in2, Code new_in2)
{
    int nchanges = 0;
    for (Char *ch = _encoding.begin(); ch != _encoding.end(); ch++) {
        for (Ligature *l = ch->ligatures.begin(); l != ch->ligatures.end(); l++)
            if (l->in2 == old_in2) {
                if (new_in2 >= 0)
                    l->in2 = new_in2;
                else {
                    *l = ch->ligatures.back();
                    ch->ligatures.pop_back();
                    l--;
                }
                nchanges++;
            }
        for (Kern *k = ch->kerns.begin(); k != ch->kerns.end(); k++)
            if (k->in2 == old_in2) {
                if (new_in2 >= 0)
                    k->in2 = new_in2;
                else {
                    *k = ch->kerns.back();
                    ch->kerns.pop_back();
                    k--;
                }
                nchanges++;
            }
        // XXX?
        if (ch->context_setting(-1, old_in2) && new_in2 >= 0 && ch->built_in1 >= 0)
            ch->built_in2 = new_in2;
    }
    return nchanges;
}


/*****************************************************************************/
/* positioning                                                               */

void
Metrics::add_single_positioning(Code c, int pdx, int pdy, int adx)
{
    assert(valid_code(c));
    Char &ch = _encoding[c];
    ch.pdx += pdx;
    ch.pdy += pdy;
    ch.adx += adx;
}


/*****************************************************************************/
/* changed_context structure                                                 */

class Metrics::ChangedContext { public:
    ChangedContext(int ncodes);
    ~ChangedContext();
    typedef Metrics::Code Code;

    enum Context { CH_NONE = 0, CH_SOME = 1, CH_ALL = 2 };
    bool allowed(Code, bool left_context) const;
    bool pair_allowed(Code, Code) const;
    bool virgin(Code) const;
    void disallow(Code);
    void disallow_pair(Code, Code);
  private:
    Vector<Vector<uint32_t> *> _v;
    int _initial_ncodes;
    mutable Vector<uint32_t> _all_sentinel;
    ChangedContext(const ChangedContext &);
    ChangedContext &operator=(const ChangedContext &);
    static inline bool bit(const Vector<uint32_t> &, Code);
    inline void ensure_all(Code) const;
};

Metrics::ChangedContext::ChangedContext(int ncodes)
    : _v(ncodes, 0), _initial_ncodes(ncodes), _all_sentinel(((ncodes - 1) >> 5) + 1, 0xFFFFFFFFU)
{
}

Metrics::ChangedContext::~ChangedContext()
{
    for (Vector<uint32_t> **v = _v.begin(); v != _v.end(); v++)
        if (*v != &_all_sentinel)
            delete *v;
}

inline void
Metrics::ChangedContext::ensure_all(Code c) const
{
    if (c >= 0 && (c >> 5) >= _all_sentinel.size())
        _all_sentinel.resize((c >> 5) + 1, 0xFFFFFFFFU);
}

inline bool
Metrics::ChangedContext::bit(const Vector<uint32_t> &v, Code c)
{
    if (c < 0 || (c >> 5) >= v.size())
        return false;
    else
        return (v[c >> 5] & (1 << (c & 0x1F))) != 0;
}

bool
Metrics::ChangedContext::allowed(Code c, bool left_context) const
{
    if (c < 0)
        return false;
    else if (c >= _v.size())
        return left_context;
    else
        return (_v[c] != &_all_sentinel);
}

bool
Metrics::ChangedContext::pair_allowed(Code c1, Code c2) const
{
    ensure_all(c2);
    if (c1 < 0 || c2 < 0)
        return false;
    else if (c1 >= _v.size() || c2 >= _v.size() || !_v[c1])
        return true;
    else
        return !bit(*_v[c1], c2);
}

bool
Metrics::ChangedContext::virgin(Code c) const
{
    return (c >= 0 && (c >= _v.size() || _v[c] == 0));
}

void
Metrics::ChangedContext::disallow(Code c)
{
    assert(c >= 0);
    if (c >= _v.size())
        _v.resize(c + 1, 0);
    if (_v[c] != &_all_sentinel) {
        delete _v[c];
        _v[c] = &_all_sentinel;
    }
}

void
Metrics::ChangedContext::disallow_pair(Code c1, Code c2)
{
    assert(c1 >= 0 && c2 >= 0);
    if (c1 >= _v.size())
        _v.resize(c1 + 1, 0);
    if (!_v[c1])
        _v[c1] = new Vector<uint32_t>;
    if (_v[c1] != &_all_sentinel) {
        if ((c2 >> 5) >= _v[c1]->size())
            _v[c1]->resize((c2 >> 5) + 1, 0);
        (*_v[c1])[c2 >> 5] |= 1 << (c2 & 0x1F);
    }
}


/*****************************************************************************/
/* applying GSUB substitutions                                               */

void
Metrics::apply_single(Code cin, const Substitution *s, int lookup,
                ChangedContext &ctx, const GlyphFilter &glyph_filter,
                const Vector<PermString> &glyph_names)
{
    // check if encoded
    if (!ctx.allowed(cin, false))
        /* not encoded before this substitution began, or completely changed;
           ingore */
        return;

    // check if substitution of this code allowed
    if (!glyph_filter.allow_substitution(s->in_glyph(), glyph_names, unicode(cin)))
        return;

    // look for an allowed alternate
    Glyph out = -1;
    for (int i = 0; out < 0 && i < s->out_nglyphs(); i++)
        if (glyph_filter.allow_alternate(s->out_glyph(i), glyph_names, unicode(cin)))
            out = s->out_glyph(i);
    if (out < 0)                // no allowed alternate
        return;

    // apply substitution
    if (ctx.virgin(cin)) {
        // no one has changed this glyph yet, change it unilaterally
        assign_emap(s->in_glyph(), -2);
        assign_emap(out, cin);
        assert(!_encoding[cin].virtual_char);
        _encoding[cin].glyph = out;
    } else {
        // some contextual substitutions have changed this glyph, add
        // contextual substitutions for the remaining possibilities
        Code cout = force_encoding(out, lookup);
        for (Code right = 0; right < _encoding.size(); right++)
            if (_encoding[right].visible() && !_encoding[right].flag(Char::BUILT) && ctx.pair_allowed(cin, right)) {
                Code pair = pair_code(cout, right, lookup);
                _encoding[cout].flags &= ~Char::INTERMEDIATE;
                add_ligature(cin, right, pair);
            }
    }

    // no more substitutions for cin
    ctx.disallow(cin);
}

void
Metrics::apply_ligature(const Vector<Code> &in, const Substitution *s, int lookup)
{
    // build up the character pair
    int cin1 = in[0];
    for (const Code *inp = in.begin() + 1; inp < in.end() - 1; inp++)
        cin1 = pair_code(cin1, *inp, lookup);
    int cin2 = in.back();

    // build up the output character
    Vector<Code> out;
    s->all_out_glyphs(out);
    int cout = -1;
    for (Glyph *outp = out.begin(); outp < out.end(); outp++) {
        *outp = force_encoding(*outp, lookup);
        cout = (cout < 0 ? *outp : pair_code(cout, *outp, lookup));
    }
    _encoding[cout].flags &= ~Char::INTERMEDIATE;

    // check for replacing a fake ligature
    int old_out = -1;
    if (Ligature *l = ligature_obj(cin1, cin2)) {
        if (l->out == cout)     // already created this same ligature
            return;
        if (_encoding[l->out].flags & Char::BUILT)
            old_out = l->out;
    }

    // make the final ligature
    add_ligature(cin1, cin2, cout);

    //fprintf(stderr, "%s : %d/%s %d/%s => %d/%s [was %d/%s]\n", s->unparse().c_str(), cin1, code_str(cin1), cin2, code_str(cin2), cout, code_str(cout), old_out, code_str(old_out));

    // if appropriate, swap old ligatures to point to the new result
    if (old_out >= 0)
        for (Char *ch = _encoding.begin(); ch != _encoding.end(); ch++)
            for (Ligature *l = ch->ligatures.begin(); l != ch->ligatures.end(); l++)
                if (l->out == old_out)
                    repoint_ligature(ch - _encoding.begin(), l, cout);
}

void
Metrics::apply_simple_context_ligature(const Vector<Code> &codes,
             const Substitution *s, int lookup, ChangedContext &ctx,
             const GlyphFilter &glyph_filter,
             const Vector<PermString>& glyph_names)
{
    int nleft = s->left_nglyphs(), nin = s->in_nglyphs();
    assert(codes.size() >= 2);

    // check if context allows substitutions
    for (int i = 0; i < codes.size(); ++i) {
        if (!ctx.allowed(codes[i], i < nleft)
            || !glyph_filter.allow_substitution(s->in_glyph(i), glyph_names, unicode(codes[i])))
            return;
    }

    // check if any part of the combination has already changed
    int ncheck = nleft + (nin > 2 ? 2 : nin);
    if (ncheck == codes.size())
        --ncheck;
    for (const Code *inp = codes.begin(); inp < codes.begin() + ncheck; ++inp)
        if (!ctx.pair_allowed(inp[0], inp[1]))
            return;

    // mark this combination as changed if appropriate
    if (codes.size() == 2 && nin == 1)
        ctx.disallow_pair(codes[0], codes[1]);

    // actually apply ligature
    apply_ligature(codes, s, lookup);
}

bool
Metrics::next_encoding(Vector<Code> &codes, const Vector<Glyph> &glyphs) const
{
    if (!codes.size()) {
        codes.assign(glyphs.size(), 0);
        for (int i = 0; i < glyphs.size(); ++i)
            if ((codes[i] = encoding(glyphs[i], 0)) < 0)
                return false;
        return true;
    } else {
        for (int i = 0; i < glyphs.size(); ++i)
            if ((codes[i] = encoding(glyphs[i], codes[i] + 1)) >= 0)
                return true;
            else
                codes[i] = encoding(glyphs[i], 0);
        return false;
    }
}

int
Metrics::apply(const Vector<Substitution>& sv, bool allow_single, int lookup, const GlyphFilter& glyph_filter, const Vector<PermString>& glyph_names)
{
    Vector<Glyph> glyphs;
    Vector<Code> codes;

    // keep track of what substitutions we have performed
    ChangedContext ctx(_encoding.size());

    // loop over substitutions
    int failures = 0;
    for (const Substitution *s = sv.begin(); s != sv.end(); s++) {
        bool is_single = s->is_single() || s->is_alternate();
        bool is_apply_single = is_single && allow_single;
        bool is_apply_simple_context_ligature = !is_single && !s->is_multiple() && s->is_simple_context();

        if (is_apply_single || is_apply_simple_context_ligature) {
            s->all_in_glyphs(glyphs);
            for (codes.clear(); next_encoding(codes, glyphs); ) {
                if (is_apply_single)
                    apply_single(codes[0], s, lookup, ctx, glyph_filter, glyph_names);
                else
                    apply_simple_context_ligature(codes, s, lookup, ctx, glyph_filter, glyph_names);
            }
        } else
            failures++;
    }

    return sv.size() - failures;
}

void
Metrics::apply_alternates_single(Code cin, const Substitution *s, int lookup,
                const GlyphFilter &glyph_filter,
                const Vector<PermString> &glyph_names)
{
    for (const Kern *as = _altselectors.begin(); as != _altselectors.end(); as++)
        if (as->kern == 0) {
            Code last = cin;
            uint32_t u = unicode(cin);
            for (int i = 0; i < s->out_nglyphs(); i++)
                if (glyph_filter.allow_alternate(s->out_glyph(i), glyph_names, u)) {
                    Code out = force_encoding(s->out_glyph(i), lookup);
                    add_ligature(last, as->in2, out);
                    last = out;
                }
        } else if (as->kern <= s->out_nglyphs()) {
            Code out = force_encoding(s->out_glyph(as->kern - 1), lookup);
            add_ligature(cin, as->in2, out);
        }
}

void
Metrics::apply_alternates_ligature(const Vector<Code> &codes,
                const Substitution *s, int lookup,
                const GlyphFilter &glyph_filter,
                const Vector<PermString> &glyph_names)
{
    // check whether the output character is allowed
    if (!glyph_filter.allow_alternate(s->out_glyph(), glyph_names, 0))
        return;

    // find alternate selector and apply ligature if appropriate
    for (const Kern *as = _altselectors.begin(); as != _altselectors.end(); as++)
        if (as->kern == 0) {
            Vector<Code> lig(codes);
            lig.insert(lig.begin() + 1, as->in2);
            apply_ligature(lig, s, lookup);
        }
}

void
Metrics::apply_alternates(const Vector<Substitution>& sv, int lookup, const GlyphFilter& glyph_filter, const Vector<PermString>& glyph_names)
{
    Vector<Glyph> glyphs;
    Vector<Code> codes;

    for (const Substitution *s = sv.begin(); s != sv.end(); s++) {
        bool is_single = s->is_single() || s->is_alternate();
        if (is_single || s->is_ligature()) {
            s->all_in_glyphs(glyphs);
            for (codes.clear(); next_encoding(codes, glyphs); ) {
                if (is_single)
                    apply_alternates_single(codes[0], s, lookup, glyph_filter, glyph_names);
                else
                    apply_alternates_ligature(codes, s, lookup, glyph_filter, glyph_names);
            }
        }
    }
}


/*****************************************************************************/
/* applying GPOS positionings                                                */

static bool                     // returns old value
assign_bitvec(int*& bitvec, int e, int n)
{
    if (e >= 0 && e < n) {
        if (!bitvec) {
            bitvec = new int[((n - 1) >> 5) + 1];
            memset(bitvec, 0, sizeof(int) * (((n - 1) >> 5) + 1));
        }
        bool result = (bitvec[e >> 5] & (1 << (e & 0x1F))) != 0;
        bitvec[e >> 5] |= (1 << (e & 0x1F));
        return result;
    } else
        return false;
}

int
Metrics::apply(const Vector<Positioning>& pv)
{
    // keep track of what substitutions we have performed
    int *single_changed = 0;
    Vector<int *> pair_changed(_encoding.size(), 0);
    Vector<Glyph> glyphs;
    Vector<Code> codes;

    // loop over substitutions
    int success = 0;
    for (const Positioning *p = pv.begin(); p != pv.end(); p++) {
        bool is_single = p->is_single();
        if (is_single || p->is_pairkern()) {
            p->all_in_glyphs(glyphs);
            for (codes.clear(); next_encoding(codes, glyphs); )
                if (is_single) {
                    if (!assign_bitvec(single_changed, codes[0], _encoding.size())) {
                        _encoding[codes[0]].pdx += p->left().pdx;
                        _encoding[codes[0]].pdy += p->left().pdy;
                        _encoding[codes[0]].adx += p->left().adx;
                    }
                } else {
                    if (!assign_bitvec(pair_changed[codes[0]], codes[1], _encoding.size()))
                        add_kern(codes[0], codes[1], p->left().adx);
                }
            success++;
        }
    }

    delete[] single_changed;
    for (int i = 0; i < pair_changed.size(); i++)
        delete[] pair_changed[i];
    return success;
}


/*****************************************************************************/
/* liveness marking, Ligature3s                                              */

String Metrics::Ligature3::unparse(const Metrics& m) const {
    StringAccum sa;
    sa << '[' << m.code_name(in1) << ' ' << m.code_name(in2)
       << " -> " << m.code_name(out) << ']';
    return sa.take_string();
}

inline bool
operator<(const Metrics::Ligature3 &l1, const Metrics::Ligature3 &l2)
{
    // topological < : is l1's output one of l2's inputs?
    if (l1.out == l2.in1 || l1.out == l2.in2)
        return true;
    else
        return l1.in1 < l2.in1
            || (l1.in1 == l2.in1 && (l1.in2 < l2.in2
                                     || (l1.in2 == l2.in2 && l1.out < l2.out)));
}

void
Metrics::all_ligatures(Vector<Ligature3> &all_ligs) const
{
    /* Develop a topologically-sorted ligature list. */
    all_ligs.clear();
    for (Code code = 0; code < _encoding.size(); code++)
        for (const Ligature *l = _encoding[code].ligatures.begin(); l != _encoding[code].ligatures.end(); l++)
            all_ligs.push_back(Ligature3(code, l->in2, l->out));
    std::sort(all_ligs.begin(), all_ligs.end());
}

void
Metrics::mark_liveness(int size, const Vector<Ligature3> *all_ligs)
{
    _liveness_marked = true;
    bool changed;

    // make sure we have ligatures
    Vector<Ligature3> my_ligs;
    if (!all_ligs) {
        all_ligatures(my_ligs);
        all_ligs = &my_ligs;
    }

    /* Characters below 'size' are in both virtual and base encodings. */
    for (Char *ch = _encoding.begin(); ch < _encoding.begin() + size; ch++)
        if (ch->visible())
            ch->flags |= Char::LIVE | (ch->virtual_char ? 0 : Char::BASE_LIVE);

    /* Characters reachable from live chars by live ligatures are live. */
  redo_live_reachable:
    for (const Ligature3 *l = all_ligs->begin(); l != all_ligs->end(); l++)
        if (_encoding[l->in1].flag(Char::LIVE) && _encoding[l->in2].flag(Char::LIVE)) {
            Char &ch = _encoding[l->out];
            if (!ch.flag(Char::LIVE))
                ch.flags |= Char::LIVE | Char::CONTEXT_ONLY | (ch.virtual_char ? 0 : Char::BASE_LIVE);
            if (ch.flag(Char::CONTEXT_ONLY) && !ch.context_setting(l->in1, l->in2))
                ch.flags &= ~Char::CONTEXT_ONLY;
        }

    /* Characters reachable from context-only ligatures are live. */
    changed = false;
    for (Char *ch = _encoding.begin(); ch != _encoding.end(); ch++)
        if (ch->flag(Char::CONTEXT_ONLY)) {
            Char &ch1 = _encoding[ch->built_in1];
            Char &ch2 = _encoding[ch->built_in2];
            if (!ch1.flag(Char::LIVE) || !ch2.flag(Char::LIVE)) {
                ch1.flags |= Char::LIVE;
                ch2.flags |= Char::LIVE;
                changed = true;
            }
        }
    if (changed)
        goto redo_live_reachable;

    /* Characters reachable from live settings are base-live. */
    for (Char *ch = _encoding.begin(); ch != _encoding.end(); ch++)
        if (ch->flag(Char::LIVE))
            if (VirtualChar *vc = ch->virtual_char) {
                int font_number = 0;
                for (Setting *s = vc->setting.begin(); s != vc->setting.end(); s++)
                    if (s->op == Setting::SHOW && font_number == 0
                        && _encoding[s->x].base_code >= 0)
                        _encoding[s->x].flags |= Char::BASE_LIVE;
                    else if (s->op == Setting::FONT)
                        font_number = s->x;
            }
}

void
Metrics::reencode(const Vector<Code> &reencoding)
{
    for (Char *ch = _encoding.begin(); ch != _encoding.end(); ch++) {
        for (Ligature *l = ch->ligatures.begin(); l != ch->ligatures.end(); l++) {
            l->in2 = reencoding[l->in2];
            l->out = reencoding[l->out];
        }
        for (Kern *k = ch->kerns.begin(); k != ch->kerns.end(); k++)
            k->in2 = reencoding[k->in2];
        if (VirtualChar *vc = ch->virtual_char) {
            int font_number = 0;
            for (Setting *s = vc->setting.begin(); s != vc->setting.end(); s++)
                if (s->op == Setting::SHOW && font_number == 0)
                    s->x = reencoding[s->x];
                else if (s->op == Setting::FONT)
                    font_number = s->x;
        }
        if (ch->built_in1 >= 0) {
            ch->built_in1 = reencoding[ch->built_in1];
            ch->built_in2 = reencoding[ch->built_in2];
        }
        if (ch->base_code >= 0)
            ch->base_code = reencoding[ch->base_code];
    }
    _emap.clear();
}


/*****************************************************************************/
/* shrinking the encoding                                                    */

bool
Metrics::Char::context_setting(Code in1, Code in2) const
{
    // return true iff this character could represent the context setting of
    // 'in1' and 'in2'
    if (!virtual_char || ligatures.size())
        return false;
    else
        return (in1 == built_in1 || in2 == built_in2);
}

void
Metrics::cut_encoding(int size)
{
    /* Function makes it so that characters below 'size' do not point to
       characters above 'size', except for context ligatures. */

    /* Change "emptyslot"s to ".notdef"s. */
    for (Char *ch = _encoding.begin(); ch != _encoding.end(); ch++)
        if (ch->glyph == emptyslot_glyph()) {
            ch->glyph = 0;
            ch->base_code = -1;
            // 21.Feb.2007: Character isn't live any more.
            ch->flags &= ~(Char::BASE_LIVE | Char::LIVE);
        }

    /* Maybe we don't need to do anything else. */
    if (_encoding.size() <= size) {
        _encoding.resize(size, Char());
        return;
    }

    /* Need liveness markings. */
    if (!_liveness_marked)
        mark_liveness(size);

    /* Characters below 'size' are 'good'.
       Characters above 'size' are not 'good'. */
    Vector<int> good(_encoding.size(), 1);
    for (Code c = size; c < _encoding.size(); c++)
        good[c] = 0;

    /* Characters encoded via base_code are 'good', though. */
    for (Char *ch = _encoding.begin(); ch < _encoding.begin() + size; ch++)
        if (ch->base_code >= size)
            good[ch->base_code] = 1;

    /* Some fake characters might point beyond 'size'; remove them too. No
       need for a multipass algorithm since virtual chars never point to
       virtual chars. */
    for (Code c = 0; c < _encoding.size(); c++) {
        if (VirtualChar *vc = _encoding[c].virtual_char) {
            int font_number = 0;
            for (Setting *s = vc->setting.begin(); s != vc->setting.end(); s++)
                if (s->op == Setting::SHOW && font_number == 0 && !good[s->x]) {
                    _encoding[c].clear();
                    goto bad_virtual_char;
                } else if (s->op == Setting::FONT)
                    font_number = s->x;
        }
        if (c < size)
            good[c] = 1;
      bad_virtual_char: ;
    }

    /* Certainly none of the later ligatures or kerns will be meaningful. */
    for (Code c = size; c < _encoding.size(); c++) {
        _encoding[c].ligatures.clear();
        _encoding[c].kerns.clear();
    }

    /* Remove ligatures and kerns that point beyond 'size', except for valid
       context ligatures. Also remove ligatures that have non-live
       components. */
    /* 30.May.2005 -- Kerns might point involve a too-high character; kill
       them. */
    for (Code c = 0; c < size; c++) {
        Char &ch = _encoding[c];
        for (Ligature *l = ch.ligatures.begin(); l != ch.ligatures.end(); l++)
            if (!good[l->in2] || l->in2 >= size
                || (!good[l->out] && !_encoding[l->out].context_setting(c, l->in2))) {
                *l = ch.ligatures.back();
                ch.ligatures.pop_back();
                l--;
            }
        for (Kern *k = ch.kerns.begin(); k != ch.kerns.end(); k++)
            if (!good[k->in2] || k->in2 >= size) {
                *k = ch.kerns.back();
                ch.kerns.pop_back();
                k--;
            }
    }

    /* We are done! */
}

namespace {
// preference-sorting extra characters
enum {
    CONVENTIONAL_F_LIGATURE_SCORE = 4,
    CONVENTIONAL_F_F_LIGATURE_SCORE = 5,
    BASIC_LATIN_LOWER_SCORE = 3,
    BASIC_LATIN_UPPER_SCORE = 4,
    BASIC_LATIN_OTHER_SCORE = 5,
    LATIN1_SUPPLEMENT_SCORE = 6,
    LOW_16_SCORE = 7,
    OTHER_SCORE = 8,
    NOCHAR_SCORE = 100000,
    CONTEXT_PENALTY = 4
};

static int
unicode_score(uint32_t u)
{
    if (u == 0)
        return NOCHAR_SCORE;
    else if (u >= 'a' && u <= 'z')
        return BASIC_LATIN_LOWER_SCORE;
    else if (u >= 'A' && u <= 'Z')
        return BASIC_LATIN_UPPER_SCORE;
    else if (u < 0x0080)
        return BASIC_LATIN_OTHER_SCORE;
    else if (u < 0x0100)
        return LATIN1_SUPPLEMENT_SCORE;
    else if (u < 0x8000)
        return LOW_16_SCORE;
    else
        return OTHER_SCORE;
}

struct Slot {
    Metrics::Code old_code;
    Metrics::Code new_code;
    Metrics::Glyph glyph;
    int score;
    int lookup_source;
};

inline bool
operator<(const Slot &a, const Slot &b)
{
    // note: will give real glyphs priority over virtual ones at a given
    // priority
    // 6.Jul.2014 -- Make sure you include an old_code comparison; this
    // is required to ensure that a ligature's inputs (which have smaller
    // old_codes) are included whenever its outputs are included.
    if (a.lookup_source != b.lookup_source)
        return a.lookup_source < b.lookup_source;
    if (a.score != b.score)
        return a.score < b.score;
    if (a.glyph != b.glyph)
        return a.glyph < b.glyph;
    return a.old_code < b.old_code;
}
}

void
Metrics::shrink_encoding(int size, const DvipsEncoding &dvipsenc, ErrorHandler *errh)
{
    /* Move characters around. */

    /* Maybe we don't need to do anything. */
    if (_encoding.size() <= size) {
        cut_encoding(size);
        return;
    }

    /* Need a list of all ligatures.. */
    Vector<Ligature3> all_ligs;
    all_ligatures(all_ligs);

    /* Need liveness markings. */
    if (!_liveness_marked)
        mark_liveness(size, &all_ligs);

    /* Score characters by importance. Importance relates first to Unicode
       values, and then recursively to the importances of characters that form
       a ligature. */

    /* Create an initial set of scores, based on Unicode values. */
    Vector<int> scores(_encoding.size(), NOCHAR_SCORE);
    for (int i = 0; i < _encoding.size(); i++)
        if (_encoding[i].unicode)
            scores[i] = unicode_score(_encoding[i].unicode);

    /* Prefer conventional f-ligatures. */
    bool has_ff = false;
    for (Ligature3* l = all_ligs.begin(); l != all_ligs.end(); ++l)
        if (_encoding[l->in1].unicode == 'f'
            && (_encoding[l->in2].unicode == 'f'
                || _encoding[l->in2].unicode == 'i'
                || _encoding[l->in2].unicode == 'l')) {
            if (scores[l->out] > CONVENTIONAL_F_LIGATURE_SCORE)
                scores[l->out] = CONVENTIONAL_F_LIGATURE_SCORE;
            if (_encoding[l->in2].unicode == 'f') {
                _encoding[l->out].flags |= Char::IS_FF;
                has_ff = true;
            }
        }
    if (has_ff)
        for (Ligature3* l = all_ligs.begin(); l != all_ligs.end(); ++l)
            if (_encoding[l->in1].flag(Char::IS_FF)
                && (_encoding[l->in2].unicode == 'i'
                    || _encoding[l->in2].unicode == 'l')
                && scores[l->out] > CONVENTIONAL_F_F_LIGATURE_SCORE)
                scores[l->out] = CONVENTIONAL_F_F_LIGATURE_SCORE;

    /* Repeat these steps until you reach a stable set of scores: Score
       ligatures (ligscore = SUM[char scores]), then score characters touched
       only by fakes. */
    bool changed = true;
    while (changed) {
        changed = false;
        for (Ligature3 *l = all_ligs.begin(); l != all_ligs.end(); l++) {
            int score = scores[l->in1] + scores[l->in2];
            if (scores[l->out] > score)
                scores[l->out] = score, changed = true;
        }

        for (Code c = 0; c < _encoding.size(); c++)
            if (VirtualChar *vc = _encoding[c].virtual_char) {
                /* Make sure that if this virtual character appears, its parts
                   will also appear, by scoring the parts less */
                int score = scores[c] - 1, font_number = 0;
                for (Setting *s = vc->setting.begin(); s != vc->setting.end(); s++)
                    if (s->op == Setting::SHOW && font_number == 0
                        && score < scores[s->x])
                        scores[s->x] = score, changed = true;
                    else if (s->op == Setting::FONT)
                        font_number = s->x;
            }
    }

    /* Rescore intermediates to not be better off than their endpoints. */
    /* XXX multiple layers of intermediate? */
    for (Code c = 0; c < _encoding.size(); c++) {
        Char &ch = _encoding[c];
        if (ch.flag(Char::INTERMEDIATE))
            for (Ligature *l = ch.ligatures.begin(); l != ch.ligatures.end(); l++)
                if (scores[c] < scores[l->out] && !_encoding[l->out].context_setting(c, l->in2))
                    scores[c] = scores[l->out];
    }

    /* Collect characters that want to be reassigned. */
    Vector<Slot> slots;
    for (Code c = size; c < _encoding.size(); c++)
        if (scores[c] < NOCHAR_SCORE
            && !(_encoding[c].flags & Char::CONTEXT_ONLY)
            && (_encoding[c].flags & (Char::LIVE | Char::BASE_LIVE))) {
            Slot slot = { c, -1, _encoding[c].glyph, scores[c], _encoding[c].lookup_source };
            slots.push_back(slot);
        }
    // Sort them by score, then by glyph.
    std::sort(slots.begin(), slots.end());

    /* Prefer their old slots, if available. */
    for (Slot *slot = slots.begin(); slot < slots.end(); slot++)
        if (PermString g = code_name(slot->old_code)) {
            int c = dvipsenc.encoding_of(g);
            if (c >= 0 && _encoding[c].glyph == 0) {
                _encoding[c].swap(_encoding[slot->old_code]);
                slot->new_code = c;
            }
        }

    /* List empty slots in two phases: Those not encoded by the input
       encoding, then those encoded by the input encoding (but that character
       wasn't available). */
    Vector<Code> empty_codes;
    for (int want_encoded = 0; want_encoded < 2; want_encoded++)
        for (Code c = 0; c < size; c++)
            if (_encoding[c].base_code < 0
                && dvipsenc.encoded(c) == (bool) want_encoded)
                empty_codes.push_back(c);

    /* Then, assign codes to the unencoded characters. */
    int nunencoded = 0;

    for (Slot *slot = slots.begin(); slot != slots.end(); slot++) {
        if (slot->new_code >= 0)
            continue;

        int needs = (_encoding[slot->old_code].visible_base() ? 1 : 0)
            + (_encoding[slot->old_code].flag(Char::LIVE) ? 2 : 0);
        assert(needs > 0);

        Code dest = -1;
        for (Code *h = empty_codes.begin(); h < empty_codes.end() && dest < 0; h++) {
            int haves = (_encoding[*h].base_code < 0 ? 1 : 0)
                + (!_encoding[*h].visible() ? 2 : 0);
            if ((needs & haves) == needs)
                dest = *h;
        }

        if (dest >= 0) {
            if (needs & 2) {
                assert(!_encoding[dest].visible());
                _encoding[dest].swap(_encoding[slot->old_code]);
                slot->new_code = dest;
            } else {
                _encoding[slot->old_code].base_code = dest;
                slot->new_code = slot->old_code;
            }
            if (needs & 1) {
                assert(_encoding[dest].base_code < 0 || _encoding[dest].base_code == slot->old_code);
                _encoding[dest].base_code = slot->old_code;
            }
        } else
            nunencoded++;
    }

    /* Complain if some characters can't fit. */
    if (nunencoded) {
        // collect names of unencoded glyphs
        Vector<String> unencoded;
        for (Slot *slot = slots.begin(); slot != slots.end(); slot++)
            if (slot->new_code < 0)
                unencoded.push_back(code_name(slot->old_code));
        std::sort(unencoded.begin(), unencoded.end());
        StringAccum sa;
        for (const String* a = unencoded.begin(); a < unencoded.end(); a++)
            sa << *a << ' ';
        sa.pop_back();
        sa.append_break_lines(sa.take_string(), 68, "  ");
        sa.pop_back();
        errh->lwarning(" ", (unencoded.size() == 1 ? "not enough room in encoding, ignoring %d glyph" : "not enough room in encoding, ignoring %d glyphs"), unencoded.size());
        errh->lmessage(" ", "(\
The font uses more glyphs than the encoding has available slots,\n\
so these glyphs have been left out:\n%s\n\
To select specific glyphs, add them to the input encoding.)", sa.c_str());
    }

    /* Reencode changed slots. */
    Vector<Code> reencoding;
    for (Code c = 0; c < _encoding.size(); c++)
        reencoding.push_back(c);
    for (Slot *s = slots.begin(); s != slots.end(); s++)
        if (s->new_code >= 0)
            reencoding[s->old_code] = s->new_code;
    reencode(reencoding);

    check();
}

void
Metrics::make_base(int size)
{
    Vector<Code> reencoding;
    for (Code c = 0; c < size && c < _encoding.size(); c++) {
        Char &ch = _encoding[c];
        if (ch.base_code >= 0 && ch.base_code != c) {
            if (!reencoding.size())
                for (Code cc = 0; cc < _encoding.size(); cc++)
                    reencoding.push_back(cc);
            reencoding[ch.base_code] = c;
            reencoding[c] = ch.base_code;
            _encoding[c].swap(_encoding[ch.base_code]);
        }
        if (ch.virtual_char)    // remove it
            ch.clear();
    }
    if (reencoding.size()) {
        reencode(reencoding);
        cut_encoding(size);
    }
    check();
}


/*****************************************************************************/
/* output                                                                    */

bool
Metrics::need_virtual(int size) const
{
    if (size > _encoding.size())
        size = _encoding.size();
    for (const Char *ch = _encoding.begin(); ch < _encoding.begin() + size; ch++)
        if (ch->glyph /* actually encoded */
            && (ch->pdx || ch->pdy || ch->adx || ch->virtual_char))
            return true;
    return false;
}

bool
Metrics::need_base()
{
    if (!_liveness_marked)
        mark_liveness(_encoding.size());
    for (const Char *ch = _encoding.begin(); ch < _encoding.end(); ch++)
        if ((ch->flags & Char::BASE_LIVE) && ch->glyph != _boundary_glyph)
            return true;
    return false;
}

bool
Metrics::setting(Code code, Vector<Setting> &v, SettingMode sm) const
{
    extern int letterspace;

    if (!(sm & SET_KEEP))
        v.clear();

    if (!valid_code(code) || _encoding[code].glyph == 0)
        return false;

    const Char &ch = _encoding[code];

    if (const VirtualChar *vc = ch.virtual_char) {
        bool good = true;
        int font_number = 0;

        if (ch.pdx != 0 || ch.pdy != 0)
            v.push_back(Setting(Setting::MOVE, ch.pdx, ch.pdy));

        for (const Setting *s = vc->setting.begin(); s != vc->setting.end(); s++)
            switch (s->op) {
              case Setting::MOVE:
              case Setting::RULE:
              case Setting::PUSH:
              case Setting::POP:
              case Setting::SPECIAL:
                v.push_back(*s);
                break;
              case Setting::FONT:
                v.push_back(*s);
                font_number = s->x;
                break;
              case Setting::SHOW:
                if (font_number == 0)
                    good &= setting(s->x, v, (SettingMode)(sm | SET_KEEP));
                else
                    v.push_back(*s);
                break;
              case Setting::KERN:
              case Setting::KERNX:
                if (sm & SET_INTERMEDIATE)
                    v.push_back(*s);
                else if (font_number == 0 && s > vc->setting.begin()
                         && s + 1 < vc->setting.end()
                         && s[-1].op == Setting::SHOW
                         && s[1].op == Setting::SHOW) {
                    int k = kern(s[-1].x, s[1].x);
                    if (s->op == Setting::KERNX)
                        k -= letterspace;
                    if (k)
                        v.push_back(Setting(Setting::MOVE, k, 0));
                }
                break;
            }

        if (ch.pdy != 0 || ch.adx - ch.pdx != 0)
            v.push_back(Setting(Setting::MOVE, ch.adx - ch.pdx, -ch.pdy));
        return good;

    } else if (ch.base_code >= 0) {
        if (ch.pdx != 0 || ch.pdy != 0)
            v.push_back(Setting(Setting::MOVE, ch.pdx, ch.pdy));

        v.push_back(Setting(Setting::SHOW, ch.base_code, ch.glyph));

        if (ch.pdy != 0 || ch.adx - ch.pdx != 0)
            v.push_back(Setting(Setting::MOVE, ch.adx - ch.pdx, -ch.pdy));
        return true;

    } else
        return false;
}

int
Metrics::ligatures(Code in1, Vector<Code> &in2, Vector<Code> &out, Vector<int> &context) const
{
    in2.clear();
    out.clear();
    context.clear();

    const Char &in1ch = _encoding[in1];
    for (const Ligature *l = in1ch.ligatures.begin(); l != in1ch.ligatures.end(); l++) {
        in2.push_back(l->in2);
        const Char &outch = _encoding[l->out];
        if (outch.context_setting(in1, l->in2)) {
            if (in1 == outch.built_in1 && l->in2 == outch.built_in2)
                in2.pop_back();
            else if (in1 == outch.built_in1) {
                out.push_back(outch.built_in2);
                context.push_back(-1);
            } else {
                out.push_back(outch.built_in1);
                context.push_back(1);
            }
        } else {
            out.push_back(l->out);
            context.push_back(0);
        }
    }

    return in2.size();
}

int
Metrics::kerns(Code in1, Vector<Code> &in2, Vector<int> &kern) const
{
    in2.clear();
    kern.clear();

    const Char &in1ch = _encoding[in1];
    for (const Kern *k = in1ch.kerns.begin(); k != in1ch.kerns.end(); k++)
        if (k->kern != 0) {
            in2.push_back(k->in2);
            kern.push_back(k->kern);
        }

    return in2.size();
}


/*****************************************************************************/
/* debugging                                                                 */

void
Metrics::unparse(const Char *ch) const
{
    Code c;
    if (ch >= _encoding.begin() && ch < _encoding.end())
        c = ch - _encoding.begin();
    else
        c = -1;
    fprintf(stderr, "%4d/%s%s%s%s%s%s\n", c, code_str(c),
            (ch->flag(Char::LIVE) ? " [L]" : ""),
            (ch->flag(Char::BASE_LIVE) ? " [B]" : ""),
            (ch->flag(Char::CONTEXT_ONLY) ? " [C]" : ""),
            (ch->flag(Char::BUILT) ? " [!]" : ""),
            (ch->base_code >= 0 ? " <BC>" : ""));
    if (ch->base_code >= 0 && ch->base_code != c)
        fprintf(stderr, "\tBASE %d/%s\n", ch->base_code, code_str(ch->base_code));
    if (const VirtualChar *vc = ch->virtual_char) {
        fprintf(stderr, "\t*");
        int curfont = 0;
        for (const Setting *s = vc->setting.begin(); s != vc->setting.end(); s++)
            switch (s->op) {
              case Setting::FONT:
                fprintf(stderr, " {F%d}", s->x);
                curfont = s->x;
                break;
              case Setting::SHOW:
                fprintf(stderr, " %d", s->x);
                if (curfont == 0)
                    fprintf(stderr, "/%s", code_str(s->x));
                break;
              case Setting::KERN:
                fprintf(stderr, " <>");
                break;
              case Setting::MOVE:
                fprintf(stderr, " <%+d,%+d>", s->x, s->y);
                break;
              case Setting::RULE:
                fprintf(stderr, " [%d,%d]", s->x, s->y);
                break;
              case Setting::PUSH:
                fprintf(stderr, " (");
                break;
              case Setting::POP:
                fprintf(stderr, " )");
                break;
              case Setting::SPECIAL:
                fprintf(stderr, " S{%s}", s->s.c_str());
                break;
            }
        fprintf(stderr, "  ((%d/%s, %d/%s))\n", ch->built_in1, code_str(ch->built_in1), ch->built_in2, code_str(ch->built_in2));
    }
    for (const Ligature *l = ch->ligatures.begin(); l != ch->ligatures.end(); l++)
        fprintf(stderr, "\t[%d/%s => %d/%s]%s\n", l->in2, code_str(l->in2), l->out, code_str(l->out), (_encoding[l->out].context_setting(c, l->in2) ? " [C]" : ""));
#if 0
    for (const Kern *k = ch->kerns.begin(); k != ch->kerns.end(); k++)
        fprintf(stderr, "\t{%d/%s %+d}\n", k->in2, code_str(k->in2), k->kern);
#endif
}

void
Metrics::unparse() const
{
    for (const Char *ch = _encoding.begin(); ch < _encoding.end(); ch++)
        if (ch->glyph)
            unparse(ch);
}
