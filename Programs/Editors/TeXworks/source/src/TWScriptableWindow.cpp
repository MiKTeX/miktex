/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2009-2023  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#if defined(MIKTEX_WINDOWS)
#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>
#endif
#include "TWScriptableWindow.h"

#include "ScriptManagerWidget.h"
#include "Settings.h"
#include "TWApp.h"
#include "TWScriptManager.h"
#include "scripting/ECMAScriptInterface.h"
#if WITH_QTSCRIPT
#	include "scripting/JSScriptInterface.h"
#endif
#include "scripting/ScriptLanguageInterface.h"
#include "scripting/ScriptObject.h"
#include "utils/WindowManager.h"

#include <QAction>
#include <QDockWidget>
#include <QFile>
#include <QMenu>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>

void
TWScriptableWindow::initScriptable(QMenu* theScriptsMenu,
							 QAction* aboutScriptsAction,
							 QAction* manageScriptsAction,
							 QAction* updateScriptsAction,
							 QAction* showScriptsFolderAction)
{
	scriptsMenu = theScriptsMenu;
	connect(aboutScriptsAction, &QAction::triggered, this, &TWScriptableWindow::doAboutScripts);
	connect(manageScriptsAction, &QAction::triggered, this, &TWScriptableWindow::doManageScripts);
	connect(updateScriptsAction, &QAction::triggered, TWApp::instance(), &TWApp::updateScriptsList);
	connect(showScriptsFolderAction, &QAction::triggered, TWApp::instance(), &TWApp::showScriptsFolder);
	staticScriptMenuItemCount = scriptsMenu->actions().count();

	connect(TWApp::instance(), &TWApp::scriptListChanged, this, &TWScriptableWindow::updateScriptsMenu);

	updateScriptsMenu();
}

void
TWScriptableWindow::updateScriptsMenu()
{
	TWScriptManager * scriptManager = TWApp::instance()->getScriptManager();

	removeScriptsFromMenu(scriptsMenu, staticScriptMenuItemCount);
	addScriptsToMenu(scriptsMenu, scriptManager->getScripts());
}

void
TWScriptableWindow::removeScriptsFromMenu(QMenu *menu, QList<QAction*>::size_type startIndex /* = 0 */)
{
	if (!menu)
		return;

	QList<QAction*> actions = menu->actions();
	for (auto i = startIndex; i < actions.count(); ++i) {
		// if this is a popup menu, make sure all its children are destroyed
		// first, or else old QActions may still be floating around somewhere
		if (actions[i]->menu())
			removeScriptsFromMenu(actions[i]->menu());
		scriptsMenu->removeAction(actions[i]);
		actions[i]->deleteLater();
	}
}

int
TWScriptableWindow::addScriptsToMenu(QMenu *menu, TWScriptList *scripts)
{
	int count = 0;
	foreach (QObject *obj, scripts->children()) {
		Tw::Scripting::ScriptObject *scriptObject = qobject_cast<Tw::Scripting::ScriptObject*>(obj);
		if (scriptObject) {
			if (!scriptObject->isEnabled())
				continue;
			if (scriptObject->getContext().isEmpty() || scriptObject->getContext() == scriptContext()) {
				QAction *a = menu->addAction(scriptObject->getTitle());
				connect(scriptObject, &Tw::Scripting::ScriptObject::destroyed, this, [this,a]() { scriptsMenu->removeAction(a); });
				if (!scriptObject->getKeySequence().isEmpty())
					a->setShortcut(scriptObject->getKeySequence());
//				a->setEnabled(script->isEnabled());
				// give the action an object name so it could possibly included in the
				// customization process of keyboard shortcuts in the future
				a->setObjectName(QString::fromLatin1("Script: %1").arg(scriptObject->getTitle()));
				a->setStatusTip(scriptObject->getDescription());
				connect(a, &QAction::triggered, this, [this, scriptObject](){ runScript(scriptObject); });
				++count;
			}
			continue;
		}
		TWScriptList *list = qobject_cast<TWScriptList*>(obj);
		if (list) {
			QMenu *m = menu->addMenu(list->getName());
			if (addScriptsToMenu(m, list) == 0)
				menu->removeAction(m->menuAction());
		}
	}
	return count;
}

void
TWScriptableWindow::runScript(QObject* scriptObj, Tw::Scripting::Script::ScriptType scriptType)
{
	QVariant result;

	TWScriptManager * sm = TWApp::instance()->getScriptManager();
	if (!sm)
		return;

	Tw::Scripting::ScriptObject * so = qobject_cast<Tw::Scripting::ScriptObject*>(scriptObj);
	if (!so) {
		return;
	}
	Tw::Scripting::Script * s = so->getScript();
	if (!s || s->getType() != scriptType)
		return;

	bool success = sm->runScript(scriptObj, this, result, scriptType);

	if (success) {
		if (!result.isNull() && !result.toString().isEmpty()) {
			if (scriptType == Tw::Scripting::Script::ScriptHook)
				statusBar()->showMessage(tr("Script \"%1\": %2").arg(s->getTitle(), result.toString()), kStatusMessageDuration);
			else
				QMessageBox::information(this, tr("Script result"), result.toString(), QMessageBox::Ok, QMessageBox::Ok);
		}
	}
	else {
		if (result.isNull())
			result = tr("unknown error");
		QMessageBox::information(this, tr("Script error"), tr("Script \"%1\": %2").arg(s->getTitle(), result.toString()), QMessageBox::Ok, QMessageBox::Ok);
	}
}

void
TWScriptableWindow::runHooks(const QString& hookName)
{
	foreach (Tw::Scripting::ScriptObject *s, TWApp::instance()->getScriptManager()->getHookScripts(hookName)) {
		// Don't use TWScriptManager::runHooks here to get status bar messages
		runScript(s, Tw::Scripting::Script::ScriptHook);
	}
}

void
TWScriptableWindow::doAboutScripts()
{
	Tw::Settings settings;
	bool enableScriptsPlugins = settings.value(QString::fromLatin1("enableScriptingPlugins"), false).toBool();

	QString scriptingLink = QString::fromLatin1("<a href=\"%1\">%1</a>").arg(QString::fromLatin1("https://github.com/TeXworks/texworks/wiki/ScriptingTeXworks"));
	QString aboutText = QLatin1String("<p>");
	aboutText += tr("Scripts may be used to add new commands to %1, "
					"and to extend or modify its behavior.").arg(QCoreApplication::applicationName());
	aboutText += QLatin1String("</p><p><small>");
	aboutText += tr("For more information on creating and using scripts, see %1</p>").arg(scriptingLink);
	aboutText += QLatin1String("</small></p><p>");
	aboutText += tr("Scripting languages currently available in this copy of %1:").arg(QCoreApplication::applicationName());
	aboutText += QLatin1String("</p><ul>");
	foreach (const QObject * plugin,
			 TWApp::instance()->getScriptManager()->languages()) {
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
		const Tw::Scripting::ScriptLanguageInterface * i = qobject_cast<Tw::Scripting::ScriptLanguageInterface*>(plugin);
#else
		const Tw::Scripting::ScriptLanguageInterface * i = qobject_cast<const Tw::Scripting::ScriptLanguageInterface*>(plugin);
#endif
		if(!i) continue;
		const bool isPlugin = (
#if WITH_QTSCRIPT
			qobject_cast<const Tw::Scripting::JSScriptInterface*>(plugin) == nullptr &&
#endif
			qobject_cast<const Tw::Scripting::ECMAScriptInterface*>(plugin) == nullptr
		);
		aboutText += QString::fromLatin1("<li><a href=\"%1\">%2</a>").arg(i->scriptLanguageURL(), i->scriptLanguageName());
		if (isPlugin && !enableScriptsPlugins) {
			//: This string is appended to a script language name to indicate it is currently disabled
			aboutText += QChar::fromLatin1(' ') + tr("(disabled in the preferences)");
		}
		aboutText += QLatin1String("</li>");
	}
	QMessageBox::about(nullptr, tr("About Scripts"), aboutText);
}

void
TWScriptableWindow::doManageScripts()
{
	ScriptManagerWidget::showManageScripts();
}

void TWScriptableWindow::hideFloatersUnlessThis(QWidget* currWindow)
{
	TWScriptableWindow* p = qobject_cast<TWScriptableWindow*>(currWindow);
	if (p == this)
		return;
	foreach (QObject* child, children()) {
		QToolBar* tb = qobject_cast<QToolBar*>(child);
		if (tb && tb->isVisible() && tb->isFloating()) {
			latentVisibleWidgets.append(tb);
			tb->hide();
			continue;
		}
		QDockWidget* dw = qobject_cast<QDockWidget*>(child);
		if (dw && dw->isVisible() && dw->isFloating()) {
			latentVisibleWidgets.append(dw);
			dw->hide();
			continue;
		}
	}
}

void TWScriptableWindow::showFloaters()
{
	foreach (QWidget* w, latentVisibleWidgets)
	w->show();
	latentVisibleWidgets.clear();
}

void TWScriptableWindow::placeOnLeft()
{
	Tw::Utils::WindowManager::zoomToHalfScreen(this, false);
}

void TWScriptableWindow::placeOnRight()
{
	Tw::Utils::WindowManager::zoomToHalfScreen(this, true);
}

void TWScriptableWindow::selectWindow(bool activate)
{
	show();
	raise();
	if (activate) {
		activateWindow();
		emit TWApp::instance()->focusObjectChanged(this);
	}
	if (isMinimized())
		showNormal();
}
