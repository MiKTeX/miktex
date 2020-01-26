// -*- related-file-name: "../include/efont/otfgsub.hh" -*-

/* otfgsub.{cc,hh} -- OpenType GSUB table
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
#include <efont/otfgsub.hh>
#include <efont/otfname.hh>
#include <lcdf/error.hh>
#include <lcdf/straccum.hh>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

namespace Efont { namespace OpenType {

void
Substitution::clear(Substitute &s, uint8_t &t)
{
    switch (t) {
      case T_GLYPHS:
        delete[] s.gids;
        break;
      case T_COVERAGE:
        delete s.coverage;
        break;
    }
    t = T_NONE;
}

void
Substitution::assign_space(Substitute &s, uint8_t &t, int n)
{
    clear(s, t);
    if (n == 1)
        t = T_GLYPH;
    else if (n > 1) {
        s.gids = new Glyph[n + 1];
        s.gids[0] = n;
        t = T_GLYPHS;
    }
}

void
Substitution::assign(Substitute &s, uint8_t &t, Glyph gid)
{
    clear(s, t);
    s.gid = gid;
    t = T_GLYPH;
}

void
Substitution::assign(Substitute &s, uint8_t &t, int ngids, const Glyph *gids)
{
    clear(s, t);
    assert(ngids > 0);
    if (ngids == 1) {
        s.gid = gids[0];
        t = T_GLYPH;
    } else {
        s.gids = new Glyph[ngids + 1];
        s.gids[0] = ngids;
        memcpy(s.gids + 1, gids, ngids * sizeof(Glyph));
        t = T_GLYPHS;
    }
}

void
Substitution::assign(Substitute &s, uint8_t &t, const Coverage &coverage)
{
    clear(s, t);
    s.coverage = new Coverage(coverage);
    t = T_COVERAGE;
}

void
Substitution::assign(Substitute &s, uint8_t &t, const Substitute &os, uint8_t ot)
{
    if (&s == &os)
        return;
    switch (ot) {
      case T_NONE:
        clear(s, t);
        break;
      case T_GLYPH:
        assign(s, t, os.gid);
        break;
      case T_GLYPHS:
        assign(s, t, os.gids[0], os.gids + 1);
        break;
      case T_COVERAGE:
        assign(s, t, *os.coverage);
        break;
      default:
        assert(0);
    }
}

Substitution::Substitution(const Substitution &o)
    : _left_is(T_NONE), _in_is(T_NONE), _out_is(T_NONE), _right_is(T_NONE),
      _alternate(o._alternate)
{
    assign(_left, _left_is, o._left, o._left_is);
    assign(_in, _in_is, o._in, o._in_is);
    assign(_out, _out_is, o._out, o._out_is);
    assign(_right, _right_is, o._right, o._right_is);
}

Substitution::Substitution(Glyph in, Glyph out)
    : _left_is(T_NONE), _in_is(T_GLYPH), _out_is(T_GLYPH), _right_is(T_NONE)
{
    _in.gid = in;
    _out.gid = out;
}

Substitution::Substitution(Glyph in, const Vector<Glyph> &out, bool is_alternate)
    : _left_is(T_NONE), _in_is(T_GLYPH), _out_is(T_NONE), _right_is(T_NONE),
      _alternate(is_alternate)
{
    assert(out.size() > 0);
    _in.gid = in;
    assign(_out, _out_is, out.size(), &out[0]);
}

Substitution::Substitution(Glyph in1, Glyph in2, Glyph out)
    : _left_is(T_NONE), _in_is(T_GLYPHS), _out_is(T_GLYPH), _right_is(T_NONE)
{
    _in.gids = new Glyph[3];
    _in.gids[0] = 2;
    _in.gids[1] = in1;
    _in.gids[2] = in2;
    _out.gid = out;
}

Substitution::Substitution(const Vector<Glyph> &in, Glyph out)
    : _left_is(T_NONE), _in_is(T_NONE), _out_is(T_GLYPH), _right_is(T_NONE)
{
    assert(in.size() > 0);
    assign(_in, _in_is, in.size(), &in[0]);
    _out.gid = out;
}

Substitution::Substitution(int nin, const Glyph *in, Glyph out)
    : _left_is(T_NONE), _in_is(T_NONE), _out_is(T_GLYPH), _right_is(T_NONE)
{
    assert(nin > 0);
    assign(_in, _in_is, nin, in);
    _out.gid = out;
}

Substitution::Substitution(int nleft, int nin, int nout, int nright)
    : _left_is(T_NONE), _in_is(T_NONE), _out_is(T_NONE), _right_is(T_NONE)
{
    if (nleft)
        assign_space(_left, _left_is, nleft);
    if (nin)
        assign_space(_in, _in_is, nin);
    if (nout)
        assign_space(_out, _out_is, nout);
    if (nright)
        assign_space(_right, _right_is, nright);
}

Substitution::~Substitution()
{
    clear(_left, _left_is);
    clear(_in, _in_is);
    clear(_out, _out_is);
    clear(_right, _right_is);
}

Substitution &
Substitution::operator=(const Substitution &o)
{
    assign(_left, _left_is, o._left, o._left_is);
    assign(_in, _in_is, o._in, o._in_is);
    assign(_out, _out_is, o._out, o._out_is);
    assign(_right, _right_is, o._right, o._right_is);
    _alternate = o._alternate;
    return *this;
}

bool
Substitution::substitute_in(const Substitute &s, uint8_t t, const Coverage &c)
{
    switch (t) {
      case T_NONE:
        return true;
      case T_GLYPH:
        return c.covers(s.gid);
      case T_GLYPHS:
        for (int i = 1; i <= s.gids[0]; i++)
            if (!c.covers(s.gids[i]))
                return false;
        return true;
      case T_COVERAGE:
        return *s.coverage <= c;
      default:
        assert(0);
        return false;
    }
}

bool
Substitution::substitute_in(const Substitute &s, uint8_t t, const GlyphSet &gs)
{
    switch (t) {
      case T_NONE:
        return true;
      case T_GLYPH:
        return gs.covers(s.gid);
      case T_GLYPHS:
        for (int i = 1; i <= s.gids[0]; i++)
            if (!gs.covers(s.gids[i]))
                return false;
        return true;
      case T_COVERAGE:
        for (Coverage::iterator i = s.coverage->begin(); i; i++)
            if (!gs.covers(*i))
                return false;
        return true;
      default:
        assert(0);
        return false;
    }
}

bool
Substitution::context_in(const Coverage &c) const
{
    return substitute_in(_left, _left_is, c)
        && substitute_in(_in, _in_is, c)
        && substitute_in(_right, _right_is, c);
}

bool
Substitution::context_in(const GlyphSet &gs) const
{
    return substitute_in(_left, _left_is, gs)
        && substitute_in(_in, _in_is, gs)
        && substitute_in(_right, _right_is, gs);
}

Glyph
Substitution::extract_glyph(const Substitute &s, uint8_t t) noexcept
{
    return (t == T_GLYPH ? s.gid : 0);
}

Glyph
Substitution::extract_glyph(const Substitute &s, int which, uint8_t t) noexcept
{
    switch (t) {
      case T_GLYPH:
        return (which == 0 ? s.gid : 0);
      case T_GLYPHS:
        return (s.gids[0] > which ? s.gids[which + 1] : 0);
      case T_COVERAGE:
        for (Coverage::iterator ci = s.coverage->begin(); ci; ci++, which--)
            if (which == 0)
                return *ci;
        return 0;
      default:
        return 0;
    }
}

bool
Substitution::extract_glyphs(const Substitute &s, uint8_t t, Vector<Glyph> &v, bool coverage_ok) noexcept
{
    switch (t) {
      case T_GLYPH:
        v.push_back(s.gid);
        return true;
      case T_GLYPHS:
        for (int i = 1; i <= s.gids[0]; i++)
            v.push_back(s.gids[i]);
        return true;
      case T_COVERAGE:
        if (coverage_ok) {
            for (Coverage::iterator i = s.coverage->begin(); i; i++)
                v.push_back(*i);
            return true;
        } else
            return false;
      default:
        return false;
    }
}

Glyph *
Substitution::extract_glyphptr(const Substitute &s, uint8_t t) noexcept
{
    switch (t) {
      case T_GLYPH:
        return const_cast<Glyph *>(&s.gid);
      case T_GLYPHS:
        return &s.gids[1];
      default:
        return 0;
    }
}

int
Substitution::extract_nglyphs(const Substitute &s, uint8_t t, bool coverage_ok) noexcept
{
    switch (t) {
      case T_GLYPH:
        return 1;
      case T_GLYPHS:
        return s.gids[0];
      case T_COVERAGE:
        return (coverage_ok ? 1 : 0);
      default:
        return 0;
    }
}

bool
Substitution::matches(const Substitute &s, uint8_t t, int pos, Glyph g) noexcept
{
    switch (t) {
      case T_GLYPH:
        return (pos == 0 && s.gid == g);
      case T_GLYPHS:
        return (pos >= 0 && pos < s.gids[0] && s.gids[1 + pos] == g);
      case T_COVERAGE:
        return (pos == 0 && s.coverage->covers(g));
      default:
        return false;
    }
}

bool
Substitution::is_noop() const
{
    return (_in_is == T_GLYPH && _out_is == T_GLYPH && _in.gid == _out.gid)
        || (_in_is == T_GLYPHS && _out_is == T_GLYPHS
            && _in.gids[0] == _out.gids[0]
            && memcmp(_in.gids, _out.gids, (_in.gids[0] + 1) * sizeof(Glyph)) == 0);
}

bool
Substitution::all_in_glyphs(Vector<Glyph> &gs) const
{
    bool ok = true;
    gs.clear();
    if (_left_is != T_NONE)
        ok &= extract_glyphs(_left, _left_is, gs, false);
    ok &= extract_glyphs(_in, _in_is, gs, false);
    if (_right_is != T_NONE)
        ok &= extract_glyphs(_right, _right_is, gs, false);
    return ok;
}

bool
Substitution::all_out_glyphs(Vector<Glyph> &v) const
{
    bool ok = true;
    if (_left_is != T_NONE)
        ok &= extract_glyphs(_left, _left_is, v, false);
    ok &= extract_glyphs(_out, _out_is, v, false);
    if (_right_is != T_NONE)
        ok &= extract_glyphs(_right, _right_is, v, false);
    return ok;
}

void
Substitution::assign_append(Substitute &s, uint8_t &t, const Substitute &ls, uint8_t lt, const Substitute &rs, uint8_t rt)
{
    if (lt == T_NONE)
        assign(s, t, rs, rt);
    else if (rt == T_NONE)
        assign(s, t, ls, lt);
    else if (lt != T_COVERAGE && rt != T_COVERAGE) {
        int nl = extract_nglyphs(ls, lt, false);
        int nr = extract_nglyphs(rs, rt, false);
        Glyph *gids = new Glyph[nl + nr + 1];
        gids[0] = nl + nr;
        memcpy(&gids[1], extract_glyphptr(ls, lt), nl * sizeof(Glyph));
        memcpy(&gids[1 + nl], extract_glyphptr(rs, rt), nr * sizeof(Glyph));
        clear(s, t);
        s.gids = gids;
        t = T_GLYPHS;
    } else
        throw Error();
}

void
Substitution::assign_append(Substitute &s, uint8_t &t, const Substitute &ls, uint8_t lt, Glyph rg)
{
    Substitute rs;
    rs.gid = rg;
    assign_append(s, t, ls, lt, rs, T_GLYPH);
}

Substitution
Substitution::in_out_append_glyph(Glyph g) const
{
    Substitution s;
    assign(s._left, s._left_is, _left, _left_is);
    assign(s._right, s._right_is, _right, _right_is);
    assign_append(s._in, s._in_is, _in, _in_is, g);
    assign_append(s._out, s._out_is, _out, _out_is, g);
    return s;
}

void
Substitution::add_outer_left(Glyph g)
{
    Substitute ls;
    ls.gid = g;
    assign_append(_left, _left_is, ls, T_GLYPH, _left, _left_is);
}

void
Substitution::remove_outer_left()
{
    if (_left_is == T_GLYPH)
        _left_is = T_NONE;
    else if (_left_is == T_GLYPHS) {
        if (_left.gids[0] == 2)
            assign(_left, _left_is, _left.gids[2]);
        else {
            _left.gids[0]--;
            memmove(_left.gids + 1, _left.gids + 2, _left.gids[0] * sizeof(Glyph));
        }
    }
}

void
Substitution::add_outer_right(Glyph g)
{
    assign_append(_right, _right_is, _right, _right_is, g);
}

void
Substitution::remove_outer_right()
{
    if (_right_is == T_GLYPH)
        _right_is = T_NONE;
    else if (_right_is == T_GLYPHS) {
        if (_right.gids[0] == 2)
            assign(_right, _right_is, _right.gids[1]);
        else
            _right.gids[0]--;
    }
}

bool
Substitution::out_alter(const Substitution &o, int pos) noexcept
{
    const Glyph *g = out_glyphptr();
    int ng = out_nglyphs();
    const Glyph *out_g = o.out_glyphptr();
    int out_ng = o.out_nglyphs();
    int in_ng = o.in_nglyphs();
    if (pos + in_ng > ng || out_ng == 0)
        return false;

    // check that input substitution actually matches us
    for (int i = 0; i < in_ng; i++)
        if (!o.in_matches(i, g[pos+i]))
            return false;

    // actually change output
    Vector<Glyph> new_g;
    for (int i = 0; i < pos; i++)
        new_g.push_back(g[i]);
    for (int i = 0; i < out_ng; i++)
        new_g.push_back(out_g[i]);
    for (int i = pos + in_ng; i < ng; i++)
        new_g.push_back(g[i]);
    assign(_out, _out_is, new_g.size(), &new_g[0]);

    return true;
}

static void
unparse_glyphid(StringAccum &sa, Glyph gid, const Vector<PermString> *gns) noexcept
{
    if (gid > 0 && gns && gns->size() > gid && (*gns)[gid])
        sa << (*gns)[gid];
    else
        sa << "g" << gid;
}

void
Substitution::unparse_glyphids(StringAccum &sa, const Substitute &s, uint8_t t, const Vector<PermString> *gns) noexcept
{
    if (t == T_GLYPH)
        unparse_glyphid(sa, s.gid, gns);
    else if (t == T_GLYPHS) {
        for (int i = 1; i <= s.gids[0]; i++) {
            if (i != 1)
                sa << ' ';
            unparse_glyphid(sa, s.gids[i], gns);
        }
    } else if (t == T_COVERAGE)
        sa << "<coverage>";
    else
        sa << "-";
}

void
Substitution::unparse(StringAccum &sa, const Vector<PermString> *gns) const
{
    if (!*this)
        sa << "NULL[]";
    else {
        if (is_single())
            sa << "SINGLE[";
        else if (is_ligature())
            sa << "LIGATURE[";
        else if (is_multiple())
            sa << "MULTIPLE[";
        else if (is_alternate())
            sa << "ALTERNATE[";
        else if (is_simple_context())
            sa << "SIMPLECONTEXT[";
        else
            sa << "UNKNOWN[";

        if (_left_is != T_NONE) {
            unparse_glyphids(sa, _left, _left_is, gns);
            sa << " | ";
        }
        unparse_glyphids(sa, _in, _in_is, gns);
        sa << " => ";
        unparse_glyphids(sa, _out, _out_is, gns);
        if (_right_is != T_NONE) {
            sa << " | ";
            unparse_glyphids(sa, _right, _right_is, gns);
        }

        sa << ']';
    }
}

String
Substitution::unparse(const Vector<PermString> *gns) const
{
    StringAccum sa;
    unparse(sa, gns);
    return sa.take_string();
}



/**************************
 * Gsub                   *
 *                        *
 **************************/

Gsub::Gsub(const Data &d, const Font *otf, ErrorHandler *errh)
    : _chaincontext_reverse_backtrack(false)
{
    // Fixed    Version
    // Offset   ScriptList
    // Offset   FeatureList
    // Offset   LookupList
    if (d.length() == 0)
        throw BlankTable("GSUB");
    if (d.u16(0) != 1)
        throw Format("GSUB");
    if (_script_list.assign(d.offset_subtable(4), errh) < 0)
        throw Format("GSUB script list");
    if (_feature_list.assign(d.offset_subtable(6), errh) < 0)
        throw Format("GSUB feature list");
    _lookup_list = d.offset_subtable(8);

    if (!otf)
        return;

    // Check for "correct" chaining context rules, as suggested by Adobe's
    // OpenType FDK
    try {
        Name nametable(otf->table("name"), ErrorHandler::silent_handler());
        _chaincontext_reverse_backtrack = nametable.version_chaincontext_reverse_backtrack();
    } catch (Error) {
    }
}

int
Gsub::nlookups() const
{
    return _lookup_list.u16(0);
}

GsubLookup
Gsub::lookup(unsigned i) const
{
    if (i >= _lookup_list.u16(0))
        throw Error("GSUB lookup out of range");
    else
        return GsubLookup(_lookup_list.offset_subtable(2 + i*2));
}


/**************************
 * GsubLookup             *
 *                        *
 **************************/

GsubLookup::GsubLookup(const Data &d)
    : _d(d)
{
    if (_d.length() < 6)
        throw Format("GSUB Lookup table");
    _type = _d.u16(0);
    if (_type == L_EXTENSION && _d.u16(4) != 0) {
        Data first_subtable = _d.offset_subtable(HEADERSIZE);
        if (first_subtable.length() < 8 || first_subtable.u16(0) != 1)
            throw Format("GSUB Extension Lookup table");
        _type = first_subtable.u16(2);
    }
}

Data
GsubLookup::subtable(int i) const
{
    Data subd = _d.offset_subtable(HEADERSIZE + i*RECSIZE);
    if (_d.u16(0) != L_EXTENSION)
        return subd;
    else if (subd.length() >= 8 && subd.u16(0) == 1 && subd.u16(2) == _type)
        return subd.subtable(subd.u32(4));
    else
        return Data();
}

void
GsubLookup::mark_out_glyphs(const Gsub &gsub, Vector<bool> &gmap) const
{
    int nlookup = _d.u16(4);
    switch (_type) {
      case L_SINGLE:
        for (int i = 0; i < nlookup; i++) {
            GsubSingle x(subtable(i)); // this pattern makes gcc-3.3.4 happier
            x.mark_out_glyphs(gmap);
        }
        return;
      case L_MULTIPLE:
        for (int i = 0; i < nlookup; i++) {
            GsubMultiple x(subtable(i));
            x.mark_out_glyphs(gmap);
        }
        return;
      case L_ALTERNATE:
        for (int i = 0; i < nlookup; i++) {
            GsubMultiple x(subtable(i));
            x.mark_out_glyphs(gmap);
        }
        return;
      case L_LIGATURE:
        for (int i = 0; i < nlookup; i++) {
            GsubLigature x(subtable(i));
            x.mark_out_glyphs(gmap);
        }
        return;
    case L_CONTEXT:
        for (int i = 0; i < nlookup; i++) {
            GsubContext x(subtable(i));
            x.mark_out_glyphs(gsub, gmap);
        }
        return;
    case L_CHAIN:
        for (int i = 0; i < nlookup; i++) {
            GsubChainContext x(subtable(i));
            x.mark_out_glyphs(gsub, gmap);
        }
        return;
    }
}

bool
GsubLookup::unparse_automatics(const Gsub &gsub, Vector<Substitution> &v, const Coverage &limit) const
{
    int nlookup = _d.u16(4);
    switch (_type) {
      case L_SINGLE:
        for (int i = 0; i < nlookup; i++) {
            GsubSingle x(subtable(i)); // this pattern makes gcc-3.3.4 happier
            x.unparse(v, limit);
        }
        return true;
      case L_MULTIPLE:
        for (int i = 0; i < nlookup; i++) {
            GsubMultiple x(subtable(i));
            x.unparse(v);
        }
        return true;
      case L_ALTERNATE:
        for (int i = 0; i < nlookup; i++) {
            GsubMultiple x(subtable(i));
            x.unparse(v, true);
        }
        return true;
      case L_LIGATURE:
        for (int i = 0; i < nlookup; i++) {
            GsubLigature x(subtable(i));
            x.unparse(v);
        }
        return true;
      case L_CONTEXT: {
          bool understood = true;
          for (int i = 0; i < nlookup; i++) {
              GsubContext x(subtable(i));
              understood &= x.unparse(gsub, v, limit);
          }
          return understood;
      }
      case L_CHAIN: {
          bool understood = true;
          for (int i = 0; i < nlookup; i++) {
              GsubChainContext x(subtable(i));
              understood &= x.unparse(gsub, v, limit);
          }
          return understood;
      }
      default:
        return false;
    }
}

bool
GsubLookup::apply(const Glyph *g, int pos, int n, Substitution &s) const
{
    int nlookup = _d.u16(4);
    switch (_type) {
      case L_SINGLE:
        for (int i = 0; i < nlookup; i++) {
            GsubSingle x(subtable(i));
            if (x.apply(g, pos, n, s))
                return true;
        }
        return false;
      case L_MULTIPLE:
        for (int i = 0; i < nlookup; i++) {
            GsubMultiple x(subtable(i));
            if (x.apply(g, pos, n, s))
                return true;
        }
        return false;
      case L_ALTERNATE:
        for (int i = 0; i < nlookup; i++) {
            GsubMultiple x(subtable(i));
            if (x.apply(g, pos, n, s, true))
                return true;
        }
        return false;
      case L_LIGATURE:
        for (int i = 0; i < nlookup; i++) {
            GsubLigature x(subtable(i));
            if (x.apply(g, pos, n, s))
                return true;
        }
        return false;
      default:                  // XXX
        return false;
    }
}


/**************************
 * GsubSingle             *
 *                        *
 **************************/

GsubSingle::GsubSingle(const Data &d)
    : _d(d)
{
    if (_d[0] != 0
        || (_d[1] != 1 && _d[1] != 2))
        throw Format("GSUB Single Substitution");
    Coverage coverage(_d.offset_subtable(2));
    if (!coverage.ok()
        || (_d[1] == 2 && coverage.size() > _d.u16(4)))
        throw Format("GSUB Single Substitution coverage");
}

Coverage
GsubSingle::coverage() const noexcept
{
    return Coverage(_d.offset_subtable(2), 0, false);
}

Glyph
GsubSingle::map(Glyph g) const
{
    int ci = coverage().coverage_index(g);
    if (ci < 0)
        return g;
    else if (_d[1] == 1)
        return g + _d.s16(4);
    else
        return _d.u16(HEADERSIZE + FORMAT2_RECSIZE*ci);
}

void
GsubSingle::mark_out_glyphs(Vector<bool> &gmap) const
{
    if (_d[1] == 1) {
        int delta = _d.s16(4);
        for (Coverage::iterator i = coverage().begin(); i; i++)
            gmap[*i + delta] = true;
    } else {
        for (Coverage::iterator i = coverage().begin(); i; i++)
            gmap[_d.u16(HEADERSIZE + i.coverage_index()*FORMAT2_RECSIZE)] = true;
    }
}

void
GsubSingle::unparse(Vector<Substitution> &v, const Coverage &limit) const
{
    if (_d[1] == 1) {
        int delta = _d.s16(4);
        for (Coverage::iterator it = coverage().begin(); it; ++it)
            if (limit.covers(*it))
                v.push_back(Substitution(*it, *it + delta));
    } else {
        for (Coverage::iterator it = coverage().begin(); it; ++it)
            if (limit.covers(*it))
                v.push_back(Substitution(*it, _d.u16(HEADERSIZE + it.coverage_index()*FORMAT2_RECSIZE)));
    }
}

bool
GsubSingle::apply(const Glyph *g, int pos, int n, Substitution &s) const
{
    int ci;
    if (pos < n && (ci = coverage().coverage_index(g[pos])) >= 0) {
        if (_d[1] == 1)
            s = Substitution(g[pos], g[pos] + _d.s16(4));
        else
            s = Substitution(g[pos], _d.u16(HEADERSIZE + ci*FORMAT2_RECSIZE));
        return true;
    } else
        return false;
}


/**************************
 * GsubMultiple           *
 *                        *
 **************************/

GsubMultiple::GsubMultiple(const Data &d)
    : _d(d)
{
    if (_d[0] != 0 || _d[1] != 1)
        throw Format("GSUB Multiple Substitution");
    Coverage coverage(_d.offset_subtable(2));
    if (!coverage.ok()
        || coverage.size() > _d.u16(4))
        throw Format("GSUB Multiple Substitution coverage");
}

Coverage
GsubMultiple::coverage() const noexcept
{
    return Coverage(_d.offset_subtable(2), 0, false);
}

bool
GsubMultiple::map(Glyph g, Vector<Glyph> &v) const
{
    v.clear();
    int ci = coverage().coverage_index(g);
    if (ci < 0) {
        v.push_back(g);
        return false;
    } else {
        Data seq = _d.offset_subtable(HEADERSIZE + ci*RECSIZE);
        for (int i = 0; i < seq.u16(0); i++)
            v.push_back(seq.u16(SEQ_HEADERSIZE + i*SEQ_RECSIZE));
        return true;
    }
}

void
GsubMultiple::mark_out_glyphs(Vector<bool> &gmap) const
{
    for (Coverage::iterator i = coverage().begin(); i; ++i) {
        Data seq = _d.offset_subtable(HEADERSIZE + i.coverage_index()*RECSIZE);
        for (int j = 0; j < seq.u16(0); ++j)
            gmap[seq.u16(SEQ_HEADERSIZE + j*SEQ_RECSIZE)] = true;
    }
}

void
GsubMultiple::unparse(Vector<Substitution> &v, bool is_alternate) const
{
    Vector<Glyph> result;
    for (Coverage::iterator i = coverage().begin(); i; i++) {
        Data seq = _d.offset_subtable(HEADERSIZE + i.coverage_index()*RECSIZE);
        result.clear();
        for (int j = 0; j < seq.u16(0); j++)
            result.push_back(seq.u16(SEQ_HEADERSIZE + j*SEQ_RECSIZE));
        v.push_back(Substitution(*i, result, is_alternate));
    }
}

bool
GsubMultiple::apply(const Glyph *g, int pos, int n, Substitution &s, bool is_alternate) const
{
    int ci;
    if (pos < n && (ci = coverage().coverage_index(g[pos])) >= 0) {
        Vector<Glyph> result;
        Data seq = _d.offset_subtable(HEADERSIZE + ci*RECSIZE);
        for (int j = 0; j < seq.u16(0); j++)
            result.push_back(seq.u16(SEQ_HEADERSIZE + j*SEQ_RECSIZE));
        s = Substitution(g[pos], result, is_alternate);
        return true;
    } else
        return false;
}


/**************************
 * GsubLigature           *
 *                        *
 **************************/

GsubLigature::GsubLigature(const Data &d)
    : _d(d)
{
    if (_d[0] != 0
        || _d[1] != 1)
        throw Format("GSUB Ligature Substitution");
    Coverage coverage(_d.offset_subtable(2));
    if (!coverage.ok()
        || coverage.size() > _d.u16(4))
        throw Format("GSUB Ligature Substitution coverage");
}

Coverage
GsubLigature::coverage() const noexcept
{
    return Coverage(_d.offset_subtable(2), 0, false);
}

bool
GsubLigature::map(const Vector<Glyph> &gs, Glyph &result, int &consumed) const
{
    assert(gs.size() > 0);
    result = gs[0];
    consumed = 1;
    int ci = coverage().coverage_index(gs[0]);
    if (ci < 0)
        return false;
    Data ligset = _d.offset_subtable(HEADERSIZE + ci*RECSIZE);
    int nligset = ligset.u16(0);
    for (int i = 0; i < nligset; i++) {
        Data lig = ligset.offset_subtable(SET_HEADERSIZE + i*SET_RECSIZE);
        int nlig = lig.u16(2);
        if (nlig > gs.size() - 1)
            goto bad;
        for (int j = 0; j < nlig - 1; j++)
            if (lig.u16(LIG_HEADERSIZE + j*LIG_RECSIZE) != gs[j + 1])
                goto bad;
        result = lig.u16(0);
        consumed = nlig + 1;
        return true;
      bad: ;
    }
    return false;
}

void
GsubLigature::mark_out_glyphs(Vector<bool> &gmap) const
{
    for (Coverage::iterator i = coverage().begin(); i; i++) {
        Data ligset = _d.offset_subtable(HEADERSIZE + i.coverage_index()*RECSIZE);
        int nligset = ligset.u16(0);
        Vector<Glyph> components(1, *i);
        for (int j = 0; j < nligset; j++) {
            Data lig = ligset.offset_subtable(SET_HEADERSIZE + j*SET_RECSIZE);
            gmap[lig.u16(0)] = true;
        }
    }
}

void
GsubLigature::unparse(Vector<Substitution> &v) const
{
    for (Coverage::iterator i = coverage().begin(); i; i++) {
        Data ligset = _d.offset_subtable(HEADERSIZE + i.coverage_index()*RECSIZE);
        int nligset = ligset.u16(0);
        Vector<Glyph> components(1, *i);
        for (int j = 0; j < nligset; j++) {
            Data lig = ligset.offset_subtable(SET_HEADERSIZE + j*SET_RECSIZE);
            int nlig = lig.u16(2);
            components.resize(1);
            for (int k = 0; k < nlig - 1; k++)
                components.push_back(lig.u16(LIG_HEADERSIZE + k*LIG_RECSIZE));
            v.push_back(Substitution(components, lig.u16(0)));
        }
    }
}

bool
GsubLigature::apply(const Glyph *g, int pos, int n, Substitution &s) const
{
    int ci;
    if (pos < n && (ci = coverage().coverage_index(g[pos])) >= 0) {
        Data ligset = _d.offset_subtable(HEADERSIZE + ci*RECSIZE);
        int nligset = ligset.u16(0);
        for (int j = 0; j < nligset; j++) {
            Data lig = ligset.offset_subtable(SET_HEADERSIZE + j*SET_RECSIZE);
            int nlig = lig.u16(2);
            if (pos + nlig <= n) {
                for (int k = 0; k < nlig - 1; k++)
                    if (lig.u16(LIG_HEADERSIZE + k*LIG_RECSIZE) != g[pos + k + 1])
                        goto ligature_failed;
                s = Substitution(nlig, &g[pos], lig.u16(0));
                return true;
            }
          ligature_failed: ;
        }
    }
    return false;
}


/**************************
 * GsubContext            *
 *                        *
 **************************/

GsubContext::GsubContext(const Data &d)
    : _d(d)
{
    switch (_d.u16(0)) {
      case 1:
      case 2:
        break;
      case 3: {
          int ninput = _d.u16(2);
          if (ninput < 1)
              throw Format("GSUB Context Substitution input sequence");
          Coverage coverage(_d.offset_subtable(F3_HSIZE));
          if (!coverage.ok())
              throw Format("GSUB Context Substitution coverage");
          break;
      }
      default:
        throw Format("GSUB Context Substitution");
    }
}

Coverage
GsubContext::coverage() const noexcept
{
    if (_d[1] == 3)
        return Coverage(_d.offset_subtable(F3_HSIZE), 0, false);
    else
        return Coverage();
}

void
GsubContext::subruleset_mark_out_glyphs(const Data &data, int nsub,
                                        int subtab_offset, const Gsub &gsub,
                                        Vector<bool> &gmap)
{
    for (int j = 0; j < nsub; ++j) {
        int lookup_index = data.u16(subtab_offset + SUBRECSIZE*j + 2);
        gsub.lookup(lookup_index).mark_out_glyphs(gsub, gmap);
    }
}

void
GsubContext::mark_out_glyphs(const Gsub &gsub, Vector<bool> &gmap) const
{
    if (_d.u16(0) != 3)         // XXX
        return;
    int nglyph = _d.u16(2);
    int nsubst = _d.u16(4);
    subruleset_mark_out_glyphs(_d, nsubst, F3_HSIZE + nglyph*2, gsub, gmap);
}

bool
GsubContext::f1_unparse(const Data& data,
                        int nsub, int subtab_offset,
                        const Gsub& gsub, Vector<Substitution>& outsubs,
                        Substitution s) {
    Substitution subtab_sub;
    int napplied = 0;
    for (int j = 0; j < nsub; j++) {
        int seq_index = data.u16(subtab_offset + SUBRECSIZE*j);
        int lookup_index = data.u16(subtab_offset + SUBRECSIZE*j + 2);
        // XXX check seq_index against size of output glyphs?
        if (gsub.lookup(lookup_index).apply(s.out_glyphptr(), seq_index, s.out_nglyphs(), subtab_sub)) {
            napplied++;
            s.out_alter(subtab_sub, seq_index);
        }
    }
    outsubs.push_back(s);
    return true;
}

bool
GsubContext::f3_unparse(const Data &data,
                        int nglyph, int glyphtab_offset, const Coverage &limit,
                        int nsub, int subtab_offset,
                        const Gsub &gsub, Vector<Substitution> &outsubs,
                        const Substitution &prototype_sub)
{
    Vector<Substitution> subs;
    subs.push_back(prototype_sub);
    Vector<Substitution> work_subs;

    // get array of possible substitutions including contexts
    for (int i = 0; i < nglyph; i++) {
        assert(!work_subs.size());
        Coverage c(data.offset_subtable(glyphtab_offset + i*2));
        for (Coverage::iterator ci = (c & limit).begin(); ci; ci++)
            for (int j = 0; j < subs.size(); j++)
                work_subs.push_back(subs[j].in_out_append_glyph(*ci));
        subs.clear();
        subs.swap(work_subs);
    }

    // now, apply referred lookups to the resulting substitution array
    Substitution subtab_sub;
    for (int i = 0; i < subs.size(); i++) {
        Substitution &s = subs[i];
        int napplied = 0;
        for (int j = 0; j < nsub; j++) {
            int seq_index = data.u16(subtab_offset + SUBRECSIZE*j);
            int lookup_index = data.u16(subtab_offset + SUBRECSIZE*j + 2);
            // XXX check seq_index against size of output glyphs?
            if (gsub.lookup(lookup_index).apply(s.out_glyphptr(), seq_index, s.out_nglyphs(), subtab_sub)) {
                napplied++;
                s.out_alter(subtab_sub, seq_index);
            }
        }
        // 26.Jun.2003 -- always push substitution back, since the no-op might
        // override a following substitution
        outsubs.push_back(s);
    }

    return true;                // XXX
}

bool
GsubContext::unparse(const Gsub &gsub, Vector<Substitution> &v, const Coverage &limit) const
{
    if (_d.u16(0) != 3)         // XXX
        return false;
    int nglyph = _d.u16(2);
    int nsubst = _d.u16(4);
    return f3_unparse(_d, nglyph, F3_HSIZE, limit, nsubst, F3_HSIZE + nglyph*2, gsub, v, Substitution());
}


/**************************
 * GsubChainContext       *
 *                        *
 **************************/

GsubChainContext::GsubChainContext(const Data &d)
    : _d(d)
{
    switch (_d.u16(0)) {
    case 1: {
        Coverage coverage(_d.offset_subtable(2));
        if (!coverage.ok()
            || coverage.size() != _d.u16(4))
            throw Format("ChainContext Substitution coverage");
        break;
    }
      case 2:
        break;
      case 3: {
          int nbacktrack = _d.u16(2);
          int input_offset = F3_HSIZE + nbacktrack*2;
          int ninput = _d.u16(input_offset);
          if (ninput < 1)
              throw Format("GSUB ChainContext Substitution input sequence");
          Coverage coverage(_d.offset_subtable(input_offset + F3_INPUT_HSIZE));
          if (!coverage.ok())
              throw Format("GSUB ChainContext Substitution coverage");
          break;
      }
      default:
        throw Format("GSUB ChainContext Substitution");
    }
}

Coverage
GsubChainContext::coverage() const noexcept
{
    switch (_d.u16(0)) {
    case 1:
        return Coverage(_d.offset_subtable(2), 0, false);
    case 3: {
        int nbacktrack = _d.u16(2);
        int input_offset = F3_HSIZE + nbacktrack*2;
        return Coverage(_d.offset_subtable(input_offset + F3_INPUT_HSIZE), 0, false);
    }
    default:
        return Coverage();
    }
}

void
GsubChainContext::mark_out_glyphs(const Gsub &gsub, Vector<bool> &gmap) const
{
    switch (_d.u16(0)) {
    case 1: {
        int nsubruleset = _d.u16(4);
        for (int i = 0; i != nsubruleset; ++i) {
            int srs_offset = _d.u16(6 + i*2);
            int nsubrule = _d.u16(srs_offset);
            for (int j = 0; j != nsubrule; ++j) {
                int subrule_offset = srs_offset + _d.u16(srs_offset + 2 + j*2);
                int nbacktrack = _d.u16(subrule_offset);
                int input_offset = subrule_offset + 2 + nbacktrack*2;
                int ninput = _d.u16(input_offset);
                int lookahead_offset = input_offset + 2 + (ninput-1)*2;
                int nlookahead = _d.u16(lookahead_offset);
                int subst_offset = lookahead_offset + 2 + nlookahead*2;
                int nsubst = _d.u16(subst_offset);

                GsubContext::subruleset_mark_out_glyphs(_d, nsubst, subst_offset + 2, gsub, gmap);
            }
        }
        break;
    }
    case 3: {
        int nbacktrack = _d.u16(2);
        int input_offset = F3_HSIZE + nbacktrack*2;
        int ninput = _d.u16(input_offset);
        int lookahead_offset = input_offset + F3_INPUT_HSIZE + ninput*2;
        int nlookahead = _d.u16(lookahead_offset);
        int subst_offset = lookahead_offset + F3_LOOKAHEAD_HSIZE + nlookahead*2;
        int nsubst = _d.u16(subst_offset);

        GsubContext::subruleset_mark_out_glyphs(_d, nsubst, subst_offset + F3_SUBST_HSIZE, gsub, gmap);
        break;
    }
    default:
        return;
    }
}

bool
GsubChainContext::f1_unparse(const Gsub &gsub, Vector<Substitution> &v, const Coverage &limit) const
{
    Coverage input0_coverage(_d.offset_subtable(2), 0, false);
    Coverage::iterator i0iter = input0_coverage.begin();

    for (int i0index = 0; i0index != input0_coverage.size();
         ++i0index, ++i0iter) {
        int srs_offset = _d.u16(6 + i0index*2);
        int nsubrule = _d.u16(srs_offset);
        for (int srindex = 0; srindex != nsubrule; ++srindex) {
            int sr_offset = srs_offset + _d.u16(srs_offset + 2 + srindex*2);
            int nbacktrack = _d.u16(sr_offset);
            int input_offset = sr_offset + 2 + nbacktrack*2;
            int ninput = _d.u16(input_offset);
            int lookahead_offset = input_offset + 2 + (ninput-1)*2;
            int nlookahead = _d.u16(lookahead_offset);
            int subst_offset = lookahead_offset + 2 + nlookahead*2;
            int nsubst = _d.u16(subst_offset);
            int subtab_offset = subst_offset + 2;

            Substitution s(nbacktrack, ninput, ninput, nlookahead);
            if (gsub.chaincontext_reverse_backtrack()) {
                for (int i = 0; i != nbacktrack; ++i)
                    s.left_glyphptr()[i] = _d.u16(sr_offset + 2 + i*2);
            } else {
                for (int i = nbacktrack - 1; i != -1; --i)
                    s.left_glyphptr()[nbacktrack - 1 - i] = _d.u16(sr_offset + 2 + i*2);
            }
            Glyph* in_begin = s.in_glyphptr();
            Glyph* out_begin = s.out_glyphptr();
            in_begin[0] = out_begin[0] = *i0iter;
            for (int i = 1; i != ninput; ++i)
                in_begin[i] = out_begin[i] = _d.u16(input_offset + 2 + (i-1)*2);
            for (int i = 0; i != ninput; ++i)
                if (!limit.covers(in_begin[i]))
                    goto skip;
            for (int i = 0; i != nlookahead; ++i)
                s.right_glyphptr()[i] = _d.u16(lookahead_offset + 2 + i*2);

            // now, apply referred lookups to the resulting substitution array
            GsubContext::f1_unparse(_d, nsubst, subtab_offset, gsub, v, s);
        skip: ;
        }
    }

    return true;
}

bool
GsubChainContext::f3_unparse(const Gsub &gsub, Vector<Substitution> &v, const Coverage &limit) const
{
    int nbacktrack = _d.u16(2);
    int input_offset = F3_HSIZE + nbacktrack*2;
    int ninput = _d.u16(input_offset);
    int lookahead_offset = input_offset + F3_INPUT_HSIZE + ninput*2;
    int nlookahead = _d.u16(lookahead_offset);
    int subst_offset = lookahead_offset + F3_LOOKAHEAD_HSIZE + nlookahead*2;
    int nsubst = _d.u16(subst_offset);

    Vector<Coverage> backtrackc;
    Vector<Coverage> lookaheadc;
    if (gsub.chaincontext_reverse_backtrack()) {
        for (int i = 0; i < nbacktrack; i++)
            backtrackc.push_back(Coverage(_d.offset_subtable(F3_HSIZE + i*2)) & limit);
    } else {
        for (int i = nbacktrack - 1; i >= 0; i--)
            backtrackc.push_back(Coverage(_d.offset_subtable(F3_HSIZE + i*2)) & limit);
    }
    for (int i = 0; i < nlookahead; i++)
        lookaheadc.push_back(Coverage(_d.offset_subtable(lookahead_offset + F3_LOOKAHEAD_HSIZE + i*2)) & limit);

    // give up if would generate too many substitutions
    double n = 1;
    for (int i = 0; i < nbacktrack; ++i)
        n *= backtrackc[i].size();
    for (int i = 0; i < nlookahead; ++i)
        n *= lookaheadc[i].size();
    for (int i = 0; i < ninput; ++i)
        n *= (Coverage(_d.offset_subtable(input_offset + F3_INPUT_HSIZE + i*2)) & limit).size();
    if (n > 1000000)            // arbitrary cutoff
        return false;

    Vector<Coverage::iterator> backtracki;
    Vector<Coverage::iterator> lookaheadi;
    for (int i = 0; i < nbacktrack; i++)
        backtracki.push_back(backtrackc[i].begin());
    for (int i = 0; i < nlookahead; i++)
        lookaheadi.push_back(lookaheadc[i].begin());

    bool any = false;

    while (1) {

        // run GsubContext
        Substitution s(nbacktrack, 0, 0, nlookahead);
        Glyph *left_begin = s.left_glyphptr();
        for (int i = 0; i < nbacktrack; i++)
            left_begin[i] = *backtracki[i];
        Glyph *right_begin = s.right_glyphptr();
        for (int i = 0; i < nlookahead; i++)
            right_begin[i] = *lookaheadi[i];

        any |= GsubContext::f3_unparse(_d, ninput, input_offset + F3_INPUT_HSIZE, limit, nsubst, subst_offset + F3_SUBST_HSIZE, gsub, v, s);

        // step iterators
        for (int i = nlookahead - 1; i >= 0; i--) {
            lookaheadi[i]++;
            if (lookaheadi[i])
                goto next;
            lookaheadi[i] = lookaheadc[i].begin();
        }
        for (int i = nbacktrack - 1; i >= 0; i--) {
            backtracki[i]++;
            if (backtracki[i])
                goto next;
            backtracki[i] = backtrackc[i].begin();
        }
        break;

      next: ;
    }

    return any;
}

bool
GsubChainContext::unparse(const Gsub &gsub, Vector<Substitution> &v, const Coverage &limit) const
{
    if (_d.u16(0) == 1)
        return f1_unparse(gsub, v, limit);
    else if (_d.u16(0) == 3)
        return f3_unparse(gsub, v, limit);
    else
        return false;
}


}}
