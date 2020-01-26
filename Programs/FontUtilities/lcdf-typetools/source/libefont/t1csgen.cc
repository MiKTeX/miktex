// -*- related-file-name: "../include/efont/t1csgen.hh" -*-

/* t1csgen.{cc,hh} -- Type 1 charstring generation
 *
 * Copyright (c) 1998-2019 Eddie Kohler
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
#include <efont/t1csgen.hh>
#include <efont/t1font.hh>
#include <efont/t1item.hh>
#include <math.h>
namespace Efont {

static const char * const command_desc[] = {
    0, 0, 0, 0, "y",
    "xy", "x", "y", "xyxyxy", 0,

    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,

    0, "xy", "x", 0, 0,
    0, 0, 0, 0, 0,

    "yxyx", "xxyy", 0, 0, 0,
    0, 0, 0, 0, 0,

    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,

    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,

    0, 0, 0, 0, 0,
    "XY", 0, 0, 0, 0
};

Type1CharstringGen::Type1CharstringGen(int precision)
{
    if (precision >= 1 && precision <= 107)
        _precision = precision;
    else
        _precision = 5;
    _f_precision = _precision;
    clear();
}

void
Type1CharstringGen::clear()
{
    _ncs.clear();
    _true = _false = Point(0, 0);
    _state = S_INITIAL;
}


void
Type1CharstringGen::gen_rational(int big_val, int divisor)
{
    int frac = big_val % divisor;
    int val = (frac == 0 ? big_val / divisor : big_val);

    if (val >= -107 && val <= 107)
        _ncs.append((char)(val + 139));

    else if (val >= -1131 && val <= 1131) {
        int base = val < 0 ? 251 : 247;
        if (val < 0) val = -val;
        val -= 108;
        int w = val % 256;
        val = (val - w) / 256;
        _ncs.append((char)(val + base));
        _ncs.append((char)w);

    } else {
        _ncs.append('\377');
        long l = val;
        _ncs.append((char)((l >> 24) & 0xFF));
        _ncs.append((char)((l >> 16) & 0xFF));
        _ncs.append((char)((l >> 8) & 0xFF));
        _ncs.append((char)((l >> 0) & 0xFF));
    }

    if (frac != 0) {
        _ncs.append((char)(divisor + 139));
        _ncs.append((char)Charstring::cEscape);
        _ncs.append((char)(Charstring::cDiv - Charstring::cEscapeDelta));
    }
}

void
Type1CharstringGen::gen_number(double float_val, int kind)
{
    switch (kind) {
      case 'x':
        _true.x += float_val;
        float_val = _true.x - _false.x;
        break;
      case 'y':
        _true.y += float_val;
        float_val = _true.y - _false.y;
        break;
      case 'X':
        _true.x = float_val;
        break;
      case 'Y':
        _true.y = float_val;
        break;
    }

    // 30.Jul.2003 - Avoid rounding differences between platforms with the
    // extra 0.00001.
    int big_val = (int)floor(float_val * _f_precision + 0.50001);

    gen_rational(big_val, _precision);

    float_val = big_val / _f_precision;
    switch (kind) {
      case 'x':
        _false.x += float_val;
        break;
      case 'y':
        _false.y += float_val;
        break;
      case 'X':
        _false.x = float_val;
        break;
      case 'Y':
        _false.y = float_val;
        break;
    }
}


void
Type1CharstringGen::gen_command(int command)
{
    if (command >= Charstring::cEscapeDelta) {
        _ncs.append((char)Charstring::cEscape);
        _ncs.append((char)(command - Charstring::cEscapeDelta));
        if (command != Charstring::cSbw)
            _state = S_GEN;
    } else {
        _ncs.append((char)command);
        if (command > Charstring::cVmoveto && command != Charstring::cHsbw)
            _state = S_GEN;
    }
}

bool
Type1CharstringGen::gen_stem3_stack(CharstringInterp &interp)
{
    // special handling to ensure rounding doesn't generate an invalid stem3
    // hint
    if (interp.size() < 6)
        return false;

    // sort hints
    int i0, i1, i2;
    if (interp.at(0) > interp.at(2))
        i0 = 2, i1 = 0;
    else
        i0 = 0, i1 = 2;
    if (interp.at(4) < interp.at(i0))
        i2 = i1, i1 = i0, i0 = 4;
    else if (interp.at(4) < interp.at(i1))
        i2 = i1, i1 = 4;
    else
        i2 = 4;

    // check constraints. count "almost equal" as equal
    double stemw0 = interp.at(i0+1), stemw2 = interp.at(i2+1);
    if ((int)(1024*(stemw0 - stemw2) + .5) != 0)
        return false;

    double c0 = interp.at(i0) + interp.at(i0+1)/2;
    double c1 = interp.at(i1) + interp.at(i1+1)/2;
    double c2 = interp.at(i2) + interp.at(i2+1)/2;
    if ((int)(1024*((c1 - c0) - (c2 - c1)) + .5) != 0)
        return false;

    // if all constraints are satisfied now, make sure they are also satisfied
    // after rounding
    int big_v0 = (int)floor(interp.at(i0) * _f_precision + 0.50001);
    int big_v2 = (int)floor(interp.at(i2) * _f_precision + 0.50001);
    int big_stemw0 = (int)floor(stemw0 * _f_precision + 0.50001);
    int big_stemw1 = (int)floor(interp.at(i1+1) * _f_precision + 0.50001);

    int big_v1_times2 = big_v0 + big_v2 + big_stemw0 - big_stemw1;

    gen_rational(big_v0, _precision);
    gen_rational(big_stemw0, _precision);
    if (big_v1_times2 % 2)
        gen_rational(big_v1_times2, 2 * _precision);
    else
        gen_rational(big_v1_times2 / 2, _precision);
    gen_rational(big_stemw1, _precision);
    gen_rational(big_v2, _precision);
    gen_rational(big_stemw0, _precision);

    interp.clear();
    return true;
}

void
Type1CharstringGen::gen_stack(CharstringInterp &interp, int for_cmd)
{
    const char *str = ((unsigned)for_cmd <= Charstring::cLastCommand ? command_desc[for_cmd] : (const char *)0);
    if ((for_cmd == Charstring::cHstem3 || for_cmd == Charstring::cVstem3)
        && gen_stem3_stack(interp))
        return;

    int i;
    for (i = 0; str && *str && i < interp.size(); i++, str++)
        gen_number(interp.at(i), *str);
    for (; i < interp.size(); i++)
        gen_number(interp.at(i));
    interp.clear();
}

void
Type1CharstringGen::gen_moveto(const Point &p, bool closepath, bool always)
{
    // make sure we generate some moveto on the first command

    Point d = p - _true;
    int big_dx = (int)floor(d.x * _f_precision + 0.50001);
    int big_dy = (int)floor(d.y * _f_precision + 0.50001);

    if (big_dx == 0 && big_dy == 0 && _state != S_INITIAL && !always)
        /* do nothing */;
    else {
        if (closepath)
            gen_command(Charstring::cClosepath);
        if (big_dy == 0) {
            gen_number(d.x, 'x');
            gen_command(Charstring::cHmoveto);
        } else if (big_dx == 0) {
            gen_number(d.y, 'y');
            gen_command(Charstring::cVmoveto);
        } else {
            gen_number(d.x, 'x');
            gen_number(d.y, 'y');
            gen_command(Charstring::cRmoveto);
        }
    }

    _true = p;
}

void
Type1CharstringGen::append_charstring(const String &s)
{
    _ncs << s;
}

Type1Charstring *
Type1CharstringGen::output()
{
    return new Type1Charstring(take_string());
}

void
Type1CharstringGen::output(Type1Charstring &cs)
{
    cs.assign(take_string());
}

String
Type1CharstringGen::callsubr_string(int subr)
{
    Type1CharstringGen csg;
    csg.gen_number(subr);
    csg.gen_command(Charstring::cCallsubr);
    return csg._ncs.take_string();
}


/*****
 * Type1CharstringGenInterp
 **/

Type1CharstringGenInterp::Type1CharstringGenInterp(int precision)
    : _csgen(precision), _hint_csgen(precision),
      _direct_hr(false), _hr_storage(0),
      _max_flex_height(0),
      _had_flex(false), _had_bad_flex(false), _had_hr(false)
{
}

void
Type1CharstringGenInterp::set_hint_replacement_storage(Type1Font *font)
{
    _hr_storage = font;
    _hr_firstsubr = font->nsubrs();
}


// generating charstring commands

inline void
Type1CharstringGenInterp::gen_number(double n, int what)
{
    _csgen.gen_number(n, what);
}

inline void
Type1CharstringGenInterp::gen_command(int what)
{
    _csgen.gen_command(what);
}

void
Type1CharstringGenInterp::gen_sbw(bool hints_follow)
{
    if (!hints_follow && nhints())
        act_hintmask(Cs::cHintmask, 0, nhints());
    else if (left_sidebearing().y == 0 && _width.y == 0) {
        gen_number(left_sidebearing().x, 'X');
        gen_number(_width.x);
        gen_command(Cs::cHsbw);
    } else {
        gen_number(left_sidebearing().x, 'X');
        gen_number(left_sidebearing().y, 'Y');
        gen_number(_width.x);
        gen_number(_width.y);
        gen_command(Cs::cSbw);
    }
    _state = S_CLOSED;
}

void
Type1CharstringGenInterp::act_width(int, const Point &p)
{
    _width = p;
}

void
Type1CharstringGenInterp::act_seac(int, double asb, double adx, double ady, int bchar, int achar)
{
    if (_state == S_INITIAL)
        gen_sbw(false);
    gen_number(asb);
    gen_number(adx);
    gen_number(ady);
    gen_number(bchar);
    gen_number(achar);
    gen_command(Cs::cSeac);
    _state = S_SEAC;
}

void
Type1CharstringGenInterp::swap_stem_hints()
{
    _stem_pos.clear();
    _stem_width.clear();
    _stem_hstem.clear();
    _in_hr = true;
}

void
Type1CharstringGenInterp::act_hstem(int, double pos, double width)
{
    if (_state != S_INITIAL && !_in_hr)
        swap_stem_hints();
    _stem_pos.push_back(pos);
    _stem_width.push_back(width);
    _stem_hstem.push_back(1);
}

void
Type1CharstringGenInterp::act_vstem(int, double pos, double width)
{
    if (_state != S_INITIAL && !_in_hr)
        swap_stem_hints();
    _stem_pos.push_back(pos);
    _stem_width.push_back(width);
    _stem_hstem.push_back(0);
}

String
Type1CharstringGenInterp::gen_hints(const unsigned char *data, int nhints) const
{
    _hint_csgen.clear();
    unsigned char mask = 0x80;
    for (int i = 0; i < nhints; i++) {
        if (*data & mask) {
            double offset = (_stem_hstem[i] ? left_sidebearing().y : left_sidebearing().x);
            _hint_csgen.gen_number(_stem_pos[i] - offset);
            _hint_csgen.gen_number(_stem_width[i]);
            _hint_csgen.gen_command(_stem_hstem[i] ? Cs::cHstem : Cs::cVstem);
        }
        if ((mask >>= 1) == 0)
            data++, mask = 0x80;
    }
    return _hint_csgen.take_string();
}

void
Type1CharstringGenInterp::act_hintmask(int cmd, const unsigned char *data, int nhints)
{
    if (cmd == Cs::cCntrmask || nhints > Type1CharstringGenInterp::nhints())
        return;

    String data_holder;
    if (!data) {
        data_holder = String::make_fill('\377', ((nhints - 1) >> 3) + 1);
        data = data_holder.udata();
    }

    String hints = gen_hints(data, nhints);
    _in_hr = false;

    if (_state == S_INITIAL || _direct_hr) {
        _last_hints = hints;
        if (_state == S_INITIAL)
            gen_sbw(true);
        _csgen.append_charstring(hints);
    } else if (_hr_storage && hints != _last_hints) {
        _last_hints = hints;
        hints += (char)(Cs::cReturn);

        int subrno = -1, nsubrs = _hr_storage->nsubrs();
        for (int i = _hr_firstsubr; i < nsubrs; i++)
            if (Type1Subr *s = _hr_storage->subr_x(i))
                if (s->t1cs() == hints) {
                    subrno = i;
                    break;
                }

        if (subrno < 0 && _hr_storage->set_subr(nsubrs, Type1Charstring(hints)))
            subrno = nsubrs;

        if (subrno >= 0) {
            _had_hr = true;
            _csgen.gen_number(subrno);
            _csgen.gen_number(4);
            _csgen.gen_command(Cs::cCallsubr);
        }
    }
}

void
Type1CharstringGenInterp::act_line(int cmd, const Point &a, const Point &b)
{
    if (_state == S_INITIAL)
        gen_sbw(false);
    else if (_in_hr)
        act_hintmask(cmd, 0, nhints());
    _csgen.gen_moveto(a, _state == S_OPEN, false);
    _state = S_OPEN;
    if (a.x == b.x) {
        gen_number(b.y - a.y, 'y');
        gen_command(Cs::cVlineto);
    } else if (a.y == b.y) {
        gen_number(b.x - a.x, 'x');
        gen_command(Cs::cHlineto);
    } else {
        gen_number(b.x - a.x, 'x');
        gen_number(b.y - a.y, 'y');
        gen_command(Cs::cRlineto);
    }
}

void
Type1CharstringGenInterp::act_curve(int cmd, const Point &a, const Point &b, const Point &c, const Point &d)
{
    if (_state == S_INITIAL)
        gen_sbw(false);
    else if (_in_hr)
        act_hintmask(cmd, 0, nhints());
    _csgen.gen_moveto(a, _state == S_OPEN, false);
    _state = S_OPEN;
    if (b.y == a.y && d.x == c.x) {
        gen_number(b.x - a.x, 'x');
        gen_number(c.x - b.x, 'x');
        gen_number(c.y - b.y, 'y');
        gen_number(d.y - c.y, 'y');
        gen_command(Cs::cHvcurveto);
    } else if (b.x == a.x && d.y == c.y) {
        gen_number(b.y - a.y, 'y');
        gen_number(c.x - a.x, 'x');
        gen_number(c.y - b.y, 'y');
        gen_number(d.x - c.x, 'x');
        gen_command(Cs::cVhcurveto);
    } else {
        gen_number(b.x - a.x, 'x');
        gen_number(b.y - a.y, 'y');
        gen_number(c.x - b.x, 'x');
        gen_number(c.y - b.y, 'y');
        gen_number(d.x - c.x, 'x');
        gen_number(d.y - c.y, 'y');
        gen_command(Cs::cRrcurveto);
    }
}

void
Type1CharstringGenInterp::act_flex(int cmd, const Point &p0, const Point &p1, const Point &p2, const Point &p3_4, const Point &p5, const Point &p6, const Point &p7, double flex_depth)
{
    if (_state == S_INITIAL)
        gen_sbw(false);
    else if (_in_hr)
        act_hintmask(cmd, 0, nhints());
    _csgen.gen_moveto(p0, _state == S_OPEN, false);
    _state = S_OPEN;

    // 1. Outer endpoints must have same x (or y) coordinate
    bool v_ok = (p0.x == p7.x);
    bool h_ok = (p0.y == p7.y);

    // 2. Join point and its neighboring controls must be at an extreme
    if (v_ok && p2.x == p3_4.x && p3_4.x == p5.x) {
        double distance = fabs(p3_4.x - p0.x);
        int sign = (p3_4.x < p0.x ? -1 : 1);
        if (sign * (p1.x - p0.x) < 0 || sign * (p1.x - p0.x) > distance
            || sign * (p6.x - p0.x) < 0 || sign * (p6.x - p0.x) > distance)
            v_ok = false;
    } else
        v_ok = false;

    if (h_ok && p2.y == p3_4.y && p3_4.y == p5.y) {
        double distance = fabs(p3_4.y - p0.y);
        int sign = (p3_4.y < p0.y ? -1 : 1);
        if (sign * (p1.y - p0.y) < 0 || sign * (p1.y - p0.y) > distance
            || sign * (p6.y - p0.y) < 0 || sign * (p6.y - p0.y) > distance)
            h_ok = false;
    } else
        h_ok = false;

    // 3. Flex height <= 20
    if (v_ok && fabs(p3_4.x - p0.x) > 20)
        v_ok = false;
    if (h_ok && fabs(p3_4.y - p0.y) > 20)
        h_ok = false;

    // generate flex commands
    if (v_ok || h_ok) {
        _had_flex = true;
        Point p_reference = (h_ok ? Point(p3_4.x, p0.y) : Point(p0.x, p3_4.y));

        _csgen.gen_number(1);
        _csgen.gen_command(Cs::cCallsubr);

        _csgen.gen_moveto(p_reference, false, true);
        _csgen.gen_number(2);
        _csgen.gen_command(Cs::cCallsubr);

        _csgen.gen_moveto(p1, false, true);
        _csgen.gen_number(2);
        _csgen.gen_command(Cs::cCallsubr);

        _csgen.gen_moveto(p2, false, true);
        _csgen.gen_number(2);
        _csgen.gen_command(Cs::cCallsubr);

        _csgen.gen_moveto(p3_4, false, true);
        _csgen.gen_number(2);
        _csgen.gen_command(Cs::cCallsubr);

        _csgen.gen_moveto(p5, false, true);
        _csgen.gen_number(2);
        _csgen.gen_command(Cs::cCallsubr);

        _csgen.gen_moveto(p6, false, true);
        _csgen.gen_number(2);
        _csgen.gen_command(Cs::cCallsubr);

        _csgen.gen_moveto(p7, false, true);
        _csgen.gen_number(2);
        _csgen.gen_command(Cs::cCallsubr);

        _csgen.gen_number(flex_depth);
        _csgen.gen_number(p7.x, 'X');
        _csgen.gen_number(p7.y, 'Y');
        _csgen.gen_number(0);
        _csgen.gen_command(Cs::cCallsubr);

        double flex_height = fabs(h_ok ? p3_4.y - p0.y : p3_4.x - p0.x);
        if (flex_height > _max_flex_height)
            _max_flex_height = flex_height;
    } else {
        _had_bad_flex = true;
        act_curve(cmd, p0, p1, p2, p3_4);
        act_curve(cmd, p3_4, p5, p6, p7);
    }
}

void
Type1CharstringGenInterp::act_closepath(int cmd)
{
    if (_in_hr)
        act_hintmask(cmd, 0, nhints());
    gen_command(Cs::cClosepath);
    _state = S_CLOSED;
}

void
Type1CharstringGenInterp::intermediate_output(Type1Charstring &out)
{
    _csgen.output(out);
    _state = S_INITIAL;
    act_hintmask(Cs::cEndchar, 0, nhints());
}

void
Type1CharstringGenInterp::run(const CharstringContext &g, Type1Charstring &out)
{
    _width = Point(0, 0);
    _csgen.clear();
    swap_stem_hints();
    _state = S_INITIAL;
    _in_hr = false;

    CharstringInterp::interpret(g);

    if (_state == S_INITIAL)
        gen_sbw(false);
    else if (_in_hr)
        act_hintmask(Cs::cEndchar, 0, nhints());
    if (_state != S_SEAC)
        _csgen.gen_command(Cs::cEndchar);

    _csgen.output(out);
}

}
