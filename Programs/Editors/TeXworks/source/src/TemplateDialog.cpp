/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2008-2019  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "TemplateDialog.h"
#include "TeXHighlighter.h"
#include "TWUtils.h"
#include "Settings.h"
#include "document/TeXDocument.h"

#include <QDirModel>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

TemplateDialog::TemplateDialog()
	: QDialog(nullptr)
{
	init();
}

void TemplateDialog::init()
{
	setupUi(this);
	Tw::Document::TeXDocument * texDoc = new Tw::Document::TeXDocument(textEdit);
	textEdit->setDocument(texDoc);

	QString templatePath = TWUtils::getLibraryPath(QString::fromLatin1("templates"));
		// do this before creating the model, as getLibraryPath might initialize a new dir
		
	model = new QDirModel(this);
	treeView->setModel(model);
	treeView->setRootIndex(model->index(templatePath));
	treeView->expandAll();
	treeView->resizeColumnToContents(0);
	treeView->hideColumn(2);
	treeView->collapseAll();
	
	connect(treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
			this, SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
	
	connect(treeView, SIGNAL(activated(const QModelIndex&)), this, SLOT(itemActivated(const QModelIndex&)));

	Tw::Settings settings;
	if (settings.value(QString::fromLatin1("syntaxColoring"), true).toBool()) {
		TeXHighlighter * highlighter = new TeXHighlighter(texDoc);
		// For now, we use "LaTeX" highlighting for all files (which is probably
		// reasonable in most/typical cases)
		int idx = TeXHighlighter::syntaxOptions().indexOf(QStringLiteral("LaTeX"));
		if (idx >= 0)
			highlighter->setActiveIndex(idx);
	}
}

void TemplateDialog::itemActivated(const QModelIndex & index)
{
	QDirModel * model = qobject_cast<QDirModel*>(treeView->model());
	if (model && !model->isDir(index))
		accept();
}

void TemplateDialog::selectionChanged(const QItemSelection &selected, const QItemSelection &/*deselected*/)
{
	textEdit->clear();
	if (selected.indexes().count() > 0) {
		QString filePath(model->filePath(selected.indexes()[0]));
		QFileInfo fileInfo(filePath);
		if (fileInfo.isFile() && fileInfo.isReadable()) {
			QFile file(filePath);
			if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
				QTextStream in(&file);
				textEdit->setPlainText(in.readAll());
			}
		}
	}
}

QString TemplateDialog::doTemplateDialog()
{
	QString rval;

	TemplateDialog dlg;
	dlg.show();
	DialogCode result = static_cast<DialogCode>(dlg.exec());

	if (result == Accepted) {
		QModelIndexList selection = dlg.treeView->selectionModel()->selectedRows();
		if (selection.count() > 0) {
			QString filePath(dlg.model->filePath(selection[0]));
			QFileInfo fileInfo(filePath);
			if (fileInfo.isFile() && fileInfo.isReadable())
				rval = filePath;
		}
	}
	
	return rval;
}
