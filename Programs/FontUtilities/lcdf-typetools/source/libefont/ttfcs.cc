// -*- related-file-name: "../include/efont/ttfcs.hh" -*-

/* ttfcs.{cc,hh} -- TrueType "charstring" emulation
 *
 * Copyright (c) 2006-2019 Eddie Kohler
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
#include <efont/ttfcs.hh>
#include <efont/ttfhead.hh>
#include <efont/t1csgen.hh>
#include <efont/otfpost.hh>
#include <efont/otfcmap.hh>
#include <lcdf/hashmap.hh>
#include <algorithm>
namespace Efont {
typedef OpenType::Glyph Glyph;

TrueTypeBoundsCharstringProgram::TrueTypeBoundsCharstringProgram(const OpenType::Font* otf)
    : CharstringProgram(otf->units_per_em()),
      _otf(otf), _nglyphs(-1), _loca_long(false),
      _loca(otf->table("loca")), _glyf(otf->table("glyf")),
      _hmtx(otf->table("hmtx")), _got_glyph_names(false), _got_unicodes(false)
{
    OpenType::Data maxp(otf->table("maxp"));
    if (maxp.length() >= 6)
        _nglyphs = maxp.u16(4);

    OpenType::Head head(otf->table("head"), 0);
    if (head.ok())
        _loca_long = head.index_to_loc_format() != 0;
    if (_loca_long)
        _loca.align_long();
    int loca_onesize = (_loca_long ? 4 : 2);
    if (_nglyphs >= _loca.length() / loca_onesize)
        _nglyphs = (_loca.length() / loca_onesize) - 1;

    // horizontal metrics
    OpenType::Data hhea(_otf->table("hhea"));
    // HHEA format:
    // 0        Fixed   Table version number    0x00010000 for version 1.0.
    // 4        FWORD   Ascender
    // 6        FWORD   Descender
    // 8        FWORD   LineGap
    // 10       UFWORD  advanceWidthMax
    // 12       FWORD   minLeftSideBearing
    // 14       FWORD   minRightSideBearing
    // 16       FWORD   xMaxExtent
    // 18       SHORT   caretSlopeRise
    // 20       SHORT   caretSlopeRun
    // 22       SHORT   caretOffset
    // 24       SHORT   (reserved)
    // 26       SHORT   (reserved)
    // 28       SHORT   (reserved)
    // 30       SHORT   (reserved)
    // 32       SHORT   metricDataFormat
    // 34       USHORT  numberOfHMetrics
    if (hhea.length() >= 36
        && hhea.u32(0) == 0x10000)
        _nhmtx = hhea.u16(34);
    if (_nhmtx * 4 > _hmtx.length())
        _nhmtx = _hmtx.length() / 4;
}

TrueTypeBoundsCharstringProgram::~TrueTypeBoundsCharstringProgram()
{
    for (Charstring **cs = _charstrings.begin(); cs < _charstrings.end(); cs++)
        delete *cs;
}

void TrueTypeBoundsCharstringProgram::font_matrix(double matrix[6]) const {
    matrix[0] = matrix[3] = 1.0 / _otf->units_per_em();
    matrix[1] = matrix[2] = matrix[4] = matrix[5] = 0;
}

int
TrueTypeBoundsCharstringProgram::nglyphs() const
{
    return _nglyphs;
}

PermString
TrueTypeBoundsCharstringProgram::glyph_name(int gi) const
{
    // generate glyph names based on what pdftex can understand
    if (gi == 0)
        return PermString(".notdef");

    // try 'post' table glyph names
    if (!_got_glyph_names) {
        OpenType::Post post(_otf->table("post"));
        if (post.ok())
            post.glyph_names(_glyph_names);
        HashMap<PermString, int> name2glyph(-1);
        // some 'post' tables are bogus, reject multiply-encoded names
        for (int gi = 0; gi < _glyph_names.size(); ++gi) {
            int& xgi = name2glyph.find_force(_glyph_names[gi]);
            if (xgi == -1)
                xgi = gi;
            else if (xgi == 0)
                _glyph_names[gi] = PermString();
            else
                _glyph_names[gi] = _glyph_names[xgi] = PermString();
        }
        _got_glyph_names = true;
    }
    if (gi >= 0 && gi < _glyph_names.size() && _glyph_names[gi])
        return _glyph_names[gi];

    // try 'uniXXXX' names
    if (!_got_unicodes) {
        OpenType::Cmap cmap(_otf->table("cmap"));
        if (cmap.ok()) {
            Vector<std::pair<uint32_t, Glyph> > ugp;
            cmap.unmap_all(ugp);
            std::sort(ugp.begin(), ugp.end());
            for (Vector<std::pair<uint32_t, Glyph> >::iterator it = ugp.begin();
                 it != ugp.end(); ) {
                Vector<std::pair<uint32_t, Glyph> >::iterator nit = it + 1;
                // ignore code points with multiple glyph mappings
                if (nit == ugp.end() || nit->first != it->first) {
                    if (it->second >= _unicodes.size())
                        _unicodes.resize(it->second + 1, 0);
                    if (!_unicodes[it->second])
                        _unicodes[it->second] = it->first;
                } else
                    while (nit != ugp.end() && nit->first == it->first)
                        ++nit;
                it = nit;
            }
        }
        _got_unicodes = true;
    }

    if (gi >= 0 && gi < _unicodes.size() && _unicodes[gi] > 0 && _unicodes[gi] <= 0xFFFF) {
        char buf[10];
        sprintf(buf, "uni%04X", _unicodes[gi]);
        return PermString(buf);
    } else
        return permprintf("index%d", gi);
}

void
TrueTypeBoundsCharstringProgram::glyph_names(Vector<PermString> &gn) const
{
    gn.clear();
    for (int gi = 0; gi < _nglyphs; gi++)
        gn.push_back(glyph_name(gi));
}

Charstring *
TrueTypeBoundsCharstringProgram::glyph(int gi) const
{
    if (gi < 0 || gi >= _nglyphs)
        return 0;
    if (_charstrings.size() <= gi)
        _charstrings.resize(gi + 1, (Charstring *) 0);
    if (!_charstrings[gi]) {
        // calculate glyf offsets
        uint32_t offset, end_offset;
        if (_loca_long) {
            offset = _loca.u32(gi * 4);
            end_offset = _loca.u32(gi * 4 + 4);
        } else {
            offset = _loca.u16(gi * 2) * 2;
            end_offset = _loca.u16(gi * 2 + 2) * 2;
        }

        // fetch bounding box from glyf
        int ncontours, xmin, ymin, xmax, ymax;
        if (offset != end_offset) {
            if (offset > end_offset || offset + 10 > end_offset
                || end_offset > (uint32_t) _glyf.length())
                return 0;

            ncontours = _glyf.s16(offset);
            xmin = _glyf.s16(offset + 2);
            ymin = _glyf.s16(offset + 4);
            xmax = _glyf.s16(offset + 6);
            ymax = _glyf.s16(offset + 8);
        } else
            ncontours = xmin = ymin = xmax = ymax = 0;

        // fetch horizontal metrics
        int advance_width, lsb;
        if (gi >= _nhmtx) {
            advance_width = (_nhmtx ? _hmtx.u16((_nhmtx - 1) * 4) : 0);
            int hmtx_offset = _nhmtx * 4 + (gi - _nhmtx) * 2;
            lsb = (hmtx_offset + 2 <= _hmtx.length() ? _hmtx.s16(hmtx_offset) : 0);
        } else {
            advance_width = _hmtx.u16(gi * 4);
            lsb = _hmtx.s16(gi * 4 + 2);
        }

        // make charstring
        Type1CharstringGen gen;
        if (ncontours == 0) {
            gen.gen_number(0, 'X');
            gen.gen_number(advance_width);
            gen.gen_command(Charstring::cHsbw);
        } else {
            gen.gen_number(lsb, 'X');
            gen.gen_number(advance_width);
            gen.gen_command(Charstring::cHsbw);
            gen.gen_moveto(Point(xmin, ymin), false, false);
            if (xmax != xmin || ymax == ymin)
                gen.gen_number(xmax - xmin, 'x');
            if (ymax != ymin)
                gen.gen_number(ymax - ymin, 'y');
            gen.gen_command(ymax == ymin ? Charstring::cHlineto
                            : (xmax == xmin ? Charstring::cVlineto
                               : Charstring::cRlineto));
            gen.gen_command(Charstring::cClosepath);
        }
        gen.gen_command(Charstring::cEndchar);

        _charstrings[gi] = gen.output();
    }
    return _charstrings[gi];
}

}
