// -*- related-file-name: "../include/efont/ttfkern.hh" -*-

/* ttfkern.{cc,hh} -- TrueType kern table
 *
 * Copyright (c) 2009-2019 Eddie Kohler
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
#include <efont/ttfkern.hh>
#include <lcdf/error.hh>
#include <lcdf/straccum.hh>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

namespace Efont { namespace OpenType {

inline Data
KernTable::subtable(uint32_t &off_in_out) const
{
    uint32_t off = off_in_out, len;
    if (_version == 0) {
        if (_d.u16(off) != 0)
            throw new Format("kern subtable");
        len = _d.u16(off + 2);
    } else
        len = _d.u32(off);
    if (len < 6 || off + len > (uint32_t) _d.length())
        throw new Bounds();
    off_in_out = off + len;
    return _d.substring(off, len);
}

KernTable::KernTable(const Data &d, ErrorHandler *)
    : _d(d), _error(-1)
{
    // USHORT   Version
    // USHORT   nTables
    if (d.length() == 0)
        throw BlankTable("kern");

    uint32_t ntables, off;
    if (d.u16(0) == 0) {
        ntables = d.u16(2);
        _version = 0;
        off = 4;
    } else if (d.u16(0) == 1) {
        ntables = d.u32(4);
        _version = 1;
        off = 8;
    } else
        throw Format("kern");

    for (uint32_t i = 0; i < ntables; ++i)
        (void) subtable(off);

    _error = 0;
}

bool
KernTable::unparse_automatics(Vector<Positioning> &v, ErrorHandler *errh) const
{
    uint32_t ntables = this->ntables();
    uint32_t off = first_offset();
    int success = 0;
    if (_error < 0)
        return false;

    for (uint16_t i = 0; i < ntables; ++i) {
        Data subt = _d.subtable(off);
        uint16_t coverage = subt.u16(4);

        if (_version == 0) {
            if ((coverage & COV_V0_HORIZONTAL) == 0
                || (coverage & (COV_V0_MINIMUM | COV_V0_CROSS_STREAM
                                | COV_V0_OVERRIDE)) != 0
                || (coverage & COV_V0_FORMAT) != COV_V0_FORMAT0)
                continue;
        } else {
            if ((coverage & (COV_V1_VERTICAL | COV_V1_CROSS_STREAM
                             | COV_V1_VARIATION)) != 0
                || (coverage & COV_V1_FORMAT) != COV_V1_FORMAT0)
                continue;
        }

        try {
            uint32_t off = (_version ? 16 : 14);
            uint16_t npairs = subt.u16(off - 8);
            for (uint16_t j = 0; j < npairs; ++j, off += 6) {
                v.push_back(Positioning(Position(subt.u16(off), 0, 0, subt.s16(off + 4), 0),
                                        Position(subt.u16(off + 2), 0, 0, 0, 0)));
                ++success;
            }
        } catch (Error e) {
            if (errh)
                errh->warning("%s, continuing", e.description.c_str());
        }
    }

    return success > 0;
}

}}

#include <lcdf/vector.cc>
