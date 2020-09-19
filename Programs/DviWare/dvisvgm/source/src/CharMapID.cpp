/*************************************************************************
** CharMapID.cpp                                                        **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2020 Martin Gieseking <martin.gieseking@uos.de>   **
**                                                                      **
** This program is free software; you can redistribute it and/or        **
** modify it under the terms of the GNU General Public License as       **
** published by the Free Software Foundation; either version 3 of       **
** the License, or (at your option) any later version.                  **
**                                                                      **
** This program is distributed in the hope that it will be useful, but  **
** WITHOUT ANY WARRANTY; without even the implied warranty of           **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         **
** GNU General Public License for more details.                         **
**                                                                      **
** You should have received a copy of the GNU General Public License    **
** along with this program; if not, see <http://www.gnu.org/licenses/>. **
*************************************************************************/

#include "CharMapID.hpp"

const CharMapID CharMapID::NONE(0, 0);
const CharMapID CharMapID::WIN_SYMBOL(3, 0);
const CharMapID CharMapID::WIN_UCS2(3, 1);
const CharMapID CharMapID::WIN_SHIFTJIS(3, 2);
const CharMapID CharMapID::WIN_PRC(3, 3);
const CharMapID CharMapID::WIN_BIG5(3, 4);
const CharMapID CharMapID::WIN_WANSUNG(3, 5);
const CharMapID CharMapID::WIN_JOHAB(3, 6);
const CharMapID CharMapID::WIN_UCS4(3, 10);

const CharMapID CharMapID::MAC_JAPANESE(1, 1);
const CharMapID CharMapID::MAC_TRADCHINESE(1, 2);
const CharMapID CharMapID::MAC_KOREAN(1, 3);
const CharMapID CharMapID::MAC_SIMPLCHINESE(1, 25);
