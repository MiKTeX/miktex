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

#include "Settings.h"
#include "TWUtils.h"
#include "TeXHighlighter.h"
#include "TemplateDialog.h"
#include "document/TeXDocument.h"
#include "utils/ResourcesLibrary.h"

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

	QString templatePath = Tw::Utils::ResourcesLibrary::getLibraryPath(QStringLiteral("templates"));
		// do this before creating the model, as getLibraryPath might initialize a new dir

	model = new QFileSystemModel(this);
	model->setRootPath(templatePath);
	treeView->setModel(model);
	treeView->setRootIndex(model->index(templatePath));
	treeView->hideColumn(2);

	connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TemplateDialog::selectionChanged);

	connect(treeView, &QTreeView::activated, this, &TemplateDialog::itemActivated);

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
	QFileSystemModel * model = qobject_cast<QFileSystemModel*>(treeView->model());
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

void TemplateDialog::showEvent(QShowEvent * event)
{
	QDialog::showEvent(event);

	// Resize the first column to take all available screen space. This can only
	// be done once the dialog is fully layouted to be shown - hence here.

	// Only resize columns the first time the dialog is shown. After that, keep
	// using what the user are used to (or have even configured themselves)
	if (!_shouldResizeColumns) {
		return;
	}
	_shouldResizeColumns = false;

	QHeaderView * h = treeView->header();
	if (!h) {
		return;
	}
	// Do not use the real section sizes as reference, as the last section might
	// be expanding and is hence larger than necessary. The default section size
	// seems to work well here.
	h->resizeSection(0, h->length() - 2 * h->defaultSectionSize());
}
