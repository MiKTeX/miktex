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

#ifndef ScriptManager_H
#define ScriptManager_H

#include <QDialog>
#include "ui_ScriptManager.h"

#include "TWScriptable.h"


class ScriptManager : public QWidget, private Ui::ScriptManager
{
	Q_OBJECT

public:
	static void showManageScripts();
	static void refreshScriptList();

protected:
	virtual void closeEvent(QCloseEvent * event);

signals:
	void scriptListChanged();
	
private slots:
	void treeItemClicked(QTreeWidgetItem * item, int column);
	void treeItemActivated(QTreeWidgetItem * item, int column);
	void treeSelectionChanged();

private:
	ScriptManager(QWidget * parent = NULL) : QWidget(parent) { init(); }
	
	void init();
	
	void populateTree();
	void populateTree(QTreeWidget * tree, QTreeWidgetItem * parentItem, const TWScriptList * scripts);
	void setFolderCheckedState(QTreeWidgetItem * item);

	void addDetailsRow(QString& html, const QString label, const QString value);
	
	static ScriptManager * gManageScriptsWindow;
	static QRect           gGeometry;
};

#endif // !defined(ScriptManager_H)
