// -*- related-file-name: "../include/efont/otfdata.hh" -*-

/* otfdata.{cc,hh} -- OpenType bounds-checked string type
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
#include <efont/otfdata.hh>

namespace Efont { namespace OpenType {

Data
Data::subtable(unsigned offset) const
{
    if (offset > (unsigned) _str.length())
        throw Bounds();
    return Data(_str.substring(offset));
}

Data
Data::offset_subtable(unsigned offset_offset) const
{
    int offset = u16(offset_offset);
    if (offset > _str.length())
        throw Bounds();
    return Data(_str.substring(offset));
}

}}

// template instantiations
#include <lcdf/vector.cc>
