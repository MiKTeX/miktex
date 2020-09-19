/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2009-2019  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef TWScriptManager_H
#define TWScriptManager_H

#include "scripting/Script.h"

#include <QList>
#include <QObject>
#include <QString>

class TWScriptList : public QObject
{
	Q_OBJECT

public:
	TWScriptList() = default;

	explicit TWScriptList(const TWScriptList & orig)
	: QObject(orig.parent())
	, name(orig.name)
	{ }

	explicit TWScriptList(QObject * parent, const QString & str = QString())
	: QObject(parent), name(str)
	{ }

	const QString& getName() const { return name; }

private:
	QString name; // name of the folder/submenu
	// scripts and subfolders are stored as children of the QObject
};

class TWScriptManager
{
public:
	TWScriptManager();
	virtual ~TWScriptManager() = default;

	bool addScript(QObject* scriptList, Tw::Scripting::Script* script);
	void addScriptsInDirectory(const QDir& dir, const QStringList& disabled, const QStringList& ignore = QStringList()) {
		addScriptsInDirectory(&m_Scripts, &m_Hooks, dir, disabled, ignore);
	}
	void clear();

	TWScriptList* getScripts() { return &m_Scripts; }
	TWScriptList* getHookScripts() { return &m_Hooks; }
	QList<Tw::Scripting::Script*> getHookScripts(const QString& hook) const;

	bool runScript(QObject * script, QObject * context, QVariant & result, Tw::Scripting::Script::ScriptType scriptType = Tw::Scripting::Script::ScriptStandalone);
	// Convenience overload if no result is required
	bool runScript(QObject * script, QObject * context, Tw::Scripting::Script::ScriptType scriptType = Tw::Scripting::Script::ScriptStandalone) {
		QVariant result;
		return runScript(script, context, result, scriptType);
	}
	void runHooks(const QString& hookName, QObject * context = nullptr);

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

#endif // !defined(TWScriptManager)
