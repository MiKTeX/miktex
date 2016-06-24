/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2013  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef PrefsDialog_H
#define PrefsDialog_H

#include <QDialog>
#include <QList>

#include "TWUtils.h"

#include "ui_PrefsDialog.h"
#include "ui_ToolConfig.h"

class QListWidgetItem;

class PrefsDialog : public QDialog, private Ui::PrefsDialog
{
	Q_OBJECT

public:
	PrefsDialog(QWidget *parent);

	static DialogCode doPrefsDialog(QWidget *parent);

private slots:
	void buttonClicked(QAbstractButton *whichButton);
	
	void changedTabPanel(int index);

	void updatePathButtons();
	void movePathUp();
	void movePathDown();
	void addPath();
	void removePath();
	
	void updateToolButtons();
	void moveToolUp();
	void moveToolDown();
	void addTool();
	void removeTool();
	void editTool(QListWidgetItem* item = NULL);

private:
	void init();
	void restoreDefaults();
	void refreshDefaultTool();
	void initPathAndToolLists();
	
	QList<Engine> engineList;
	
	bool pathsChanged;
	bool toolsChanged;
	
	static int sCurrentTab;
};

class ToolConfig : public QDialog, private Ui::ToolConfigDialog
{
	Q_OBJECT
	
public:
	ToolConfig(QWidget *parent);
	
	static DialogCode doToolConfig(QWidget *parent, Engine &engine);

private slots:
	void updateArgButtons();
	void moveArgUp();
	void moveArgDown();
	void addArg();
	void removeArg();
	void browseForProgram();

private:
	void init();
};

#endif
