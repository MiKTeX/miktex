/* poppler-optcontent.cc: qt interface to poppler
 *
 * Copyright (C) 2007, Brad Hards <bradh@kde.org>
 * Copyright (C) 2008, 2014, Pino Toscano <pino@kde.org>
 * Copyright (C) 2008, Carlos Garcia Campos <carlosgc@gnome.org>
 * Copyright (C) 2015-2019, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2017, Hubert Figuière <hub@figuiere.net>
 * Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
 * Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
 * Copyright (C) 2019, 2020 Oliver Sander <oliver.sander@tu-dresden.de>
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

#include "poppler-optcontent.h"

#include "poppler-optcontent-private.h"

#include "poppler-private.h"
#include "poppler-link-private.h"

#include <QtCore/QDebug>
#include <QtCore/QtAlgorithms>

#include "poppler/OptionalContent.h"
#include "poppler/Link.h"

namespace Poppler {
// TODO use qAsConst when we can depend on Qt 5.7
//      or std::as_const when we can depend on C++17
template<class T>
constexpr std::add_const_t<T> &as_const(T &t) noexcept
{
    return t;
}

RadioButtonGroup::RadioButtonGroup(OptContentModelPrivate *ocModel, Array *rbarray)
{
    itemsInGroup.reserve(rbarray->getLength());
    for (int i = 0; i < rbarray->getLength(); ++i) {
        const Object &ref = rbarray->getNF(i);
        if (!ref.isRef()) {
            qDebug() << "expected ref, but got:" << ref.getType();
        }
        OptContentItem *item = ocModel->itemFromRef(QString::number(ref.getRefNum()));
        itemsInGroup.append(item);
    }
    for (OptContentItem *item : as_const(itemsInGroup)) {
        item->appendRBGroup(this);
    }
}

RadioButtonGroup::~RadioButtonGroup() { }

QSet<OptContentItem *> RadioButtonGroup::setItemOn(OptContentItem *itemToSetOn)
{
    QSet<OptContentItem *> changedItems;
    for (OptContentItem *thisItem : as_const(itemsInGroup)) {
        if (thisItem != itemToSetOn) {
            QSet<OptContentItem *> newChangedItems;
            thisItem->setState(OptContentItem::Off, false /*obeyRadioGroups*/, newChangedItems);
            changedItems += newChangedItems;
        }
    }
    return changedItems;
}

OptContentItem::OptContentItem(OptionalContentGroup *group)
{
    m_group = group;
    m_parent = nullptr;
    m_name = UnicodeParsedString(group->getName());
    if (group->getState() == OptionalContentGroup::On) {
        m_state = OptContentItem::On;
    } else {
        m_state = OptContentItem::Off;
    }
    m_stateBackup = m_state;
    m_enabled = true;
}

OptContentItem::OptContentItem(const QString &label)
{
    m_parent = nullptr;
    m_name = label;
    m_group = nullptr;
    m_state = OptContentItem::HeadingOnly;
    m_stateBackup = m_state;
    m_enabled = true;
}

OptContentItem::OptContentItem() : m_parent(nullptr), m_enabled(true) { }

OptContentItem::~OptContentItem() { }

void OptContentItem::appendRBGroup(RadioButtonGroup *rbgroup)
{
    m_rbGroups.append(rbgroup);
}

void OptContentItem::setState(ItemState state, bool obeyRadioGroups, QSet<OptContentItem *> &changedItems)
{
    if (state == m_state)
        return;

    m_state = state;
    m_stateBackup = m_state;
    changedItems.insert(this);
    QSet<OptContentItem *> empty;
    Q_FOREACH (OptContentItem *child, m_children) {
        ItemState oldState = child->m_stateBackup;
        child->setState(state == OptContentItem::On ? child->m_stateBackup : OptContentItem::Off, true /*obeyRadioGroups*/, empty);
        child->m_enabled = state == OptContentItem::On;
        child->m_stateBackup = oldState;
    }
    if (!m_group) {
        return;
    }
    if (state == OptContentItem::On) {
        m_group->setState(OptionalContentGroup::On);
        if (obeyRadioGroups) {
            for (RadioButtonGroup *rbgroup : as_const(m_rbGroups)) {
                changedItems += rbgroup->setItemOn(this);
            }
        }
    } else if (state == OptContentItem::Off) {
        m_group->setState(OptionalContentGroup::Off);
    }
}

void OptContentItem::addChild(OptContentItem *child)
{
    m_children += child;
    child->setParent(this);
}

QSet<OptContentItem *> OptContentItem::recurseListChildren(bool includeMe) const
{
    QSet<OptContentItem *> ret;
    if (includeMe) {
        ret.insert(const_cast<OptContentItem *>(this));
    }
    Q_FOREACH (OptContentItem *child, m_children) {
        ret += child->recurseListChildren(true);
    }
    return ret;
}

OptContentModelPrivate::OptContentModelPrivate(OptContentModel *qq, OCGs *optContent) : q(qq)
{
    m_rootNode = new OptContentItem();
    const auto &ocgs = optContent->getOCGs();

    for (const auto &ocg : ocgs) {
        OptContentItem *node = new OptContentItem(ocg.second.get());
        m_optContentItems.insert(QString::number(ocg.first.num), node);
    }

    if (optContent->getOrderArray() == nullptr) {
        // no Order array, so drop them all at the top level
        QMapIterator<QString, OptContentItem *> i(m_optContentItems);
        while (i.hasNext()) {
            i.next();
            addChild(m_rootNode, i.value());
        }
    } else {
        parseOrderArray(m_rootNode, optContent->getOrderArray());
    }

    parseRBGroupsArray(optContent->getRBGroupsArray());
}

OptContentModelPrivate::~OptContentModelPrivate()
{
    qDeleteAll(m_optContentItems);
    qDeleteAll(m_rbgroups);
    qDeleteAll(m_headerOptContentItems);
    delete m_rootNode;
}

void OptContentModelPrivate::parseOrderArray(OptContentItem *parentNode, Array *orderArray)
{
    OptContentItem *lastItem = parentNode;
    for (int i = 0; i < orderArray->getLength(); ++i) {
        Object orderItem = orderArray->get(i);
        if (orderItem.isDict()) {
            const Object &item = orderArray->getNF(i);
            if (item.isRef()) {
                OptContentItem *ocItem = m_optContentItems.value(QString::number(item.getRefNum()));
                if (ocItem) {
                    addChild(parentNode, ocItem);
                    lastItem = ocItem;
                } else {
                    qDebug() << "could not find group for object" << item.getRefNum();
                }
            }
        } else if ((orderItem.isArray()) && (orderItem.arrayGetLength() > 0)) {
            parseOrderArray(lastItem, orderItem.getArray());
        } else if (orderItem.isString()) {
            const GooString *label = orderItem.getString();
            OptContentItem *header = new OptContentItem(UnicodeParsedString(label));
            m_headerOptContentItems.append(header);
            addChild(parentNode, header);
            parentNode = header;
            lastItem = header;
        } else {
            qDebug() << "something unexpected";
        }
    }
}

void OptContentModelPrivate::parseRBGroupsArray(Array *rBGroupArray)
{
    if (!rBGroupArray) {
        return;
    }
    // This is an array of array(s)
    for (int i = 0; i < rBGroupArray->getLength(); ++i) {
        Object rbObj = rBGroupArray->get(i);
        if (!rbObj.isArray()) {
            qDebug() << "expected inner array, got:" << rbObj.getType();
            return;
        }
        Array *rbarray = rbObj.getArray();
        RadioButtonGroup *rbg = new RadioButtonGroup(this, rbarray);
        m_rbgroups.append(rbg);
    }
}

OptContentModel::OptContentModel(OCGs *optContent, QObject *parent) : QAbstractItemModel(parent)
{
    d = new OptContentModelPrivate(this, optContent);
}

OptContentModel::~OptContentModel()
{
    delete d;
}

void OptContentModelPrivate::setRootNode(OptContentItem *node)
{
    q->beginResetModel();
    delete m_rootNode;
    m_rootNode = node;
    q->endResetModel();
}

QModelIndex OptContentModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0) {
        return QModelIndex();
    }

    OptContentItem *parentNode = d->nodeFromIndex(parent);
    if (row < parentNode->childList().count()) {
        return createIndex(row, column, parentNode->childList().at(row));
    }
    return QModelIndex();
}

QModelIndex OptContentModel::parent(const QModelIndex &child) const
{
    OptContentItem *childNode = d->nodeFromIndex(child);
    if (!childNode) {
        return QModelIndex();
    }
    return d->indexFromItem(childNode->parent(), child.column());
}

QModelIndex OptContentModelPrivate::indexFromItem(OptContentItem *node, int column) const
{
    if (!node) {
        return QModelIndex();
    }
    OptContentItem *parentNode = node->parent();
    if (!parentNode) {
        return QModelIndex();
    }
    const int row = parentNode->childList().indexOf(node);
    return q->createIndex(row, column, node);
}

int OptContentModel::rowCount(const QModelIndex &parent) const
{
    OptContentItem *parentNode = d->nodeFromIndex(parent);
    if (!parentNode) {
        return 0;
    } else {
        return parentNode->childList().count();
    }
}

int OptContentModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant OptContentModel::data(const QModelIndex &index, int role) const
{
    OptContentItem *node = d->nodeFromIndex(index, true);
    if (!node) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return node->name();
        break;
    case Qt::EditRole:
        if (node->state() == OptContentItem::On) {
            return true;
        } else if (node->state() == OptContentItem::Off) {
            return false;
        }
        break;
    case Qt::CheckStateRole:
        if (node->state() == OptContentItem::On) {
            return Qt::Checked;
        } else if (node->state() == OptContentItem::Off) {
            return Qt::Unchecked;
        }
        break;
    }

    return QVariant();
}

bool OptContentModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    OptContentItem *node = d->nodeFromIndex(index, true);
    if (!node) {
        return false;
    }

    switch (role) {
    case Qt::CheckStateRole: {
        const bool newvalue = value.toBool();
        QSet<OptContentItem *> changedItems;
        node->setState(newvalue ? OptContentItem::On : OptContentItem::Off, true /*obeyRadioGroups*/, changedItems);

        if (!changedItems.isEmpty()) {
            changedItems += node->recurseListChildren(false);
            QModelIndexList indexes;
            Q_FOREACH (OptContentItem *item, changedItems) {
                indexes.append(d->indexFromItem(item, 0));
            }
            std::stable_sort(indexes.begin(), indexes.end());
            Q_FOREACH (const QModelIndex &changedIndex, indexes) {
                emit dataChanged(changedIndex, changedIndex);
            }
            return true;
        }
        break;
    }
    }

    return false;
}

Qt::ItemFlags OptContentModel::flags(const QModelIndex &index) const
{
    OptContentItem *node = d->nodeFromIndex(index);
    Qt::ItemFlags itemFlags = Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    if (node->isEnabled()) {
        itemFlags |= Qt::ItemIsEnabled;
    }
    return itemFlags;
}

QVariant OptContentModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QAbstractItemModel::headerData(section, orientation, role);
}

void OptContentModel::applyLink(LinkOCGState *link)
{
    LinkOCGStatePrivate *linkPrivate = link->d_func();

    QSet<OptContentItem *> changedItems;

    const std::vector<::LinkOCGState::StateList> &statesList = linkPrivate->stateList;
    for (const ::LinkOCGState::StateList &stateList : statesList) {
        const std::vector<Ref> &refsList = stateList.list;
        for (const Ref &ref : refsList) {
            OptContentItem *item = d->itemFromRef(QString::number(ref.num));

            if (stateList.st == ::LinkOCGState::On) {
                item->setState(OptContentItem::On, linkPrivate->preserveRB, changedItems);
            } else if (stateList.st == ::LinkOCGState::Off) {
                item->setState(OptContentItem::Off, linkPrivate->preserveRB, changedItems);
            } else {
                OptContentItem::ItemState newState = item->state() == OptContentItem::On ? OptContentItem::Off : OptContentItem::On;
                item->setState(newState, linkPrivate->preserveRB, changedItems);
            }
        }
    }

    if (!changedItems.isEmpty()) {
        QSet<OptContentItem *> aux;
        Q_FOREACH (OptContentItem *item, aux) {
            changedItems += item->recurseListChildren(false);
        }

        QModelIndexList indexes;
        Q_FOREACH (OptContentItem *item, changedItems) {
            indexes.append(d->indexFromItem(item, 0));
        }
        std::stable_sort(indexes.begin(), indexes.end());
        Q_FOREACH (const QModelIndex &changedIndex, indexes) {
            emit dataChanged(changedIndex, changedIndex);
        }
    }
}

void OptContentModelPrivate::addChild(OptContentItem *parent, OptContentItem *child)
{
    parent->addChild(child);
}

OptContentItem *OptContentModelPrivate::itemFromRef(const QString &ref) const
{
    return m_optContentItems.value(ref);
}

OptContentItem *OptContentModelPrivate::nodeFromIndex(const QModelIndex &index, bool canBeNull) const
{
    if (index.isValid()) {
        return static_cast<OptContentItem *>(index.internalPointer());
    } else {
        return canBeNull ? nullptr : m_rootNode;
    }
}
}
