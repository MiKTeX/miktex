/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2009-2015  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef TWScriptable_H
#define TWScriptable_H

#include "TWScript.h"

#include <QMainWindow>
#include <QList>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QProcess>

class QMenu;
class QAction;
class QSignalMapper;

class TWScriptList : public QObject
{
	Q_OBJECT

public:
	TWScriptList()
	{ }
	
	TWScriptList(const TWScriptList& orig)
	: QObject(orig.parent())
	, name(orig.name)
	{ }
	
	TWScriptList(QObject* parent, const QString& str = QString())
	: QObject(parent), name(str)
	{ }
	
	const QString& getName() const { return name; }

private:
	QString name; // name of the folder/submenu
	// scripts and subfolders are stored as children of the QObject
};

class JSScript : public TWScript
{
	Q_OBJECT
	Q_INTERFACES(TWScript)
	
public:
	JSScript(QObject * plugin, const QString& filename)
		: TWScript(plugin, filename) { }
		
	virtual bool parseHeader() { return doParseHeader("", "", "//"); };

protected:
	virtual bool execute(TWScriptAPI *tw) const;
};

// for JSScript, we provide a plugin-like factory, but it's actually compiled
// and linked directly with the main application (at least for now)
class JSScriptInterface : public QObject, public TWScriptLanguageInterface
{
	Q_OBJECT
	Q_INTERFACES(TWScriptLanguageInterface)
	
public:
	JSScriptInterface() {};
	virtual ~JSScriptInterface() {};

	virtual TWScript* newScript(const QString& fileName);

	virtual QString scriptLanguageName() const { return QString("QtScript"); }
	virtual QString scriptLanguageURL() const { return QString("http://doc.qt.io/qt-5/qtscript-index.html"); }
	virtual bool canHandleFile(const QFileInfo& fileInfo) const { return fileInfo.suffix() == QString("js"); }
};

class TWScriptManager
{
public:
	TWScriptManager();
	virtual ~TWScriptManager() {};
	
	bool addScript(QObject* scriptList, TWScript* script);
	void addScriptsInDirectory(const QDir& dir, const QStringList& disabled, const QStringList& ignore = QStringList()) {
		addScriptsInDirectory(&m_Scripts, &m_Hooks, dir, disabled, ignore);
	}
	void clear();
		
	TWScriptList* getScripts() { return &m_Scripts; }
	TWScriptList* getHookScripts() { return &m_Hooks; }
	QList<TWScript*> getHookScripts(const QString& hook) const;

	bool runScript(QObject * script, QObject * context, QVariant & result, TWScript::ScriptType scriptType = TWScript::ScriptStandalone);
	// Convenience overload if no result is required
	bool runScript(QObject * script, QObject * context, TWScript::ScriptType scriptType = TWScript::ScriptStandalone) {
		QVariant result;
		return runScript(script, context, result, scriptType);
	}
	void runHooks(const QString& hookName, QObject * context = NULL);

	const QList<QObject*>& languages() const { return scriptLanguages; }

	void reloadScripts(bool forceAll = false);
	void saveDisabledList();

protected:
	void addScriptsInDirectory(TWScriptList *scriptList,
							   TWScriptList *hookList,
							   const QDir& dir,
							   const QStringList& disabled,
							   const QStringList& ignore);
	void loadPlugins();
	void reloadScriptsInList(TWScriptList * list, QStringList & processed);
	
private:
	TWScriptList m_Scripts; // hierarchical list of standalone scripts
	TWScriptList m_Hooks; // hierarchical list of hook scripts

	QList<QObject*> scriptLanguages;
};

// parent class for document windows (i.e. both the source and PDF window types);
// handles the Scripts menu and other common functionality
class TWScriptable : public QMainWindow
{
	Q_OBJECT

public:
	TWScriptable();
	virtual ~TWScriptable() { }
	
public slots:
	void updateScriptsMenu();
	void runScript(QObject * script, TWScript::ScriptType scriptType = TWScript::ScriptStandalone);
	void runHooks(const QString& hookName);
	
	void selectWindow(bool activate = true);
	void placeOnLeft();
	void placeOnRight();

private slots:
	void doManageScripts();
	void doAboutScripts();
	
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

private:
	QMenu* scriptsMenu;
	QSignalMapper* scriptMapper;
	int staticScriptMenuItemCount;

	QList<QWidget*> latentVisibleWidgets;
};

#endif
