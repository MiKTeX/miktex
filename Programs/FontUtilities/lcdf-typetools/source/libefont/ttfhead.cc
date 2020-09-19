// -*- related-file-name: "../include/efont/ttfhead.hh" -*-

/* ttfhead.{cc,hh} -- TrueType head table
 *
 * Copyright (c) 2007-2019 Eddie Kohler
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
#include <efont/ttfhead.hh>
#include <lcdf/error.hh>
#include <lcdf/straccum.hh>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

namespace Efont { namespace OpenType {

Head::Head(const String &s, ErrorHandler *errh)
    : _d(s)
{
    _error = parse_header(errh ? errh : ErrorHandler::silent_handler());
}

int
Head::parse_header(ErrorHandler *errh)
{
    // HEAD format:
    // 0        Fixed           Table version number    0x00010000 (ver. 1.0)
    // 4        Fixed           fontRevision
    // 8        ULONG           checkSumAdjustment
    // 12       ULONG           magicNumber             Set to 0x5F0F3CF5
    // 16       USHORT          flags
    // 18       USHORT          unitsPerEm
    // 20       LONGDATETIME    created
    // 28       LONGDATETIME    modified
    // 36       USHORT          xMin
    // 38       SHORT           yMin
    // 40       SHORT           xMax
    // 42       SHORT           yMax
    // 44       USHORT          macStyle
    // 46       USHORT          lowestRecPPEM
    // 48       SHORT           fontDirectionHint
    // 50       SHORT           indexToLocFormat
    // 52       SHORT           glyphDataFormat
    int len = _d.length();
    const uint8_t *data = _d.udata();
    if (len == 0)
        return errh->error("font has no 'head' table"), -EFAULT;
    if (54 > len)
        return errh->error("'head' table too small"), -EFAULT;
    if (!(data[0] == '\000' && data[1] == '\001'))
        return errh->error("bad 'head' version number"), -ERANGE;
    if (_d.u32(12) != 0x5F0F3CF5)
        return errh->error("bad 'head' magic number"), -ERANGE;
    return 0;
}

}}
