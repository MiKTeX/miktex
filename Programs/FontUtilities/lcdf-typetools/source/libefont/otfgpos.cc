// -*- related-file-name: "../include/efont/otfgpos.hh" -*-

/* otfgpos.{cc,hh} -- OpenType GPOS table
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
#include <efont/otfgpos.hh>
#include <lcdf/error.hh>
#include <lcdf/straccum.hh>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

namespace Efont { namespace OpenType {


/**************************
 * Gpos                   *
 *                        *
 **************************/

Gpos::Gpos(const Data &d, ErrorHandler *errh)
{
    // Fixed    Version
    // Offset   ScriptList
    // Offset   FeatureList
    // Offset   LookupList
    if (d.length() == 0)
        throw BlankTable("GPOS");
    if (d.u16(0) != 1)
        throw Format("GPOS");
    if (_script_list.assign(d.offset_subtable(4), errh) < 0)
        throw Format("GPOS script list");
    if (_feature_list.assign(d.offset_subtable(6), errh) < 0)
        throw Format("GPOS feature list");
    _lookup_list = d.offset_subtable(8);
}

int
Gpos::nlookups() const
{
    return _lookup_list.u16(0);
}

GposLookup
Gpos::lookup(unsigned i) const
{
    if (i >= _lookup_list.u16(0))
        throw Error("GPOS lookup out of range");
    else
        return GposLookup(_lookup_list.offset_subtable(2 + i*2));
}


/**************************
 * GposValue              *
 *                        *
 **************************/

const int GposValue::nibble_bitcount_x2[] = { 0, 2, 2, 4, 2, 4, 4, 6,
                                              2, 4, 4, 6, 4, 6, 6, 8 };


/**************************
 * GposLookup             *
 *                        *
 **************************/

GposLookup::GposLookup(const Data &d)
    : _d(d)
{
    if (_d.length() < 6)
        throw Format("GPOS Lookup table");
    _type = _d.u16(0);
    if (_type == L_EXTENSION && _d.u16(4) != 0) {
        Data first_subtable = _d.offset_subtable(HEADERSIZE);
        if (first_subtable.length() < 8 || first_subtable.u16(0) != 1)
            throw Format("GPOS Extension Lookup table");
        _type = first_subtable.u16(2);
    }
}

Data
GposLookup::subtable(int i) const
{
    Data subd = _d.offset_subtable(HEADERSIZE + i*RECSIZE);
    if (_d.u16(0) != L_EXTENSION)
        return subd;
    else if (subd.length() >= 8 && subd.u16(0) == 1 && subd.u16(2) == _type)
        return subd.subtable(subd.u32(4));
    else
        return Data();
}

bool
GposLookup::unparse_automatics(Vector<Positioning> &v, ErrorHandler *errh) const
{
    int nlookup = _d.u16(4), success = 0;
    switch (_type) {
      case L_SINGLE:
        for (int i = 0; i < nlookup; i++)
            try {
                GposSingle s(subtable(i));
                s.unparse(v);
                success++;
            } catch (Error e) {
                if (errh)
                    errh->warning("%s, continuing", e.description.c_str());
            }
        return success > 0;
      case L_PAIR:
        for (int i = 0; i < nlookup; i++)
            try {
                GposPair p(subtable(i));
                p.unparse(v);
                success++;
            } catch (Error e) {
                if (errh)
                    errh->warning("%s, continuing", e.description.c_str());
            }
        return success > 0;
      default:
        return false;
    }
}


/**************************
 * GposSingle             *
 *                        *
 **************************/

GposSingle::GposSingle(const Data &d)
    : _d(d)
{
    if (_d[0] != 0
        || (_d[1] != 1 && _d[1] != 2))
        throw Format("GPOS Single Positioning");
    Coverage coverage(_d.offset_subtable(2));
    if (!coverage.ok()
        || (_d[1] == 2 && coverage.size() > _d.u16(6)))
        throw Format("GPOS Single Positioning coverage");
}

Coverage
GposSingle::coverage() const noexcept
{
    return Coverage(_d.offset_subtable(2), 0, false);
}

void
GposSingle::unparse(Vector<Positioning> &v) const
{
    if (_d[1] == 1) {
        int format = _d.u16(4);
        Data value = _d.subtable(6);
        for (Coverage::iterator i = coverage().begin(); i; i++)
            v.push_back(Positioning(Position(*i, format, value)));
    } else {
        int format = _d.u16(4);
        int size = GposValue::size(format);
        for (Coverage::iterator i = coverage().begin(); i; i++)
            v.push_back(Positioning(Position(*i, format, _d.subtable(F2_HEADERSIZE + size*i.coverage_index()))));
    }
}


/**************************
 * GposPair               *
 *                        *
 **************************/

GposPair::GposPair(const Data &d)
    : _d(d)
{
    if (_d[0] != 0
        || (_d[1] != 1 && _d[1] != 2))
        throw Format("GPOS Pair Positioning");
    Coverage coverage(_d.offset_subtable(2));
    if (!coverage.ok()
        || (_d[1] == 1 && coverage.size() > _d.u16(8)))
        throw Format("GPOS Pair Positioning coverage");
}

Coverage
GposPair::coverage() const noexcept
{
    return Coverage(_d.offset_subtable(2), 0, false);
}

void
GposPair::unparse(Vector<Positioning> &v) const
{
    if (_d[1] == 1) {
        int format1 = _d.u16(4);
        int format2 = _d.u16(6);
        int f2_pos = PAIRVALUE_HEADERSIZE + GposValue::size(format1);
        int pairvalue_size = f2_pos + GposValue::size(format2);
        for (Coverage::iterator i = coverage().begin(); i; i++) {
            Data pairset = _d.offset_subtable(F1_HEADERSIZE + i.coverage_index()*F1_RECSIZE);
            int npair = pairset.u16(0);
            for (int j = 0; j < npair; j++) {
                Data pair = pairset.subtable(PAIRSET_HEADERSIZE + j*pairvalue_size);
                v.push_back(Positioning(Position(*i, format1, pair.subtable(PAIRVALUE_HEADERSIZE)),
                                        Position(pair.u16(0), format2, pair.subtable(f2_pos))));
            }
        }
    } else {                    // _d[1] == 2
        int format1 = _d.u16(4);
        int format2 = _d.u16(6);
        int f2_pos = GposValue::size(format1);
        int recsize = f2_pos + GposValue::size(format2);
        ClassDef class1(_d.offset_subtable(8));
        ClassDef class2(_d.offset_subtable(10));
        Coverage coverage = this->coverage();
        int nclass1 = _d.u16(12);
        int nclass2 = _d.u16(14);
        int offset = F2_HEADERSIZE;
        for (int c1 = 0; c1 < nclass1; c1++)
            for (int c2 = 0; c2 < nclass2; c2++, offset += recsize) {
                Position p1(format1, _d.subtable(offset));
                Position p2(format2, _d.subtable(offset + f2_pos));
                if (p1 || p2) {
                    for (ClassDef::class_iterator c1i = class1.begin(c1, coverage); c1i; c1i++)
                        for (ClassDef::class_iterator c2i = class2.begin(c2); c2i; c2i++)
                            v.push_back(Positioning(Position(*c1i, p1), Position(*c2i, p2)));
                }
            }
    }
}


/**************************
 * Positioning            *
 *                        *
 **************************/

static void
unparse_glyphid(StringAccum &sa, Glyph gid, const Vector<PermString> *gns)
{
    if (!gns)
        gns = &debug_glyph_names;
    if (gid && gns && gns->size() > gid && (*gns)[gid])
        sa << (*gns)[gid];
    else
        sa << "g" << gid;
}

void
Position::unparse(StringAccum &sa, const Vector<PermString> *gns) const
{
    unparse_glyphid(sa, g, gns);
    if (placed())
        sa << '@' << pdx << ',' << pdy;
    sa << '+' << adx;
    if (ady)
        sa << '/' << ady;
}

String
Position::unparse(const Vector<PermString> *gns) const
{
    StringAccum sa;
    unparse(sa, gns);
    return sa.take_string();
}

bool
Positioning::context_in(const Coverage &c) const
{
    return (c.covers(_left.g) || !_left.g) && (!_right.g || c.covers(_right.g));
}

bool
Positioning::context_in(const GlyphSet &gs) const
{
    return (gs.covers(_left.g) || !_left.g) && (!_right.g || gs.covers(_right.g));
}

void
Positioning::unparse(StringAccum &sa, const Vector<PermString> *gns) const
{
    if (!*this)
        sa << "NULL[]";
    else if (is_single()) {
        sa << "SINGLE[";
        _left.unparse(sa, gns);
        sa << ']';
    } else if (is_pairkern()) {
        sa << "KERN[";
        unparse_glyphid(sa, _left.g, gns);
        sa << ' ';
        unparse_glyphid(sa, _right.g, gns);
        sa << "+" << _left.adx << ']';
    } else if (is_pair()) {
        sa << "PAIR[";
        _left.unparse(sa, gns);
        sa << ' ';
        _right.unparse(sa, gns);
        sa << ']';
    } else
        sa << "UNKNOWN[]";
}

String
Positioning::unparse(const Vector<PermString> *gns) const
{
    StringAccum sa;
    unparse(sa, gns);
    return sa.take_string();
}

}}

#include <lcdf/vector.cc>
