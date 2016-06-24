/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2008-2015  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	For links to further information, or to contact the authors,
	see <http://www.tug.org/texworks/>.
*/

#include "TeXDocks.h"

#include "TeXDocument.h"

#include <QTreeWidget>
#include <QHeaderView>
#include <QScrollBar>

TeXDock::TeXDock(const QString& title, TeXDocument *doc)
	: QDockWidget(title, doc), document(doc), filled(false)
{
	connect(this, SIGNAL(visibilityChanged(bool)), SLOT(myVisibilityChanged(bool)));
}

TeXDock::~TeXDock()
{
}

void TeXDock::myVisibilityChanged(bool visible)
{
	if (visible && document && !filled) {
		fillInfo();
		filled = true;
	}
}

//////////////// TAGS ////////////////

TagsDock::TagsDock(TeXDocument *doc)
	: TeXDock(tr("Tags"), doc)
{
	setObjectName("tags");
	setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	tree = new TeXDockTreeWidget(this);
	tree->header()->hide();
	tree->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	setWidget(tree);
	connect(doc, SIGNAL(tagListUpdated()), this, SLOT(listChanged()));
	saveScrollValue = 0;
}

TagsDock::~TagsDock()
{
}

void TagsDock::fillInfo()
{
	disconnect(tree, SIGNAL(itemSelectionChanged()), this, SLOT(followTagSelection()));
	disconnect(tree, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(followTagSelection()));
	disconnect(tree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(followTagSelection()));
	tree->clear();
	const QList<TeXDocument::Tag>& tags = document->getTags();
	if (tags.size() > 0) {
		QTreeWidgetItem *item = 0, *bmItem = 0;
		QTreeWidgetItem *bookmarks = new QTreeWidgetItem(tree);
		bookmarks->setText(0, tr("Bookmarks"));
		bookmarks->setFlags(Qt::ItemIsEnabled);
		bookmarks->setForeground(0, Qt::blue);
		tree->expandItem(bookmarks);
		QTreeWidgetItem *outline = new QTreeWidgetItem(tree, bookmarks);
		outline->setText(0, tr("Outline"));
		outline->setFlags(Qt::ItemIsEnabled);
		outline->setForeground(0, Qt::blue);
		tree->expandItem(outline);
		for (int index = 0; index < tags.size(); ++index) {
			const TeXDocument::Tag& bm = tags[index];
			if (bm.level < 1) {
				bmItem = new QTreeWidgetItem(bookmarks, QTreeWidgetItem::UserType);
				bmItem->setText(0, bm.text);
				bmItem->setText(1, QString::number(index));
			}
			else  {
				while (item != 0 && item->type() >= QTreeWidgetItem::UserType + bm.level)
					item = item->parent();
				if (item == 0)
					item = new QTreeWidgetItem(outline, QTreeWidgetItem::UserType + bm.level);
				else
					item = new QTreeWidgetItem(item, QTreeWidgetItem::UserType + bm.level);
				item->setText(0, bm.text);
				item->setText(1, QString::number(index));
				tree->expandItem(item);
			}
		}
		if (bookmarks->childCount() == 0)
			bookmarks->setHidden(true);
		if (outline->childCount() == 0)
			outline->setHidden(true);
		if (saveScrollValue > 0) {
			tree->verticalScrollBar()->setValue(saveScrollValue);
			saveScrollValue = 0;
		}
		connect(tree, SIGNAL(itemSelectionChanged()), this, SLOT(followTagSelection()));
		connect(tree, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(followTagSelection()));
		connect(tree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(followTagSelection()));
	} else {
		QTreeWidgetItem *item = new QTreeWidgetItem();
		item->setText(0, tr("No tags"));
		item->setFlags(item->flags() & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
		tree->addTopLevelItem(item);
	}
}

void TagsDock::listChanged()
{
	saveScrollValue = tree->verticalScrollBar()->value();
	tree->clear();
	filled = false;
	if (document && isVisible())
		fillInfo();
}

void TagsDock::followTagSelection()
{
	QList<QTreeWidgetItem*> items = tree->selectedItems();
	if (items.count() > 0) {
		QTreeWidgetItem* item = items.first();
		QString dest = item->text(1);
		if (!dest.isEmpty())
			document->goToTag(dest.toInt());
	}
}

TeXDockTreeWidget::TeXDockTreeWidget(QWidget* parent)
	: QTreeWidget(parent)
{
	setIndentation(10);
}

TeXDockTreeWidget::~TeXDockTreeWidget()
{
}

QSize TeXDockTreeWidget::sizeHint() const
{
	return QSize(180, 300);
}
