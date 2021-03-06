/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2017-2020  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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
#ifndef __CITATION_SELECT_DIALOG_H
#define __CITATION_SELECT_DIALOG_H

#include "BibTeXFile.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QTableView>

class CitationModel : public QAbstractTableModel {
	Q_OBJECT
public:
	CitationModel(QObject * parent = nullptr);
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

	void setSelectedKeys(const QStringList & keys);
	QStringList selectedKeys() const { return _selectedKeys.values(); }

	const BibTeXFile::Entry * getEntry(const unsigned int idx) const { return (static_cast<int>(idx) < _entries.size() ? _entries[static_cast<int>(idx)] : nullptr); }
	const BibTeXFile::Entry * getEntry(const QString & key) const;

	void addBibTeXFile(const BibTeXFile & file);
protected slots:
	void rebuildEntryCache();
protected:
	QList<BibTeXFile> _bibFiles;
	QVector<const BibTeXFile::Entry *> _entries;
	QSet<QString> _selectedKeys;
};

class CitationProxyModel : public QSortFilterProxyModel
{
public:
	CitationProxyModel(QObject * parent = nullptr) : QSortFilterProxyModel(parent) { }
	bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override { setSortRole(column == 0 ? Qt::CheckStateRole : Qt::DisplayRole); QSortFilterProxyModel::sort(column, order); }
};

class CitationTableView : public QTableView
{
	Q_OBJECT
public:
	CitationTableView(QWidget * parent = nullptr) : QTableView(parent) { }
protected:
	void keyPressEvent(QKeyEvent * event) override;
};

// The ui header must be included after declaring CitationTableView, as the
// latter is used in the .ui file
#include "ui_CitationSelectDialog.h"

class CitationSelectDialog : public QDialog, private Ui::CitationSelectDialog
{
	Q_OBJECT
public:

	explicit CitationSelectDialog(QWidget * parent);
	~CitationSelectDialog() override = default;

	CitationModel & model() { return _model; }

	void setInitialKeys(const QStringList & keys) { _initialKeys = keys; _initialKeys.removeAll(QLatin1String("")); _model.setSelectedKeys(_initialKeys); }

	void addBibTeXFile(const BibTeXFile & file) {
		_model.addBibTeXFile(file);
		_proxyModel.sort(0, Qt::DescendingOrder);
	}
	void addBibTeXFile(const QString & filename) { addBibTeXFile(BibTeXFile(filename)); }

	QStringList getSelectedKeys(const bool ordered = true) const;

public slots:
	void buttonClicked(QAbstractButton * button);
	void resetData();
	int exec() override { tableView->resizeColumnsToContents(); return QDialog::exec(); }


protected:
	CitationProxyModel _proxyModel;
	CitationModel _model;
	QStringList _initialKeys;
};


class KeyForwarder : public QObject
{
	Q_OBJECT
public:
	explicit KeyForwarder(QObject * target, QObject * parent = nullptr);
protected:
	bool eventFilter(QObject * watched, QEvent * event) override;
	QObject * _target;
	QSet<int> _keysToForward;
};






#endif // !defined(__CITATION_SELECT_DIALOG_H)
