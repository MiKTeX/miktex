// -*- related-file-name: "../include/efont/encoding.hh" -*-

/* encoding.{cc,hh} -- 8-bit encodings
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
#include <efont/encoding.hh>
namespace Efont {

void
Encoding8::reserve_glyphs(int count)
{
  if (count <= _codes.size()) return;
  _codes.resize(count, -1);
}

}
