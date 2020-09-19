// -*- related-file-name: "../include/efont/t1bounds.hh" -*-

/* t1bounds.{cc,hh} -- charstring bounding box finder
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
#include <efont/t1bounds.hh>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

namespace Efont {

CharstringBounds::CharstringBounds()
    : _lb(UNKDOUBLE, UNKDOUBLE), _rt(UNKDOUBLE, UNKDOUBLE),
      _last_xf_program(0)
{
}

CharstringBounds::CharstringBounds(const Transform& nonfont_xf)
    : _lb(UNKDOUBLE, UNKDOUBLE), _rt(UNKDOUBLE, UNKDOUBLE),
      _nonfont_xf(nonfont_xf), _last_xf_program(0)
{
}

CharstringBounds::CharstringBounds(const Transform &nonfont_xf, const Vector<double> &weight)
    : CharstringInterp(weight),
      _lb(UNKDOUBLE, UNKDOUBLE), _rt(UNKDOUBLE, UNKDOUBLE),
      _nonfont_xf(nonfont_xf), _last_xf_program(0)
{
}

void
CharstringBounds::clear()
{
    _lb = _rt = Point(UNKDOUBLE, UNKDOUBLE);
    _width = Point(0, 0);
}

void
CharstringBounds::xf_mark(const Bezier &b)
{
    Bezier b1, b2;
    b.halve(b1, b2);
    xf_mark(b1.point(3));
    if (!xf_controls_inside(b1))
        xf_mark(b1);
    if (!xf_controls_inside(b2))
        xf_mark(b2);
}

void
CharstringBounds::act_width(int, const Point &w)
{
    _width = w * _xf;
}

void
CharstringBounds::act_line(int, const Point &p0, const Point &p1)
{
    mark(p0);
    mark(p1);
}

void
CharstringBounds::act_curve(int, const Point &p0, const Point &p1, const Point &p2, const Point &p3)
{
    Point q0 = p0 * _xf;
    Point q1 = p1 * _xf;
    Point q2 = p2 * _xf;
    Point q3 = p3 * _xf;

    xf_mark(q0);
    xf_mark(q3);

    if (!xf_inside(q1) || !xf_inside(q2)) {
        Bezier b(q0, q1, q2, q3);
        xf_mark(b);
    }
}

void
CharstringBounds::set_xf(const CharstringProgram *program)
{
    if (_last_xf_program != program) {
        _last_xf_program = program;
        double matrix[6];
        program->font_matrix(matrix);
        Transform font_xf = Transform(matrix).scaled(program->units_per_em());
        font_xf.check_null(0.001);
        _xf = _nonfont_xf * font_xf;
    }
}

bool
CharstringBounds::char_bounds(const CharstringContext &g, bool shift)
{
    set_xf(g.program);
    CharstringInterp::interpret(g);
    if (shift) {
        _xf.raw_translate(_width - _xf.translation());
        _nonfont_xf.raw_translate(_width - _nonfont_xf.translation());
        _width = Point(0, 0);
    }
    return error() >= 0;
}

void
CharstringBounds::translate(double dx, double dy)
{
    _xf.translate(dx, dy);
    _nonfont_xf.translate(dx, dy);
}

bool
CharstringBounds::output(double bb[4], double& width, bool use_cur_width) const
{
    if (!KNOWN(_lb.x))
        bb[0] = bb[1] = bb[2] = bb[3] = 0;
    else {
        bb[0] = _lb.x;
        bb[1] = _lb.y;
        bb[2] = _rt.x;
        bb[3] = _rt.y;
    }
    if (use_cur_width)
        width = _width.x;
    else {
        Point p = Point(0, 0) * _xf;
        width = p.x;
    }
    return error() >= 0;
}

bool
CharstringBounds::bounds(const Transform& transform, const CharstringContext& g,
                         double bb[4], double& width)
{
    CharstringBounds b(transform);
    b.char_bounds(g, false);
    return b.output(bb, width, true);
}

bool
CharstringBounds::bounds(const CharstringContext &g,
                         double bb[4], double& width)
{
    CharstringBounds b;
    b.char_bounds(g, false);
    return b.output(bb, width, true);
}

} // namespace Efont
