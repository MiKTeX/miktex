/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2008-2020  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef ScriptManagerWidget_H
#define ScriptManagerWidget_H

#include "ui_ScriptManagerWidget.h"

#include <QDialog>

class TWScriptList;

class ScriptManagerWidget : public QWidget, private Ui::ScriptManagerWidget
{
	Q_OBJECT

public:
	static void showManageScripts();
	static void refreshScriptList();

protected:
	void closeEvent(QCloseEvent * event) override;

signals:
	void scriptListChanged();

private slots:
	void treeItemClicked(QTreeWidgetItem * item, int column);
	void treeItemActivated(QTreeWidgetItem * item, int column);
	void treeSelectionChanged();

private:
	ScriptManagerWidget(QWidget * parent = nullptr) : QWidget(parent) { init(); }

	void init();

	void populateTree();
	void populateTree(QTreeWidget * tree, QTreeWidgetItem * parentItem, const TWScriptList * scripts);
	void setFolderCheckedState(QTreeWidgetItem * item);

	void addDetailsRow(QString& html, const QString & label, const QString & value);

	static ScriptManagerWidget * gManageScriptsWindow;
	static QRect           gGeometry;
};

#endif // !defined(ScriptManagerWidget_H)
