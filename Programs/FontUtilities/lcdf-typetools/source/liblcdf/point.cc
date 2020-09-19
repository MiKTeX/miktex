// -*- related-file-name: "../include/lcdf/point.hh" -*-

/* point.{cc,hh} -- 2D points
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
#include <lcdf/point.hh>

Point
Point::rotated(double rotation) const noexcept
{
    double r = length();
    double theta = angle() + rotation;
    return Point(r * cos(theta), r * sin(theta));
}

Point
Point::midpoint(const Point &a, const Point &b) noexcept
{
    return Point((a.x + b.x)/2, (a.y + b.y)/2);
}

bool
Point::on_line(const Point &a, const Point &b, double tolerance) const noexcept
{
    Point c = b - a;
    double d = c.x * (y - a.y) - c.y * (x - a.x);
    return (d * d <= tolerance * tolerance * c.squared_length());
}

bool
Point::on_segment(const Point &a, const Point &b, double t) const noexcept
{
    double tt;
    Point c = b - a;
    if (fabs(c.x) > fabs(c.y))
	tt = (x - a.x) / c.x;
    else if (c.y)
	tt = (y - a.y) / c.y;
    else
	tt = 0;
    if (tt < 0 || tt > 1)
	return 0;
    return on_line(a, b, t);
}
