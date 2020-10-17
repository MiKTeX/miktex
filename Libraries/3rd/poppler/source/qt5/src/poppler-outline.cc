/* poppler-outline.cc: qt interface to poppler
 *
 * Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
 * Copyright (C) 2019 Oliver Sander <oliver.sander@tu-dresden.de>
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

#include <poppler-qt5.h>
#include <poppler-link.h>

#include "poppler-private.h"
#include "poppler-outline-private.h"

#include "Link.h"
#include "Outline.h"

namespace Poppler {

OutlineItem::OutlineItem() : m_data { new OutlineItemData { nullptr, nullptr } } { }

OutlineItem::OutlineItem(OutlineItemData *data) : m_data { data } { }

OutlineItem::~OutlineItem()
{
    delete m_data;
    m_data = nullptr;
}

OutlineItem::OutlineItem(const OutlineItem &other) : m_data { new OutlineItemData { *other.m_data } } { }

OutlineItem &OutlineItem::operator=(const OutlineItem &other)
{
    if (this == &other)
        return *this;

    auto *data = new OutlineItemData { *other.m_data };
    qSwap(m_data, data);
    delete data;

    return *this;
}

OutlineItem::OutlineItem(OutlineItem &&other) noexcept : m_data { other.m_data }
{
    other.m_data = nullptr;
}

OutlineItem &OutlineItem::operator=(OutlineItem &&other) noexcept
{
    qSwap(m_data, other.m_data);

    return *this;
}

bool OutlineItem::isNull() const
{
    return !m_data->data;
}

QString OutlineItem::name() const
{
    QString &name = m_data->name;

    if (name.isEmpty()) {
        if (const ::OutlineItem *data = m_data->data) {
            name = unicodeToQString(data->getTitle(), data->getTitleLength());
        }
    }

    return name;
}

bool OutlineItem::isOpen() const
{
    bool isOpen = false;

    if (const ::OutlineItem *data = m_data->data) {
        isOpen = data->isOpen();
    }

    return isOpen;
}

QSharedPointer<const LinkDestination> OutlineItem::destination() const
{
    QSharedPointer<const LinkDestination> &destination = m_data->destination;

    if (!destination) {
        if (const ::OutlineItem *data = m_data->data) {
            if (const ::LinkAction *action = data->getAction()) {
                if (action->getKind() == actionGoTo) {
                    const auto *linkGoTo = static_cast<const LinkGoTo *>(action);
                    destination.reset(new LinkDestination(LinkDestinationData(linkGoTo->getDest(), linkGoTo->getNamedDest(), m_data->documentData, false)));
                } else if (action->getKind() == actionGoToR) {
                    const auto *linkGoToR = static_cast<const LinkGoToR *>(action);
                    const bool external = linkGoToR->getFileName() != nullptr;
                    destination.reset(new LinkDestination(LinkDestinationData(linkGoToR->getDest(), linkGoToR->getNamedDest(), m_data->documentData, external)));
                }
            }
        }
    }

    return destination;
}

QString OutlineItem::externalFileName() const
{
    QString &externalFileName = m_data->externalFileName;

    if (externalFileName.isEmpty()) {
        if (const ::OutlineItem *data = m_data->data) {
            if (const ::LinkAction *action = data->getAction()) {
                if (action->getKind() == actionGoToR) {
                    if (const GooString *fileName = static_cast<const LinkGoToR *>(action)->getFileName()) {
                        externalFileName = UnicodeParsedString(fileName);
                    }
                }
            }
        }
    }

    return externalFileName;
}

QString OutlineItem::uri() const
{
    QString &uri = m_data->uri;

    if (uri.isEmpty()) {
        if (const ::OutlineItem *data = m_data->data) {
            if (const ::LinkAction *action = data->getAction()) {
                if (action->getKind() == actionURI) {
                    uri = UnicodeParsedString(static_cast<const LinkURI *>(action)->getURI());
                }
            }
        }
    }

    return uri;
}

bool OutlineItem::hasChildren() const
{
    bool result = false;

    if (::OutlineItem *data = m_data->data) {
        result = data->hasKids();
    }

    return result;
}

QVector<OutlineItem> OutlineItem::children() const
{
    QVector<OutlineItem> result;

    if (::OutlineItem *data = m_data->data) {
        data->open();
        if (const std::vector<::OutlineItem *> *kids = data->getKids()) {
            for (void *kid : *kids) {
                result.push_back(OutlineItem { new OutlineItemData { static_cast<::OutlineItem *>(kid), m_data->documentData } });
            }
        }
    }

    return result;
}

}
