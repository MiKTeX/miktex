/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2008-2022  Stefan Löffler, Jonathan Kew

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

#ifndef ListSelectDialog_H
#define ListSelectDialog_H

#include "ui_ListSelectDialog.h"

#include <QDialog>
#include <QStringList>

namespace Tw {
namespace UI {

class ListSelectDialog : public QDialog, protected Ui::ListSelectDialog
{
	Q_OBJECT

public:
	ListSelectDialog(QWidget * parent = nullptr);

	void clearItems();
	void addItems(const QStringList & labels);
	QStringList checkedItems() const;

	QString listLabel() const;
	void setListLabel(const QString & newPrompt);

signals:
	void itemChanged(QListWidgetItem *);

protected slots:
	void checkAll();
	void uncheckAll();
	void toggleCheckState();
};

} // namespace UI
} // namespace Tw

#endif
