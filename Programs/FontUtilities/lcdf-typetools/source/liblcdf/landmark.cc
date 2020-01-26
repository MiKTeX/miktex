// -*- related-file-name: "../include/lcdf/landmark.hh" -*-

/* landmark.{cc,hh} -- FILE:LINE type landmarks
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
#include <lcdf/landmark.hh>

Landmark
operator+(const Landmark &landmark, int offset)
{
    if (landmark.has_line())
	return Landmark(landmark.file(), landmark.line() + offset);
    else
	return landmark;
}

Landmark::operator String() const
{
    if (_file && has_line())
	return _file + ":" + String(_line);
    else
	return _file;
}
