/*
 * Copyright (C) 2008, 2019, 2021, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2017, Adrian Johnson <ajohnson@redneon.com>
 * Copyright (C) 2018, Adam Reichold <adam.reichold@t-online.de>
 * Copyright (C) 2019, Oliver Sander <oliver.sander@tu-dresden.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <algorithm>

#include "printencodings.h"

#include "GlobalParams.h"
#include "goo/GooString.h"

void printEncodings()
{
    std::vector<std::string> encNames = globalParams->getEncodingNames();

    std::sort(encNames.begin(), encNames.end());

    printf("Available encodings are:\n");
    for (const std::string &enc : encNames) {
        printf("%s\n", enc.c_str());
    }
}
