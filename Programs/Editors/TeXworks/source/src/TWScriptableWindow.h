/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2009-2020  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef TWScriptableWindow_H
#define TWScriptableWindow_H

#include "TWScriptManager.h"
#include "scripting/Script.h"

#include <QList>
#include <QMainWindow>

class QAction;
class QMenu;
class QSignalMapper;

// parent class for document windows (i.e. both the source and PDF window types);
// handles the Scripts menu and other common functionality
class TWScriptableWindow : public QMainWindow
{
	Q_OBJECT

public:
	TWScriptableWindow() = default;
	~TWScriptableWindow() override = default;

public slots:
	void updateScriptsMenu();
	void runScript(QObject * script, Tw::Scripting::Script::ScriptType scriptType = Tw::Scripting::Script::ScriptStandalone);
	void runHooks(const QString& hookName);

	void selectWindow(bool activate = true);
	void placeOnLeft();
	void placeOnRight();

private slots:
	void doManageScripts();
	void doAboutScripts();

protected slots:
	void hideFloatersUnlessThis(QWidget* currWindow);

protected slots:
	void scriptDeleted(QObject * obj);

protected:
	void initScriptable(QMenu* scriptsMenu,
						QAction* aboutScriptsAction,
						QAction* manageScriptsAction,
						QAction* updateScriptsAction,
						QAction* showScriptsFolderAction);

	int addScriptsToMenu(QMenu *menu, TWScriptList *scripts);
	void removeScriptsFromMenu(QMenu *menu, int startIndex = 0);

	void showFloaters();

	virtual QString scriptContext() const = 0;

private:
	QMenu * scriptsMenu{nullptr};
	QSignalMapper * scriptMapper{nullptr};
	int staticScriptMenuItemCount{0};

	QList<QWidget*> latentVisibleWidgets;
};

#endif // !defined(TWScriptableWindow_H)
