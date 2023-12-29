/*
 * Copyright (C) 2009-2010, Pino Toscano <pino@kde.org>
 * Copyright (C) 2018, Albert Astals Cid <aacid@kde.org>
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

#include "poppler-version.h"

QString Poppler::Version::string()
{
    return QStringLiteral(POPPLER_VERSION);
}

unsigned int Poppler::Version::major()
{
    return POPPLER_VERSION_MAJOR;
}

unsigned int Poppler::Version::minor()
{
    return POPPLER_VERSION_MINOR;
}

unsigned int Poppler::Version::micro()
{
    return POPPLER_VERSION_MICRO;
}
