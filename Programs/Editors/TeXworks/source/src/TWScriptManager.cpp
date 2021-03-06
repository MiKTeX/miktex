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

#if defined(MIKTEX)
#if defined(MIKTEX_WINDOWS)
#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>
#endif
#include <miktex/miktex-texworks.hpp>
#include <miktex/Trace/StopWatch>
#endif
#include "ScriptManagerWidget.h"
#include "Settings.h"
#include "TWScriptManager.h"
#include "scripting/ECMAScriptInterface.h"
#if WITH_QTSCRIPT
#	include "scripting/JSScriptInterface.h"
#endif
#include "scripting/ScriptAPI.h"
#include "scripting/ScriptLanguageInterface.h"
#include "utils/ResourcesLibrary.h"

#include <QDir>
#include <QPluginLoader>

#if STATIC_LUA_SCRIPTING_PLUGIN
#include <QtPlugin>
using namespace Tw::Scripting;
Q_IMPORT_PLUGIN(LuaScriptInterface)
#endif
#if STATIC_PYTHON_SCRIPTING_PLUGIN
#include <QtPlugin>
using namespace Tw::Scripting;
Q_IMPORT_PLUGIN(PythonScriptInterface)
#endif


TWScriptManager::TWScriptManager()
{
	loadPlugins();
	reloadScripts();
}

void
TWScriptManager::saveDisabledList()
{
	QDir scriptRoot(Tw::Utils::ResourcesLibrary::getLibraryPath(QStringLiteral("scripts")));
	QStringList disabled;

	QList<QObject*> list = m_Scripts.findChildren<QObject*>();
	foreach (QObject* i, list) {
		Tw::Scripting::Script * s = qobject_cast<Tw::Scripting::Script*>(i);
		if (!s || s->isEnabled())
			continue;
		disabled << scriptRoot.relativeFilePath(s->getFilename());
	}
	list = m_Hooks.findChildren<QObject*>();
	foreach (QObject* i, list) {
		Tw::Scripting::Script * s = qobject_cast<Tw::Scripting::Script*>(i);
		if (!s || s->isEnabled())
			continue;
		disabled << scriptRoot.relativeFilePath(s->getFilename());
	}

	Tw::Settings settings;
	settings.setValue(QString::fromLatin1("disabledScripts"), disabled);
}

void TWScriptManager::loadPlugins()
{
	// the JSScript interface isn't really a plugin, but provides the same interface
#if WITH_QTSCRIPT
	scriptLanguages += new Tw::Scripting::JSScriptInterface();
#endif
	scriptLanguages += new Tw::Scripting::ECMAScriptInterface();

	// get any static plugins
	foreach (QObject *plugin, QPluginLoader::staticInstances()) {
		if (qobject_cast<Tw::Scripting::ScriptLanguageInterface*>(plugin))
			scriptLanguages += plugin;
	}

#ifdef TW_PLUGINPATH
	// allow a hard-coded path for distro packagers
	QDir pluginsDir = QDir(QString::fromLatin1(TW_PLUGINPATH));
#else
	// find the plugins directory, relative to the executable
	QDir pluginsDir = QDir(qApp->applicationDirPath());
#if defined(Q_OS_WIN)
	if (pluginsDir.dirName().toLower() == QLatin1String("debug") || pluginsDir.dirName().toLower() == QLatin1String("release"))
		pluginsDir.cdUp();
#elif defined(Q_OS_DARWIN) // "plugins" directory is alongside "MacOS" within the package's Contents dir
	if (pluginsDir.dirName() == QLatin1String("MacOS"))
		pluginsDir.cdUp();
	if (!pluginsDir.exists(QString::fromLatin1("plugins"))) { // if not found, try for a dir alongside the .app package
		pluginsDir.cdUp();
		pluginsDir.cdUp();
	}
#endif
	pluginsDir.cd(QString::fromLatin1("plugins"));
#endif

	// allow an env var to override the default plugin path
#if defined(MIKTEX_WINDOWS)
        QString pluginPath = QString::fromUtf8(getenv("TW_PLUGINPATH"));
#else
	QString pluginPath = QString::fromLocal8Bit(getenv("TW_PLUGINPATH"));
#endif
	if (!pluginPath.isEmpty())
		pluginsDir.cd(pluginPath);

	foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
		QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
		// (At least) Python 2.6 requires the symbols in the secondary libraries
		// to be put in the global scope if modules are imported that load
		// additional shared libraries (e.g. datetime)
		loader.setLoadHints(QLibrary::ExportExternalSymbolsHint);
		QObject *plugin = loader.instance();
		if (qobject_cast<Tw::Scripting::ScriptLanguageInterface*>(plugin))
			scriptLanguages += plugin;
	}
}

void TWScriptManager::reloadScripts(bool forceAll /* = false */)
{
	Tw::Settings settings;
	QStringList disabled = settings.value(QString::fromLatin1("disabledScripts"), QStringList()).toStringList();
	QStringList processed;

	// canonicalize the paths
	QDir scriptsDir(Tw::Utils::ResourcesLibrary::getLibraryPath(QStringLiteral("scripts")));
	for (int i = 0; i < disabled.size(); ++i)
		disabled[i] = QFileInfo(scriptsDir.absoluteFilePath(disabled[i])).canonicalFilePath();

	if (forceAll)
		clear();

	reloadScriptsInList(&m_Scripts, processed);
	reloadScriptsInList(&m_Hooks, processed);

	addScriptsInDirectory(scriptsDir, disabled, processed);

	ScriptManagerWidget::refreshScriptList();
}

void TWScriptManager::reloadScriptsInList(TWScriptList * list, QStringList & processed)
{
	Tw::Settings settings;
	bool enableScriptsPlugins = settings.value(QString::fromLatin1("enableScriptingPlugins"), false).toBool();

	foreach(QObject * item, list->children()) {
		if (qobject_cast<TWScriptList*>(item))
			reloadScriptsInList(qobject_cast<TWScriptList*>(item), processed);
		else if (qobject_cast<Tw::Scripting::Script*>(item)) {
			Tw::Scripting::Script * s = qobject_cast<Tw::Scripting::Script*>(item);
			if (s->hasChanged()) {
				// File has been removed
				if (!(QFileInfo(s->getFilename()).exists())) {
					delete s;
					continue;
				}
				// File has been changed - reparse; if an error occurs or the
				// script type has changed treat it as if has been removed (and
				// possibly re-add it later)
				Tw::Scripting::Script::ScriptType oldType = s->getType();
				if (!s->parseHeader() || s->getType() != oldType) {
					delete s;
					continue;
				}
			}
			const bool needsPlugin = (
#if WITH_QTSCRIPT
				qobject_cast<const Tw::Scripting::JSScriptInterface*>(s->getScriptLanguagePlugin()) == nullptr &&
#endif
				qobject_cast<const Tw::Scripting::ECMAScriptInterface*>(s->getScriptLanguagePlugin()) == nullptr
			);
			if (needsPlugin && !enableScriptsPlugins) {
				// the plugin necessary to execute this scripts has been disabled
				delete s;
				continue;
			}
			processed << s->getFilename();
		}
		else {
		} // should never happen
	}
}


void TWScriptManager::clear()
{
	foreach (QObject *s, m_Scripts.children())
		delete s;

	foreach (QObject *s, m_Hooks.children())
		delete s;
}

bool TWScriptManager::addScript(QObject* scriptList, Tw::Scripting::Script * script)
{
	/// \TODO This no longer works since we introduced multiple levels of scripts
/*
	foreach (QObject* obj, scriptList->children()) {
		Tw::Scripting::Script *s = qobject_cast<Tw::Scripting::Script*>(obj);
		if (!s)
			continue;
		if (*s == *script)
			return false;
	}
*/
	script->setParent(scriptList);
	return true;
}

static bool scriptListLessThan(const TWScriptList* l1, const TWScriptList* l2)
{
	return l1->getName().toLower() < l2->getName().toLower();
}

static bool scriptLessThan(const Tw::Scripting::Script* s1, const Tw::Scripting::Script* s2)
{
	return s1->getTitle().toLower() < s2->getTitle().toLower();
}


void TWScriptManager::addScriptsInDirectory(TWScriptList *scriptList,
											TWScriptList *hookList,
											const QDir& dir,
											const QStringList& disabled,
											const QStringList& ignore)
{
#if defined(MIKTEX)
        std::unique_ptr<MiKTeX::Trace::StopWatch> stopWatch =
          MiKTeX::Trace::StopWatch::Start(MiKTeX::TeXworks::Wrapper::GetInstance()->GetTraceStream(), "texworks", dir.absolutePath().toUtf8().data());
#endif
	Tw::Settings settings;
	QFileInfo info;
	bool scriptingPluginsEnabled = settings.value(QString::fromLatin1("enableScriptingPlugins"), false).toBool();

	foreach (const QFileInfo& constInfo,
			 dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Readable, QDir::DirsLast)) {
		// Get a non-const copy in case we need to resolve symlinks later on
		info = constInfo;
		// Should not happen, unless we're dealing with an invalid symlink
		if (!info.exists())
			continue;

		if (info.isDir()) {
			// Only create a new sublist if a matching one doesn't already exist
			TWScriptList *subScriptList = nullptr;
			// Note: Using children() returns a const list; findChildren does not
			foreach (TWScriptList * l, scriptList->findChildren<TWScriptList*>()) {
				if (l->getName() == info.fileName()) {
					subScriptList = l;
					break;
				}
			}
			if(!subScriptList) subScriptList = new TWScriptList(scriptList, info.fileName());

			// Only create a new sublist if a matching one doesn't already exist
			TWScriptList *subHookList = nullptr;
			// Note: Using children() returns a const list; findChildren does not
			foreach (TWScriptList * l, hookList->findChildren<TWScriptList*>()) {
				if (l->getName() == info.fileName()) {
					subHookList = l;
					break;
				}
			}
			if (!subHookList)
				subHookList = new TWScriptList(hookList, info.fileName());

			addScriptsInDirectory(subScriptList, subHookList, info.absoluteFilePath(), disabled, ignore);
			if (subScriptList->children().isEmpty())
				delete subScriptList;
			if (subHookList->children().isEmpty())
				delete subHookList;
			continue;
		}

		// not a directory

		// resolve symlinks
		while (info.isSymLink())
			info = QFileInfo(info.symLinkTarget());
		// sanity check (should be caught already at the start of the loop)
		if (!info.exists())
			continue;

		if (ignore.contains(info.absoluteFilePath()))
			continue;

		foreach (QObject * plugin, scriptLanguages) {
			Tw::Scripting::ScriptLanguageInterface * i = qobject_cast<Tw::Scripting::ScriptLanguageInterface*>(plugin);
			if (!i)
				continue;
			const bool isPlugin = (
#if WITH_QTSCRIPT
				qobject_cast<Tw::Scripting::JSScriptInterface*>(plugin) == nullptr &&
#endif
				qobject_cast<Tw::Scripting::ECMAScriptInterface*>(plugin) == nullptr
			);
			if (isPlugin && !scriptingPluginsEnabled)
				continue;
			if (!i->canHandleFile(info))
				continue;
			Tw::Scripting::Script *script = i->newScript(info.absoluteFilePath());
			if (script) {
				if (disabled.contains(info.canonicalFilePath()))
					script->setEnabled(false);
				script->parseHeader();
				switch (script->getType()) {
					case Tw::Scripting::Script::ScriptHook:
						if (!addScript(hookList, script))
							delete script;
						break;

					case Tw::Scripting::Script::ScriptStandalone:
						if (!addScript(scriptList, script))
							delete script;
						break;

					default: // must be unknown/invalid
						delete script;
						break;
				}
				break;
			}
		}
	}

	// perform custom sorting
	// since QObject::children() is const, we have to work around that limitation
	// by unsetting all parents first, sort, and finally reset parents in the
	// correct order

	QList<TWScriptList*> childLists;
	QList<Tw::Scripting::Script*> childScripts;

	// Note: we can't use QObject::findChildren here because it's recursive
	const QObjectList& children = scriptList->children();
	foreach (QObject *obj, children) {
		if (Tw::Scripting::Script *script = qobject_cast<Tw::Scripting::Script*>(obj))
			childScripts.append(script);
		else if (TWScriptList *list = qobject_cast<TWScriptList*>(obj))
			childLists.append(list);
		else { // shouldn't happen
		}
	}

	// unset parents; this effectively removes the objects from
	// scriptList->children()
	foreach (Tw::Scripting::Script* childScript, childScripts)
		childScript->setParent(nullptr);
	foreach (TWScriptList* childList, childLists)
		childList->setParent(nullptr);

	// sort the sublists
	std::sort(childLists.begin(), childLists.end(), scriptListLessThan);
	std::sort(childScripts.begin(), childScripts.end(), scriptLessThan);

	// add the scripts again, one-by-one
	foreach (Tw::Scripting::Script* childScript, childScripts)
		childScript->setParent(scriptList);
	foreach (TWScriptList* childList, childLists)
		childList->setParent(scriptList);
}

QList<Tw::Scripting::Script *> TWScriptManager::getHookScripts(const QString& hook) const
{
	QList<Tw::Scripting::Script*> result;

	foreach (QObject *obj, m_Hooks.findChildren<QObject*>()) {
		Tw::Scripting::Script *script = qobject_cast<Tw::Scripting::Script*>(obj);
		if (!script)
			continue;
		if (!script->isEnabled())
			continue;
		if (script->getHook().compare(hook, Qt::CaseInsensitive) == 0)
			result.append(script);
	}
	return result;
}

bool
TWScriptManager::runScript(QObject* script, QObject * context, QVariant & result, Tw::Scripting::Script::ScriptType scriptType)
{
	Tw::Settings settings;

	Tw::Scripting::Script * s = qobject_cast<Tw::Scripting::Script*>(script);
	if (!s || s->getType() != scriptType)
		return false;

	const bool needsPlugin = (
#if WITH_QTSCRIPT
		qobject_cast<const Tw::Scripting::JSScriptInterface*>(s->getScriptLanguagePlugin()) == nullptr &&
#endif
		qobject_cast<const Tw::Scripting::ECMAScriptInterface*>(s->getScriptLanguagePlugin()) == nullptr
	);
	if (needsPlugin && !settings.value(QString::fromLatin1("enableScriptingPlugins"), false).toBool())
		return false;

	if (!s->isEnabled())
		return false;

	Tw::Scripting::ScriptAPI api(s, qApp, context, result);
#if defined(MIKTEX)
        std::unique_ptr<MiKTeX::Trace::StopWatch> stopWatch =
          MiKTeX::Trace::StopWatch::Start(MiKTeX::TeXworks::Wrapper::GetInstance()->GetTraceStream(), "texworks", s->getFilename().toUtf8().data());
#endif
	return s->run(api);
}

void
TWScriptManager::runHooks(const QString& hookName, QObject * context /* = nullptr */)
{
	foreach (Tw::Scripting::Script *s, getHookScripts(hookName)) {
		runScript(s, context, Tw::Scripting::Script::ScriptHook);
	}
}
