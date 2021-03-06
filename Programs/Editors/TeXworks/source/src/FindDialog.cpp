/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2020  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "FindDialog.h"
#include "PDFDocumentWindow.h"
#include "Settings.h"
#include "TWApp.h"
#include "TeXDocumentWindow.h"

#include <QFileInfo>
#include <QHeaderView>
#include <QKeyEvent>
#include <QPushButton>
#include <QShortcut>
#include <QTableWidget>
#include <QTextBlock>
#include <QTextBoundaryFinder>

const int kMaxRecentStrings = 10;

FindDialog::FindDialog(QTextEdit *parent)
	: QDialog(parent)
{
	init(parent);
}

bool RecentStringsKeyFilter::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *e = dynamic_cast<QKeyEvent*>(event);
		Qt::KeyboardModifiers mods = e->modifiers();
		if ((mods & Qt::ControlModifier) != Qt::NoModifier) {
			if (e->key() == Qt::Key_Up) {
				setRecentString(obj, -1);
				return true;
			}
			if (e->key() == Qt::Key_Down) {
				setRecentString(obj, 1);
				return true;
			}
		}
	}

	// standard event processing
	return QObject::eventFilter(obj, event);
}

void RecentStringsKeyFilter::setRecentString(QObject *obj, int dir)
{
	QLineEdit *lineEdit = qobject_cast<QLineEdit*>(obj);
	if (!lineEdit)
		return;

	if (strings.empty())
		return;

	int index = strings.indexOf(lineEdit->text());
	if (index == -1)
		index = (dir == 1) ? 0 : strings.size() - 1;
	else {
		index += dir;
		if (index < 0)
			index = strings.size() - 1;
		else if (index >= strings.size())
			index = 0;
	}

	lineEdit->setText(strings[index]);
	lineEdit->selectAll();
}

void FindDialog::init(QTextEdit *document)
{
	setupUi(this);

	buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Find"));

	connect(checkBox_allFiles, &QCheckBox::toggled, this, &FindDialog::toggledAllFilesOption);
	connect(checkBox_findAll, &QCheckBox::toggled, this, &FindDialog::toggledFindAllOption);
	connect(checkBox_regex, &QCheckBox::toggled, this, &FindDialog::toggledRegexOption);
	connect(checkBox_selection, &QCheckBox::toggled, this, &FindDialog::toggledSelectionOption);
	connect(searchText, &QLineEdit::textChanged, this, &FindDialog::checkRegex);

	Tw::Settings settings;
	QString	str = settings.value(QString::fromLatin1("searchText")).toString();
	searchText->setText(str);
	searchText->selectAll();

	bool regexOption = settings.value(QString::fromLatin1("searchRegex")).toBool();
	checkBox_regex->setChecked(regexOption);
	checkBox_words->setEnabled(!regexOption);

	bool findAll = settings.value(QString::fromLatin1("searchFindAll")).toBool();
	checkBox_findAll->setChecked(findAll);

	bool allFiles = settings.value(QString::fromLatin1("searchAllFiles")).toBool();
	checkBox_allFiles->setEnabled(TeXDocumentWindow::documentList().count() > 1);
	checkBox_allFiles->setChecked(allFiles && checkBox_allFiles->isEnabled());

	bool selectionOption = settings.value(QString::fromLatin1("searchSelection")).toBool();
	checkBox_selection->setEnabled(document->textCursor().hasSelection() && !findAll);
	checkBox_selection->setChecked(selectionOption && checkBox_selection->isEnabled());

	bool wrapOption = settings.value(QString::fromLatin1("searchWrap")).toBool();
	checkBox_wrap->setEnabled(!(checkBox_selection->isEnabled() && checkBox_selection->isChecked()) && !findAll);
	checkBox_wrap->setChecked(wrapOption);

	QTextDocument::FindFlags flags = static_cast<QTextDocument::FindFlags>(settings.value(QString::fromLatin1("searchFlags")).toInt());
	checkBox_case->setChecked((flags & QTextDocument::FindCaseSensitively) != 0);
	checkBox_words->setChecked((flags & QTextDocument::FindWholeWords) != 0);
	checkBox_backwards->setChecked((flags & QTextDocument::FindBackward) != 0);
	checkBox_backwards->setEnabled(!findAll);

	QMenu *recentItemsMenu = new QMenu(this);
	QStringList recentStrings = settings.value(QString::fromLatin1("recentSearchStrings")).toStringList();
	if (recentStrings.empty())
		recentItemsMenu->addAction(tr("No recent search strings"))->setEnabled(false);
	else {
		foreach (const QString& str, recentStrings)
			connect(recentItemsMenu->addAction(str), &QAction::triggered, this, &FindDialog::setSearchText);
	}
	recentSearches->setMenu(recentItemsMenu);
	searchText->installEventFilter(new RecentStringsKeyFilter(this, recentStrings));
}

void FindDialog::setSearchText()
{
	QAction *act = qobject_cast<QAction*>(sender());
	if (act) {
		searchText->setText(act->text());
		searchText->selectAll();
	}
}

void FindDialog::toggledAllFilesOption(bool checked)
{
	QTextEdit* document = qobject_cast<QTextEdit*>(parent());
	checkBox_selection->setEnabled(document && document->textCursor().hasSelection() && !checked && !checkBox_findAll->isChecked());
	checkBox_wrap->setEnabled(!(checkBox_selection->isEnabled() && checkBox_selection->isChecked()) && !checked && !checkBox_findAll->isChecked());
	checkBox_backwards->setEnabled(!checked && !checkBox_findAll->isChecked());
	checkBox_findAll->setEnabled(!checked);
}

void FindDialog::toggledFindAllOption(bool checked)
{
	QTextEdit* document = qobject_cast<QTextEdit*>(parent());
	checkBox_selection->setEnabled(document && document->textCursor().hasSelection() && !checked);
	checkBox_wrap->setEnabled(!(checkBox_selection->isEnabled() && checkBox_selection->isChecked()) && !checked);
	checkBox_backwards->setEnabled(!checked);
}

void FindDialog::toggledRegexOption(bool checked)
{
	checkBox_words->setEnabled(!checked);
	if (checked)
		checkRegex(searchText->text());
	else
		regexStatus->setText(QString());
}

void FindDialog::toggledSelectionOption(bool checked)
{
	checkBox_wrap->setEnabled(!checked);
}

void FindDialog::checkRegex(const QString& str)
{
	if (checkBox_regex->isChecked()) {
		QRegularExpression regex(str);
		if (regex.isValid())
			regexStatus->setText(QString());
		else
			regexStatus->setText(tr("(invalid)"));
	}
}

QDialog::DialogCode FindDialog::doFindDialog(QTextEdit *document)
{
	FindDialog dlg(document);

	dlg.show();
	DialogCode result = static_cast<DialogCode>(dlg.exec());

	if (result == Accepted) {
		Tw::Settings settings;
		QString str = dlg.searchText->text();
		settings.setValue(QString::fromLatin1("searchText"), str);

		QStringList recentStrings = settings.value(QString::fromLatin1("recentSearchStrings")).toStringList();
		recentStrings.removeAll(str);
		recentStrings.prepend(str);
		while (recentStrings.count() > kMaxRecentStrings)
			recentStrings.removeLast();
		settings.setValue(QString::fromLatin1("recentSearchStrings"), recentStrings);

		int flags = 0;
		if (dlg.checkBox_case->isChecked())
			flags |= QTextDocument::FindCaseSensitively;
		if (dlg.checkBox_words->isChecked())
			flags |= QTextDocument::FindWholeWords;
		if (dlg.checkBox_backwards->isChecked())
			flags |= QTextDocument::FindBackward;
		settings.setValue(QString::fromLatin1("searchFlags"), static_cast<int>(flags));

		settings.setValue(QString::fromLatin1("searchRegex"), dlg.checkBox_regex->isChecked());
		settings.setValue(QString::fromLatin1("searchWrap"), dlg.checkBox_wrap->isChecked());
		settings.setValue(QString::fromLatin1("searchSelection"), dlg.checkBox_selection->isChecked());
		settings.setValue(QString::fromLatin1("searchFindAll"), dlg.checkBox_findAll->isChecked());
		settings.setValue(QString::fromLatin1("searchAllFiles"), dlg.checkBox_allFiles->isChecked());
	}

	return result;
}

ReplaceDialog::ReplaceDialog(QTextEdit *parent)
	: QDialog(parent)
{
	init(parent);
}

void ReplaceDialog::init(QTextEdit *document)
{
	setupUi(this);

	connect(checkBox_allFiles, &QCheckBox::toggled, this, &ReplaceDialog::toggledAllFilesOption);
	connect(checkBox_regex, &QCheckBox::toggled, this, &ReplaceDialog::toggledRegexOption);
	connect(checkBox_selection, &QCheckBox::toggled, this, &ReplaceDialog::toggledSelectionOption);
	connect(searchText, &QLineEdit::textChanged, this, &ReplaceDialog::checkRegex);

	// using "standard" buttons then changing the labels means that Qt can reorder them appropriately for the platform
	// whereas if we just put our own named buttons in place manually, they'd always stay in the same order
	buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Replace"));
	connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &ReplaceDialog::clickedReplace);
	buttonBox->button(QDialogButtonBox::SaveAll)->setText(tr("Replace All"));
	connect(buttonBox->button(QDialogButtonBox::SaveAll), &QPushButton::clicked, this, &ReplaceDialog::clickedReplaceAll);
	buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &ReplaceDialog::reject);

	Tw::Settings settings;
	QString	str = settings.value(QString::fromLatin1("searchText")).toString();
	searchText->setText(str);
	searchText->selectAll();
	str = settings.value(QString::fromLatin1("replaceText")).toString();
	replaceText->setText(str);

	bool regexOption = settings.value(QString::fromLatin1("searchRegex")).toBool();
	checkBox_regex->setChecked(regexOption);
	checkBox_words->setEnabled(!regexOption);

	bool allFiles = settings.value(QString::fromLatin1("searchAllFiles")).toBool();
	checkBox_allFiles->setEnabled(TeXDocumentWindow::documentList().count() > 1);
	checkBox_allFiles->setChecked(allFiles && checkBox_allFiles->isEnabled());

	bool selectionOption = settings.value(QString::fromLatin1("searchSelection")).toBool();
	checkBox_selection->setEnabled(document->textCursor().hasSelection());
	checkBox_selection->setChecked(selectionOption && checkBox_selection->isEnabled());

	bool wrapOption = settings.value(QString::fromLatin1("searchWrap")).toBool();
	checkBox_wrap->setEnabled(!(checkBox_selection->isEnabled() && checkBox_selection->isChecked()));
	checkBox_wrap->setChecked(wrapOption);

	QTextDocument::FindFlags flags = static_cast<QTextDocument::FindFlags>(settings.value(QString::fromLatin1("searchFlags")).toInt());
	checkBox_case->setChecked((flags & QTextDocument::FindCaseSensitively) != 0);
	checkBox_words->setChecked((flags & QTextDocument::FindWholeWords) != 0);
	checkBox_backwards->setChecked((flags & QTextDocument::FindBackward) != 0);

	QMenu *recentItemsMenu = new QMenu(this);
	QStringList recentStrings = settings.value(QString::fromLatin1("recentSearchStrings")).toStringList();
	if (recentStrings.empty())
		recentItemsMenu->addAction(tr("No recent search strings"))->setEnabled(false);
	else {
		foreach (const QString& str, recentStrings)
			connect(recentItemsMenu->addAction(str), &QAction::triggered, this, &ReplaceDialog::setSearchText);
	}
	recentSearches->setMenu(recentItemsMenu);
	searchText->installEventFilter(new RecentStringsKeyFilter(this, recentStrings));

	recentItemsMenu = new QMenu(this);
	recentStrings = settings.value(QString::fromLatin1("recentReplaceStrings")).toStringList();
	if (recentStrings.empty())
		recentItemsMenu->addAction(tr("No recent replacement strings"))->setEnabled(false);
	else {
		foreach (const QString& str, recentStrings)
			connect(recentItemsMenu->addAction(str), &QAction::triggered, this, &ReplaceDialog::setReplaceText);
	}
	recentReplacements->setMenu(recentItemsMenu);
	replaceText->installEventFilter(new RecentStringsKeyFilter(this, recentStrings));
}

void ReplaceDialog::setSearchText()
{
	QAction *act = qobject_cast<QAction*>(sender());
	if (act) {
		searchText->setText(act->text());
		searchText->selectAll();
	}
}

void ReplaceDialog::setReplaceText()
{
	QAction *act = qobject_cast<QAction*>(sender());
	if (act) {
		replaceText->setText(act->text());
		replaceText->selectAll();
	}
}

void ReplaceDialog::toggledAllFilesOption(bool checked)
{
	QTextEdit* document = qobject_cast<QTextEdit*>(parent());
	checkBox_selection->setEnabled(document && document->textCursor().hasSelection() && !checked);
	checkBox_wrap->setEnabled(!(checkBox_selection->isEnabled() && checkBox_selection->isChecked()) && !checked);
	checkBox_backwards->setEnabled(!checked);
	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!checked);
}

void ReplaceDialog::toggledRegexOption(bool checked)
{
	checkBox_words->setEnabled(!checked);
	if (checked)
		checkRegex(searchText->text());
	else
		regexStatus->setText(QString());
}

void ReplaceDialog::toggledSelectionOption(bool checked)
{
	checkBox_wrap->setEnabled(!checked);
}

void ReplaceDialog::checkRegex(const QString& str)
{
	if (checkBox_regex->isChecked()) {
		QRegularExpression regex(str);
		if (regex.isValid())
			regexStatus->setText(QString());
		else
			regexStatus->setText(tr("(invalid)"));
	}
}

void ReplaceDialog::clickedReplace()
{
	done(1);
}

void ReplaceDialog::clickedReplaceAll()
{
	done(2);
}

ReplaceDialog::DialogCode ReplaceDialog::doReplaceDialog(QTextEdit *document)
{
	ReplaceDialog dlg(document);

	dlg.show();
	int	result = dlg.exec();

	if (result == 0)
		return Cancel;

	Tw::Settings settings;
	QString str = dlg.searchText->text();
	settings.setValue(QString::fromLatin1("searchText"), str);

	QStringList recentStrings = settings.value(QString::fromLatin1("recentSearchStrings")).toStringList();
	recentStrings.removeAll(str);
	recentStrings.prepend(str);
	while (recentStrings.count() > kMaxRecentStrings)
		recentStrings.removeLast();
	settings.setValue(QString::fromLatin1("recentSearchStrings"), recentStrings);

	str = dlg.replaceText->text();
	settings.setValue(QString::fromLatin1("replaceText"), str);

	recentStrings = settings.value(QString::fromLatin1("recentReplaceStrings")).toStringList();
	recentStrings.removeAll(str);
	recentStrings.prepend(str);
	while (recentStrings.count() > kMaxRecentStrings)
		recentStrings.removeLast();
	settings.setValue(QString::fromLatin1("recentReplaceStrings"), recentStrings);

	int flags = 0;
	if (dlg.checkBox_case->isChecked())
		flags |= QTextDocument::FindCaseSensitively;
	if (dlg.checkBox_words->isChecked())
		flags |= QTextDocument::FindWholeWords;
	if (dlg.checkBox_backwards->isChecked())
		flags |= QTextDocument::FindBackward;
	settings.setValue(QString::fromLatin1("searchFlags"), static_cast<int>(flags));

	settings.setValue(QString::fromLatin1("searchRegex"), dlg.checkBox_regex->isChecked());
	settings.setValue(QString::fromLatin1("searchWrap"), dlg.checkBox_wrap->isChecked());
	settings.setValue(QString::fromLatin1("searchSelection"), dlg.checkBox_selection->isChecked());
	settings.setValue(QString::fromLatin1("searchAllFiles"), dlg.checkBox_allFiles->isChecked());

	return (result == 2) ? ReplaceAll : ReplaceOne;
}


SearchResults::SearchResults(QWidget* parent)
	: QDockWidget(parent)
{
	setupUi(this);
	setFocusProxy(parent);
	connect(table, &QTableWidget::itemSelectionChanged, this, &SearchResults::showSelectedEntry);
	connect(table, &QTableWidget::itemPressed, this, &SearchResults::showEntry);
	connect(table, &QTableWidget::itemActivated, this, &SearchResults::goToSource);
	QShortcut * sc = new QShortcut(Qt::Key_Escape, table);
	sc->setContext(Qt::WidgetShortcut);
	connect(sc, &QShortcut::activated, this, &SearchResults::goToSourceAndClose);
}

void SearchResults::goToSource()
{
	QList<QTableWidgetSelectionRange> ranges = table->selectedRanges();
	if (ranges.count() == 0)
		return;
	int row = ranges.first().topRow();
	QString fileName;
	QTableWidgetItem* item = table->item(row, 0);
	if (!item)
		return;
	fileName = item->toolTip();

	if (!fileName.isEmpty()) {
		QWidget *theDoc = TeXDocumentWindow::openDocument(fileName);
		if (theDoc) {
			QTextEdit *editor = theDoc->findChild<QTextEdit*>(QString::fromLatin1("textEdit"));
			if (editor)
				editor->setFocus();
		}
	}
}

void SearchResults::goToSourceAndClose()
{
	goToSource();
	deleteLater();
}

#define MAXIMUM_CHARACTERS_BEFORE_SEARCH_RESULT 40
#define MAXIMUM_CHARACTERS_AFTER_SEARCH_RESULT 80

void SearchResults::presentResults(const QString& searchText,
								   const QList<SearchResult>& results,
								   QMainWindow* parent, bool singleFile)
{
	if (singleFile) {
		// remove any existing results dock from this parent window
		QList<SearchResults*> children = parent->findChildren<SearchResults*>();
		foreach (SearchResults* child, children) {
			parent->removeDockWidget(child);
			child->deleteLater();
		}
	}

	SearchResults* resultsWindow = new SearchResults(parent);
	resultsWindow->setWindowTitle(tr("Search Results - %1 (%2 found)").arg(searchText).arg(results.count()));

	resultsWindow->table->setRowCount(results.count());
	int i = 0;
	foreach (const SearchResult &result, results) {
		QTableWidgetItem *item = new QTableWidgetItem(QFileInfo(result.doc->fileName()).fileName());
		item->setToolTip(result.doc->fileName());
		resultsWindow->table->setItem(i, 0, item);
		resultsWindow->table->setItem(i, 1, new QTableWidgetItem(QString::number(result.lineNo)));
		resultsWindow->table->setItem(i, 2, new QTableWidgetItem(QString::number(result.selStart)));
		resultsWindow->table->setItem(i, 3, new QTableWidgetItem(QString::number(result.selEnd)));

		// Only show a limited number of characters before and after the
		// specified search string to keep the results clear
		bool truncateStart = true, truncateEnd = true;
		QString text = result.doc->getLineText(result.lineNo);
		int iStart = result.selStart - MAXIMUM_CHARACTERS_BEFORE_SEARCH_RESULT;
		int iEnd = result.selEnd + MAXIMUM_CHARACTERS_AFTER_SEARCH_RESULT;
		if (iStart < 0) {
			iStart = 0;
			truncateStart = false;
		}
		if (iEnd > text.length()) {
			iEnd = text.length();
			truncateEnd = false;
		}
		if (truncateStart || truncateEnd) {
			// ensure the truncation happens on appropriate boundaries, not mid-cluster
			QTextBoundaryFinder tbf(QTextBoundaryFinder::Grapheme, text);
			if (truncateStart) {
				tbf.setPosition(iStart);
				if (!tbf.isAtBoundary()) {
					tbf.toPreviousBoundary();
					iStart = tbf.position();
				}
			}
			if (truncateEnd) {
				tbf.setPosition(iEnd);
				if (!tbf.isAtBoundary()) {
					tbf.toNextBoundary();
					iEnd = tbf.position();
				}
			}
		}
		text = text.mid(iStart, iEnd - iStart);
		if (truncateStart)
			text.prepend(tr("..."));
		if (truncateEnd)
			text.append(tr("..."));
		resultsWindow->table->setItem(i, 4, new QTableWidgetItem(text));

		++i;
	}

	resultsWindow->table->setHorizontalHeaderLabels(QStringList() << tr("File") << tr("Line") << tr("Start") << tr("End") << tr("Text"));
	resultsWindow->table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
	resultsWindow->table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	resultsWindow->table->verticalHeader()->hide();
	resultsWindow->table->setColumnHidden(2, true);
	resultsWindow->table->setColumnHidden(3, true);

	resultsWindow->table->resizeColumnsToContents();
	resultsWindow->table->resizeRowsToContents();

	if (singleFile) {
		resultsWindow->setAllowedAreas(Qt::TopDockWidgetArea|Qt::BottomDockWidgetArea);
		resultsWindow->setFloating(false);
		parent->addDockWidget(Qt::TopDockWidgetArea, resultsWindow);
	}
	else {
		resultsWindow->setAllowedAreas(Qt::NoDockWidgetArea);
		resultsWindow->setFeatures(QDockWidget::NoDockWidgetFeatures);
		resultsWindow->setParent(nullptr);
		resultsWindow->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
	}

	resultsWindow->show();
}

void SearchResults::showEntry(QTableWidgetItem * item)
{
	if (!item)
		return;
	int row = item->row();
	item = table->item(row, 0);
	QString fileName = item->toolTip();
	item = table->item(row, 1);
	int lineNo = item->text().toInt();
	item = table->item(row, 2);
	int selStart = item->text().toInt();
	item = table->item(row, 3);
	int selEnd = item->text().toInt();

	if (!fileName.isEmpty())
		TeXDocumentWindow::openDocument(fileName, false, true, lineNo, selStart, selEnd);
}

void SearchResults::showSelectedEntry()
{
	QList<QTableWidgetSelectionRange> ranges = table->selectedRanges();
	if (ranges.count() == 0)
		return;
	int row = ranges.first().topRow();
	QTableWidgetItem* item = table->item(row, 0);
	if (!item)
		return;
	showEntry(item);
}

PDFFindDialog::PDFFindDialog(PDFDocumentWindow *document)
	: QDialog(document)
{
	init(document);
}

void PDFFindDialog::init(PDFDocumentWindow *document)
{
	setupUi(this);

	buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Find"));
/*
	connect(checkBox_allFiles, &QCheckBox::toggled, this, &PDFFindDialog::toggledAllFilesOption);
	connect(checkBox_findAll, &QCheckBox::toggled, this, &PDFFindDialog::toggledFindAllOption);
	connect(checkBox_regex, &QCheckBox::toggled, this, &PDFFindDialog::toggledRegexOption);
	connect(checkBox_selection, &QCheckBox::toggled, this, &PDFFindDialog::toggledSelectionOption);
	connect(searchText, &QLineEdit::textChanged, this, &PDFFindDialog::checkRegex);
*/
	Tw::Settings settings;
	QString	str = settings.value(QString::fromLatin1("searchText")).toString();
	searchText->setText(str);
	searchText->selectAll();

	// if findAll is enabled, revisit the saving of settings in
	// PDFFindDialog::doFindDialog (and enable saving findAll if it's reasonable
	// to override the setting (also for searches in the editor))
	checkBox_findAll->setEnabled(false);
	bool findAll = false;
/*
	bool findAll = settings.value(QString::fromLatin1("searchFindAll")).toBool();
	checkBox_findAll->setChecked(findAll);
*/
	bool wrapOption = settings.value(QString::fromLatin1("searchWrap")).toBool();
	checkBox_wrap->setEnabled(!findAll);
	checkBox_wrap->setChecked(wrapOption);

	QTextDocument::FindFlags flags = static_cast<QTextDocument::FindFlags>(settings.value(QString::fromLatin1("searchFlags")).toInt());
	checkBox_case->setChecked((flags & QTextDocument::FindCaseSensitively) != 0);
//	checkBox_words->setChecked((flags & QTextDocument::FindWholeWords) != 0);
//	checkBox_backwards->setChecked((flags & QTextDocument::FindBackward) != 0);
//	checkBox_backwards->setEnabled(!findAll);

	// Searching backwards currently doesn't work
	// Might be a bug in Poppler
	checkBox_backwards->setEnabled(false);

	checkBox_sync->setChecked(settings.value(QString::fromLatin1("searchPdfSync")).toBool());
	checkBox_sync->setEnabled(document->hasSyncData());

	QMenu *recentItemsMenu = new QMenu(this);
	QStringList recentStrings = settings.value(QString::fromLatin1("recentSearchStrings")).toStringList();
	if (recentStrings.empty())
		recentItemsMenu->addAction(tr("No recent search strings"))->setEnabled(false);
	else {
		foreach (const QString& str, recentStrings)
			connect(recentItemsMenu->addAction(str), &QAction::triggered, this, &PDFFindDialog::setSearchText);
	}
	recentSearches->setMenu(recentItemsMenu);
	searchText->installEventFilter(new RecentStringsKeyFilter(this, recentStrings));
}

QDialog::DialogCode PDFFindDialog::doFindDialog(PDFDocumentWindow *document)
{
	PDFFindDialog dlg(document);

	dlg.show();
	DialogCode result = static_cast<DialogCode>(dlg.exec());

	if (result == Accepted) {
		Tw::Settings settings;
		QString str = dlg.searchText->text();
		settings.setValue(QString::fromLatin1("searchText"), str);

		QStringList recentStrings = settings.value(QString::fromLatin1("recentSearchStrings")).toStringList();
		recentStrings.removeAll(str);
		recentStrings.prepend(str);
		while (recentStrings.count() > kMaxRecentStrings)
			recentStrings.removeLast();
		settings.setValue(QString::fromLatin1("recentSearchStrings"), recentStrings);

		QTextDocument::FindFlags oldFlags = static_cast<QTextDocument::FindFlags>(settings.value(QString::fromLatin1("searchFlags")).toInt());
		int flags = 0;
		if (dlg.checkBox_case->isChecked())
			flags |= QTextDocument::FindCaseSensitively;

		flags |= (oldFlags & QTextDocument::FindWholeWords);

//		if (dlg.checkBox_backwards->isChecked())
//			flags |= QTextDocument::FindBackward;
		flags |= (oldFlags & QTextDocument::FindBackward);

		settings.setValue(QString::fromLatin1("searchFlags"), static_cast<int>(flags));

//		settings.setValue(QString::fromLatin1("searchRegex"), dlg.checkBox_regex->isChecked());
		settings.setValue(QString::fromLatin1("searchWrap"), dlg.checkBox_wrap->isChecked());
//		settings.setValue(QString::fromLatin1("searchSelection"), dlg.checkBox_selection->isChecked());
//		settings.setValue(QString::fromLatin1("searchFindAll"), dlg.checkBox_findAll->isChecked());
//		settings.setValue(QString::fromLatin1("searchAllFiles"), dlg.checkBox_allFiles->isChecked());
		settings.setValue(QString::fromLatin1("searchPdfSync"), dlg.checkBox_sync->isChecked());
	}

	return result;
}

void PDFFindDialog::setSearchText()
{
	QAction *act = qobject_cast<QAction*>(sender());
	if (act) {
		searchText->setText(act->text());
		searchText->selectAll();
	}
}

