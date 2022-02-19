/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2022  Stefan Löffler, Jonathan Kew

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

#include "ListSelectDialog.h"

#include <QListWidgetItem>
#include <QPushButton>

namespace Tw {
namespace UI {

ListSelectDialog::ListSelectDialog(QWidget * parent)
	: QDialog(parent)
{
	setupUi(this);
	connect(bSelectAll, &QPushButton::clicked, this, &ListSelectDialog::checkAll);
	connect(bSelectNone, &QPushButton::clicked, this, &ListSelectDialog::uncheckAll);
	connect(bToggleSelection, &QPushButton::clicked, this, &ListSelectDialog::toggleCheckState);
	connect(listWidget, &QListWidget::itemChanged, this, &ListSelectDialog::itemChanged);
}

void ListSelectDialog::clearItems()
{
	listWidget->clear();
}

void ListSelectDialog::addItems(const QStringList &labels)
{
	listWidget->addItems(labels);
}

QStringList ListSelectDialog::checkedItems() const
{
	QStringList retVal;
	for (int i = 0; i < listWidget->count(); ++i) {
		const QListWidgetItem * const item = listWidget->item(i);
		if (item->checkState() == Qt::Checked) {
			retVal.append(item->text());
		}
	}
	return retVal;
}

QString ListSelectDialog::listLabel() const
{
	return groupBox->title();
}

void ListSelectDialog::setListLabel(const QString &newPrompt)
{
	groupBox->setTitle(newPrompt);
}

void ListSelectDialog::checkAll()
{
	for (int i = 0; i < listWidget->count(); ++i) {
		listWidget->item(i)->setCheckState(Qt::Checked);
	}
}

void ListSelectDialog::uncheckAll()
{
	for (int i = 0; i < listWidget->count(); ++i) {
		listWidget->item(i)->setCheckState(Qt::Unchecked);
	}
}

void ListSelectDialog::toggleCheckState()
{
	for (int i = 0; i < listWidget->count(); ++i) {
		const Qt::CheckState checked = listWidget->item(i)->checkState();
		listWidget->item(i)->setCheckState(checked == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
	}
}

} // namespace UI
} // namespace Tw
