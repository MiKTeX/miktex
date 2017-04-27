/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2016  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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
#include "TeXDocument.h"
#include "PDFDocument.h"
#include "TWApp.h"

#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QTextBlock>
#include <QFileInfo>
#include <QKeyEvent>
#include <QShortcut>
#if QT_VERSION >= 0x040400
#include <QTextBoundaryFinder>
#endif

const int kMaxRecentStrings = 10;

FindDialog::FindDialog(QTextEdit *parent)
	: QDialog(parent)
{
	init(parent);
}

bool RecentStringsKeyFilter::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *e = static_cast<QKeyEvent*>(event);
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

	connect(checkBox_allFiles, SIGNAL(toggled(bool)), this, SLOT(toggledAllFilesOption(bool)));
	connect(checkBox_findAll, SIGNAL(toggled(bool)), this, SLOT(toggledFindAllOption(bool)));
	connect(checkBox_regex, SIGNAL(toggled(bool)), this, SLOT(toggledRegexOption(bool)));
	connect(checkBox_selection, SIGNAL(toggled(bool)), this, SLOT(toggledSelectionOption(bool)));
	connect(searchText, SIGNAL(textChanged(const QString&)), this, SLOT(checkRegex(const QString&)));

	QSETTINGS_OBJECT(settings);
	QString	str = settings.value("searchText").toString();
	searchText->setText(str);
	searchText->selectAll();

	bool regexOption = settings.value("searchRegex").toBool();
	checkBox_regex->setChecked(regexOption);
	checkBox_words->setEnabled(!regexOption);

	bool findAll = settings.value("searchFindAll").toBool();
	checkBox_findAll->setChecked(findAll);
	
	bool allFiles = settings.value("searchAllFiles").toBool();
	checkBox_allFiles->setEnabled(TeXDocument::documentList().count() > 1);
	checkBox_allFiles->setChecked(allFiles && checkBox_allFiles->isEnabled());

	bool selectionOption = settings.value("searchSelection").toBool();
	checkBox_selection->setEnabled(document->textCursor().hasSelection() && !findAll);
	checkBox_selection->setChecked(selectionOption && checkBox_selection->isEnabled());

	bool wrapOption = settings.value("searchWrap").toBool();
	checkBox_wrap->setEnabled(!(checkBox_selection->isEnabled() && checkBox_selection->isChecked()) && !findAll);
	checkBox_wrap->setChecked(wrapOption);

	QTextDocument::FindFlags flags = (QTextDocument::FindFlags)settings.value("searchFlags").toInt();
	checkBox_case->setChecked((flags & QTextDocument::FindCaseSensitively) != 0);
	checkBox_words->setChecked((flags & QTextDocument::FindWholeWords) != 0);
	checkBox_backwards->setChecked((flags & QTextDocument::FindBackward) != 0);
	checkBox_backwards->setEnabled(!findAll);
	
	QMenu *recentItemsMenu = new QMenu(this);
	QStringList recentStrings = settings.value("recentSearchStrings").toStringList();
	if (recentStrings.size() == 0)
		recentItemsMenu->addAction(tr("No recent search strings"))->setEnabled(false);
	else {
		foreach (const QString& str, recentStrings)
			connect(recentItemsMenu->addAction(str), SIGNAL(triggered()), this, SLOT(setSearchText()));
	}
	recentSearches->setMenu(recentItemsMenu);
	searchText->installEventFilter(new RecentStringsKeyFilter(this, recentStrings));
}

void FindDialog::setSearchText()
{
	QAction *act = qobject_cast<QAction*>(sender());
	if (act != NULL) {
		searchText->setText(act->text());
		searchText->selectAll();
	}
}

void FindDialog::toggledAllFilesOption(bool checked)
{
	QTextEdit* document = qobject_cast<QTextEdit*>(parent());
	checkBox_selection->setEnabled(document != NULL && document->textCursor().hasSelection() && !checked && !checkBox_findAll->isChecked());
	checkBox_wrap->setEnabled(!(checkBox_selection->isEnabled() && checkBox_selection->isChecked()) && !checked && !checkBox_findAll->isChecked());
	checkBox_backwards->setEnabled(!checked && !checkBox_findAll->isChecked());
	checkBox_findAll->setEnabled(!checked);
}

void FindDialog::toggledFindAllOption(bool checked)
{
	QTextEdit* document = qobject_cast<QTextEdit*>(parent());
	checkBox_selection->setEnabled(document != NULL && document->textCursor().hasSelection() && !checked);
	checkBox_wrap->setEnabled(!(checkBox_selection->isEnabled() && checkBox_selection->isChecked()) && !checked);
	checkBox_backwards->setEnabled(!checked);
}

void FindDialog::toggledRegexOption(bool checked)
{
	checkBox_words->setEnabled(!checked);
	if (checked)
		checkRegex(searchText->text());
	else
		regexStatus->setText("");
}

void FindDialog::toggledSelectionOption(bool checked)
{
	checkBox_wrap->setEnabled(!checked);
}

void FindDialog::checkRegex(const QString& str)
{
	if (checkBox_regex->isChecked()) {
		QRegExp regex(str);
		if (regex.isValid())
			regexStatus->setText("");
		else
			regexStatus->setText(tr("(invalid)"));
	}
}

QDialog::DialogCode FindDialog::doFindDialog(QTextEdit *document)
{
	FindDialog dlg(document);

	dlg.show();
	DialogCode	result = (DialogCode)dlg.exec();
	
	if (result == Accepted) {
		QSETTINGS_OBJECT(settings);
		QString str = dlg.searchText->text();
		settings.setValue("searchText", str);
		
		QStringList recentStrings = settings.value("recentSearchStrings").toStringList();
		recentStrings.removeAll(str);
		recentStrings.prepend(str);
		while (recentStrings.count() > kMaxRecentStrings)
			recentStrings.removeLast();
		settings.setValue("recentSearchStrings", recentStrings);

		int flags = 0;
		if (dlg.checkBox_case->isChecked())
			flags |= QTextDocument::FindCaseSensitively;
		if (dlg.checkBox_words->isChecked())
			flags |= QTextDocument::FindWholeWords;
		if (dlg.checkBox_backwards->isChecked())
			flags |= QTextDocument::FindBackward;
		settings.setValue("searchFlags", (int)flags);

		settings.setValue("searchRegex", dlg.checkBox_regex->isChecked());
		settings.setValue("searchWrap", dlg.checkBox_wrap->isChecked());
		settings.setValue("searchSelection", dlg.checkBox_selection->isChecked());
		settings.setValue("searchFindAll", dlg.checkBox_findAll->isChecked());
		settings.setValue("searchAllFiles", dlg.checkBox_allFiles->isChecked());
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

	connect(checkBox_allFiles, SIGNAL(toggled(bool)), this, SLOT(toggledAllFilesOption(bool)));
	connect(checkBox_regex, SIGNAL(toggled(bool)), this, SLOT(toggledRegexOption(bool)));
	connect(checkBox_selection, SIGNAL(toggled(bool)), this, SLOT(toggledSelectionOption(bool)));
	connect(searchText, SIGNAL(textChanged(const QString&)), this, SLOT(checkRegex(const QString&)));

	// using "standard" buttons then changing the labels means that Qt can reorder them appropriately for the platform
	// whereas if we just put our own named buttons in place manually, they'd always stay in the same order
	buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Replace"));
	connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(clickedReplace()));
	buttonBox->button(QDialogButtonBox::SaveAll)->setText(tr("Replace All"));
	connect(buttonBox->button(QDialogButtonBox::SaveAll), SIGNAL(clicked()), this, SLOT(clickedReplaceAll()));
	buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));

	QSETTINGS_OBJECT(settings);
	QString	str = settings.value("searchText").toString();
	searchText->setText(str);
	searchText->selectAll();
	str = settings.value("replaceText").toString();
	replaceText->setText(str);

	bool regexOption = settings.value("searchRegex").toBool();
	checkBox_regex->setChecked(regexOption);
	checkBox_words->setEnabled(!regexOption);

	bool allFiles = settings.value("searchAllFiles").toBool();
	checkBox_allFiles->setEnabled(TeXDocument::documentList().count() > 1);
	checkBox_allFiles->setChecked(allFiles && checkBox_allFiles->isEnabled());

	bool selectionOption = settings.value("searchSelection").toBool();
	checkBox_selection->setEnabled(document->textCursor().hasSelection());
	checkBox_selection->setChecked(selectionOption && checkBox_selection->isEnabled());

	bool wrapOption = settings.value("searchWrap").toBool();
	checkBox_wrap->setEnabled(!(checkBox_selection->isEnabled() && checkBox_selection->isChecked()));
	checkBox_wrap->setChecked(wrapOption);

	QTextDocument::FindFlags flags = (QTextDocument::FindFlags)settings.value("searchFlags").toInt();
	checkBox_case->setChecked((flags & QTextDocument::FindCaseSensitively) != 0);
	checkBox_words->setChecked((flags & QTextDocument::FindWholeWords) != 0);
	checkBox_backwards->setChecked((flags & QTextDocument::FindBackward) != 0);

	QMenu *recentItemsMenu = new QMenu(this);
	QStringList recentStrings = settings.value("recentSearchStrings").toStringList();
	if (recentStrings.size() == 0)
		recentItemsMenu->addAction(tr("No recent search strings"))->setEnabled(false);
	else {
		foreach (const QString& str, recentStrings)
			connect(recentItemsMenu->addAction(str), SIGNAL(triggered()), this, SLOT(setSearchText()));
	}
	recentSearches->setMenu(recentItemsMenu);
	searchText->installEventFilter(new RecentStringsKeyFilter(this, recentStrings));

	recentItemsMenu = new QMenu(this);
	recentStrings = settings.value("recentReplaceStrings").toStringList();
	if (recentStrings.size() == 0)
		recentItemsMenu->addAction(tr("No recent replacement strings"))->setEnabled(false);
	else {
		foreach (const QString& str, recentStrings)
			connect(recentItemsMenu->addAction(str), SIGNAL(triggered()), this, SLOT(setReplaceText()));
	}
	recentReplacements->setMenu(recentItemsMenu);
	replaceText->installEventFilter(new RecentStringsKeyFilter(this, recentStrings));
}

void ReplaceDialog::setSearchText()
{
	QAction *act = qobject_cast<QAction*>(sender());
	if (act != NULL) {
		searchText->setText(act->text());
		searchText->selectAll();
	}
}

void ReplaceDialog::setReplaceText()
{
	QAction *act = qobject_cast<QAction*>(sender());
	if (act != NULL) {
		replaceText->setText(act->text());
		replaceText->selectAll();
	}
}

void ReplaceDialog::toggledAllFilesOption(bool checked)
{
	QTextEdit* document = qobject_cast<QTextEdit*>(parent());
	checkBox_selection->setEnabled(document != NULL && document->textCursor().hasSelection() && !checked);
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
		regexStatus->setText("");
}

void ReplaceDialog::toggledSelectionOption(bool checked)
{
	checkBox_wrap->setEnabled(!checked);
}

void ReplaceDialog::checkRegex(const QString& str)
{
	if (checkBox_regex->isChecked()) {
		QRegExp regex(str);
		if (regex.isValid())
			regexStatus->setText("");
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
	else {
		QSETTINGS_OBJECT(settings);
		QString str = dlg.searchText->text();
		settings.setValue("searchText", str);
		
		QStringList recentStrings = settings.value("recentSearchStrings").toStringList();
		recentStrings.removeAll(str);
		recentStrings.prepend(str);
		while (recentStrings.count() > kMaxRecentStrings)
			recentStrings.removeLast();
		settings.setValue("recentSearchStrings", recentStrings);
		
		str = dlg.replaceText->text();
		settings.setValue("replaceText", str);

		recentStrings = settings.value("recentReplaceStrings").toStringList();
		recentStrings.removeAll(str);
		recentStrings.prepend(str);
		while (recentStrings.count() > kMaxRecentStrings)
			recentStrings.removeLast();
		settings.setValue("recentReplaceStrings", recentStrings);
		
		int flags = 0;
		if (dlg.checkBox_case->isChecked())
			flags |= QTextDocument::FindCaseSensitively;
		if (dlg.checkBox_words->isChecked())
			flags |= QTextDocument::FindWholeWords;
		if (dlg.checkBox_backwards->isChecked())
			flags |= QTextDocument::FindBackward;
		settings.setValue("searchFlags", (int)flags);

		settings.setValue("searchRegex", dlg.checkBox_regex->isChecked());
		settings.setValue("searchWrap", dlg.checkBox_wrap->isChecked());
		settings.setValue("searchSelection", dlg.checkBox_selection->isChecked());
		settings.setValue("searchAllFiles", dlg.checkBox_allFiles->isChecked());

		return (result == 2) ? ReplaceAll : ReplaceOne;
	}
}


SearchResults::SearchResults(QWidget* parent)
	: QDockWidget(parent)
{
	setupUi(this);
	connect(table, SIGNAL(itemSelectionChanged()), this, SLOT(showSelectedEntry()));
	connect(table, SIGNAL(itemPressed(QTableWidgetItem*)), this, SLOT(showEntry(QTableWidgetItem*)));
	connect(table, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(goToSource()));
	QShortcut *sc;
	sc = new QShortcut(Qt::Key_Escape, table);
	sc->setContext(Qt::WidgetShortcut);
	connect(sc, SIGNAL(activated()), this, SLOT(goToSourceAndClose()));
	
	TeXDocument * texDoc = qobject_cast<TeXDocument*>(parent);
	if (texDoc) {
		editorModifiedPalette = editorOriginalPalette = texDoc->palette();
		editorModifiedPalette.setColor(QPalette::Highlight, editorOriginalPalette.color(QPalette::Active, QPalette::Highlight));
		editorModifiedPalette.setColor(QPalette::HighlightedText, editorOriginalPalette.color(QPalette::Active, QPalette::HighlightedText));
	}
	connect(TWApp::instance(), SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(focusChanged(QWidget*,QWidget*)));
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
		QWidget *theDoc = TeXDocument::openDocument(fileName);
		if (theDoc) {
			QTextEdit *editor = theDoc->findChild<QTextEdit*>("textEdit");
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
		int iStart, iEnd;
		QString text = result.doc->getLineText(result.lineNo);
		iStart = result.selStart - MAXIMUM_CHARACTERS_BEFORE_SEARCH_RESULT;
		iEnd = result.selEnd + MAXIMUM_CHARACTERS_AFTER_SEARCH_RESULT;
		if (iStart < 0) {
			iStart = 0;
			truncateStart = false;
		}
		if (iEnd > text.length()) {
			iEnd = text.length();
			truncateEnd = false;
		}
#if QT_VERSION >= 0x040400 // QTextBoundaryFinder is new in Qt 4.4
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
#endif
		text = text.mid(iStart, iEnd - iStart);
		if (truncateStart)
			text.prepend(tr("..."));
		if (truncateEnd)
			text.append(tr("..."));
		resultsWindow->table->setItem(i, 4, new QTableWidgetItem(text));

		++i;
	}

	resultsWindow->table->setHorizontalHeaderLabels(QStringList() << tr("File") << tr("Line") << tr("Start") << tr("End") << tr("Text"));
	#if QT_VERSION >= 0x050000
	resultsWindow->table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
	resultsWindow->table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	#else
	resultsWindow->table->horizontalHeader()->setResizeMode(4, QHeaderView::Stretch);
	resultsWindow->table->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	#endif
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
		resultsWindow->setParent(NULL);
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
		TeXDocument::openDocument(fileName, false, true, lineNo, selStart, selEnd);
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

void SearchResults::focusChanged(QWidget * old, QWidget * now)
{
	bool previouslyFocused, nowFocused;
	TeXDocument * texDoc = qobject_cast<TeXDocument*>(parent());

	if (!texDoc)
		return;

	if (old == NULL)
		previouslyFocused = false;
	else
		previouslyFocused = isAncestorOf(old);

	if (now == NULL)
		nowFocused = false;
	else
		nowFocused = isAncestorOf(now);

	if (!previouslyFocused && nowFocused)
		texDoc->editor()->setPalette(editorModifiedPalette);
	if (previouslyFocused && !nowFocused)
		texDoc->editor()->setPalette(editorOriginalPalette);
}

PDFFindDialog::PDFFindDialog(PDFDocument *document)
	: QDialog(document)
{
	init(document);
}

void PDFFindDialog::init(PDFDocument *document)
{
	setupUi(this);

	buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Find"));
/*
	connect(checkBox_allFiles, SIGNAL(toggled(bool)), this, SLOT(toggledAllFilesOption(bool)));
	connect(checkBox_findAll, SIGNAL(toggled(bool)), this, SLOT(toggledFindAllOption(bool)));
	connect(checkBox_regex, SIGNAL(toggled(bool)), this, SLOT(toggledRegexOption(bool)));
	connect(checkBox_selection, SIGNAL(toggled(bool)), this, SLOT(toggledSelectionOption(bool)));
	connect(searchText, SIGNAL(textChanged(const QString&)), this, SLOT(checkRegex(const QString&)));
*/
	QSETTINGS_OBJECT(settings);
	QString	str = settings.value("searchText").toString();
	searchText->setText(str);
	searchText->selectAll();
	
	// if findAll is enabled, revisit the saving of settings in
	// PDFFindDialog::doFindDialog (and enable saving findAll if it's reasonable
	// to override the setting (also for searches in the editor))
	checkBox_findAll->setEnabled(false);
	bool findAll = false;
/*
	bool findAll = settings.value("searchFindAll").toBool();
	checkBox_findAll->setChecked(findAll);
*/
	bool wrapOption = settings.value("searchWrap").toBool();
	checkBox_wrap->setEnabled(!findAll);
	checkBox_wrap->setChecked(wrapOption);

	QTextDocument::FindFlags flags = (QTextDocument::FindFlags)settings.value("searchFlags").toInt();
	checkBox_case->setChecked((flags & QTextDocument::FindCaseSensitively) != 0);
//	checkBox_words->setChecked((flags & QTextDocument::FindWholeWords) != 0);
//	checkBox_backwards->setChecked((flags & QTextDocument::FindBackward) != 0);
//	checkBox_backwards->setEnabled(!findAll);

	// Searching backwards currently doesn't work
	// Might be a bug in Poppler
	checkBox_backwards->setEnabled(false);
	
	checkBox_sync->setChecked(settings.value("searchPdfSync").toBool());
	checkBox_sync->setEnabled(document->hasSyncData());
	
	QMenu *recentItemsMenu = new QMenu(this);
	QStringList recentStrings = settings.value("recentSearchStrings").toStringList();
	if (recentStrings.size() == 0)
		recentItemsMenu->addAction(tr("No recent search strings"))->setEnabled(false);
	else {
		foreach (const QString& str, recentStrings)
			connect(recentItemsMenu->addAction(str), SIGNAL(triggered()), this, SLOT(setSearchText()));
	}
	recentSearches->setMenu(recentItemsMenu);
	searchText->installEventFilter(new RecentStringsKeyFilter(this, recentStrings));
}

QDialog::DialogCode PDFFindDialog::doFindDialog(PDFDocument *document)
{
	PDFFindDialog dlg(document);

	dlg.show();
	DialogCode	result = (DialogCode)dlg.exec();

	if (result == Accepted) {
		QSETTINGS_OBJECT(settings);
		QString str = dlg.searchText->text();
		settings.setValue("searchText", str);
		
		QStringList recentStrings = settings.value("recentSearchStrings").toStringList();
		recentStrings.removeAll(str);
		recentStrings.prepend(str);
		while (recentStrings.count() > kMaxRecentStrings)
			recentStrings.removeLast();
		settings.setValue("recentSearchStrings", recentStrings);

		QTextDocument::FindFlags oldFlags = (QTextDocument::FindFlags)settings.value("searchFlags").toInt();
		int flags = 0;
		if (dlg.checkBox_case->isChecked())
			flags |= QTextDocument::FindCaseSensitively;

		flags |= (oldFlags & QTextDocument::FindWholeWords);

//		if (dlg.checkBox_backwards->isChecked())
//			flags |= QTextDocument::FindBackward;
		flags |= (oldFlags & QTextDocument::FindBackward);
		
		settings.setValue("searchFlags", (int)flags);

//		settings.setValue("searchRegex", dlg.checkBox_regex->isChecked());
		settings.setValue("searchWrap", dlg.checkBox_wrap->isChecked());
//		settings.setValue("searchSelection", dlg.checkBox_selection->isChecked());
//		settings.setValue("searchFindAll", dlg.checkBox_findAll->isChecked());
//		settings.setValue("searchAllFiles", dlg.checkBox_allFiles->isChecked());
		settings.setValue("searchPdfSync", dlg.checkBox_sync->isChecked());
	}

	return result;
}

void PDFFindDialog::setSearchText()
{
	QAction *act = qobject_cast<QAction*>(sender());
	if (act != NULL) {
		searchText->setText(act->text());
		searchText->selectAll();
	}
}

