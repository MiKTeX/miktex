/* poppler-outline-private.h: qt interface to poppler
 *
 * Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
 * Copyright (C) 2019 Albert Astals Cid <aacid@kde.org>
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

#ifndef _POPPLER_OUTLINE_PRIVATE_H_
#define _POPPLER_OUTLINE_PRIVATE_H_

#include <QtCore/QSharedPointer>
#include <QtCore/QString>

class OutlineItem;

namespace Poppler {

class DocumentData;
class LinkDestination;

struct OutlineItemData
{
    OutlineItemData(::OutlineItem *oi, DocumentData *dd) : data { oi }, documentData { dd } { }
    ::OutlineItem *data;
    DocumentData *documentData;

    mutable QString name;
    mutable QSharedPointer<const LinkDestination> destination;
    mutable QString externalFileName;
    mutable QString uri;
};

}

#endif
