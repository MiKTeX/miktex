/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2010-2015  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "ScriptManager.h"
#include "TWApp.h"
#include "TWScript.h"

#include <QLabel>
#include <QCloseEvent>
#include <QHeaderView>
#include <QDesktopServices>
#include <QUrl>

ScriptManager * ScriptManager::gManageScriptsWindow = NULL;
QRect           ScriptManager::gGeometry;

void ScriptManager::init()
{
	setupUi(this);
	
	hookTree->header()->hide();
	standaloneTree->header()->hide();
	
	populateTree();
	
	connect(scriptTabs, SIGNAL(currentChanged(int)), this, SLOT(treeSelectionChanged()));

	connect(hookTree, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(treeItemClicked(QTreeWidgetItem *, int)));
	connect(hookTree, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(treeItemActivated(QTreeWidgetItem *, int)));
	connect(hookTree, SIGNAL(itemSelectionChanged()), this, SLOT(treeSelectionChanged()));
	connect(standaloneTree, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(treeItemClicked(QTreeWidgetItem *, int)));
	connect(standaloneTree, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(treeItemActivated(QTreeWidgetItem *, int)));
	connect(standaloneTree, SIGNAL(itemSelectionChanged()), this, SLOT(treeSelectionChanged()));

	connect(this, SIGNAL(scriptListChanged()), qApp, SIGNAL(scriptListChanged()));
}

void ScriptManager::closeEvent(QCloseEvent * event)
{
	gGeometry = geometry();
	hide();
	event->accept();
}

/*static*/
void ScriptManager::showManageScripts()
{
	if (!gManageScriptsWindow)
		gManageScriptsWindow = new ScriptManager(NULL);

	if (!gGeometry.isNull())
		gManageScriptsWindow->setGeometry(gGeometry);
	
	gManageScriptsWindow->show();
	gManageScriptsWindow->raise();
	gManageScriptsWindow->activateWindow();
}

/*static*/
void ScriptManager::refreshScriptList()
{
	if (gManageScriptsWindow)
		gManageScriptsWindow->populateTree();
}

void ScriptManager::populateTree()
{
	TWScriptManager * scriptManager = TWApp::instance()->getScriptManager();
	TWScriptList * scripts = scriptManager->getScripts();
	TWScriptList * hooks = scriptManager->getHookScripts();
	
	hookTree->clear();
	standaloneTree->clear();
	
	populateTree(hookTree, NULL, hooks);
	populateTree(standaloneTree, NULL, scripts);
	
	hookTree->expandAll();
	standaloneTree->expandAll();
}

#define kScriptType (QTreeWidgetItem::UserType + 1)
#define kFolderType (QTreeWidgetItem::UserType + 2)

void ScriptManager::populateTree(QTreeWidget * tree, QTreeWidgetItem * parentItem, const TWScriptList * scripts)
{
	QTreeWidgetItem * item;

	foreach (QObject * obj, scripts->children()) {
		TWScript * script = qobject_cast<TWScript*>(obj);
		if (script && script->getType() != TWScript::ScriptUnknown) {
			QStringList strList(script->getTitle());
			item = parentItem ? new QTreeWidgetItem(parentItem, strList, kScriptType) : new QTreeWidgetItem(tree, strList, kScriptType);
			item->setData(0, Qt::UserRole, qVariantFromValue((void*)script));
			item->setCheckState(0, script->isEnabled() ? Qt::Checked : Qt::Unchecked);
			continue;
		}
		TWScriptList * list = qobject_cast<TWScriptList*>(obj);
		if (list) {
			QStringList strList(list->getName());
			item = parentItem ? new QTreeWidgetItem(parentItem, strList, kFolderType) : new QTreeWidgetItem(tree, strList, kFolderType);
			QFont f = item->font(0);
			f.setBold(true);
			item->setFont(0, f);
			populateTree(NULL, item, list);
			setFolderCheckedState(item);
		}
	}
}

void ScriptManager::treeItemClicked(QTreeWidgetItem * item, int /*column*/)
{
	if (item->type() == kScriptType) {
		TWScript * s = static_cast<TWScript*>(item->data(0, Qt::UserRole).value<void*>());
		if (s) {
			s->setEnabled(item->checkState(0) == Qt::Checked);
			setFolderCheckedState(item->parent());
			emit scriptListChanged();
		}
	}
	else if (item->type() == kFolderType) {
		Qt::CheckState checked = item->checkState(0);
		for (int i = 0; i < item->childCount(); ++i) {
			item->child(i)->setCheckState(0, checked);
			treeItemClicked(item->child(i), 0);
		}
	}
}

void ScriptManager::setFolderCheckedState(QTreeWidgetItem * item)
{
	if (!item || item->type() != kFolderType)
		return;
	bool anyChecked = false;
	bool allChecked = true;
	for (int i = 0; i < item->childCount(); ++i) {
		if (item->child(i)->checkState(0) != Qt::Checked)
			allChecked = false;
		if (item->child(i)->checkState(0) != Qt::Unchecked)
			anyChecked = true;
	}
	item->setCheckState(0, allChecked ? Qt::Checked : anyChecked ? Qt::PartiallyChecked : Qt::Unchecked);
	setFolderCheckedState(item->parent());
}

void ScriptManager::treeItemActivated(QTreeWidgetItem * item, int /*column*/)
{
	if (item->type() == kScriptType) {
		TWScript * s = static_cast<TWScript*>(item->data(0, Qt::UserRole).value<void*>());
		if (s)
			QDesktopServices::openUrl(QUrl::fromLocalFile(s->getFilename()));
	}
}

void ScriptManager::treeSelectionChanged()
{
	details->setPlainText("");

	QTreeWidget * tree = scriptTabs->currentWidget() == standaloneTab ? standaloneTree : hookTree;
	QList<QTreeWidgetItem*> selection = tree->selectedItems();
	if (selection.size() != 1)
		return;
	
	if (selection[0]->type() != kScriptType)
		return;
	
	TWScript * s = static_cast<TWScript*>(selection[0]->data(0, Qt::UserRole).value<void*>());
	if (!s)
		return;

	QString rows;
	addDetailsRow(rows, tr("Name: "), s->getTitle());
	addDetailsRow(rows, tr("Context: "), s->getContext());
	addDetailsRow(rows, tr("Description: "), s->getDescription());
	addDetailsRow(rows, tr("Author: "), s->getAuthor());
	addDetailsRow(rows, tr("Version: "), s->getVersion());
	addDetailsRow(rows, tr("Shortcut: "), s->getKeySequence().toString());
	addDetailsRow(rows, tr("File: "), QFileInfo(s->getFilename()).fileName());
	
	const TWScriptLanguageInterface * sli = qobject_cast<TWScriptLanguageInterface*>(s->getScriptLanguagePlugin());
	if(sli) {
		QString url = sli->scriptLanguageURL();
		QString str = sli->scriptLanguageName();
		if (!url.isEmpty())
			str = "<a href=\"" + url + "\">" + str + "</a>";
		addDetailsRow(rows, tr("Language: "), str);
	}

	if (s->getType() == TWScript::ScriptHook)
		addDetailsRow(rows, tr("Hook: "), s->getHook());

	details->setHtml("<table>" + rows + "</table>");
}

void ScriptManager::addDetailsRow(QString& html, const QString label, const QString value)
{
	if (!value.isEmpty())
		html += "<tr><td>" + label + "</td><td>" + value + "</td></tr>";
}

