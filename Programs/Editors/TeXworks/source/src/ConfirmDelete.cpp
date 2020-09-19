/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2008-2019  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "ConfirmDelete.h"

#include <QPushButton>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

ConfirmDelete::ConfirmDelete(QWidget *parent)
	: QDialog(parent)
{
	init();
}

void ConfirmDelete::init()
{
	setupUi(this);
	buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Delete"));
	connect(selectAll, SIGNAL(clicked()), this, SLOT(doSelectAll()));
	connect(selectNone, SIGNAL(clicked()), this, SLOT(doSelectNone()));
	connect(toggleSelection, SIGNAL(clicked()), this, SLOT(doToggleSelection()));
	connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(doToggleItemSelection(QListWidgetItem*)));
	connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(setDeleteButtonEnabledStatus()));
}

void ConfirmDelete::doConfirmDelete(const QDir& dir, const QStringList& fileList)
{
	ConfirmDelete dlg;

	dlg.listWidget->addItems(fileList);
	for (int i = 0; i < dlg.listWidget->count(); ++i)
		dlg.listWidget->item(i)->setCheckState(Qt::Checked);

	dlg.show();
	DialogCode result = static_cast<DialogCode>(dlg.exec());

	if (result == Accepted) {
		bool failed = false;
		for (int i = 0; i < dlg.listWidget->count(); ++i) {
			if (dlg.listWidget->item(i)->checkState() == Qt::Checked) {
				QFileInfo fileInfo(dir, dlg.listWidget->item(i)->text());
				if (!QFile::remove(fileInfo.canonicalFilePath()))
					failed = true;
			}
		}
		if (failed)
			(void)QMessageBox::warning(nullptr, tr("Unable to delete"),
									   tr("Some of the auxiliary files could not be removed. Perhaps you don't have permission to delete them."));
	}
}

void ConfirmDelete::doSelectAll()
{
	for (int i = 0; i < listWidget->count(); ++i)
		listWidget->item(i)->setCheckState(Qt::Checked);
	setDeleteButtonEnabledStatus();
}

void ConfirmDelete::doSelectNone()
{
	for (int i = 0; i < listWidget->count(); ++i)
		listWidget->item(i)->setCheckState(Qt::Unchecked);
	setDeleteButtonEnabledStatus();
}

void ConfirmDelete::doToggleSelection()
{
	for (int i = 0; i < listWidget->count(); ++i) {
		Qt::CheckState checked = listWidget->item(i)->checkState();
		listWidget->item(i)->setCheckState(checked == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
	}
	setDeleteButtonEnabledStatus();
}

void ConfirmDelete::doToggleItemSelection(QListWidgetItem * item)
{
	if (!item)
		return;
	item->setCheckState(item->checkState() == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
}

void ConfirmDelete::setDeleteButtonEnabledStatus()
{
	bool anyChecked = false;
	for (int i = 0; i < listWidget->count() && !anyChecked; ++i)
		if (listWidget->item(i)->checkState() == Qt::Checked)
			anyChecked = true;
	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(anyChecked);
}
