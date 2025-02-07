/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2024  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "TeXDocumentWindow.h"

#include "CitationSelectDialog.h"
#include "DefaultPrefs.h"
#include "Engine.h"
#include "FindDialog.h"
#include "HardWrapDialog.h"
#include "PDFDocumentWindow.h"
#include "Settings.h"
#include "TWApp.h"
#include "TWUtils.h"
#include "TeXDocks.h"
#include "TeXHighlighter.h"
#include "TemplateDialog.h"
#include "scripting/ScriptAPI.h"
#include "ui/ClickableLabel.h"
#include "ui/RemoveAuxFilesDialog.h"
#include "utils/CmdKeyFilter.h"
#include "utils/WindowManager.h"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractTextDocumentLayout>
#include <QActionGroup>
#include <QClipboard>
#include <QCloseEvent>
#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileSystemWatcher>
#include <QFontDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QScrollBar>
#include <QSignalMapper>
#include <QStatusBar>
#include <QStringList>
#include <QTextBrowser>
#include <QTextCodec>
#include <QUrl>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

QList<TeXDocumentWindow*> TeXDocumentWindow::docList;

TeXDocumentWindow::TeXDocumentWindow()
	: _texDoc(new Tw::Document::TeXDocument(this))
{
	init();
	statusBar()->showMessage(tr("New document"), kStatusMessageDuration);
}

TeXDocumentWindow::TeXDocumentWindow(const QString &fileName, bool asTemplate)
	: _texDoc(new Tw::Document::TeXDocument(this))
{
	init();
	loadFile(QFileInfo(fileName), asTemplate);
}

TeXDocumentWindow::~TeXDocumentWindow()
{
	// Disconnect from notifications to avoid getting signals after our
	// TeXDocumentWindow is destroyed (but before the base QObject is destroyed,
	// in which case the system wouldn't know what to do with the signal)
	disconnect(TWApp::instance(), &TWApp::windowListChanged, this, nullptr);
	disconnect(TWApp::instance(), &TWApp::recentFileActionsChanged, this, nullptr);
	docList.removeAll(this);
	updateWindowMenu();
	// Because _texDoc->parent() == this, _texDoc will be destroyed
	// automatically by ~QObject()
#if defined(Q_OS_DARWIN)
	// Work around QTBUG-17941
	QTimer::singleShot(0, TWApp::instance(), &TWApp::recreateSpecialMenuItems);
#endif // defined(Q_OS_DARWIN)
}

static bool dictActionLessThan(const QAction * a1, const QAction * a2) {
	return a1->text().toLower() < a2->text().toLower();
}

void TeXDocumentWindow::init()
{
	codec = TWApp::instance()->getDefaultCodec();
	pdfDoc = nullptr;
	process = nullptr;
	utf8BOM = false;
#if defined(Q_OS_WIN)
	lineEndings = kLineEnd_CRLF;
#else
	lineEndings = kLineEnd_LF;
#endif

	setupUi(this);
	editor()->setDocument(textDoc());

	setAttribute(Qt::WA_DeleteOnClose, true);

	setContextMenuPolicy(Qt::NoContextMenu);

	makeUntitled();
	hideConsole();
	keepConsoleOpen = false;
	connect(consoleTabs, &Tw::UI::ClosableTabWidget::requestClose, actionShow_Hide_Console, &QAction::trigger);

	statusBar()->addPermanentWidget(lineEndingLabel = new Tw::UI::ClickableLabel());
	lineEndingLabel->setFrameStyle(QFrame::StyledPanel);
	lineEndingLabel->setFont(statusBar()->font());
	connect(lineEndingLabel, &Tw::UI::ClickableLabel::mouseLeftClick, this, &TeXDocumentWindow::lineEndingLabelClick);
	showLineEndingSetting();

	statusBar()->addPermanentWidget(encodingLabel = new Tw::UI::ClickableLabel());
	encodingLabel->setFrameStyle(QFrame::StyledPanel);
	encodingLabel->setFont(statusBar()->font());
	connect(encodingLabel, &Tw::UI::ClickableLabel::mouseLeftClick, this, &TeXDocumentWindow::encodingLabelClick);
	showEncodingSetting();

	statusBar()->addPermanentWidget(lineNumberLabel = new Tw::UI::ClickableLabel());
	lineNumberLabel->setFrameStyle(QFrame::StyledPanel);
	lineNumberLabel->setFont(statusBar()->font());
	connect(lineNumberLabel, &Tw::UI::ClickableLabel::mouseLeftClick, this, &TeXDocumentWindow::doLineDialog);
	showCursorPosition();

	engineActions = new QActionGroup(this);
	connect(engineActions, &QActionGroup::triggered, this, static_cast<void (TeXDocumentWindow::*)(QAction*)>(&TeXDocumentWindow::selectedEngine));

	codec = TWApp::instance()->getDefaultCodec();
	engineName = TWApp::instance()->getDefaultEngine().name();
	engine = new QComboBox(this);
	engine->setEditable(false);
	engine->setFocusPolicy(Qt::NoFocus);
	engine->setSizeAdjustPolicy(QComboBox::AdjustToContents);
#if defined(Q_OS_DARWIN)
	engine->setStyleSheet(QString::fromLatin1("padding:4px;"));
	engine->setMinimumWidth(150);
#endif
	toolBar_run->addWidget(engine);
	updateEngineList();
	connect(engine, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, static_cast<void (TeXDocumentWindow::*)(int)>(&TeXDocumentWindow::selectedEngine));

	connect(TWApp::instance(), &TWApp::engineListChanged, this, &TeXDocumentWindow::updateEngineList);

	connect(actionNew, &QAction::triggered, this, &TeXDocumentWindow::newFile);
	connect(actionNew_from_Template, &QAction::triggered, this, &TeXDocumentWindow::newFromTemplate);
	connect(actionOpen, &QAction::triggered, this, static_cast<void (TeXDocumentWindow::*)()>(&TeXDocumentWindow::open));
	connect(actionAbout_TW, &QAction::triggered, TWApp::instance(), &TWApp::about);
	connect(actionSettings_and_Resources, &QAction::triggered, TWApp::instance(), &TWApp::doResourcesDialog);
	connect(actionGoToHomePage, &QAction::triggered, TWApp::instance(), &TWApp::goToHomePage);
	connect(actionWriteToMailingList, &QAction::triggered, TWApp::instance(), &TWApp::writeToMailingList);

	connect(actionSave, &QAction::triggered, this, &TeXDocumentWindow::save);
	connect(actionSave_As, &QAction::triggered, this, &TeXDocumentWindow::saveAs);
	connect(actionSave_All, &QAction::triggered, this, &TeXDocumentWindow::saveAll);
	connect(actionRevert_to_Saved, &QAction::triggered, this, &TeXDocumentWindow::revert);
	connect(actionClose, &QAction::triggered, this, &TeXDocumentWindow::close);

	connect(actionRemove_Aux_Files, &QAction::triggered, this, &TeXDocumentWindow::removeAuxFiles);

	connect(actionQuit_TeXworks, &QAction::triggered, TWApp::instance(), &TWApp::maybeQuit);

	connect(actionClear, &QAction::triggered, this, &TeXDocumentWindow::clear);

	connect(actionFont, &QAction::triggered, this, &TeXDocumentWindow::doFontDialog);
	connect(actionGo_to_Line, &QAction::triggered, this, &TeXDocumentWindow::doLineDialog);
	connect(actionFind, &QAction::triggered, this, &TeXDocumentWindow::doFindDialog);
	connect(actionFind_Again, &QAction::triggered, this, &TeXDocumentWindow::doFindAgain);
	connect(actionReplace, &QAction::triggered, this, &TeXDocumentWindow::doReplaceDialog);
	connect(actionReplace_Again, &QAction::triggered, this, &TeXDocumentWindow::doReplaceAgain);

	connect(actionCopy_to_Find, &QAction::triggered, this, &TeXDocumentWindow::copyToFind);
	connect(actionCopy_to_Replace, &QAction::triggered, this, &TeXDocumentWindow::copyToReplace);
	connect(actionFind_Selection, &QAction::triggered, this, &TeXDocumentWindow::findSelection);

	connect(actionShow_Selection, &QAction::triggered, this, &TeXDocumentWindow::showSelection);

	connect(actionIndent, &QAction::triggered, this, &TeXDocumentWindow::doIndent);
	connect(actionUnindent, &QAction::triggered, this, &TeXDocumentWindow::doUnindent);

	connect(actionComment, &QAction::triggered, this, &TeXDocumentWindow::doComment);
	connect(actionUncomment, &QAction::triggered, this, &TeXDocumentWindow::doUncomment);

	connect(actionHard_Wrap, &QAction::triggered, this, &TeXDocumentWindow::doHardWrapDialog);
	connect(actionInsert_Citations, &QAction::triggered, this, &TeXDocumentWindow::doInsertCitationsDialog);

	connect(actionTo_Uppercase, &QAction::triggered, this, &TeXDocumentWindow::toUppercase);
	connect(actionTo_Lowercase, &QAction::triggered, this, &TeXDocumentWindow::toLowercase);
	connect(actionToggle_Case, &QAction::triggered, this, &TeXDocumentWindow::toggleCase);

	connect(actionBalance_Delimiters, &QAction::triggered, this, &TeXDocumentWindow::balanceDelimiters);

	connect(textDoc(), &Tw::Document::TeXDocument::modificationChanged, this, &TeXDocumentWindow::setWindowModified);
	connect(textDoc(), &Tw::Document::TeXDocument::modificationChanged, this, &TeXDocumentWindow::maybeEnableSaveAndRevert);
	connect(textDoc(), &Tw::Document::TeXDocument::modelinesChanged, this, &TeXDocumentWindow::handleModelineChange);
	connect(textEdit, &CompletingEdit::cursorPositionChanged, this, &TeXDocumentWindow::showCursorPosition);
	connect(textEdit, &CompletingEdit::selectionChanged, this, &TeXDocumentWindow::showCursorPosition);
	connect(textEdit, &CompletingEdit::syncClick, this, &TeXDocumentWindow::syncClick);
	connect(this, &TeXDocumentWindow::syncFromSource, TWApp::instance(), &TWApp::syncPdf);

	connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &TeXDocumentWindow::clipboardChanged);
	clipboardChanged();

	connect(actionTypeset, &QAction::triggered, this, &TeXDocumentWindow::typeset);

	updateRecentFileActions();
	connect(TWApp::instance(), &TWApp::recentFileActionsChanged, this, &TeXDocumentWindow::updateRecentFileActions);
	connect(TWApp::instance(), &TWApp::windowListChanged, this, &TeXDocumentWindow::updateWindowMenu);
	connect(actionClear_Recent_Files, &QAction::triggered, TWApp::instance(), &TWApp::clearRecentFiles);

	connect(TWApp::instance(), &TWApp::hideFloatersExcept, this, &TeXDocumentWindow::hideFloatersUnlessThis);
	connect(this, &TeXDocumentWindow::activatedWindow, TWApp::instance(), &TWApp::activatedWindow);

	connect(&(TWApp::instance()->typesetManager()), &Tw::Utils::TypesetManager::typesettingStarted, this, &TeXDocumentWindow::updateTypesettingAction);
	connect(&(TWApp::instance()->typesetManager()), &Tw::Utils::TypesetManager::typesettingStopped, this, &TeXDocumentWindow::updateTypesettingAction);
	connect(&(TWApp::instance()->typesetManager()), &Tw::Utils::TypesetManager::typesettingStarted, this, &TeXDocumentWindow::conditionallyEnableRemoveAuxFiles);
	connect(&(TWApp::instance()->typesetManager()), &Tw::Utils::TypesetManager::typesettingStopped, this, &TeXDocumentWindow::conditionallyEnableRemoveAuxFiles);

	connect(actionStack, &QAction::triggered, TWApp::instance(), &TWApp::stackWindows);
	connect(actionTile, &QAction::triggered, TWApp::instance(), &TWApp::tileWindows);
	connect(actionSide_by_Side, &QAction::triggered, this, &TeXDocumentWindow::sideBySide);
	connect(actionPlace_on_Left, &QAction::triggered, this, &TeXDocumentWindow::placeOnLeft);
	connect(actionPlace_on_Right, &QAction::triggered, this, &TeXDocumentWindow::placeOnRight);
	connect(actionShow_Hide_Console, &QAction::triggered, this, &TeXDocumentWindow::toggleConsoleVisibility);
	connect(actionGo_to_Preview, &QAction::triggered, this, &TeXDocumentWindow::goToPreview);

	connect(this, &TeXDocumentWindow::destroyed, TWApp::instance(), &TWApp::updateWindowMenus);

	connect(actionPreferences, &QAction::triggered, TWApp::instance(), &TWApp::preferences);

	connect(menuEdit, &QMenu::aboutToShow, this, &TeXDocumentWindow::editMenuAboutToShow);

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
	// Work around QTBUG-36281
	textEdit->installEventFilter(Tw::Utils::CmdKeyFilter::filter());
#endif

	connect(inputLine, &QLineEdit::returnPressed, this, &TeXDocumentWindow::acceptInputLine);

	Tw::Settings settings;
	TWUtils::applyToolbarOptions(this, settings.value(QString::fromLatin1("toolBarIconSize"), 2).toInt(), settings.value(QString::fromLatin1("toolBarShowText"), false).toBool());

	QFont font = textEdit->font();
	if (settings.contains(QString::fromLatin1("font"))) {
		QString fontString = settings.value(QString::fromLatin1("font")).toString();
		if (!fontString.isEmpty()) {
			font.fromString(fontString);
			textEdit->setFont(font);
		}
	}
	font.setPointSize(font.pointSize() - 1);
	inputLine->setFont(font);
	inputLine->setLayoutDirection(Qt::LeftToRight);
	textEdit_console->setFont(font);
	textEdit_console->setLayoutDirection(Qt::LeftToRight);

	setLineSpacing(settings.value(QStringLiteral("lineSpacing"), kDefault_LineSpacing).toReal());

	bool b = settings.value(QString::fromLatin1("wrapLines"), true).toBool();
	actionWrap_Lines->setChecked(b);
	setWrapLines(b);

	b = settings.value(QString::fromLatin1("lineNumbers"), false).toBool();
	actionLine_Numbers->setChecked(b);
	setLineNumbers(b);

	actionAuto_Follow_Focus->setChecked(settings.value(QStringLiteral("autoFollowFocusEnabled"), kDefault_AutoFollowFocusEnabled).toBool());

	QStringList options = TeXHighlighter::syntaxOptions();

	QSignalMapper *syntaxMapper = new QSignalMapper(this);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	connect(syntaxMapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped), this, &TeXDocumentWindow::setSyntaxColoring);
#else
	connect(syntaxMapper, &QSignalMapper::mappedInt, this, &TeXDocumentWindow::setSyntaxColoring);
#endif
	syntaxMapper->setMapping(actionSyntaxColoring_None, -1);
	connect(actionSyntaxColoring_None, &QAction::triggered, syntaxMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));

	QActionGroup *syntaxGroup = new QActionGroup(this);
	syntaxGroup->addAction(actionSyntaxColoring_None);

	int index = 0;
	foreach (const QString& opt, options) {
		QAction *action = menuSyntax_Coloring->addAction(opt, syntaxMapper, SLOT(map()));
		action->setCheckable(true);
		syntaxGroup->addAction(action);
		syntaxMapper->setMapping(action, index);
		++index;
	}

	// kDefault_TabWidth is defined in DefaultPrefs.h
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
	textEdit->setTabStopWidth(settings.value(QString::fromLatin1("tabWidth"), kDefault_TabWidth).toInt());
	textEdit_console->setTabStopWidth(settings.value(QString::fromLatin1("tabWidth"), kDefault_TabWidth).toInt());
#else
	textEdit->setTabStopDistance(settings.value(QString::fromLatin1("tabWidth"), kDefault_TabWidth).toReal());
	textEdit_console->setTabStopDistance(settings.value(QString::fromLatin1("tabWidth"), kDefault_TabWidth).toReal());
#endif

	// It is VITAL that this connection is queued! Calling showMessage directly
	// from TeXDocument::contentsChanged would otherwise result in a seg fault
	// (for whatever reason)
	connect(this, &TeXDocumentWindow::asyncFlashStatusBarMessage, statusBar(), &QStatusBar::showMessage, Qt::QueuedConnection);

	QString indentOption = settings.value(QString::fromLatin1("autoIndent")).toString();
	options = CompletingEdit::autoIndentModes();

	QSignalMapper *indentMapper = new QSignalMapper(this);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	connect(indentMapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped), textEdit, &CompletingEdit::setAutoIndentMode);
#else
	connect(indentMapper, &QSignalMapper::mappedInt, textEdit, &CompletingEdit::setAutoIndentMode);
#endif
	indentMapper->setMapping(actionAutoIndent_None, -1);
	connect(actionAutoIndent_None, &QAction::triggered, indentMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));

	QActionGroup *indentGroup = new QActionGroup(this);
	indentGroup->addAction(actionAutoIndent_None);

	index = 0;
	foreach (const QString& opt, options) {
		QAction *action = menuAuto_indent_Mode->addAction(opt, indentMapper, SLOT(map()));
		action->setCheckable(true);
		indentGroup->addAction(action);
		indentMapper->setMapping(action, index);
		if (opt == indentOption) {
			action->setChecked(true);
			textEdit->setAutoIndentMode(index);
		}
		++index;
	}

	QString quotesOption = settings.value(QString::fromLatin1("smartQuotes")).toString();
	options = CompletingEdit::smartQuotesModes();

	QSignalMapper *quotesMapper = new QSignalMapper(this);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	connect(quotesMapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped), textEdit, &CompletingEdit::setSmartQuotesMode);
#else
	connect(quotesMapper, &QSignalMapper::mappedInt, textEdit, &CompletingEdit::setSmartQuotesMode);
#endif
	quotesMapper->setMapping(actionSmartQuotes_None, -1);
	connect(actionSmartQuotes_None, &QAction::triggered, quotesMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));

	QActionGroup *quotesGroup = new QActionGroup(this);
	quotesGroup->addAction(actionSmartQuotes_None);

	menuSmart_Quotes_Mode->removeAction(actionApply_to_Selection);
	index = 0;
	foreach (const QString& opt, options) {
		QAction *action = menuSmart_Quotes_Mode->addAction(opt, quotesMapper, SLOT(map()));
		action->setCheckable(true);
		quotesGroup->addAction(action);
		quotesMapper->setMapping(action, index);
		if (opt == quotesOption) {
			action->setChecked(true);
			textEdit->setSmartQuotesMode(index);
		}
		++index;
	}
	if (!options.empty())
		menuSmart_Quotes_Mode->addSeparator();
	menuSmart_Quotes_Mode->addAction(actionApply_to_Selection);
	connect(actionApply_to_Selection, &QAction::triggered, textEdit, &CompletingEdit::smartenQuotes);

	connect(actionLine_Numbers, &QAction::triggered, this, &TeXDocumentWindow::setLineNumbers);
	connect(actionWrap_Lines, &QAction::triggered, this, &TeXDocumentWindow::setWrapLines);

	connect(actionNone, &QAction::triggered, &dictSignalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
	dictSignalMapper.setMapping(actionNone, QString());
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	connect(&dictSignalMapper, static_cast<void (QSignalMapper::*)(const QString&)>(&QSignalMapper::mapped), this, &TeXDocumentWindow::setLangInternal);
#else
	connect(&dictSignalMapper, &QSignalMapper::mappedString, this, &TeXDocumentWindow::setLangInternal);
#endif

	QActionGroup *group = new QActionGroup(this);
	group->addAction(actionNone);

	reloadSpellcheckerMenu();
	connect(Tw::Document::SpellChecker::instance(), &Tw::Document::SpellChecker::dictionaryListChanged, this, &TeXDocumentWindow::reloadSpellcheckerMenu);

	menuShow->addAction(toolBar_run->toggleViewAction());
	menuShow->addAction(toolBar_edit->toggleViewAction());
	menuShow->addSeparator();

	Tw::Utils::WindowManager::zoomToHalfScreen(this);

	QDockWidget *dw = new TagsDock(this);
	dw->hide();
	addDockWidget(Qt::LeftDockWidgetArea, dw);
	menuShow->addAction(dw->toggleViewAction());

	watcher = new QFileSystemWatcher(this);
	connect(watcher, &QFileSystemWatcher::fileChanged, this, &TeXDocumentWindow::reloadIfChangedOnDisk, Qt::QueuedConnection);
	connect(watcher, &QFileSystemWatcher::directoryChanged, this, &TeXDocumentWindow::reloadIfChangedOnDisk, Qt::QueuedConnection);

	docList.append(this);

	TWApp::instance()->updateWindowMenus();

	initScriptable(menuScripts, actionAbout_Scripts, actionManage_Scripts,
				   actionUpdate_Scripts, actionShow_Scripts_Folder);

	TWUtils::insertHelpMenuItems(menuHelp);
#if defined(MIKTEX)
        actionAbout_MiKTeX = new QAction(this);
        actionAbout_MiKTeX->setIcon(QIcon(QStringLiteral(":/MiKTeX/miktex16x16.png")));
        actionAbout_MiKTeX->setObjectName(QStringLiteral("actionAbout_MiKTeX"));
        actionAbout_MiKTeX->setText(QApplication::translate("TeXDocument", "Apropos MiKTeX..."));
        connect(actionAbout_MiKTeX, SIGNAL(triggered()), qApp, SLOT(aboutMiKTeX()));
#if 1
        menuHelp->addAction(actionAbout_MiKTeX);
#endif
#endif
	TWUtils::installCustomShortcuts(this);
	delayedInit();
}

void TeXDocumentWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		QString title = windowTitle();
		retranslateUi(this);
		TWUtils::insertHelpMenuItems(menuHelp);
		setWindowTitle(title);
		showCursorPosition();
	}
	else if (event->type() == QEvent::ActivationChange) {
		// If this window was activated, inform the linked pdf (if any) of it
		// so that future "Goto Source" actions point here.
		if (this == QApplication::activeWindow() && pdfDoc)
			pdfDoc->texActivated(this);
	}
	QMainWindow::changeEvent(event);
}

void TeXDocumentWindow::setLangInternal(const QString& lang)
{
	if (_texDoc == nullptr)
		return;

	TeXHighlighter * highlighter = _texDoc->getHighlighter();
	if (highlighter == nullptr)
		return;

	// called internally by the spelling menu actions;
	// not for use from scripts as it won't update the menu
	Tw::Document::SpellChecker::Dictionary * oldDictionary = highlighter->getSpellChecker();
	Tw::Document::SpellChecker::Dictionary * newDictionary = Tw::Document::SpellChecker::getDictionary(lang);
	// if the dictionary hasn't change, don't reset the spell checker as that
	// can result in a serious delay for long documents
	// NB: Don't delete the dictionaries; the pointers are kept by
	// Tw::Document::SpellChecker
	if (oldDictionary == newDictionary)
		return;

	highlighter->setSpellChecker(newDictionary);
}

void TeXDocumentWindow::setSpellcheckLanguage(const QString& lang)
{
	// this is called by the %!TEX spellcheck... line, or by scripts;
	// it searches the menu for the given language code, and triggers it if available

	// Determine all aliases for the specified lang
	QList<QString> langAliases;
	foreach (const QString& dictKey, Tw::Document::SpellChecker::getDictionaryList()->uniqueKeys()) {
		if(Tw::Document::SpellChecker::getDictionaryList()->values(dictKey).contains(lang))
			langAliases += Tw::Document::SpellChecker::getDictionaryList()->values(dictKey);
	}
	langAliases.removeAll(lang);
	langAliases.prepend(lang);

	if (menuSpelling) {
		QAction *chosen = menuSpelling->actions()[0]; // default is None
		foreach(QString alias, langAliases) {
			bool found = false;
			foreach (QAction *act, menuSpelling->actions()) {
				if (act->text() == alias || act->text().contains(QString::fromLatin1("(%1)").arg(alias))) {
					chosen = act;
					found = true;
					break;
				}
			}
			if(found) break;
		}
		chosen->trigger();
	}
}

QString TeXDocumentWindow::spellcheckLanguage() const
{
	if (_texDoc == nullptr)
		return QString();
	TeXHighlighter * highlighter = _texDoc->getHighlighter();
	if (highlighter == nullptr)
		return QString();
	Tw::Document::SpellChecker::Dictionary * dictionary = highlighter->getSpellChecker();
	if (dictionary == nullptr) return QString();
	return dictionary->getLanguage();
}

void TeXDocumentWindow::reloadSpellcheckerMenu()
{
	Q_ASSERT(menuSpelling);
	Q_ASSERT(!menuSpelling->actions().empty());

	QActionGroup * group = menuSpelling->actions()[0]->actionGroup();
	Q_ASSERT(group);

	// Remove all but the first menu item ("None") from the action group
	int i = 0;
	QString oldSelected;
	foreach (QAction * act, group->actions()) {
		if (act->isChecked())
			oldSelected = act->text();
		if (i > 0) {
			group->removeAction(act);
			act->deleteLater();
		}
		++i;
	}

	QList<QAction*> dictActions;
	foreach (const QString& dictKey, Tw::Document::SpellChecker::getDictionaryList()->uniqueKeys()) {
		foreach (QString dict, Tw::Document::SpellChecker::getDictionaryList()->values(dictKey)) {
			const QString label{Tw::Document::SpellChecker::labelForDict(dict)};

			QAction * act = new QAction(label, menuSpelling);
			act->setCheckable(true);
			if (!oldSelected.isEmpty() && label == oldSelected)
				act->setChecked(true);
			connect(act, &QAction::triggered, &dictSignalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
			dictSignalMapper.setMapping(act, dict);
			group->addAction(act);
			dictActions << act;
		}
	}
	std::sort(dictActions.begin(), dictActions.end(), dictActionLessThan);
	foreach (QAction* dictAction, dictActions)
		menuSpelling->addAction(dictAction);
}

void TeXDocumentWindow::clipboardChanged()
{
	actionPaste->setEnabled(textEdit->canPaste());
}

void TeXDocumentWindow::editMenuAboutToShow()
{
//	undoAction->setText(tr("Undo ") + undoStack->undoText());
//	redoAction->setText(tr("Redo ") + undoStack->redoText());
	actionSelect_All->setEnabled(!textEdit->document()->isEmpty());
}

void TeXDocumentWindow::newFile()
{
	TeXDocumentWindow *doc = new TeXDocumentWindow;
	doc->selectWindow();
	doc->textEdit->updateLineNumberAreaWidth(0);
	doc->runHooks(QString::fromLatin1("NewFile"));
}

void TeXDocumentWindow::newFromTemplate()
{
	QString templateName = TemplateDialog::doTemplateDialog();
	if (!templateName.isEmpty()) {
		TeXDocumentWindow *doc = nullptr;
		if (untitled() && textEdit->document()->isEmpty() && !isWindowModified()) {
			loadFile(QFileInfo(templateName), true);
			doc = this;
		}
		else {
			doc = new TeXDocumentWindow(templateName, true);
		}
		if (doc) {
			doc->makeUntitled();
			doc->selectWindow();
			doc->textEdit->updateLineNumberAreaWidth(0);
			doc->runHooks(QString::fromLatin1("NewFromTemplate"));
		}
	}
}

void TeXDocumentWindow::makeUntitled()
{
	setCurrentFile({});
	conditionallyEnableRemoveAuxFiles();
}

void TeXDocumentWindow::open()
{
	QFileDialog::Options options = QFileDialog::DontResolveSymlinks;
#if defined(Q_OS_WIN)
	if(TWApp::GetWindowsVersion() < 0x06000000) options |= QFileDialog::DontUseNativeDialog;
#endif
	Tw::Settings settings;
	QString lastOpenDir = settings.value(QString::fromLatin1("openDialogDir")).toString();
	if (lastOpenDir.isEmpty())
		lastOpenDir = QDir::homePath();
	QStringList files = QFileDialog::getOpenFileNames(this, QString(tr("Open File")), lastOpenDir, TWUtils::filterList()->join(QLatin1String(";;")), nullptr, options);
	foreach (QString fileName, files) {
		if (!fileName.isEmpty()) {
			TWApp::instance()->openFile(fileName); // not TeXDocument::open() - give the app a chance to open as PDF
		}
	}
}

TeXDocumentWindow* TeXDocumentWindow::open(const QString &fileName)
{
	TeXDocumentWindow *doc = nullptr;
	if (!fileName.isEmpty()) {
		doc = findDocument(fileName);
		if (!doc) {
			if (untitled() && textEdit->document()->isEmpty() && !isWindowModified()) {
				loadFile(QFileInfo(fileName));
				doc = this;
			}
			else {
				doc = new TeXDocumentWindow(fileName);
				if (doc->untitled()) {
					delete doc;
					doc = nullptr;
				}
			}
		}
	}
	if (doc)
		doc->selectWindow();
	return doc;
}

TeXDocumentWindow* TeXDocumentWindow::openDocument(const QString &fileName, bool activate, bool raiseWindow, int lineNo, int selStart, int selEnd) // static
{
	TeXDocumentWindow *doc = findDocument(fileName);
	if (!doc) {
		if (docList.count() == 1) {
			doc = docList[0];
			doc = doc->open(fileName); // open into existing window if untitled/empty
		}
		else {
			doc = new TeXDocumentWindow(fileName);
			if (doc->untitled()) {
				delete doc;
				doc = nullptr;
			}
		}
	}
	if (doc) {
		if (activate)
			doc->selectWindow();
		else {
			doc->show();
			if (raiseWindow) {
				doc->raise();
				if (doc->isMinimized())
					doc->showNormal();
			}
		}
		if (lineNo > 0)
			doc->goToLine(lineNo, selStart, selEnd);
	}
	return doc;
}

void TeXDocumentWindow::closeEvent(QCloseEvent *event)
{
	if (process) {
		if (QMessageBox::question(this, tr("Abort typesetting?"), tr("A typesetting process is still running and must be stopped before closing this window.\nDo you want to stop it now?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) {
			event->ignore();
			return;
		}
		interrupt();
		// Wait for the process to actually finish (and be destroyed) as this
		// might try to access, e.g., the log window (which could be destroyed
		// at any time once the close event goes through and this
		// TeXDocumentWindow is started to be destroyed)
		if (process) {
			process->waitForFinished();
		}
	}

	if (maybeSave()) {
		event->accept();
		saveRecentFileInfo();
		deleteLater();
	}
	else
		event->ignore();
}

bool TeXDocumentWindow::event(QEvent *event) // based on example at doc.trolltech.com/qq/qq18-macfeatures.html
{
	switch (event->type()) {
		case QEvent::IconDrag:
			if (isActiveWindow()) {
				event->accept();
				Qt::KeyboardModifiers mods = qApp->keyboardModifiers();
				if (mods == Qt::NoModifier) {
					QDrag *drag = new QDrag(this);
					QMimeData *data = new QMimeData();
					data->setUrls(QList<QUrl>() << QUrl::fromLocalFile(textDoc()->absoluteFilePath()));
					drag->setMimeData(data);
					QPixmap dragIcon(QString::fromLatin1(":/images/images/TeXworks-doc-48.png"));
					drag->setPixmap(dragIcon);
					drag->setHotSpot(QPoint(dragIcon.width() - 5, 5));
					drag->exec(Qt::LinkAction | Qt::CopyAction);
				}
				else if (mods == Qt::ShiftModifier) {
					QMenu menu(this);
					connect(&menu, &QMenu::triggered, this, &TeXDocumentWindow::openAt);
					QFileInfo info(textDoc()->getFileInfo());
					QAction *action = menu.addAction(info.fileName());
					action->setIcon(QIcon(QString::fromLatin1(":/images/images/TeXworks-doc.png")));
					QStringList folders = info.absolutePath().split(QChar::fromLatin1('/'));
					QStringListIterator it(folders);
					it.toBack();
					while (it.hasPrevious()) {
						QString str = it.previous();
						QIcon icon;
						if (!str.isEmpty()) {
							icon = style()->standardIcon(QStyle::SP_DirClosedIcon, nullptr, this);
						}
						else {
							str = QChar::fromLatin1('/');
							icon = style()->standardIcon(QStyle::SP_DriveHDIcon, nullptr, this);
						}
						action = menu.addAction(str);
						action->setIcon(icon);
#if defined(Q_OS_DARWIN)
						action->setIconVisibleInMenu(true);
#endif
					}
					QPoint pos(QCursor::pos().x() - 20, frameGeometry().y());
					menu.exec(pos);
				}
				else {
					event->ignore();
				}
				return true;
			}
			break;

		case QEvent::WindowActivate:
			showFloaters();
			emit activatedWindow(this);
			break;

		default:
			break;
	}
	return QMainWindow::event(event);
}

void TeXDocumentWindow::openAt(QAction *action)
{
	QString curFile = textDoc()->getFileInfo().filePath();
	QString path = curFile.left(curFile.indexOf(action->text())) + action->text();
	if (path == curFile)
		return;
	QProcess proc;
	proc.start(QString::fromLatin1("/usr/bin/open"), QStringList() << path, QIODevice::ReadOnly);
	proc.waitForFinished();
}

bool TeXDocumentWindow::save()
{
	if (untitled())
		return saveAs();
	return saveFile(QFileInfo(textDoc()->absoluteFilePath()));
}

bool TeXDocumentWindow::saveAll()
{
	bool savedAll = true;
	foreach (TeXDocumentWindow* doc, docList) {
		if (doc->textEdit->document()->isModified()) {
			if (!doc->save()) {
				savedAll = false;
			}
		}
	}
	return savedAll;
}

bool TeXDocumentWindow::saveAs()
{
	QFileDialog::Options options;
#if defined(Q_OS_WIN)
	if(TWApp::GetWindowsVersion() < 0x06000000) options |= QFileDialog::DontUseNativeDialog;
#endif
	QString selectedFilter = TWUtils::chooseDefaultFilter(textDoc()->absoluteFilePath(), *(TWUtils::filterList()));;

	// for untitled docs, default to the last dir used, or $HOME if no saved value
	Tw::Settings settings;
	QString lastSaveDir = settings.value(QString::fromLatin1("saveDialogDir")).toString();
	if (lastSaveDir.isEmpty() || !QDir(lastSaveDir).exists())
		lastSaveDir = QDir::homePath();
	QString suggestedDir = (textDoc()->isStoredInFilesystem() ? textDoc()->absoluteFilePath() : QDir(lastSaveDir).filePath(textDoc()->getFileInfo().filePath()));
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
													suggestedDir,
	                                                TWUtils::filterList()->join(QLatin1String(";;")),
													&selectedFilter, options);
	if (fileName.isEmpty())
		return false;

	// save the old document in "Recent Files"
	saveRecentFileInfo();

	// add extension from the selected filter, if unique and not already present
	QRegularExpression re(QStringLiteral("\\(\\*(\\.[^ *]+)\\)"));
	QRegularExpressionMatch m = re.match(selectedFilter);
	if (m.capturedStart() >= 0) {
		QString ext = m.captured(1);
		if (!fileName.endsWith(ext, Qt::CaseInsensitive) && !fileName.endsWith(QChar::fromLatin1('.')))
			fileName.append(ext);
	}

	QFileInfo info(fileName);
	if (info != textDoc()->getFileInfo() && pdfDoc) {
		// For the pdf, it is as if it's source doc was closed
		// Note that this may result in the pdf being closed!
		pdfDoc->texClosed(this);
		// The pdf connection is no longer (necessarily) valid. Detach it for
		// now (the correct connection will be reestablished on next typeset).
		detachPdf();
	}

	settings.setValue(QString::fromLatin1("saveDialogDir"), info.absolutePath());

	return saveFile(QFileInfo(fileName));
}

bool TeXDocumentWindow::maybeSave()
{
	if (textEdit->document()->isModified()) {
		QMessageBox msgBox(QMessageBox::Warning, QCoreApplication::applicationName(),
						   tr("The document \"%1\" has been modified.\n"
							  "Do you want to save your changes?")
						   .arg(textDoc()->getFileInfo().fileName()),
						   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
						   this);
		msgBox.button(QMessageBox::Discard)->setShortcut(QKeySequence(tr("Ctrl+D", "shortcut: Don't Save")));
		msgBox.setWindowModality(Qt::WindowModal);
		QMessageBox::StandardButton ret = static_cast<QMessageBox::StandardButton>(msgBox.exec());
		if (ret == QMessageBox::Save)
			return save();
		if (ret == QMessageBox::Cancel)
			return false;
	}
	return true;
}

bool TeXDocumentWindow::saveFilesHavingRoot(const QString& aRootFile)
{
	foreach (TeXDocumentWindow* doc, docList) {
		if (doc->textDoc() && doc->textDoc()->getRootFilePath() == aRootFile) {
			if (doc->textEdit->document()->isModified() && !doc->save())
				return false;
		}
	}
	return true;
}

void TeXDocumentWindow::revert()
{
	if (!untitled()) {
		QMessageBox messageBox(QMessageBox::Warning, QCoreApplication::applicationName(),
					tr("Do you want to discard all changes to the document \"%1\", and revert to the last saved version?")
					   .arg(textDoc()->getFileInfo().fileName()), QMessageBox::Cancel, this);
		QAbstractButton *revertButton = messageBox.addButton(tr("Revert"), QMessageBox::DestructiveRole);
		revertButton->setShortcut(QKeySequence(tr("Ctrl+R", "shortcut: Revert")));
		messageBox.setDefaultButton(QMessageBox::Cancel);
		messageBox.setWindowModality(Qt::WindowModal);
		messageBox.exec();
		if (messageBox.clickedButton() == revertButton)
			loadFile(QFileInfo(textDoc()->absoluteFilePath()));
	}
}

void TeXDocumentWindow::maybeEnableSaveAndRevert(bool modified)
{
	actionSave->setEnabled(modified || untitled());
	actionRevert_to_Saved->setEnabled(modified && !untitled());
}

static const char* texshopSynonyms[] = {
	"MacOSRoman",		"Apple Roman",
	"IsoLatin",			"ISO 8859-1",
	"IsoLatin2",		"ISO 8859-2",
	"IsoLatin5",		"ISO 8859-5",
	"IsoLatin9",		"ISO 8859-9",
//	"MacJapanese",		"",
//	"DOSJapanese",		"",
	"SJIS_X0213",		"Shift-JIS",
	"EUC_JP",			"EUC-JP",
//	"JISJapanese",		"",
//	"MacKorean",		"",
	"UTF-8 Unicode",	"UTF-8",
	"Standard Unicode",	"UTF-16",
//	"Mac Cyrillic",		"",
//	"DOS Cyrillic",		"",
//	"DOS Russian",		"",
	"Windows Cyrillic",	"Windows-1251",
	"KOI8_R",			"KOI8-R",
//	"Mac Chinese Traditional",	"",
//	"Mac Chinese Simplified",	"",
//	"DOS Chinese Traditional",	"",
//	"DOS Chinese Simplified",	"",
//	"GBK",				"",
//	"GB 2312",			"",
	"GB 18030",			"GB18030-0",
	nullptr
};

QTextCodec *TeXDocumentWindow::scanForEncoding(const QString &peekStr, bool &hasMetadata, QString &reqName)
{
	// peek at the file for %!TEX encoding = ....
	QRegularExpression re(QStringLiteral(u"% *!TEX +encoding *= *([^\r\n\x2029]+)[\r\n\x2029]"), QRegularExpression::CaseInsensitiveOption);
	QRegularExpressionMatch m = re.match(peekStr);
	QTextCodec *reqCodec = nullptr;
	if (m.hasMatch()) {
		hasMetadata = true;
		reqName = m.captured(1).trimmed();
		reqCodec = QTextCodec::codecForName(reqName.toLatin1());
		if (!reqCodec) {
			static QHash<QString,QString> *synonyms = nullptr;
			if (!synonyms) {
				synonyms = new QHash<QString,QString>;
				for (int i = 0; texshopSynonyms[i]; i += 2)
					synonyms->insert(QString::fromLatin1(texshopSynonyms[i]).toLower(), QString::fromLatin1(texshopSynonyms[i+1]));
			}
			if (synonyms->contains(reqName.toLower()))
				reqCodec = QTextCodec::codecForName(synonyms->value(reqName.toLower()).toLatin1());
		}
	}
	else
		hasMetadata = false;
	return reqCodec;
}

#define PEEK_LENGTH 1024

QString TeXDocumentWindow::readFile(const QFileInfo & fileInfo,
							  QTextCodec **codecUsed,
							  int *lineEndings,
							  QTextCodec * forceCodec)
	// reads the text from a file, after checking for %!TEX encoding.... metadata
	// sets codecUsed to the QTextCodec used to read the text
	// returns a null (not just empty) QString on failure
{
	if (lineEndings) {
		// initialize to default for the platform
#if defined(Q_OS_WIN)
		*lineEndings = kLineEnd_CRLF;
#else
		*lineEndings = kLineEnd_LF;
#endif
	}

	utf8BOM = false;
	QFile file(fileInfo.absoluteFilePath());
	// Not using QFile::Text because this prevents us reading "classic" Mac files
	// with CR-only line endings. See issue #242.
	if (!file.open(QFile::ReadOnly)) {
		QMessageBox::warning(this, QCoreApplication::applicationName(),
							 tr("Cannot read file \"%1\":\n%2")
							 .arg(fileInfo.absoluteFilePath(), file.errorString()));
		return QString();
	}

	QByteArray peekBytes(file.peek(PEEK_LENGTH));

	QString reqName;
	if (forceCodec)
		*codecUsed = forceCodec;
	else {
		bool hasMetadata{false};
		*codecUsed = scanForEncoding(QString::fromUtf8(peekBytes.constData()), hasMetadata, reqName);
		if (!(*codecUsed)) {
			*codecUsed = TWApp::instance()->getDefaultCodec();
			if (hasMetadata) {
				if (QMessageBox::warning(this, tr("Unrecognized encoding"),
						tr("The text encoding %1 used in %2 is not supported.\n\n"
						   "It will be interpreted as %3 instead, which may result in incorrect text.")
							.arg(reqName, fileInfo.absoluteFilePath(), QString::fromUtf8((*codecUsed)->name().constData())),
						QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok) == QMessageBox::Cancel)
					return QString();
			}
		}
	}

	// When using the UTF-8 codec (mib = 106), byte order marks (BOMs) are
	// ignored during reading and not produced when writing. To keep them in
	// files that have them, we need to check for them ourselves.
	if ((*codecUsed)->mibEnum() == 106 && peekBytes.size() >= 3 && peekBytes[0] == '\xEF' && peekBytes[1] == '\xBB' && peekBytes[2] == '\xBF')
		utf8BOM = true;

	// If the file is empty (we're already at the end), don't try to read
	// anything using QTextStream below as that would return a Null-String
	// (QString()) rather than an empty string (QString("")). Instead, return
	// an empty string right away.
	if (file.atEnd())
		return QStringLiteral("");

	QString text = (*codecUsed)->toUnicode(file.readAll());

	if (lineEndings) {
		if (text.contains(QLatin1String("\r\n"))) {
			text.replace(QLatin1String("\r\n"), QChar::fromLatin1('\n'));
			*lineEndings = kLineEnd_CRLF;
		}
		else if (text.contains(QChar::fromLatin1('\r')) && !text.contains(QChar::fromLatin1('\n'))) {
			text.replace(QChar::fromLatin1('\r'), QChar::fromLatin1('\n'));
			*lineEndings = kLineEnd_CR;
		}
		else
			*lineEndings = kLineEnd_LF;

		if (text.contains(QChar::fromLatin1('\r'))) {
			text.replace(QChar::fromLatin1('\r'), QChar::fromLatin1('\n'));
			*lineEndings |= kLineEnd_Mixed;
		}
	}

	return text;
}

void TeXDocumentWindow::loadFile(const QFileInfo & fileInfo, bool asTemplate, bool inBackground, bool reload, QTextCodec * forceCodec)
{
	QString fileContents = readFile(fileInfo, &codec, &lineEndings, forceCodec);
	showLineEndingSetting();
	showEncodingSetting();

	if (fileContents.isNull())
		return;
	bool identicalContent{fileContents == textEdit->text()};

	// Only re-set the content if it has actually changed. Setting the content
	// has many side effects, e.g., destroying the undo/redo stack.
	if (!reload || !identicalContent) {
		QApplication::setOverrideCursor(Qt::WaitCursor);

		textEdit->setPlainText(fileContents);

		// Ensure the window is shown early (before setPlainText()).
		// - this ensures it is shown before the PDF (if opening a new doc)
		// - this avoids problems during layouting (which can be broken if the
		//   geometry, highlighting, ... is changed before the window is shown)
		if (!reload)
			show();
		QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

		{
			// Try to work around QTBUG-20354
			// It seems that adding additionalFormats (as is done automatically on
			// setPlainText() by the syntax highlighter) can disturb the layouting
			// process, leaving some blocks with size zero. This causes the
			// corresponding lines to "disappear" and can even crash the application
			// in connection with the "highlight current line" feature.
			QTextDocument * doc = textEdit->document();
			Q_ASSERT(doc);
			QAbstractTextDocumentLayout * docLayout = doc->documentLayout();
			Q_ASSERT(docLayout);

			int tries{0};
			for (tries = 0; tries < 10; ++tries) {
				bool isLayoutOK = true;
				for (QTextBlock b = doc->firstBlock(); b.isValid(); b = b.next()) {
					if (docLayout->	blockBoundingRect(b).isEmpty()) {
						isLayoutOK = false;
						break;
					}
				}
				if (isLayoutOK) break;
				// Re-setting the document content naturally triggers a relayout
				// (also a rehighlight). Note that layouting only works sensibly
				// once show() was called, or else there is no valid widget geometry
				// to act as bounding box.
#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0)
				doc->setPlainText(doc->toPlainText());
#else
				doc->setPlainText(doc->toRawText());
#endif
				QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
			}
			if (tries >= 10) {
				QMessageBox::warning(this, tr("Layout Problem"), tr("A problem occurred while laying out the loaded document in the editor. This is caused by an issue in the underlying Qt framework and can cause TeXworks to crash under certain circumstances. The symptoms of this problem are hidden or overlapping lines. To work around this, please try one of the following:\n -) Turn syntax highlighting off and on\n -) Turn line numbers off and on\n -) Resize the window\n\nWe are sorry for the inconvenience."));
			}
		}

		// Reset the line spacing as setPlainText() clears all text formatting
		setLineSpacing(m_lineSpacing);

		QApplication::restoreOverrideCursor();
	}

	if (asTemplate) {
		lastModified = QDateTime();
	}
	else {
		setCurrentFile(fileInfo);
		if (!reload) {
			Tw::Settings settings;
			if (!inBackground && settings.value(QString::fromLatin1("openPDFwithTeX"), kDefault_OpenPDFwithTeX).toBool()) {
				openPdfIfAvailable(false);
				// Note: openPdfIfAvailable() enables/disables actionGo_to_Preview
				// automatically.
			}
			else {
				QString previewFileName;
				actionGo_to_Preview->setEnabled(getPreviewFileName(previewFileName));
			}
			// set openDialogDir after openPdfIfAvailable as we want the .tex file's
			// path to end up in that variable (which might be touched/changed when
			// loading the pdf
			QFileInfo info(fileInfo);
			settings.setValue(QString::fromLatin1("openDialogDir"), info.canonicalPath());
		}

		statusBar()->showMessage(tr("File \"%1\" loaded").arg(textDoc()->getFileInfo().fileName()),
								 kStatusMessageDuration);
		setupFileWatcher();
	}
	maybeEnableSaveAndRevert(false);

	if (!reload) {
		bool autoPlace = true;
		QMap<QString,QVariant> properties = TWApp::instance()->getFileProperties(fileInfo.absoluteFilePath());
		if (properties.contains(QString::fromLatin1("geometry"))) {
			restoreGeometry(properties.value(QString::fromLatin1("geometry")).toByteArray());
			autoPlace = false;
		}
		if (properties.contains(QString::fromLatin1("state")))
			restoreState(properties.value(QString::fromLatin1("state")).toByteArray(), kTeXWindowStateVersion);

		if (properties.contains(QString::fromLatin1("selStart"))) {
			QTextCursor c(textEdit->document());
			c.setPosition(properties.value(QString::fromLatin1("selStart")).toInt());
			c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, properties.value(QString::fromLatin1("selLength"), 0).toInt());
			textEdit->setTextCursor(c);
		}

		if (properties.contains(QString::fromLatin1("quotesMode")))
			setSmartQuotesMode(properties.value(QString::fromLatin1("quotesMode")).toString());
		if (properties.contains(QString::fromLatin1("indentMode")))
			setAutoIndentMode(properties.value(QString::fromLatin1("indentMode")).toString());
		if (properties.contains(QString::fromLatin1("syntaxMode")))
			setSyntaxColoringMode(properties.value(QString::fromLatin1("syntaxMode")).toString());
		if (properties.contains(QString::fromLatin1("wrapLines")))
			setWrapLines(properties.value(QString::fromLatin1("wrapLines")).toBool());
		if (properties.contains(QString::fromLatin1("lineNumbers")))
			setLineNumbers(properties.value(QString::fromLatin1("lineNumbers")).toBool());

		if (pdfDoc) {
			if (properties.contains(QString::fromLatin1("pdfgeometry"))) {
				pdfDoc->restoreGeometry(properties.value(QString::fromLatin1("pdfgeometry")).toByteArray());
				autoPlace = false;
			}
			if (properties.contains(QString::fromLatin1("pdfstate")))
				pdfDoc->restoreState(properties.value(QString::fromLatin1("pdfstate")).toByteArray(), kPDFWindowStateVersion);
		}

		if (autoPlace)
			sideBySide();

		if (pdfDoc)
			pdfDoc->show();

		selectWindow();
		saveRecentFileInfo();
	}

	editor()->updateLineNumberAreaWidth(0);

	runHooks(QString::fromLatin1("LoadFile"));
}

void TeXDocumentWindow::delayedInit()
{
	if (_texDoc && !_texDoc->getHighlighter()) {
		Tw::Settings settings;

		TeXHighlighter * highlighter = new TeXHighlighter(_texDoc);
		connect(textEdit, &CompletingEdit::rehighlight, highlighter, &TeXHighlighter::rehighlight);

		// set up syntax highlighting
		// First, use the current file's syntaxMode property (if available)
		QMap<QString,QVariant> properties = TWApp::instance()->getFileProperties(textDoc()->absoluteFilePath());
		if (properties.contains(QString::fromLatin1("syntaxMode")))
			setSyntaxColoringMode(properties.value(QString::fromLatin1("syntaxMode")).toString());
		// Secondly, try the global settings
		else if (settings.contains(QString::fromLatin1("syntaxColoring")))
			    setSyntaxColoringMode(settings.value(QString::fromLatin1("syntaxColoring")).toString());
		// Lastly, use the default setting
		else {
			// This should mimick the code in PrefsDialog::doPrefsDialog()
			QStringList syntaxOptions = TeXHighlighter::syntaxOptions();
			if (kDefault_SyntaxColoring < syntaxOptions.count())
				setSyntaxColoringMode(syntaxOptions[kDefault_SyntaxColoring]);
			else
				setSyntaxColoringMode(QString());
		}

		if (_texDoc && _texDoc->hasModeLine(QStringLiteral("spellcheck"))) {
			setSpellcheckLanguage(_texDoc->getModeLineValue(QStringLiteral("spellcheck")));
		}
		else {
			// set the default spell checking language
			setSpellcheckLanguage(settings.value(QString::fromLatin1("language")).toString());
		}
	}
}

#define FILE_MODIFICATION_ACCURACY	1000	// in msec
void TeXDocumentWindow::reloadIfChangedOnDisk()
{
	if (untitled() || !lastModified.isValid())
		return;

	// Get the file info and force-refresh it to make sure we have the current
	// modification datetime (and not some cached value)
	QFileInfo fi{textDoc()->getFileInfo()};
	fi.refresh();
	QDateTime fileModified{fi.lastModified()};
	if (!fileModified.isValid() || fileModified == lastModified)
		return;

	clearFileWatcher(); // stop watching until next save or reload
	if (textEdit->document()->isModified()) {
		if (QMessageBox::warning(this, tr("File changed on disk"),
								 tr("%1 has been modified by another program.\n\n"
									"Do you want to discard your current changes, and reload the file from disk?")
								 .arg(textDoc()->getFileInfo().filePath()),
								 QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Cancel) {
			lastModified = QDateTime();	// invalidate the timestamp
			return;
		}
	}
	// user chose to discard, or there were no local changes
	// save the current cursor position

	// Store the selection (note that oldSelStart == oldSelEnd if there is
	// no selection)
	QTextCursor cur = textEdit->textCursor();
	int oldSelStart = cur.selectionStart();
	int oldSelEnd = cur.selectionEnd();
	QString oldSel = cur.selectedText();

	// Get the block number and the offset in the block of the start of the
	// selection
	cur.setPosition(oldSelStart);
	int oldBlockStart = cur.blockNumber();
	oldSelStart -= cur.block().position();

	// Get the block number and the offset in the block of the end of the
	// selection
	cur.setPosition(oldSelEnd);
	int oldBlockEnd = cur.blockNumber();
	oldSelEnd -= cur.block().position();

	// Get the values of the scroll bars so we can later restore the view
	int xPos{0}, yPos{0};
	if (textEdit->horizontalScrollBar())
		xPos = textEdit->horizontalScrollBar()->value();
	if (textEdit->verticalScrollBar())
		yPos = textEdit->verticalScrollBar()->value();

	// Reload the file from the disk
	// Note that the file may change again before the system watcher is enabled
	// again, so we should catch that case (this sometimes occurs with version
	// control systems during commits)
	unsigned int i{0};
	// Limit this to avoid infinite loops
	for (i = 0; i < 10; ++i) {
		clearFileWatcher(); // stop watching until next save or reload
		// Only reload files at full seconds to avoid problems with limited
		// accuracy of the file system modification timestamps (if the file changes
		// twice in one second, the modification timestamp is not altered and we may
		// miss the second change otherwise)
		while (QDateTime::currentDateTime() <= textDoc()->getFileInfo().lastModified().addMSecs(FILE_MODIFICATION_ACCURACY))
			; // do nothing
		loadFile(QFileInfo(textDoc()->absoluteFilePath()), false, true, true);
		// one final safety check - if the file has not changed, we can safely end this
		if (QDateTime::currentDateTime() > textDoc()->getFileInfo().lastModified().addMSecs(FILE_MODIFICATION_ACCURACY))
			break;
	}
	if (i == 10) { // the file has been changing constantly - give up and inform the user
		QMessageBox::information(this, tr("File changed on disk"),
								 tr("%1 is constantly being modified by another program.\n\n"
									"Please use \"File > Revert to Saved\" manually when the external process has finished.")
								 .arg(textDoc()->getFileInfo().filePath()),
								 QMessageBox::Ok, QMessageBox::Ok);
	}

	// restore the cursor position
	cur = textEdit->textCursor();

	// move the cursor to the beginning (this should actually be the case,
	// but one never knows)
	cur.setPosition(0);

	// move the cursor to the starting block
	cur.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, oldBlockStart);
	cur.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
	cur.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, oldSelStart);

	// move the cursor to the end block
	cur.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor, oldBlockEnd - oldBlockStart);
	cur.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
	cur.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, oldSelEnd);

	// if the current selection doesn't match the stored selection, collapse
	// to the beginning position
	if (cur.selectedText() != oldSel)
		cur.setPosition(cur.selectionStart());

	textEdit->setTextCursor(cur);

	// restore the view
	if (textEdit->horizontalScrollBar())
		textEdit->horizontalScrollBar()->setValue(xPos);
	if (textEdit->verticalScrollBar())
		textEdit->verticalScrollBar()->setValue(yPos);
}

// get expected name of the Preview file, and return whether it exists
bool TeXDocumentWindow::getPreviewFileName(QString &pdfName)
{
	const QString & rootFilePath = textDoc()->getRootFilePath();
	if (rootFilePath.isEmpty())
		return false;
	QFileInfo fi(rootFilePath);
	pdfName = QDir(fi.canonicalPath()).filePath(fi.completeBaseName() + QLatin1String(".pdf"));
	fi.setFile(pdfName);
	return fi.exists();
}

bool TeXDocumentWindow::openPdfIfAvailable(bool show)
{
	detachPdf();
	actionSide_by_Side->setEnabled(false);
	actionGo_to_Preview->setEnabled(false);

	QString pdfName;
	if (getPreviewFileName(pdfName)) {
		PDFDocumentWindow *existingPdf = PDFDocumentWindow::findDocument(pdfName);
		if (existingPdf) {
			pdfDoc = existingPdf;
			pdfDoc->selectWindow();
			pdfDoc->linkToSource(this);
		}
		else {
			pdfDoc = new PDFDocumentWindow(pdfName, this);
			if (show)
				pdfDoc->show();
		}
	}

	if (pdfDoc) {
		actionSide_by_Side->setEnabled(true);
		actionGo_to_Preview->setEnabled(true);
		connect(pdfDoc, &PDFDocumentWindow::destroyed, this, &TeXDocumentWindow::pdfClosed);
		connect(this, &TeXDocumentWindow::destroyed, pdfDoc, &PDFDocumentWindow::texClosed);
		return true;
	}

	return false;
}

void TeXDocumentWindow::pdfClosed()
{
	pdfDoc = nullptr;
	actionSide_by_Side->setEnabled(false);
}

bool TeXDocumentWindow::saveFile(const QFileInfo & fileInfo)
{
	const auto showNotSavedMessage = [this]() {
		statusBar()->showMessage(tr("Document \"%1\" was not saved")
									 .arg(textDoc()->getFileInfo().fileName()),
								 kStatusMessageDuration);
	};
	QDateTime fileModified = fileInfo.lastModified();
	if (fileInfo == textDoc()->getFileInfo() && fileModified.isValid() && fileModified != lastModified) {
		if (QMessageBox::warning(this, tr("File changed on disk"),
								 tr("%1 has been modified by another program.\n\n"
									"Do you want to proceed with saving this file, overwriting the version on disk?")
								 .arg(fileInfo.absoluteFilePath()),
								 QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Cancel) {
			showNotSavedMessage();
			return false;
		}
	}

	QString theText = textEdit->text();
	switch (lineEndings & kLineEnd_Mask) {
		case kLineEnd_CR:
			theText.replace(QChar::fromLatin1('\n'), QChar::fromLatin1('\r'));
			break;
		case kLineEnd_LF:
			break;
		case kLineEnd_CRLF:
			theText.replace(QChar::fromLatin1('\n'), QLatin1String("\r\n"));
			break;
	}

	if (!codec)
		codec = TWApp::instance()->getDefaultCodec();
	if (!codec->canEncode(theText)) {
		if (QMessageBox::warning(this, tr("Text cannot be converted"),
				tr("This document contains characters that cannot be represented in the encoding %1.\n\n"
				   "If you proceed, they will be replaced with default codes. "
				   "Alternatively, you may wish to use a different encoding (such as UTF-8) to avoid loss of data.")
		            .arg(QString::fromUtf8(codec->name().constData())),
				QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Cancel) {
			showNotSavedMessage();
			return false;
		}
	}

	clearFileWatcher();

	{
		QSaveFile file(fileInfo.absoluteFilePath());
		file.setDirectWriteFallback(true);
		if (!file.open(QFile::WriteOnly)) {
			QMessageBox::warning(this, QCoreApplication::applicationName(),
								 tr("Cannot write file \"%1\":\n%2")
								 .arg(fileInfo.absoluteFilePath(), file.errorString()));
			setupFileWatcher();
			showNotSavedMessage();
			return false;
		}

		QApplication::setOverrideCursor(Qt::WaitCursor);

		// When using the UTF-8 codec (mib = 106), byte order marks (BOMs) are
		// ignored during reading and not produced when writing. To keep them in
		// files that have them (or the user wants them), we need to write them
		// ourselves.
		if (codec->mibEnum() == 106 && utf8BOM)
			file.write("\xEF\xBB\xBF");

		if (file.write(codec->fromUnicode(theText)) == -1) {
			QApplication::restoreOverrideCursor();
			QMessageBox::warning(this, tr("Error writing file"),
								 tr("An error may have occurred while saving the file. "
									"You might like to save a copy in a different location."),
								 QMessageBox::Ok);
			showNotSavedMessage();
			return false;
		}
		file.commit();
		QApplication::restoreOverrideCursor();
	}

	// Pass the absoluteFilePath to the function; this will create a new
	// (updated) QFileInfo instance. This is necessary as the original QFileInfo
	// has the existance of the file cached. If the file is saved for the first
	// time (i.e. it did not exist before saveFile() was called),
	// fileInfo.exists() will return the wrong (cached) info.
	setCurrentFile(QFileInfo(fileInfo.absoluteFilePath()));
	statusBar()->showMessage(tr("File \"%1\" saved")
								.arg(textDoc()->getFileInfo().fileName()),
								kStatusMessageDuration);

	QTimer::singleShot(0, this, SLOT(setupFileWatcher()));
	return true;
}

void TeXDocumentWindow::clearFileWatcher()
{
	const QStringList files = watcher->files();
	if (files.count() > 0)
		watcher->removePaths(files);
	const QStringList dirs = watcher->directories();
	if (dirs.count() > 0)
		watcher->removePaths(dirs);
}

void TeXDocumentWindow::setupFileWatcher()
{
	clearFileWatcher();
	if (!untitled()) {
		QFileInfo info(textDoc()->getFileInfo());
		lastModified = info.lastModified();
		watcher->addPath(info.absoluteFilePath());
		watcher->addPath(info.canonicalPath());
	}
}

void TeXDocumentWindow::setCurrentFile(const QFileInfo & fileInfo)
{
	bool isUntitled = !fileInfo.exists();
	textDoc()->setStoredInFilesystem(!isUntitled);
	if (isUntitled) {
		static int sequenceNumber = 1;
		textDoc()->setFileInfo(QFileInfo(tr("untitled-%1.tex").arg(sequenceNumber++)));
		setWindowIcon(QApplication::windowIcon());
	}
	else {
		textDoc()->setFileInfo(fileInfo);
		QIcon winIcon;
#if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
		// The Compiz window manager doesn't seem to support icons larger than
		// 128x128, so we add a suitable one first
		winIcon.addFile(QString::fromLatin1(":/images/images/TeXworks-doc-128.png"));
#endif
		winIcon.addFile(QString::fromLatin1(":/images/images/TeXworks-doc.png"));
		setWindowIcon(winIcon);
	}
	textEdit->document()->setModified(false);
	setWindowModified(false);

	//: Format for the window title (ex. "file.tex[*] - TeXworks")
	setWindowTitle(tr("%1[*] - %2").arg(textDoc()->getFileInfo().fileName(), QCoreApplication::applicationName()));

	conditionallyEnableRemoveAuxFiles();

	TWApp::instance()->updateWindowMenus();
}

void TeXDocumentWindow::saveRecentFileInfo()
{
	if (untitled())
		return;

	QMap<QString,QVariant> fileProperties;

	fileProperties.insert(QString::fromLatin1("path"), textDoc()->absoluteFilePath());
	fileProperties.insert(QString::fromLatin1("geometry"), saveGeometry());
	fileProperties.insert(QString::fromLatin1("state"), saveState(kTeXWindowStateVersion));
	fileProperties.insert(QString::fromLatin1("selStart"), selectionStart());
	fileProperties.insert(QString::fromLatin1("selLength"), selectionLength());
	fileProperties.insert(QString::fromLatin1("quotesMode"), textEdit->getQuotesMode());
	fileProperties.insert(QString::fromLatin1("indentMode"), textEdit->getIndentMode());
	if (_texDoc && _texDoc->getHighlighter())
		fileProperties.insert(QString::fromLatin1("syntaxMode"), _texDoc->getHighlighter()->getSyntaxMode());
	fileProperties.insert(QString::fromLatin1("lineNumbers"), textEdit->getLineNumbersVisible());
	fileProperties.insert(QString::fromLatin1("wrapLines"), textEdit->wordWrapMode() == QTextOption::WordWrap);

	if (pdfDoc) {
		fileProperties.insert(QString::fromLatin1("pdfgeometry"), pdfDoc->saveGeometry());
		fileProperties.insert(QString::fromLatin1("pdfstate"), pdfDoc->saveState(kPDFWindowStateVersion));
	}

	TWApp::instance()->addToRecentFiles(fileProperties);
}

void TeXDocumentWindow::updateRecentFileActions()
{
	TWUtils::updateRecentFileActions(this, recentFileActions, menuOpen_Recent, actionClear_Recent_Files);
}

void TeXDocumentWindow::updateWindowMenu()
{
	Tw::Utils::WindowManager::updateWindowMenu(this, menuWindow);

	const QString label = Tw::Utils::WindowManager::uniqueLabelForFile(fileName());
	if (!label.isEmpty()) {
		setWindowTitle(tr("%1[*] - %2").arg(label, QCoreApplication::applicationName()));
	}
}

void TeXDocumentWindow::updateEngineList()
{
	engine->disconnect(this);
	while (menuRun->actions().count() > 2)
		menuRun->removeAction(menuRun->actions().last());
	while (engineActions->actions().count() > 0)
		engineActions->removeAction(engineActions->actions().last());

	QStandardItemModel * model = qobject_cast<QStandardItemModel*>(engine->model());
	Q_ASSERT(model);
	model->clear();
	foreach (Engine e, TWApp::instance()->getEngineList()) {
		QAction *newAction = new QAction(e.name(), engineActions);
		newAction->setCheckable(true);
		bool available{e.isAvailable()};
		newAction->setEnabled(available);
		newAction->setObjectName(QStringLiteral("actionTypesetTool%1").arg(e.name()));
		menuRun->addAction(newAction);
		QStandardItem * item = new QStandardItem(e.name());
		item->setFlags(Qt::ItemIsSelectable | (available ? Qt::ItemIsEnabled : Qt::NoItemFlags));
		model->appendRow(item);
	}
	connect(engine, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, static_cast<void (TeXDocumentWindow::*)(int)>(&TeXDocumentWindow::selectedEngine));
	int index = engine->findText(engineName, Qt::MatchFixedString);
	if (index < 0)
		index = engine->findText(TWApp::instance()->getDefaultEngine().name(), Qt::MatchFixedString);
	if (index >= 0)
		engine->setCurrentIndex(index);
}

void TeXDocumentWindow::selectedEngine(QAction* engineAction) // sent by actions in menubar menu; update toolbar combo box
{
	engineName = engineAction->text();
	for (int i = 0; i < engine->count(); ++i)
		if (engine->itemText(i) == engineName) {
			engine->setCurrentIndex(i);
			break;
		}
}

void TeXDocumentWindow::selectedEngine(int idx) // sent by toolbar combo box; need to update menu
{
	const QString name = engine->itemText(idx);
	engineName = name;
	foreach (QAction *act, engineActions->actions()) {
		if (act->text() == name) {
			act->setChecked(true);
			break;
		}
	}
}

void TeXDocumentWindow::showCursorPosition()
{
	QTextCursor cursor = textEdit->textCursor();
	cursor.setPosition(cursor.selectionStart());
	int line = cursor.blockNumber() + 1;
	int total = textEdit->document()->blockCount();
	int col = cursor.position() - textEdit->document()->findBlock(cursor.selectionStart()).position();
	lineNumberLabel->setText(tr("Line %1 of %2; col %3").arg(line).arg(total).arg(col));
	if (actionAuto_Follow_Focus->isChecked())
		emit syncFromSource(textDoc()->absoluteFilePath(), line, col, false);
}

void TeXDocumentWindow::showLineEndingSetting()
{
	QString lineEndStr;
	switch (lineEndings & kLineEnd_Mask) {
		case kLineEnd_LF:
		    lineEndStr = tr("LF");
			break;
		case kLineEnd_CRLF:
		    lineEndStr = tr("CRLF");
			break;
		case kLineEnd_CR:
		    lineEndStr = tr("CR");
			break;
	}
	if ((lineEndings & kLineEnd_Mixed) != 0)
		lineEndStr += tr("*");
	lineEndingLabel->setText(lineEndStr);
}

void TeXDocumentWindow::lineEndingPopup(const QPoint loc)
{
	QMenu menu;
	QAction * lf = menu.addAction(tr("LF (Unix, Mac OS X)"));
	QAction * crlf = menu.addAction(tr("CRLF (Windows)"));
	QAction * cr = menu.addAction(tr("CR (Mac Classic)"));
	QAction * result = menu.exec(lineEndingLabel->mapToGlobal(loc));
	int newSetting = (lineEndings & kLineEnd_Mask);
	if (result == lf)
		newSetting = kLineEnd_LF;
	else if (result == crlf)
		newSetting = kLineEnd_CRLF;
	else if (result == cr)
		newSetting = kLineEnd_CR;
	if (newSetting != (lineEndings & kLineEnd_Mask)) {
		lineEndings = newSetting;
		showLineEndingSetting();
		textEdit->document()->setModified();
	}
}

void TeXDocumentWindow::showEncodingSetting()
{
	encodingLabel->setText(codec ? QString::fromUtf8(codec->name().constData()) : QString());
}

void TeXDocumentWindow::encodingPopup(const QPoint loc)
{
	QMenu menu;
	//: Item in the encoding popup menu
	QAction * reloadAction = new QAction(tr("Reload using selected encoding"), &menu);
	//: Tooltip for "Reload using selected encoding"
	reloadAction->setToolTip(tr("Reloads the current file with the encoding selected from this menu.\n\nThe selected encoding replaces the default one and overrides all \"%!TEX encoding\" lines."));
	QAction * BOMAction = new QAction(tr("Write UTF-8 byte order mark"), &menu);
	BOMAction->setCheckable(true);
	BOMAction->setChecked(utf8BOM);
	// Only enable this option if we are currently using the UTF-8 codec
	BOMAction->setEnabled(codec && codec->mibEnum() == 106);

	if (!untitled())
		menu.addAction(reloadAction);
	menu.addAction(BOMAction);
	menu.addSeparator();

	foreach (QTextCodec *codec, *TWUtils::findCodecs()) {
		QAction * a = new QAction(QString::fromUtf8(codec->name().constData()), &menu);
		a->setCheckable(true);
		if (codec == this->codec)
			a->setChecked(true);
		menu.addAction(a);
	}
	QAction *result = menu.exec(encodingLabel->mapToGlobal(loc));
	if (result) {
		if (result == reloadAction) {
			if (textEdit->document()->isModified()) {
				if (QMessageBox::warning(this, tr("Unsaved changes"),
										 tr("The file you are trying to reload has unsaved changes.\n\n"
											"Do you want to discard your current changes, and reload the file from disk with the encoding %1?")
				                         .arg(QString::fromUtf8(codec->name().constData())),
										 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
					return;
				}
			}
			clearFileWatcher(); // stop watching until next save or reload
			loadFile(textDoc()->getFileInfo(), false, true, true, codec);
		}
		else if (result == BOMAction) {
			utf8BOM = BOMAction->isChecked();
			// If the UTF-8 codec is selected, changing utf8BOM actually
			// modifies how the file is saved. In all other cases, it does not
			// take effect until the UTF-8 codec is selected (in which case the
			// modified flag is set anyway).
			if (codec && codec->mibEnum() == 106)
				textEdit->document()->setModified();
		}
		else {
			QTextCodec *newCodec = QTextCodec::codecForName(result->text().toLatin1());
			if (newCodec && newCodec != codec) {
				codec = newCodec;
				showEncodingSetting();
				textEdit->document()->setModified();
			}
		}
	}
}

void TeXDocumentWindow::sideBySide()
{
	if (pdfDoc) {
		Tw::Utils::WindowManager::sideBySide(this, pdfDoc);
		pdfDoc->selectWindow(false);
		selectWindow();
	}
	else
		placeOnLeft();
}

TeXDocumentWindow *TeXDocumentWindow::findDocument(const QString &fileName)
{
	QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
	if (canonicalFilePath.isEmpty())
		canonicalFilePath = fileName;
			// file doesn't exist (probably from find-results in a new untitled doc),
			// so just use the name as-is

	foreach (QWidget *widget, qApp->topLevelWidgets()) {
		TeXDocumentWindow *theDoc = qobject_cast<TeXDocumentWindow*>(widget);
		if (theDoc && theDoc->textDoc()->getFileInfo().canonicalFilePath() == canonicalFilePath)
			return theDoc;
	}
	return nullptr;
}

void TeXDocumentWindow::clear()
{
	textEdit->textCursor().removeSelectedText();
}

QString TeXDocumentWindow::getLineText(int lineNo) const
{
	QTextDocument* doc = textEdit->document();
	if (lineNo < 1 || lineNo > doc->blockCount())
		return QString();
	return doc->findBlockByNumber(lineNo - 1).text();
}

void TeXDocumentWindow::goToLine(int lineNo, int selStart, int selEnd)
{
	QTextDocument* doc = textEdit->document();
	if (lineNo < 1 || lineNo > doc->blockCount())
		return;
	int oldScrollValue = -1;
	if (textEdit->verticalScrollBar())
		oldScrollValue = textEdit->verticalScrollBar()->value();
	QTextCursor cursor(doc->findBlockByNumber(lineNo - 1));
	if (selStart >= 0 && selEnd >= selStart) {
		cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, selStart);
		cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, selEnd - selStart);
	}
	else
		cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
	textEdit->setTextCursor(cursor);
	maybeCenterSelection(oldScrollValue);
}

void TeXDocumentWindow::maybeCenterSelection(int oldScrollValue)
{
	if (oldScrollValue != -1 && textEdit->verticalScrollBar()) {
		int newScrollValue = textEdit->verticalScrollBar()->value();
		if (newScrollValue != oldScrollValue) {
			int delta = (textEdit->height() - textEdit->cursorRect().height()) / 2;
			if (newScrollValue < oldScrollValue)
				delta = -delta;
			textEdit->verticalScrollBar()->setValue(newScrollValue + delta);
		}
	}
}

void TeXDocumentWindow::doFontDialog()
{
	bool ok{false};
	QFont font = QFontDialog::getFont(&ok, textEdit->font());
	if (ok) {
		textEdit->setFont(font);
		font.setPointSize(font.pointSize() - 1);

		inputLine->setFont(font);
		textEdit_console->setFont(font);
		for (int i = 1; i < consoleTabs->count(); ++i)
			consoleTabs->widget(i)->setFont(font);
	}
}

void TeXDocumentWindow::doLineDialog()
{
	QTextCursor cursor = textEdit->textCursor();
	cursor.setPosition(cursor.selectionStart());
	bool ok{false};
	int lineNo = QInputDialog::getInt(this, tr("Go to Line"),
									tr("Line number:"), cursor.blockNumber() + 1,
									1, textEdit->document()->blockCount(), 1, &ok);
	if (ok)
		goToLine(lineNo);
}

void TeXDocumentWindow::doFindDialog()
{
	if (FindDialog::doFindDialog(textEdit) == QDialog::Accepted)
		doFindAgain(true);
}

void TeXDocumentWindow::doReplaceDialog()
{
	ReplaceDialog::DialogCode result = ReplaceDialog::doReplaceDialog(textEdit);
	if (result != ReplaceDialog::Cancel)
		doReplace(result);
}

void TeXDocumentWindow::doIndent()
{
	textEdit->prefixLines(QString::fromLatin1("\t"));
}

void TeXDocumentWindow::doComment()
{
	textEdit->prefixLines(QString::fromLatin1("%"));
}

void TeXDocumentWindow::doUnindent()
{
	textEdit->unPrefixLines(QString::fromLatin1("\t"));
}

void TeXDocumentWindow::doUncomment()
{
	textEdit->unPrefixLines(QString::fromLatin1("%"));
}

void TeXDocumentWindow::toUppercase()
{
	replaceSelection(textEdit->textCursor().selectedText().toUpper());
}

void TeXDocumentWindow::toLowercase()
{
	replaceSelection(textEdit->textCursor().selectedText().toLower());
}

void TeXDocumentWindow::toggleCase()
{
	QString theText = textEdit->textCursor().selectedText();
	for (int i = 0; i < theText.length(); ++i) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		QCharRef ch = theText[i];
#else
		QChar & ch = theText[i];
#endif
		if (ch.isLower())
			ch = ch.toUpper();
		else
			ch = ch.toLower();
	}
	replaceSelection(theText);
}

void TeXDocumentWindow::replaceSelection(const QString& newText)
{
	QTextCursor cursor = textEdit->textCursor();
	int start = cursor.selectionStart();
	cursor.insertText(newText);
	int end = cursor.selectionEnd();
	cursor.setPosition(start);
	cursor.setPosition(end, QTextCursor::KeepAnchor);
	textEdit->setTextCursor(cursor);
}

void TeXDocumentWindow::selectRange(int start, int length)
{
	QTextCursor c = textCursor();
	c.setPosition(start);
	c.setPosition(start + length, QTextCursor::KeepAnchor);
	editor()->setTextCursor(c);
}

void TeXDocumentWindow::insertText(const QString& text)
{
	textCursor().insertText(text);
}

void TeXDocumentWindow::setWindowModified(bool modified)
{
	QMainWindow::setWindowModified(modified);
	TWApp::instance()->updateWindowMenus();
}

void TeXDocumentWindow::balanceDelimiters()
{
	const QString text = textEdit->text();
	QTextCursor cursor = textEdit->textCursor();
	using pos_type = decltype(cursor.position());
	auto openPos = TWUtils::findOpeningDelim(text, cursor.selectionStart());
	if (openPos >= 0 && openPos < text.length() - 1) {
		do {
			auto closePos = TWUtils::balanceDelim(text, openPos + 1, TWUtils::closerMatching(text[openPos]), 1);
			if (closePos < 0)
				break;
			if (closePos >= cursor.selectionEnd()) {
				cursor.setPosition(static_cast<pos_type>(openPos));
				cursor.setPosition(static_cast<pos_type>(closePos + 1), QTextCursor::KeepAnchor);
				textEdit->setTextCursor(cursor);
				return;
			}
			if (openPos > 0)
				openPos = TWUtils::findOpeningDelim(text, openPos - 1);
			else
				break;
		} while (openPos >= 0);
	}
	QApplication::beep();
}

void TeXDocumentWindow::doHardWrapDialog()
{
	HardWrapDialog dlg(this);

	dlg.show();
	if (dlg.exec()) {
		dlg.saveSettings();
		doHardWrap(dlg.mode(), static_cast<int>(dlg.lineWidth()), dlg.rewrap());
	}
}

void TeXDocumentWindow::doInsertCitationsDialog()
{
	CitationSelectDialog dlg(this);
	Tw::Settings settings;
	const QString defaultCiteCmd = settings.value(QStringLiteral("citeCommand"), QStringLiteral("\\cite{%1}")).toString();

	if (!textDoc()->hasModeLine(QStringLiteral("bibfile")) && !textDoc()->hasModeLine(QStringLiteral("bibfiles"))) {
		emit asyncFlashStatusBarMessage(tr("No '%!TEX bibfile' modline found"), kStatusMessageDuration);
		return;
	}

	// Load the bibfiles
	QStringList bibFiles = textDoc()->getModeLineValue(QStringLiteral("bibfile")).split(QLatin1Char{','}) +
						   textDoc()->getModeLineValue(QStringLiteral("bibfiles")).split(QLatin1Char{','});
	for (QString bibFile : bibFiles) {
		bibFile = bibFile.trimmed();
		if (bibFile.isEmpty()) continue;
		// Assume relative paths are given with respect to the current file's
		// directory.
		bibFile = textDoc()->getFileInfo().dir().absoluteFilePath(bibFile);
		dlg.addBibTeXFile(bibFile);
	}

	// Work out the enclosing citation command and already existing BibTeX keys
	// (if any)
	// TODO: Make configurable in a config text file
	QStringList citeCmds = QStringList() << QLatin1String("cite") \
	                                     << QLatin1String("bibentry") \
	                                     << QLatin1String("citet") \
	                                     << QLatin1String("citep") \
	                                     << QLatin1String("citealt") \
	                                     << QLatin1String("citealp") \
	                                     << QLatin1String("citenum") \
	                                     << QLatin1String("citeauthor") \
	                                     << QLatin1String("citeyear") \
	                                     << QLatin1String("citeyearpar") \
	                                     << QLatin1String("citefullauthor") \
	                                     << QLatin1String("Citet") \
	                                     << QLatin1String("Citep") \
	                                     << QLatin1String("Citealt") \
	                                     << QLatin1String("Citealp") \
	                                     << QLatin1String("Citeauthor");

	QString pattern = QString::fromLatin1("(\\\\(?:");
	for (const QString & citeCmd : citeCmds)
		pattern += QRegularExpression::escape(citeCmd) + QString::fromLatin1("|");
	pattern.chop(1);
	pattern += QLatin1String(")\\*?\\s*(?:\\[[^\\]]*\\])?\\s*\\{)([^}]*)\\}");
	// (\\(?:cite|...|Citeauthor)\*?\s*(?:\[[^\]]*\])?\s*\{)([^}]*)\}
	// capture group 1: everything in front of the BibTeX keys (e.g. "\cite{"
	//                  or "\Citep*[bla]{"
	// capture group 2: the existing BibTeX keys

	QTextCursor curs(textCursor());
	using pos_type = decltype(curs.position());
	constexpr int PeekLength = 1024;

	int peekFront = qMin(PeekLength, curs.position());
	int peekBack = qMin(PeekLength, textDoc()->characterCount() - curs.position());

	curs.beginEditBlock();
	curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, peekFront);
	curs.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, peekFront + peekBack);
	curs.endEditBlock();

	const QString peekStr = curs.selectedText();
	QRegularExpression reCmd(pattern);
	QRegularExpressionMatch mCmd;

	// Check if the cursor is in one of the standard cite commands
#if QT_VERSION >= 0x050500
	auto pos = peekStr.lastIndexOf(reCmd, peekFront, &mCmd);
	Q_UNUSED(pos)
#else
	int pos = peekStr.lastIndexOf(reCmd, peekFront);
	if (pos >= 0)
		mCmd= reCmd.match(peekStr, pos);
#endif
	bool updateExisting = mCmd.hasMatch() && mCmd.capturedStart() < peekFront && mCmd.capturedEnd() > peekFront;

	if (!updateExisting) {
		// If the cursor was not in any of the standard cite commands, check if
		// it was in a (user-defined) default cite command
		const QString pattern{QStringLiteral("(") + QRegularExpression::escape(defaultCiteCmd).replace(QStringLiteral("\\%1"), QStringLiteral(")(.*?)"))};
		QRegularExpression reCmd{pattern};

#if QT_VERSION >= 0x050500
		auto pos = peekStr.lastIndexOf(reCmd, peekFront, &mCmd);
		Q_UNUSED(pos)
#else
		int pos = peekStr.lastIndexOf(reCmd, peekFront);
		if (pos >= 0)
			mCmd= reCmd.match(peekStr, pos);
#endif
		updateExisting = mCmd.hasMatch() && mCmd.capturedStart() < peekFront && mCmd.capturedEnd() > peekFront;
	}

	// If the cursor was in a recognized cite command, pre-seed the dialog with
	// the extracted BibTeX keys
	if (updateExisting) {
		dlg.setInitialKeys(mCmd.captured(2).split(QLatin1Char(',')));
	}

	// Run the dialog
	if (dlg.exec()) {
		// If the dialog succeeded, insert the changes

		// If the dialog was invoked without the cursor inside a citation
		// command, insert a new one (\cite by default)
		if (!updateExisting) {
			insertText(defaultCiteCmd.arg(dlg.getSelectedKeys().join(QLatin1String(","))));
		}
		// Otherwise, replace the argument of the existing citation command
		else {
			curs.beginEditBlock();
			// collapse the selection to the beginning
			curs.setPosition(qMin(curs.position(), curs.anchor()));
			// move to the beginning of the cite argument (just after '{')
			curs.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, static_cast<pos_type>(mCmd.capturedEnd(1)));
			// select the cite argument
			curs.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, static_cast<pos_type>(mCmd.capturedLength(2)));
			// replace the text
			curs.insertText(dlg.getSelectedKeys().join(QLatin1String(",")));
			curs.endEditBlock();
		}
	}
}

void TeXDocumentWindow::doHardWrap(int mode, int lineWidth, bool rewrap)
{
	if (mode == kHardWrapMode_Window) {
		// fudge this for now.... not accurate with proportional fonts, ignores tabs,....
		QFontMetrics fm(textEdit->currentFont());
		lineWidth = textEdit->width() / fm.averageCharWidth();
	}
	else if (mode == kHardWrapMode_Unwrap) {
		lineWidth = INT_MAX;
		rewrap = true;
	}
	if (lineWidth == 0)
		return;

	QTextCursor cur = textEdit->textCursor();
	if (!cur.hasSelection())
		cur.select(QTextCursor::Document);

	int selStart = cur.selectionStart();
	int selEnd = cur.selectionEnd();

	cur.setPosition(selStart);
	if (!cur.atBlockStart()) {
		cur.movePosition(QTextCursor::StartOfBlock);
		selStart = cur.position();
	}

	cur.setPosition(selEnd);
	if (!cur.atBlockStart()) {
		cur.movePosition(QTextCursor::NextBlock);
		selEnd = cur.position();
	}

	cur.setPosition(selStart);
	cur.setPosition(selEnd, QTextCursor::KeepAnchor);

	QString oldText = cur.selectedText();
	QRegularExpression breakPattern(QStringLiteral("\\s+"));
	QString newText;

	while (!oldText.isEmpty()) {
		QString::size_type eol = oldText.indexOf(QChar::ParagraphSeparator);
		if (eol == -1)
			eol = oldText.length();
		else
			eol += 1;
		QString line = oldText.left(eol);
		oldText.remove(0, eol);

		if (rewrap && line.trimmed().length() > 0) {
			while (!oldText.isEmpty()) {
				eol = oldText.indexOf(QChar::ParagraphSeparator);
				if (eol == -1)
					eol = oldText.length();
				QString nextLine = oldText.left(eol).trimmed();
				if (nextLine.isEmpty())
					break;
				line = line.trimmed().append(QChar::fromLatin1(' ')).append(nextLine);
				oldText.remove(0, eol + 1);
			}
		}

		if (line.length() <= lineWidth) {
			newText.append(line);
			continue;
		}

		line = line.trimmed();
		if (line.length() <= lineWidth) {
			newText.append(line);
			continue;
		}

		QString::size_type curLength = 0;
		while (!line.isEmpty()) {
			QRegularExpressionMatch breakMatch = breakPattern.match(line);
			QString::size_type breakPoint = breakMatch.capturedStart();
			QString::size_type matchLen = breakMatch.capturedLength();
			if (breakPoint == -1) {
				breakPoint = line.length();
				matchLen = 0;
			}
			if (curLength > 0 && curLength + breakPoint >= lineWidth) {
				newText.append(QChar::ParagraphSeparator);
				curLength = 0;
			}
			if (curLength > 0) {
				newText.append(QChar::fromLatin1(' '));
				curLength += 1;
			}
			newText.append(line.left(breakPoint));
			curLength += breakPoint;
			line.remove(0, breakPoint + matchLen);
		}
		newText.append(QChar::ParagraphSeparator);
	}

	cur.insertText(newText);

	selEnd = cur.position();
	cur.setPosition(selStart);
	cur.setPosition(selEnd, QTextCursor::KeepAnchor);
	textEdit->setTextCursor(cur);
}


void TeXDocumentWindow::setLineNumbers(bool displayNumbers)
{
	actionLine_Numbers->setChecked(displayNumbers);
	textEdit->setLineNumberDisplay(displayNumbers);
}

void TeXDocumentWindow::setLineSpacing(qreal percent)
{
	// percent should typically be between 100 (single spacing) and 200 (double
	// spacing). Values below 1 are simply ignored (this includes the "typical
	// invalid return value" of 0).
	if (percent <= 1.)
		return;

	Q_ASSERT(textDoc() != nullptr);

	m_lineSpacing = percent;

	// Select the entire document
	QTextCursor cur{textDoc()};
	cur.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

	// Don't set the block format if it is not necessary
	// (works around QTBUG-120451 in "common" cases)
	if (cur.blockFormat().lineHeightType() == QTextBlockFormat::ProportionalHeight && cur.blockFormat().lineHeight() == percent) {
		return;
	}
	if (cur.blockFormat().lineHeightType() == QTextBlockFormat::SingleHeight && percent == 100) {
		return;
	}

	// Remember the "modified" state. While we consider the line spacing as a
	// purely cosmetic property here, it is set on the underlying text document
	// and therefore will force its "modified" state to true (which we don't
	// want, e.g., when starting up)
	const bool wasModified = isModified();
	// Apply the modified line height
	QTextBlockFormat fmt;
	fmt.setLineHeight(percent, QTextBlockFormat::ProportionalHeight);
	cur.mergeBlockFormat(fmt);
	// Restore "modified" state
	setModified(wasModified);
}

void TeXDocumentWindow::setWrapLines(bool wrap)
{
	actionWrap_Lines->setChecked(wrap);
	textEdit->setWordWrapMode(wrap ? QTextOption::WordWrap : QTextOption::NoWrap);
}

void TeXDocumentWindow::setSyntaxColoring(int index)
{
	if (_texDoc && _texDoc->getHighlighter())
		_texDoc->getHighlighter()->setActiveIndex(index);
}

void TeXDocumentWindow::setSyntaxColoringMode(const QString& mode)
{
	QList<QAction*> actionList = menuSyntax_Coloring->actions();

	if (mode.isEmpty()) {
		Q_ASSERT(actionSyntaxColoring_None);
		actionSyntaxColoring_None->trigger();
		return;
	}
	for (int i = 0; i < actionList.count(); ++i) {
		if (actionList[i]->isCheckable() && actionList[i]->text().compare(mode, Qt::CaseInsensitive) == 0) {
			actionList[i]->trigger();
			return;
		}
	}
}

void TeXDocumentWindow::setSmartQuotesMode(const QString& mode)
{
	QList<QAction*> actionList = menuSmart_Quotes_Mode->actions();
	for (int i = 0; i < actionList.count(); ++i) {
		if (actionList[i]->isCheckable() && actionList[i]->text().compare(mode, Qt::CaseInsensitive) == 0) {
			actionList[i]->trigger();
			return;
		}
	}
	if (mode.isEmpty()) {
		actionSmartQuotes_None->trigger();
		return;
	}
}

void TeXDocumentWindow::setAutoIndentMode(const QString& mode)
{
	QList<QAction*> actionList = menuAuto_indent_Mode->actions();
	for (int i = 0; i < actionList.count(); ++i) {
		if (actionList[i]->isCheckable() && actionList[i]->text().compare(mode, Qt::CaseInsensitive) == 0) {
			actionList[i]->trigger();
			return;
		}
	}
	if (mode.isEmpty()) {
		actionAutoIndent_None->trigger();
		return;
	}
}

void TeXDocumentWindow::doFindAgain(bool fromDialog)
{
	Tw::Settings settings;
	QString	searchText = settings.value(QString::fromLatin1("searchText")).toString();
	if (searchText.isEmpty())
		return;

	QTextDocument::FindFlags flags = static_cast<QTextDocument::FindFlags>(settings.value(QString::fromLatin1("searchFlags")).toInt());

	QRegularExpression *regex = nullptr;
	if (settings.value(QString::fromLatin1("searchRegex")).toBool()) {
		regex = new QRegularExpression(searchText, ((flags & QTextDocument::FindCaseSensitively) != 0) ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
		if (!regex->isValid()) {
			qApp->beep();
			statusBar()->showMessage(tr("Invalid regular expression"), kStatusMessageDuration);
			delete regex;
			return;
		}
	}

	if (fromDialog && (settings.value(QString::fromLatin1("searchFindAll")).toBool() || settings.value(QString::fromLatin1("searchAllFiles")).toBool())) {
		QList<SearchResult> results;
		flags &= ~QTextDocument::FindBackward;

		QList<TeXDocumentWindow*> docsToSearch{this};
		if (settings.value(QString::fromLatin1("searchAllFiles")).toBool()) {
			for (TeXDocumentWindow * theDoc : docList) {
				if (!docsToSearch.contains(theDoc)) {
					docsToSearch.append(theDoc);
				}
			}
		}

		for (TeXDocumentWindow * theDoc : docsToSearch) {
			QTextCursor curs(theDoc->textDoc());
			curs.movePosition(QTextCursor::End);
			int rangeStart = 0;
			int rangeEnd = curs.position();
			while (true) {
				curs = theDoc->doSearch(searchText, regex, flags, rangeStart, rangeEnd);
				if (curs.isNull())
					break;
				int blockStart = curs.block().position();
				results.append(SearchResult(theDoc, curs.blockNumber() + 1,
								curs.selectionStart() - blockStart, curs.selectionEnd() - blockStart));
				if ((flags & QTextDocument::FindBackward) != 0)
					rangeEnd = curs.selectionStart();
				else
					rangeStart = curs.selectionEnd();
			}
		}

		if (results.count() == 0) {
			qApp->beep();
			statusBar()->showMessage(tr("Not found"), kStatusMessageDuration);
		}
		else {
			const bool singleFile = (docsToSearch.size() == 1);
			SearchResults::presentResults(searchText, results, this, singleFile);
			statusBar()->showMessage(tr("Found %n occurrence(s)", "", static_cast<int>(results.count())), kStatusMessageDuration);
		}
	}
	else {
		QTextCursor	curs = textEdit->textCursor();
		if (settings.value(QString::fromLatin1("searchSelection")).toBool() && curs.hasSelection()) {
			int rangeStart = curs.selectionStart();
			int rangeEnd = curs.selectionEnd();
			curs = doSearch(searchText, regex, flags, rangeStart, rangeEnd);
		}
		else {
			if ((flags & QTextDocument::FindBackward) != 0) {
				int rangeStart = 0;
				int rangeEnd = curs.selectionStart();
				curs = doSearch(searchText, regex, flags, rangeStart, rangeEnd);
				if (curs.isNull() && settings.value(QString::fromLatin1("searchWrap")).toBool()) {
					curs = QTextCursor(textEdit->document());
					curs.movePosition(QTextCursor::End);
					curs = doSearch(searchText, regex, flags, 0, curs.position());
				}
			}
			else {
				int rangeStart = curs.selectionEnd();
				curs.movePosition(QTextCursor::End);
				int rangeEnd = curs.position();
				curs = doSearch(searchText, regex, flags, rangeStart, rangeEnd);
				if (curs.isNull() && settings.value(QString::fromLatin1("searchWrap")).toBool())
					curs = doSearch(searchText, regex, flags, 0, rangeEnd);
			}
		}

		if (curs.isNull()) {
			qApp->beep();
			statusBar()->showMessage(tr("Not found"), kStatusMessageDuration);
		}
		else
			textEdit->setTextCursor(curs);
	}

	delete regex;
}

void TeXDocumentWindow::doReplaceAgain()
{
	doReplace(ReplaceDialog::ReplaceOne);
}

void TeXDocumentWindow::doReplace(ReplaceDialog::DialogCode mode)
{
	Tw::Settings settings;

	QString	searchText = settings.value(QString::fromLatin1("searchText")).toString();
	if (searchText.isEmpty())
		return;

	QTextDocument::FindFlags flags = static_cast<QTextDocument::FindFlags>(settings.value(QString::fromLatin1("searchFlags")).toInt());

	QRegularExpression *regex = nullptr;
	if (settings.value(QString::fromLatin1("searchRegex")).toBool()) {
		regex = new QRegularExpression(searchText, ((flags & QTextDocument::FindCaseSensitively) != 0) ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
		if (!regex->isValid()) {
			qApp->beep();
			statusBar()->showMessage(tr("Invalid regular expression"), kStatusMessageDuration);
			delete regex;
			return;
		}
	}

	QString	replacement = settings.value(QString::fromLatin1("replaceText")).toString();
	if (regex) {
		QRegularExpression escapedChar(QStringLiteral("\\\\([nt\\\\]|x([0-9A-Fa-f]{4}))"));
		QString::size_type index = -1;
		QRegularExpressionMatch escapeMatch;
		while ((escapeMatch = escapedChar.match(replacement, index + 1)).hasMatch()) {
			index = escapeMatch.capturedStart();
			QChar ch;
			if (escapeMatch.capturedLength(1) == 1) {
				// single-char escape code newline/tab/backslash
				ch = escapeMatch.captured(1)[0];
				switch (ch.unicode()) {
					case 'n':
					    ch = QChar::fromLatin1('\n');
						break;
					case 't':
					    ch = QChar::fromLatin1('\t');
						break;
					case '\\':
					    ch = QChar::fromLatin1('\\');
						break;
					default:
						// should not happen!
						break;
				}
			}
			else {
				// Unicode char number \xHHHH
				bool ok{false};
				ch = QChar(escapeMatch.captured(2).toUInt(&ok, 16));
			}
			replacement.replace(index, escapeMatch.capturedLength(), ch);
		}
	}

	bool allFiles = (mode == ReplaceDialog::ReplaceAll) && settings.value(QString::fromLatin1("searchAllFiles")).toBool();

	bool searchWrap = settings.value(QString::fromLatin1("searchWrap")).toBool();
	bool searchSel = settings.value(QString::fromLatin1("searchSelection")).toBool();

	int rangeStart{0}, rangeEnd{0};
	QTextCursor searchRange = textCursor();
	if (allFiles) {
		searchRange.select(QTextCursor::Document);
		rangeStart = searchRange.selectionStart();
		rangeEnd = searchRange.selectionEnd();
	}
	else if (searchSel) {
		rangeStart = searchRange.selectionStart();
		rangeEnd = searchRange.selectionEnd();
	}
	else {
		// Note: searchWrap is handled separately below
		if ((flags & QTextDocument::FindBackward) != 0) {
			rangeStart = 0;
			rangeEnd = searchRange.selectionEnd();
		}
		else {
			rangeStart = searchRange.selectionStart();
			searchRange.select(QTextCursor::Document);
			rangeEnd = searchRange.selectionEnd();
		}
	}

	if (mode == ReplaceDialog::ReplaceOne) {
		QTextCursor curs = doSearch(searchText, regex, flags, rangeStart, rangeEnd);
		if (curs.isNull() && searchWrap) {
			// If we haven't found anything and wrapping is enabled, try again
			// with a "wrapped" search range
			if ((flags & QTextDocument::FindBackward) != 0) {
				rangeStart = rangeEnd;
				searchRange.select(QTextCursor::Document);
				rangeEnd = searchRange.selectionEnd();
			}
			else {
				rangeEnd = rangeStart;
				rangeStart = 0;
			}
			curs = doSearch(searchText, regex, flags, rangeStart, rangeEnd);
		}
		if (curs.isNull()) {
			qApp->beep();
			statusBar()->showMessage(tr("Not found"), kStatusMessageDuration);
		}
		else {
			// do replacement
			QString target;
			if (regex)
				target = textEdit->text().mid(curs.selectionStart(), curs.selectionEnd() - curs.selectionStart()).replace(*regex, replacement);
			else
				target = replacement;
			curs.insertText(target);
			textEdit->setTextCursor(curs);
		}
	}
	else if (mode == ReplaceDialog::ReplaceAll) {
		if (allFiles) {
			int replacements = 0;
			foreach (TeXDocumentWindow* doc, docList)
				replacements += doc->doReplaceAll(searchText, regex, replacement, flags);
			QString numOccurrences = tr("%n occurrence(s)", "", replacements);
			QString numDocuments = tr("%n documents", "", static_cast<int>(docList.count()));
			QString message = tr("Replaced %1 in %2").arg(numOccurrences, numDocuments);
			statusBar()->showMessage(message, kStatusMessageDuration);
		}
		else {
			if (!searchSel) {
				// If we are not searching within a selection, we implicitly
				// search the whole document with ReplaceAll
				searchRange.select(QTextCursor::Document);
				rangeStart = searchRange.selectionStart();
				rangeEnd = searchRange.selectionEnd();
			}
			int replacements = doReplaceAll(searchText, regex, replacement, flags, rangeStart, rangeEnd);
			statusBar()->showMessage(tr("Replaced %n occurrence(s)", "", replacements), kStatusMessageDuration);
		}
	}

	delete regex;
}

int TeXDocumentWindow::doReplaceAll(const QString& searchText, QRegularExpression * regex, const QString& replacement,
								QTextDocument::FindFlags flags, int rangeStart, int rangeEnd)
{
	QTextCursor searchRange = textCursor();
	using pos_type = decltype(searchRange.position());
	searchRange.select(QTextCursor::Document);
	if (rangeStart < 0)
		rangeStart = searchRange.selectionStart();
	if (rangeEnd < 0)
		rangeEnd = searchRange.selectionEnd();

	int replacements = 0;
	bool first = true;
	while (true) {
		QTextCursor curs = doSearch(searchText, regex, flags, rangeStart, rangeEnd);
		if (curs.isNull()) {
			if (!first)
				searchRange.endEditBlock();
			break;
		}
		if (first) {
			searchRange.beginEditBlock();
			first = false;
		}
		QString target;
		pos_type oldLen = curs.selectionEnd() - curs.selectionStart();
		if (regex)
			target = textEdit->text().mid(curs.selectionStart(), oldLen).replace(*regex, replacement);
		else
			target = replacement;
		pos_type newLen = static_cast<pos_type>(target.length());
		if ((flags & QTextDocument::FindBackward) != 0)
			rangeEnd = curs.selectionStart();
		else {
			rangeStart = curs.selectionEnd() - oldLen + newLen;
			rangeEnd += newLen - oldLen;
		}
		searchRange.setPosition(curs.selectionStart());
		searchRange.setPosition(curs.selectionEnd(), QTextCursor::KeepAnchor);
		searchRange.insertText(target);
		++replacements;
	}
	if (!first) {
		searchRange.setPosition(rangeStart);
		textEdit->setTextCursor(searchRange);
	}
	return replacements;
}

QTextCursor TeXDocumentWindow::doSearch(const QString& searchText, const QRegularExpression * regex, QTextDocument::FindFlags flags, int s, int e)
{
	QTextCursor curs;
	using pos_type = decltype(curs.position());
	QTextDocument * theDoc = textEdit->document();
	const QString& docText = textEdit->text();

	if ((flags & QTextDocument::FindBackward) != 0) {
		if (regex) {
			// this doesn't seem to match \n or even \x2029 for newline
			// curs = theDoc->find(*regex, e, flags);
			QRegularExpressionMatch m;
#if QT_VERSION >= 0x050500
			QString::size_type offset = docText.lastIndexOf(*regex, e, &m);
#else
			int offset = docText.lastIndexOf(*regex, e);
			if (offset >= 0)
				m = regex->match(docText, offset);
#endif
			while (offset >= s && m.capturedEnd() > e) {
#if QT_VERSION >= 0x050500
				offset = docText.lastIndexOf(*regex, offset - 1, &m);
#else
				offset = docText.lastIndexOf(*regex, offset - 1);
				if (offset >= 0)
					m = regex->match(docText, offset);
#endif
			}
			if (offset >= s) {
				curs = QTextCursor(textEdit->document());
				curs.setPosition(static_cast<pos_type>(m.capturedStart()));
				curs.setPosition(static_cast<pos_type>(m.capturedEnd()), QTextCursor::KeepAnchor);
			}
		}
		else {
			curs = theDoc->find(searchText, e, flags);
			if (!curs.isNull()) {
				if (curs.selectionEnd() > e)
					curs = theDoc->find(searchText, curs, flags);
				if (curs.selectionStart() < s)
					curs = QTextCursor();
			}
		}
	}
	else {
		if (regex) {
			// this doesn't seem to match \n or even \x2029 for newline
			// curs = theDoc->find(*regex, s, flags);
			QRegularExpressionMatch m = regex->match(docText, s);
			if (m.hasMatch()) {
				curs = QTextCursor(theDoc);
				curs.setPosition(static_cast<pos_type>(m.capturedStart()));
				curs.setPosition(static_cast<pos_type>(m.capturedEnd()), QTextCursor::KeepAnchor);
			}
		}
		else {
			curs = theDoc->find(searchText, s, flags);
		}
		if (curs.selectionEnd() > e)
			curs = QTextCursor();
	}
	return curs;
}

void TeXDocumentWindow::copyToFind()
{
	if (textEdit->textCursor().hasSelection()) {
		QString searchText = textEdit->textCursor().selectedText();
		searchText.replace(QChar::ParagraphSeparator, QChar::LineFeed); // 0x2029 -> \n

		Tw::Settings settings;
		// Note: To search for multi-line strings, we currently need regex
		// enabled (since we only have a single search line). If it was not
		// enabled, we also need to ensure that the replaceText is escaped
		// properly
		const bool isMultiLine = searchText.contains(QChar::LineFeed);
		if (isMultiLine && !settings.value(QString::fromLatin1("searchRegex")).toBool()) {
			settings.setValue(QString::fromLatin1("searchRegex"), true);
			settings.setValue(QString::fromLatin1("replaceText"), QRegularExpression::escape(settings.value(QString::fromLatin1("replaceText")).toString()));
		}
		if (settings.value(QString::fromLatin1("searchRegex")).toBool()) {
			if (isMultiLine)
				settings.setValue(QString::fromLatin1("searchText"), QRegularExpression::escape(searchText).replace(QStringLiteral("\\\n"), QStringLiteral("\\n")));
			else
				settings.setValue(QString::fromLatin1("searchText"), QRegularExpression::escape(searchText));
		}
		else
			settings.setValue(QString::fromLatin1("searchText"), searchText);
	}
}

void TeXDocumentWindow::copyToReplace()
{
	if (textEdit->textCursor().hasSelection()) {
		QString replaceText = textEdit->textCursor().selectedText();
		replaceText.replace(QChar(0x2029), QChar::fromLatin1('\n'));
		Tw::Settings settings;
		// Note: To do multi-line replacements, we currently need regex enabled
		// (since we only have a single replace line). If it was not enabled, we
		// also need to ensure that the searchText is escaped properly
		bool isMultiLine = replaceText.contains(QChar::fromLatin1('\n'));
		if (isMultiLine && !settings.value(QString::fromLatin1("searchRegex")).toBool()) {
			settings.setValue(QString::fromLatin1("searchRegex"), true);
			settings.setValue(QString::fromLatin1("searchText"), QRegularExpression::escape(settings.value(QString::fromLatin1("searchText")).toString()));
		}
		if (settings.value(QString::fromLatin1("searchRegex")).toBool()) {
			if (isMultiLine)
				settings.setValue(QString::fromLatin1("replaceText"), QRegularExpression::escape(replaceText).replace(QChar::fromLatin1('\n'), QLatin1String("\\n")));
			else
				settings.setValue(QString::fromLatin1("replaceText"), QRegularExpression::escape(replaceText));
		}
		else
			settings.setValue(QString::fromLatin1("replaceText"), replaceText);
	}
}

void TeXDocumentWindow::findSelection()
{
	copyToFind();
	doFindAgain();
}

void TeXDocumentWindow::showSelection()
{
	int oldScrollValue = -1;
	if (textEdit->verticalScrollBar())
		oldScrollValue = textEdit->verticalScrollBar()->value();
	textEdit->ensureCursorVisible();
	maybeCenterSelection(oldScrollValue);
}

void TeXDocumentWindow::typeset()
{
	if (process)
		return;	// this shouldn't happen if we disable the command at the right time

	if (untitled() || textEdit->document()->isModified()) {
		if (!save()) {
			statusBar()->showMessage(tr("Cannot process unsaved document"), kStatusMessageDuration);
			return;
		}
	}

	const QString & rootFilePath = textDoc()->getRootFilePath();
	if (!saveFilesHavingRoot(rootFilePath))
		return;

	QFileInfo fileInfo(rootFilePath);
	if (!fileInfo.isReadable()) {
		statusBar()->showMessage(tr("Root document %1 is not readable").arg(rootFilePath), kStatusMessageDuration);
		return;
	}

	Engine e = TWApp::instance()->getNamedEngine(engine->currentText());
	if (!e.isAvailable()) {
		statusBar()->showMessage(tr("%1 is not properly configured").arg(engine->currentText()), kStatusMessageDuration);
		return;
	}

	if (!TWApp::instance()->typesetManager().startTypesetting(rootFilePath, this)) {
		statusBar()->showMessage(tr("%1 is already being processed").arg(rootFilePath), kStatusMessageDuration);
		updateTypesettingAction();
		return;
	}
	// NB: TypesetManager::startTypesetting implicitly calls
	// updateTypesettingAction() via signal-slot-connections

	QString pdfName;
	if (getPreviewFileName(pdfName))
		oldPdfTime = QFileInfo(pdfName).lastModified();
	else
		oldPdfTime = QDateTime();

	// Stop watching the pdf document while it is being changed to avoid
	// interference
	if (pdfDoc && pdfDoc->widget())
		pdfDoc->widget()->setWatchForDocumentChangesOnDisk(false);

	process = e.run(fileInfo, this);

	if (process) {
		textEdit_console->setProcess(process);
		if (consoleTabs->isHidden()) {
			keepConsoleOpen = false;
			showConsole();
		}
		else {
			inputLine->show();
		}
		// ensure the window is visible - otherwise we can't see the output
		// panel (and the typeset process appears to hang in case of an error)
		// Also ensure the window is activated (in case it wasn't; e.g. when
		// starting typesetting from the preview) so it can receive focus (to
		// the input line)
		consoleTabs->setCurrentIndex(0);
		show();
		raise();
		activateWindow();

		inputLine->setFocus(Qt::OtherFocusReason);
		showPdfWhenFinished = e.showPdf();
		userInterrupt = false;

#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
		connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error), this, &TeXDocumentWindow::processError);
#else
		connect(process, &QProcess::errorOccurred, this, &TeXDocumentWindow::processError);
#endif
		connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &TeXDocumentWindow::processFinished);
	}
	else {
		// Since the process didn't run, restart watching the output immediately
		if (pdfDoc && pdfDoc->widget())
			pdfDoc->widget()->setWatchForDocumentChangesOnDisk(true);

		TWApp::instance()->typesetManager().stopTypesetting(this);

		QMessageBox msgBox(QMessageBox::Critical, tr("Unable to execute %1").arg(e.name()),
		                      QLatin1String("<p>") + tr("The program \"%1\" was not found.").arg(e.program()) + QLatin1String("</p>") +
#if defined(Q_OS_WIN)
		                      QLatin1String("<p>") + tr("You need a <b>TeX distribution</b> like <a href=\"http://tug.org/texlive/\">TeX Live</a> or <a href=\"http://miktex.org/\">MiKTeX</a> installed on your system to typeset your document.") + QLatin1String("</p>") +
#elif defined(Q_OS_DARWIN)
		                      QLatin1String("<p>") + tr("You need a <b>TeX distribution</b> like <a href=\"http://www.tug.org/mactex/\">MacTeX</a> installed on your system to typeset your document.") + QLatin1String("</p>") +
#else // defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
		                      QLatin1String("<p>") + tr("You need a <b>TeX distribution</b> like <a href=\"http://tug.org/texlive/\">TeX Live</a> installed on your system to typeset your document. On most systems such a TeX distribution is available as prebuilt package.") + QLatin1String("</p>") +
#endif
		                      QLatin1String("<p>") + tr("When a TeX distribution is installed you may need to tell TeXworks where to find it in Edit -> Preferences -> Typesetting.") + QLatin1String("</p>"),
							  QMessageBox::Cancel, this);
		msgBox.setDetailedText(
		                      tr("Searched in directories:") + QChar::fromLatin1('\n') +
							  QLatin1String(" * ") + Engine::binPaths().join(QLatin1String("\n * ")) + QChar::fromLatin1('\n') +
							  tr("Check the configuration of the %1 tool and the path settings in the Preferences dialog.").arg(e.name()));
		msgBox.exec();
	}
}

void TeXDocumentWindow::interrupt()
{
	if (process) {
		userInterrupt = true;
		process->kill();

		// Start watching for changes in the pdf (again)
		if (pdfDoc && pdfDoc->widget())
			pdfDoc->widget()->setWatchForDocumentChangesOnDisk(true);

		// Don't notify the TypesetManager that typesetting was stopped; this is
		// delegated to processError() which will be called after process->kill()
		// and which takes care of resetting the process before notifying the
		// TypesetManager. This ensures that subsequent calls to isTypesetting()
		// return the correct value
	}
}

void TeXDocumentWindow::goToTypesettingWindow()
{
	TeXDocumentWindow * owner = qobject_cast<TeXDocumentWindow*>(TWApp::instance()->typesetManager().getOwnerForRootFile(textDoc()->getRootFilePath()));
	if (owner) {
		owner->raise();
		owner->activateWindow();
	}
}

void TeXDocumentWindow::updateTypesettingAction()
{
	TeXDocumentWindow * owner = qobject_cast<TeXDocumentWindow*>(TWApp::instance()->typesetManager().getOwnerForRootFile(textDoc()->getRootFilePath()));

	disconnect(actionTypeset, &QAction::triggered, this, nullptr);
	if (isTypesetting()) {
		// We are currently running a typesetting process
		// The button should allow the user to stop it
		actionTypeset->setIcon(QIcon::fromTheme(QStringLiteral("process-stop")));
		actionTypeset->setText(tr("Abort typesetting"));
		connect(actionTypeset, &QAction::triggered, this, &TeXDocumentWindow::interrupt);
	}
	else if (owner != nullptr) {
		// Someone else is typesetting "our" (root) document
		// The button should take the user to the window from which the process
		// was run
		actionTypeset->setIcon(QIcon::fromTheme(QStringLiteral("go-jump")));
		actionTypeset->setText(tr("Go to typesetting"));
		connect(actionTypeset, &QAction::triggered, this, &TeXDocumentWindow::goToTypesettingWindow);
	}
	else {
		// No process is currently running (for "our" root document)
		// The button should allow the user to start one
		actionTypeset->setIcon(QIcon::fromTheme(QStringLiteral("process-start")));
		actionTypeset->setText(tr("Typeset"));
		connect(actionTypeset, &QAction::triggered, this, &TeXDocumentWindow::typeset);
	}
}

void TeXDocumentWindow::conditionallyEnableRemoveAuxFiles()
{
	// In the following cases, we want to disable "Remove Aux Files"
	// 1) There cannot be any aux files (as there is no root file; i.e., the
	//    file has never been saved)
	// 2) A typesetting process is running for "our" root file which may be
	//    accessing the aux files
	const bool enable = [&](){
		QFileInfo rootFileInfo{textDoc()->getRootFilePath()};
		if (!rootFileInfo.exists())
			return false;
		if (TWApp::instance()->typesetManager().isFileBeingTypeset(rootFileInfo.canonicalFilePath()))
			return false;
		return true;
	}();
	actionRemove_Aux_Files->setEnabled(enable);
}

void TeXDocumentWindow::processError(QProcess::ProcessError /*error*/)
{
	if (userInterrupt)
		textEdit_console->append(tr("Process interrupted by user"));
	else
		textEdit_console->append(process->errorString());
	process->kill();
	textEdit_console->setProcess(nullptr, false);
	process->deleteLater();
	process = nullptr;
	inputLine->hide();

	// Start watching for changes in the pdf (again)
	if (pdfDoc && pdfDoc->widget())
		pdfDoc->widget()->setWatchForDocumentChangesOnDisk(true);

	TWApp::instance()->typesetManager().stopTypesetting(this);
}

void TeXDocumentWindow::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	// Start watching for changes in the pdf (again)
	if (pdfDoc && pdfDoc->widget())
		pdfDoc->widget()->setWatchForDocumentChangesOnDisk(true);

	textEdit_console->setProcess(nullptr, false);

	if (exitStatus != QProcess::CrashExit) {
		QString pdfName;
		if (getPreviewFileName(pdfName)) {
			actionGo_to_Preview->setEnabled(true);
			if (QFileInfo(pdfName).lastModified() != oldPdfTime) {
				// only open/refresh the PDF if it was changed by the typeset process
				if (!pdfDoc || pdfName != pdfDoc->fileName()) {
					if (showPdfWhenFinished && openPdfIfAvailable(true))
						pdfDoc->selectWindow();
				}
				else {
					pdfDoc->reload(); // always reload if it is loaded, we don't want a stale window
					if (showPdfWhenFinished)
						pdfDoc->selectWindow();
				}
			}
		}
		else
			actionGo_to_Preview->setEnabled(true);
	}

	executeAfterTypesetHooks();

	Tw::Settings settings;

	bool shouldHideConsole = false;
	QVariant hideConsoleSetting = settings.value(QString::fromLatin1("autoHideConsole"), kDefault_HideConsole);
	// Backwards compatibility to Tw 0.4.0 and before
	if (hideConsoleSetting.toString() == QString::fromLatin1("true") || hideConsoleSetting.toString() == QString::fromLatin1("false"))
		hideConsoleSetting = (hideConsoleSetting.toBool() ? kDefault_HideConsole : 0);

	switch(hideConsoleSetting.toInt()) {
		case 0: // Never hide console
			shouldHideConsole = false;
			break;
		case 1: // Hide console automatically
			shouldHideConsole = (!keepConsoleOpen && exitCode == 0 && exitStatus != QProcess::CrashExit);
			break;
		case 2: // Always hide console on success
			shouldHideConsole = (exitCode == 0 && exitStatus != QProcess::CrashExit);
			break;
		default: // Should never happen
			;
	}

	if (shouldHideConsole)
		hideConsole();
	else
		inputLine->hide();

	if (process)
		process->deleteLater();
	process = nullptr;

	TWApp::instance()->typesetManager().stopTypesetting(this);
}

void TeXDocumentWindow::executeAfterTypesetHooks()
{
	TWScriptManager * scriptManager = TWApp::instance()->getScriptManager();

	for (int i = consoleTabs->count() - 1; i > 0; --i)
		consoleTabs->removeTab(i);

	foreach (Tw::Scripting::ScriptObject *so, scriptManager->getHookScripts(QString::fromLatin1("AfterTypeset"))) {
		QVariant result;
		Tw::Scripting::ScriptAPI api(so, qApp, this, result);
		bool success = so->run(api);
		if (success && !result.isNull()) {
			QString res = result.toString();
			if (res.startsWith(QLatin1String("<html>"), Qt::CaseInsensitive)) {
				QTextBrowser *browser = new QTextBrowser(this);
				// Use console font (which is customizable)
				browser->setFont(textEdit_console->font());
				browser->setOpenLinks(false);
				connect(browser, &QTextBrowser::anchorClicked, this, &TeXDocumentWindow::anchorClicked);
				browser->setHtml(res);
				browser->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard | Qt::LinksAccessibleByMouse | Qt::TextBrowserInteraction | Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
				consoleTabs->addTab(browser, so->getTitle());
			}
			else {
				QTextEdit *textEdit = new QTextEdit(this);
				textEdit->setPlainText(res);
				textEdit->setReadOnly(true);
				textEdit->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard | Qt::LinksAccessibleByMouse | Qt::TextBrowserInteraction | Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
				consoleTabs->addTab(textEdit, so->getTitle());
			}
		}
	}
}

void TeXDocumentWindow::anchorClicked(const QUrl& url)
{
	if (url.scheme() == QString::fromLatin1("texworks")) {
		int line = 0;
		if (url.hasFragment()) {
			line = url.fragment().toInt();
		}
		TeXDocumentWindow * target = openDocument(QFileInfo(textDoc()->getRootFilePath()).absoluteDir().filePath(url.path()), true, true, line);
		if (target)
			target->textEdit->setFocus(Qt::OtherFocusReason);
	}
	else {
		TWApp::instance()->openUrl(url);
	}
}

// showConsole() and hideConsole() are used internally to update the visibility;
// they must NOT change the keepConsoleOpen setting that records user choice
void TeXDocumentWindow::showConsole()
{
	consoleTabs->show();
	if (process)
		inputLine->show();
	actionShow_Hide_Console->setText(tr("Hide Console Output"));
}

void TeXDocumentWindow::hideConsole()
{
	consoleTabs->hide();
	inputLine->hide();
	actionShow_Hide_Console->setText(tr("Show Console Output"));
}

// this is connected to the user command, so remember the choice
// for when typesetting finishes
void TeXDocumentWindow::toggleConsoleVisibility()
{
	if (consoleTabs->isVisible()) {
		hideConsole();
		keepConsoleOpen = false;
	}
	else {
		showConsole();
		keepConsoleOpen = true;
	}
}

void TeXDocumentWindow::acceptInputLine()
{
	if (process) {
		const QString str = inputLine->text() + QStringLiteral("\n");
		textEdit_console->echo(str, inputLine->palette().text().color());
		process->write(str.toUtf8());
		inputLine->clear();
	}
}

void TeXDocumentWindow::goToPreview()
{
	if (pdfDoc)
		pdfDoc->selectWindow();
	else {
		if (!openPdfIfAvailable(true)) {
			// This should only fail if the user has done something sneaky like closing the
			// preview window and then renaming the PDF file, since we opened the source
			// and checked that it exists (otherwise Go to Preview would have been disabled).
			// We could issue a status-bar warning here but it's a pretty obscure case...
			// for now just disable the command.
			actionGo_to_Preview->setEnabled(false);
			actionSide_by_Side->setEnabled(false);
		}
	}
}

void TeXDocumentWindow::syncClick(int lineNo, int col)
{
	if (!untitled()) {
		// ensure that there is a pdf to receive our signal
		goToPreview();
		emit syncFromSource(textDoc()->absoluteFilePath(), lineNo, col, true);
	}
}

void TeXDocumentWindow::handleModelineChange(QStringList changedKeys, QStringList removedKeys)
{
	Q_UNUSED(removedKeys);

	if (changedKeys.contains(QStringLiteral("program"))) {
		QString name = _texDoc->getModeLineValue(QStringLiteral("program"));
		int index = engine->findText(name, Qt::MatchFixedString);
		if (index > -1) {
			if (index != engine->currentIndex()) {
				engine->setCurrentIndex(index);
				emit asyncFlashStatusBarMessage(tr("Set engine to \"%1\"").arg(engine->currentText()), kStatusMessageDuration);
			}
		}
		else {
			emit asyncFlashStatusBarMessage(tr("Engine \"%1\" not defined").arg(name), kStatusMessageDuration);
		}
	}
	if (changedKeys.contains(QStringLiteral("encoding"))) {
		bool hasMetadata{false};
		QString reqName;
		QTextCursor curs(textDoc());
		// (begin|end)EditBlock() is a workaround for QTBUG-24718 that causes
		// movePosition() to crash the program under some circumstances.
		// Since we don't change any text in the edit block, it should be a noop
		// in the context of undo/redo.
		curs.beginEditBlock();
		curs.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, PEEK_LENGTH);
		curs.endEditBlock();

		QTextCodec *newCodec = scanForEncoding(curs.selectedText(), hasMetadata, reqName);
		if (newCodec) {
			codec = newCodec;
			showEncodingSetting();
		}
	}
	if (changedKeys.contains(QStringLiteral("spellcheck"))) {
		setSpellcheckLanguage(_texDoc->getModeLineValue(QStringLiteral("spellcheck")));
	}
}

void TeXDocumentWindow::goToTag(int index)
{
	if (_texDoc && index < _texDoc->getTags().count()) {
		textEdit->setTextCursor(_texDoc->getTags()[index].cursor);
		textEdit->setFocus(Qt::OtherFocusReason);
	}
}

bool TeXDocumentWindow::isTypesetting() const
{
	return (process != nullptr || TWApp::instance()->typesetManager().getOwnerForRootFile(textDoc()->getRootFilePath()) == this);
}

void TeXDocumentWindow::removeAuxFiles()
{
	const QString & rootFilePath = textDoc()->getRootFilePath();
	if (rootFilePath.isEmpty())
		return;

	QFileInfo fileInfo(rootFilePath);
	QString jobname = fileInfo.completeBaseName();
	QDir dir(fileInfo.dir());

	QStringList filterList = TWUtils::cleanupPatterns().split(QRegularExpression(QStringLiteral("\\s+")));
	if (filterList.count() == 0)
		return;
	for (int i = 0; i < filterList.count(); ++i)
		filterList[i].replace(QLatin1String("$jobname"), jobname);

	dir.setNameFilters(filterList);
	QStringList auxFileList = dir.entryList(QDir::Files | QDir::CaseSensitive, QDir::Name);
	if (auxFileList.count() > 0)
		Tw::UI::RemoveAuxFilesDialog::doConfirmDelete(dir, auxFileList);
	else
		(void)QMessageBox::information(this, tr("No files found"),
									   tr("No auxiliary files associated with this document at the moment."));
}

#if defined(Q_OS_DARWIN)
#define OPEN_FILE_IN_NEW_WINDOW	Qt::MoveAction // unmodified drag appears as MoveAction on Mac OS X
#define INSERT_DOCUMENT_TEXT	Qt::CopyAction
#define CREATE_INCLUDE_COMMAND	Qt::LinkAction
#else
#define OPEN_FILE_IN_NEW_WINDOW	Qt::CopyAction // ...but as CopyAction on X11
#define INSERT_DOCUMENT_TEXT	Qt::MoveAction
#define CREATE_INCLUDE_COMMAND	Qt::LinkAction
#endif

void TeXDocumentWindow::dragEnterEvent(QDragEnterEvent *event)
{
	event->ignore();
	if (event->mimeData()->hasUrls()) {
		const QList<QUrl> urls = event->mimeData()->urls();
		foreach (const QUrl& url, urls) {
			if (url.scheme() == QString::fromLatin1("file")) {
				event->acceptProposedAction();
				break;
			}
		}
	}
}

void TeXDocumentWindow::dragMoveEvent(QDragMoveEvent *event)
{
	if (event->proposedAction() == INSERT_DOCUMENT_TEXT || event->proposedAction() == CREATE_INCLUDE_COMMAND) {
		if (dragSavedCursor.isNull())
			dragSavedCursor = textEdit->textCursor();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		QTextCursor curs = textEdit->cursorForPosition(textEdit->mapFromGlobal(mapToGlobal(event->pos())));
#else
		QTextCursor curs = textEdit->cursorForPosition(textEdit->mapFromGlobal(mapToGlobal(event->position().toPoint())));
#endif
		textEdit->setTextCursor(curs);
	}
	else {
		if (!dragSavedCursor.isNull()) {
			textEdit->setTextCursor(dragSavedCursor);
			dragSavedCursor = QTextCursor();
		}
	}
	event->acceptProposedAction();
}

void TeXDocumentWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
	if (!dragSavedCursor.isNull()) {
		textEdit->setTextCursor(dragSavedCursor);
		dragSavedCursor = QTextCursor();
	}
	event->accept();
}

void TeXDocumentWindow::dropEvent(QDropEvent *event)
{
	if (event->mimeData()->hasUrls()) {
		Qt::DropAction action = event->proposedAction();
		const QList<QUrl> urls = event->mimeData()->urls();
		bool editBlockStarted = false;
		QString text;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		QTextCursor curs = textEdit->cursorForPosition(textEdit->mapFromGlobal(mapToGlobal(event->pos())));
#else
		QTextCursor curs = textEdit->cursorForPosition(textEdit->mapFromGlobal(mapToGlobal(event->position().toPoint())));
#endif
		foreach (const QUrl& url, urls) {
			if (url.scheme() == QString::fromLatin1("file")) {
				QString fileName = url.toLocalFile();
				switch (action) {
					case OPEN_FILE_IN_NEW_WINDOW:
						if(!Tw::Document::isImageFile(fileName)) {
							TWApp::instance()->openFile(fileName);
							break;
						}
						// for graphic files, fall through (there's no point in
						// trying to open binary files as text
						// fall through

					case INSERT_DOCUMENT_TEXT:
						if (!Tw::Document::isPDFfile(fileName) && !Tw::Document::isImageFile(fileName) && !Tw::Document::isPostscriptFile(fileName)) {
							QTextCodec * codecUsed{nullptr};
							text = readFile(QFileInfo(fileName), &codecUsed);
							if (!text.isNull()) {
								if (!editBlockStarted) {
									curs.beginEditBlock();
									editBlockStarted = true;
								}
								textEdit->setTextCursor(curs);
								curs.insertText(text);
							}
							break;
						}
						// for graphic files, fall through -- behave the same as the "link" action
						// fall through

					case CREATE_INCLUDE_COMMAND:
						if (!editBlockStarted) {
							curs.beginEditBlock();
							editBlockStarted = true;
						}
						textEdit->setTextCursor(curs);
						if (Tw::Document::isPDFfile(fileName))
							text = TWUtils::includePdfCommand();
						else if (Tw::Document::isImageFile(fileName))
							text = TWUtils::includeImageCommand();
						else if (Tw::Document::isPostscriptFile(fileName))
							text = TWUtils::includePostscriptCommand();
						else
							text = TWUtils::includeTextCommand();
						curs.insertText(text.arg(fileName));
						break;
					default:
						// do nothing
						break;
				}
			}
		}
		if (editBlockStarted)
			curs.endEditBlock();
	}
	dragSavedCursor = QTextCursor();
	event->accept();
}

void TeXDocumentWindow::detachPdf()
{
	if (pdfDoc) {
		disconnect(pdfDoc, &PDFDocumentWindow::destroyed, this, &TeXDocumentWindow::pdfClosed);
		disconnect(this, &TeXDocumentWindow::destroyed, pdfDoc, &PDFDocumentWindow::texClosed);
		pdfDoc = nullptr;
	}
}
