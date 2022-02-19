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

#include "RemoveAuxFilesDialog.h"

#include <QDialogButtonBox>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

namespace Tw {
namespace UI {

RemoveAuxFilesDialog::RemoveAuxFilesDialog(QWidget * parent)
	: ListSelectDialog(parent)
{
	setWindowTitle(tr("Confirm file deletion"));
	setListLabel(tr("Auxiliary files to be deleted:"));
	buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Delete"));
	connect(this, &ListSelectDialog::itemChanged, this, &RemoveAuxFilesDialog::setDeleteButtonEnabledStatus);
}

void RemoveAuxFilesDialog::doConfirmDelete(const QDir & dir, const QStringList & fileList)
{
	RemoveAuxFilesDialog dlg;

	dlg.addItems(fileList);
	dlg.checkAll();
	dlg.show();
	DialogCode result = static_cast<DialogCode>(dlg.exec());

	if (result == Accepted) {
		bool failed = false;
		Q_FOREACH(const QString & filepath, dlg.checkedItems()) {
			QFileInfo fileInfo(dir, filepath);
			if (!QFile::remove(fileInfo.canonicalFilePath()))
				failed = true;
		}
		if (failed)
			(void)QMessageBox::warning(nullptr, tr("Unable to delete"),
									   tr("Some of the auxiliary files could not be removed. Perhaps you don't have permission to delete them."));
	}
}

void RemoveAuxFilesDialog::setDeleteButtonEnabledStatus()
{
	const bool anyChecked = !checkedItems().isEmpty();
	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(anyChecked);
}

} // namespace UI
} // namespace Tw
