/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2017-2018  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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
#include "CitationSelectDialog.h"
#include <QKeyEvent>
#include <QAbstractButton>

KeyForwarder::KeyForwarder(QObject * target, QObject * parent /* = NULL */)
  : QObject(parent), _target(target)
{
	_keysToForward << Qt::Key_Up << Qt::Key_Down << Qt::Key_PageUp << Qt::Key_PageDown << Qt::Key_Space;
}

bool KeyForwarder::eventFilter(QObject * watched, QEvent * event)
{
	if (_target && (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)) {
		QKeyEvent * keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
			if (_keysToForward.contains(keyEvent->key())) {
				keyEvent->setModifiers(keyEvent->modifiers() & ~Qt::ControlModifier);
				QCoreApplication::sendEvent(_target, keyEvent);
				return true;
			}
		}
	}
	return QObject::eventFilter(watched, event);
}

//virtual
void CitationTableView::keyPressEvent(QKeyEvent * event)
{
	if (event->key() == Qt::Key_Space && event->modifiers() == Qt::NoModifier) {
		Q_FOREACH(QModelIndex idx, selectedIndexes()) {
			// Since we can only select entire rows, we will always get all columns
			// for each selected row here. Since only column 0 is checkable, we can
			// simply ignore all the others.
			if (idx.column() != 0) continue;
			model()->setData(idx, (model()->data(idx, Qt::CheckStateRole) == Qt::Unchecked ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
		}
	}
	else
		QTableView::keyPressEvent(event);
}


CitationSelectDialog::CitationSelectDialog(QWidget *parent)
  : QDialog(parent)
{
	setupUi(this);

	_proxyModel.setSourceModel(&_model);
	tableView->setModel(&_proxyModel);

	lineEdit->installEventFilter(new KeyForwarder(tableView));

	connect(lineEdit, SIGNAL(textChanged(QString)), &_proxyModel, SLOT(setFilterFixedString(QString)));
	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
}

void CitationSelectDialog::buttonClicked(QAbstractButton * button)
{
	if (buttonBox->buttonRole(button) == QDialogButtonBox::ResetRole)
		resetData();
}

void CitationSelectDialog::resetData()
{
	_model.setSelectedKeys(_initialKeys);
}

bool BibTeXEntryLessThan(const BibTeXFile::Entry * a, const BibTeXFile::Entry * b)
{
	Q_ASSERT(a != NULL);
	Q_ASSERT(b != NULL);

	// Sort by year first
	if (a->year() != b->year()) {
		if (a->year().isEmpty() && !b->year().isEmpty()) return false;
		if (!a->year().isEmpty() && b->year().isEmpty()) return true;
		return a->year() < b->year();
	}
	// If the years are equal, sort by authors
	if (a->author() != b->author())
		return a->author() < b->author();
//	if (a->title() != b->title())
//		return a->title() < b->title();

	// If all else fails, sort by BibTeX key
	return a->key() < b->key();
}

QStringList CitationSelectDialog::getSelectedKeys(const bool ordered /* = true */) const
{
	QStringList keys = _model.selectedKeys();
	QStringList unknownKeys;

	if (!ordered) return keys;

	QList<const BibTeXFile::Entry*> entries;
	// Convert keys to entry pointers so we can sort them, e.g., by their year
	// All keys that or not managed by the model will be appended unchanged
	Q_FOREACH(QString key, keys) {
		const BibTeXFile::Entry * e = _model.getEntry(key);
		if (e) entries.append(e);
		else unknownKeys.append(key);
	}

	qStableSort(entries.begin(), entries.end(), BibTeXEntryLessThan);

	keys.clear();
	Q_FOREACH(const BibTeXFile::Entry * entry, entries)
		keys.append(entry->key());
	return keys + unknownKeys;
}


CitationModel::CitationModel(QObject * parent /* = NULL */)
  : QAbstractTableModel(parent)
{
	connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rebuildEntryCache()));
	connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(rebuildEntryCache()));
	connect(this, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), this, SLOT(rebuildEntryCache()));
}

//virtual
int CitationModel::rowCount(const QModelIndex &parent /* = QModelIndex() */) const
{
	if (parent.isValid()) return 0;
	return _entries.size();
}

//virtual
int CitationModel::columnCount(const QModelIndex &parent /* = QModelIndex() */) const
{
	if (parent.isValid()) return 0;
	return 6;
}

//virtual
QVariant CitationModel::data(const QModelIndex &index, int role /* = Qt::DisplayRole */) const
{
	if (!index.isValid()) return QVariant();
	const BibTeXFile::Entry * e = static_cast<const BibTeXFile::Entry*>(index.internalPointer());

	if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {

		if (index.column() == 0 && role == Qt::ToolTipRole)
			return e->key();
		else if (index.column() == 1)
			return e->typeString();
		else if (index.column() == 2)
			return e->author();
		else if (index.column() == 3)
			return e->title();
		else if (index.column() == 4)
			return e->year();
		else if (index.column() == 5)
			return e->howPublished();
		else
			return QVariant();
	}
	if (role == Qt::CheckStateRole) {
		if (index.column() != 0) return QVariant();
		return (_selectedKeys.contains(e->key()) ? Qt::Checked : Qt::Unchecked);
	}
	if (role == Qt::SizeHintRole) {
		if (index.column() == 0) return QSize(20, 20); // checkbox
		else if (index.column() == 1) return QSize(75, 20); // type
		else if (index.column() == 2) return QSize(150, 20); // author
		else if (index.column() == 3) return QSize(150, 20); // title
		else if (index.column() == 4) return QSize(40, 20); // year
		else if (index.column() == 5) return QSize(150, 20); // journal
		else return QVariant();
	}
	return QVariant();
}

//virtual
QVariant CitationModel::headerData(int section, Qt::Orientation orientation, int role /* = Qt::DisplayRole */) const
{
	if (orientation == Qt::Vertical)
		return QVariant();

	if (role == Qt::DisplayRole) {
		if (section == 1) return CitationSelectDialog::trUtf8("Type");
		else if (section == 2) return CitationSelectDialog::trUtf8("Author");
		else if (section == 3) return CitationSelectDialog::trUtf8("Title");
		else if (section == 4) return CitationSelectDialog::trUtf8("Year");
		else if (section == 5) return CitationSelectDialog::trUtf8("Journal");
		else return QVariant();
	}
	return QVariant();
}


//virtual
QModelIndex CitationModel::index(int row, int column, const QModelIndex &parent /* = QModelIndex() */) const
{
	if (parent.isValid()) return QModelIndex();
	return createIndex(row, column, const_cast<BibTeXFile::Entry*>(_entries[row]));
}

void CitationModel::setSelectedKeys(const QStringList & keys)
{
	QModelIndex tl, br;

	for (unsigned int iRow = 0; iRow < rowCount(); ++iRow) {
		const BibTeXFile::Entry * e = _entries[iRow];
		if (_selectedKeys.contains(e->key()) != keys.contains(e->key())) {
			br = index(iRow, 0);
			if (!tl.isValid()) tl = br;
		}
	}
	_selectedKeys = keys.toSet();

	emit dataChanged(tl, br);
}

const BibTeXFile::Entry * CitationModel::getEntry(const QString & key) const
{
	Q_FOREACH(const BibTeXFile::Entry * entry, _entries) {
		if (key == entry->key()) return entry;
	}
	return NULL;
}


//virtual
Qt::ItemFlags CitationModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags retVal = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	if (index.column() == 0) retVal |= Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
	return retVal;
}

//virtual
bool CitationModel::setData(const QModelIndex &index, const QVariant &value, int role /* = Qt::EditRole */)
{
	if (!index.isValid()) return false;

	const BibTeXFile::Entry * e = static_cast<const BibTeXFile::Entry*>(index.internalPointer());

	if (value == Qt::Checked) {
		_selectedKeys.insert(e->key());
		emit dataChanged(index, index);
		return true;
	}
	if (value == Qt::Unchecked) {
		_selectedKeys.remove(e->key());
		emit dataChanged(index, index);
		return true;
	}
	return false;
}


void CitationModel::addBibTeXFile(const BibTeXFile & file)
{
	int n = rowCount();
	beginInsertRows(QModelIndex(), n, n + file.numEntries() - 1);
	_bibFiles.append(file);
	endInsertRows();
}

void CitationModel::rebuildEntryCache()
{
	_entries.clear();
	unsigned int i = 0, n = 0;

	// resize the vector first to avoid reallocations later on
	for (unsigned int iBibFile = 0; iBibFile < _bibFiles.size(); ++iBibFile)
		n += _bibFiles[iBibFile].numEntries();
	_entries.resize(n);

	for (unsigned int iBibFile = 0; iBibFile < _bibFiles.size(); ++iBibFile) {
		for (unsigned int iEntry = 0; iEntry < _bibFiles[iBibFile].numEntries(); ++iEntry, ++i) {
			_entries[i] = &(_bibFiles[iBibFile].entry(iEntry));
		}
	}
}

bool CitationProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
	static QLatin1String space(" ");
	const BibTeXFile::Entry * e = static_cast<const BibTeXFile::Entry*>(sourceModel()->index(source_row, 1).internalPointer());
	QString haystack = e->key() + space + e->typeString() + space + e->author() + space + e->title() + space + e->year() + space + e->howPublished();
	QStringList needles = filterRegExp().pattern().split(QChar::fromLatin1(' '), QString::SkipEmptyParts);

	haystack = haystack.toLower();

	Q_FOREACH(QString needle, needles) {
		if (!haystack.contains(needle.toLower())) return false;
	}
	return true;
}
