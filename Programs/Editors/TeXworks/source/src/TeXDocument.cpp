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

#include "TeXDocument.h"
#include "TeXHighlighter.h"
#include "TeXDocks.h"
#include "FindDialog.h"
#include "TemplateDialog.h"
#include "TWApp.h"
#include "TWUtils.h"
#include "PDFDocument.h"
#include "ConfirmDelete.h"
#include "HardWrapDialog.h"
#include "DefaultPrefs.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QStatusBar>
#include <QFontDialog>
#include <QInputDialog>
#include <QDesktopWidget>
#include <QClipboard>
#include <QStringList>
#include <QUrl>
#include <QComboBox>
#include <QRegExp>
#include <QProcess>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QActionGroup>
#include <QTextCodec>
#include <QSignalMapper>
#include <QDockWidget>
#include <QAbstractButton>
#include <QPushButton>
#include <QFileSystemWatcher>
#include <QTextBrowser>
#include <QAbstractTextDocumentLayout>
#if defined(MIKTEX_TODO_PRINT)
// see http://code.google.com/p/texworks/issues/detail?id=78#c1
#include <QPrinter>
#include <QPrintDialog>
#endif

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

#define kLineEnd_Mask   0x00FF
#define kLineEnd_LF     0x0000
#define kLineEnd_CRLF   0x0001
#define kLineEnd_CR     0x0002

#define kLineEnd_Flags_Mask  0xFF00
#define kLineEnd_Mixed       0x0100

const int kHardWrapDefaultWidth = 64;

QList<TeXDocument*> TeXDocument::docList;

TeXDocument::TeXDocument()
{
	init();
	statusBar()->showMessage(tr("New document"), kStatusMessageDuration);
}

TeXDocument::TeXDocument(const QString &fileName, bool asTemplate)
{
	init();
	loadFile(fileName, asTemplate);
}

TeXDocument::~TeXDocument()
{
	docList.removeAll(this);
	updateWindowMenu();
}

static bool dictActionLessThan(const QAction * a1, const QAction * a2) {
	return a1->text().toLower() < a2->text().toLower();
}

void TeXDocument::init()
{
	codec = TWApp::instance()->getDefaultCodec();
	pdfDoc = NULL;
	process = NULL;
	highlighter = NULL;
	pHunspell = NULL;
	utf8BOM = false;
#if defined(Q_OS_WIN)
	lineEndings = kLineEnd_CRLF;
#else
	lineEndings = kLineEnd_LF;
#endif
	
	setupUi(this);
#if defined(Q_OS_WIN)
	TWApp::instance()->createMessageTarget(this);
#endif

	setAttribute(Qt::WA_DeleteOnClose, true);
	setAttribute(Qt::WA_MacNoClickThrough, true);

	setContextMenuPolicy(Qt::NoContextMenu);

	makeUntitled();
	hideConsole();
	keepConsoleOpen = false;
	connect(consoleTabs, SIGNAL(requestClose()), actionShow_Hide_Console, SLOT(trigger()));

	statusBar()->addPermanentWidget(lineEndingLabel = new ClickableLabel());
	lineEndingLabel->setFrameStyle(QFrame::StyledPanel);
	lineEndingLabel->setFont(statusBar()->font());
	connect(lineEndingLabel, SIGNAL(mouseLeftClick(QMouseEvent*)), this, SLOT(lineEndingLabelClick(QMouseEvent*)));
	showLineEndingSetting();
	
	statusBar()->addPermanentWidget(encodingLabel = new ClickableLabel());
	encodingLabel->setFrameStyle(QFrame::StyledPanel);
	encodingLabel->setFont(statusBar()->font());
	connect(encodingLabel, SIGNAL(mouseLeftClick(QMouseEvent*)), this, SLOT(encodingLabelClick(QMouseEvent*)));
	showEncodingSetting();
	
	statusBar()->addPermanentWidget(lineNumberLabel = new ClickableLabel());
	lineNumberLabel->setFrameStyle(QFrame::StyledPanel);
	lineNumberLabel->setFont(statusBar()->font());
	connect(lineNumberLabel, SIGNAL(mouseLeftClick(QMouseEvent*)), this, SLOT(doLineDialog()));
	showCursorPosition();
	
	engineActions = new QActionGroup(this);
	connect(engineActions, SIGNAL(triggered(QAction*)), this, SLOT(selectedEngine(QAction*)));
	
	codec = TWApp::instance()->getDefaultCodec();
	engineName = TWApp::instance()->getDefaultEngine().name();
	engine = new QComboBox(this);
	engine->setEditable(false);
	engine->setFocusPolicy(Qt::NoFocus);
	engine->setSizeAdjustPolicy(QComboBox::AdjustToContents);
#if defined(Q_OS_DARWIN) && (QT_VERSION >= 0x040600)
	engine->setStyleSheet("padding:4px;");
	engine->setMinimumWidth(150);
#endif
	toolBar_run->addWidget(engine);
	updateEngineList();
	connect(engine, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(selectedEngine(const QString&)));
	
	connect(TWApp::instance(), SIGNAL(engineListChanged()), this, SLOT(updateEngineList()));
	
	connect(actionNew, SIGNAL(triggered()), this, SLOT(newFile()));
	connect(actionNew_from_Template, SIGNAL(triggered()), this, SLOT(newFromTemplate()));
	connect(actionOpen, SIGNAL(triggered()), this, SLOT(open()));
	connect(actionAbout_TW, SIGNAL(triggered()), qApp, SLOT(about()));
	connect(actionSettings_and_Resources, SIGNAL(triggered()), qApp, SLOT(doResourcesDialog()));
	connect(actionGoToHomePage, SIGNAL(triggered()), qApp, SLOT(goToHomePage()));
	connect(actionWriteToMailingList, SIGNAL(triggered()), qApp, SLOT(writeToMailingList()));

	connect(actionSave, SIGNAL(triggered()), this, SLOT(save()));
	connect(actionSave_As, SIGNAL(triggered()), this, SLOT(saveAs()));
	connect(actionSave_All, SIGNAL(triggered()), this, SLOT(saveAll()));
	connect(actionRevert_to_Saved, SIGNAL(triggered()), this, SLOT(revert()));
	connect(actionClose, SIGNAL(triggered()), this, SLOT(close()));

	connect(actionRemove_Aux_Files, SIGNAL(triggered()), this, SLOT(removeAuxFiles()));

	connect(actionQuit_TeXworks, SIGNAL(triggered()), TWApp::instance(), SLOT(maybeQuit()));
	
	connect(actionClear, SIGNAL(triggered()), this, SLOT(clear()));

	connect(actionFont, SIGNAL(triggered()), this, SLOT(doFontDialog()));
	connect(actionGo_to_Line, SIGNAL(triggered()), this, SLOT(doLineDialog()));
	connect(actionFind, SIGNAL(triggered()), this, SLOT(doFindDialog()));
	connect(actionFind_Again, SIGNAL(triggered()), this, SLOT(doFindAgain()));
	connect(actionReplace, SIGNAL(triggered()), this, SLOT(doReplaceDialog()));
	connect(actionReplace_Again, SIGNAL(triggered()), this, SLOT(doReplaceAgain()));

	connect(actionCopy_to_Find, SIGNAL(triggered()), this, SLOT(copyToFind()));
	connect(actionCopy_to_Replace, SIGNAL(triggered()), this, SLOT(copyToReplace()));
	connect(actionFind_Selection, SIGNAL(triggered()), this, SLOT(findSelection()));

	connect(actionShow_Selection, SIGNAL(triggered()), this, SLOT(showSelection()));

	connect(actionIndent, SIGNAL(triggered()), this, SLOT(doIndent()));
	connect(actionUnindent, SIGNAL(triggered()), this, SLOT(doUnindent()));

	connect(actionComment, SIGNAL(triggered()), this, SLOT(doComment()));
	connect(actionUncomment, SIGNAL(triggered()), this, SLOT(doUncomment()));

	connect(actionHard_Wrap, SIGNAL(triggered()), this, SLOT(doHardWrapDialog()));
	
	connect(actionTo_Uppercase, SIGNAL(triggered()), this, SLOT(toUppercase()));
	connect(actionTo_Lowercase, SIGNAL(triggered()), this, SLOT(toLowercase()));
	connect(actionToggle_Case, SIGNAL(triggered()), this, SLOT(toggleCase()));

	connect(actionBalance_Delimiters, SIGNAL(triggered()), this, SLOT(balanceDelimiters()));

	connect(textEdit->document(), SIGNAL(modificationChanged(bool)), this, SLOT(setWindowModified(bool)));
	connect(textEdit->document(), SIGNAL(modificationChanged(bool)), this, SLOT(maybeEnableSaveAndRevert(bool)));
	connect(textEdit->document(), SIGNAL(contentsChange(int,int,int)), this, SLOT(contentsChanged(int,int,int)));
	connect(textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(showCursorPosition()));
	connect(textEdit, SIGNAL(selectionChanged()), this, SLOT(showCursorPosition()));
	connect(textEdit, SIGNAL(syncClick(int, int)), this, SLOT(syncClick(int, int)));
	connect(this, SIGNAL(syncFromSource(const QString&, int, int, bool)), qApp, SIGNAL(syncPdf(const QString&, int, int, bool)));

	connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardChanged()));
	clipboardChanged();

	connect(actionTypeset, SIGNAL(triggered()), this, SLOT(typeset()));

	updateRecentFileActions();
	connect(qApp, SIGNAL(recentFileActionsChanged()), this, SLOT(updateRecentFileActions()));
	connect(qApp, SIGNAL(windowListChanged()), this, SLOT(updateWindowMenu()));
	connect(actionClear_Recent_Files, SIGNAL(triggered()), TWApp::instance(), SLOT(clearRecentFiles()));
	
	connect(qApp, SIGNAL(hideFloatersExcept(QWidget*)), this, SLOT(hideFloatersUnlessThis(QWidget*)));
	connect(this, SIGNAL(activatedWindow(QWidget*)), qApp, SLOT(activatedWindow(QWidget*)));

	connect(actionStack, SIGNAL(triggered()), qApp, SLOT(stackWindows()));
	connect(actionTile, SIGNAL(triggered()), qApp, SLOT(tileWindows()));
	connect(actionSide_by_Side, SIGNAL(triggered()), this, SLOT(sideBySide()));
	connect(actionPlace_on_Left, SIGNAL(triggered()), this, SLOT(placeOnLeft()));
	connect(actionPlace_on_Right, SIGNAL(triggered()), this, SLOT(placeOnRight()));
	connect(actionShow_Hide_Console, SIGNAL(triggered()), this, SLOT(toggleConsoleVisibility()));
	connect(actionGo_to_Preview, SIGNAL(triggered()), this, SLOT(goToPreview()));
	
	connect(this, SIGNAL(destroyed()), qApp, SLOT(updateWindowMenus()));

	connect(actionPreferences, SIGNAL(triggered()), qApp, SLOT(preferences()));

	connect(menuEdit, SIGNAL(aboutToShow()), this, SLOT(editMenuAboutToShow()));

#if defined(Q_OS_DARWIN)
	textEdit->installEventFilter(CmdKeyFilter::filter());
#endif

	connect(inputLine, SIGNAL(returnPressed()), this, SLOT(acceptInputLine()));

	QSETTINGS_OBJECT(settings);
	TWUtils::applyToolbarOptions(this, settings.value("toolBarIconSize", 2).toInt(), settings.value("toolBarShowText", false).toBool());

	QFont font = textEdit->font();
	if (settings.contains("font")) {
		QString fontString = settings.value("font").toString();
		if (fontString != "") {
			font.fromString(fontString);
			textEdit->setFont(font);
		}
	}
	font.setPointSize(font.pointSize() - 1);
	inputLine->setFont(font);
	inputLine->setLayoutDirection(Qt::LeftToRight);
	textEdit_console->setFont(font);
	textEdit_console->setLayoutDirection(Qt::LeftToRight);
	
	bool b = settings.value("wrapLines", true).toBool();
	actionWrap_Lines->setChecked(b);
	setWrapLines(b);

	b = settings.value("lineNumbers", false).toBool();
	actionLine_Numbers->setChecked(b);
	setLineNumbers(b);
	
	QStringList options = TeXHighlighter::syntaxOptions();

	QSignalMapper *syntaxMapper = new QSignalMapper(this);
	connect(syntaxMapper, SIGNAL(mapped(int)), this, SLOT(setSyntaxColoring(int)));
	syntaxMapper->setMapping(actionSyntaxColoring_None, -1);
	connect(actionSyntaxColoring_None, SIGNAL(triggered()), syntaxMapper, SLOT(map()));

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
	textEdit->setTabStopWidth(settings.value("tabWidth", kDefault_TabWidth).toInt());
	
	// It is VITAL that this connection is queued! Calling showMessage directly
	// from TeXDocument::contentsChanged would otherwise result in a seg fault
	// (for whatever reason)
	connect(this, SIGNAL(asyncFlashStatusBarMessage(QString, int)), statusBar(), SLOT(showMessage(QString, int)), Qt::QueuedConnection);
	
	QString indentOption = settings.value("autoIndent").toString();
	options = CompletingEdit::autoIndentModes();
	
	QSignalMapper *indentMapper = new QSignalMapper(this);
	connect(indentMapper, SIGNAL(mapped(int)), textEdit, SLOT(setAutoIndentMode(int)));
	indentMapper->setMapping(actionAutoIndent_None, -1);
	connect(actionAutoIndent_None, SIGNAL(triggered()), indentMapper, SLOT(map()));
	
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

	QString quotesOption = settings.value("smartQuotes").toString();
	options = CompletingEdit::smartQuotesModes();

	QSignalMapper *quotesMapper = new QSignalMapper(this);
	connect(quotesMapper, SIGNAL(mapped(int)), textEdit, SLOT(setSmartQuotesMode(int)));
	quotesMapper->setMapping(actionSmartQuotes_None, -1);
	connect(actionSmartQuotes_None, SIGNAL(triggered()), quotesMapper, SLOT(map()));

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
	if (options.size() > 0)
		menuSmart_Quotes_Mode->addSeparator();
	menuSmart_Quotes_Mode->addAction(actionApply_to_Selection);
	connect(actionApply_to_Selection, SIGNAL(triggered()), textEdit, SLOT(smartenQuotes()));

	connect(actionLine_Numbers, SIGNAL(triggered(bool)), this, SLOT(setLineNumbers(bool)));
	connect(actionWrap_Lines, SIGNAL(triggered(bool)), this, SLOT(setWrapLines(bool)));

	connect(actionNone, SIGNAL(triggered()), &dictSignalMapper, SLOT(map()));
	dictSignalMapper.setMapping(actionNone, QString());
	connect(&dictSignalMapper, SIGNAL(mapped(const QString&)), this, SLOT(setLangInternal(const QString&)));

	QActionGroup *group = new QActionGroup(this);
	group->addAction(actionNone);

	reloadSpellcheckerMenu();
	connect(TWApp::instance(), SIGNAL(dictionaryListChanged()), this, SLOT(reloadSpellcheckerMenu()));

	menuShow->addAction(toolBar_run->toggleViewAction());
	menuShow->addAction(toolBar_edit->toggleViewAction());
	menuShow->addSeparator();

	TWUtils::zoomToHalfScreen(this);

	QDockWidget *dw = new TagsDock(this);
	dw->hide();
	addDockWidget(Qt::LeftDockWidgetArea, dw);
	menuShow->addAction(dw->toggleViewAction());
	deferTagListChanges = false;

	watcher = new QFileSystemWatcher(this);
	connect(watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(reloadIfChangedOnDisk()), Qt::QueuedConnection);
	connect(watcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(reloadIfChangedOnDisk()), Qt::QueuedConnection);
	
	docList.append(this);
	
	TWApp::instance()->updateWindowMenus();
	
	initScriptable(menuScripts, actionAbout_Scripts, actionManage_Scripts,
				   actionUpdate_Scripts, actionShow_Scripts_Folder);

	TWUtils::insertHelpMenuItems(menuHelp);
	TWUtils::installCustomShortcuts(this);
#if QT_VERSION < 0x050000
	QTimer::singleShot(1000, this, SLOT(delayedInit()));
#else
	delayedInit();
#endif
#if defined(MIKTEX)
        actionAbout_MiKTeX = new QAction(this);
	actionAbout_MiKTeX->setIcon(QIcon(":/MiKTeX/miktex32x32.png"));
        actionAbout_MiKTeX->setObjectName(QString::fromUtf8("actionAbout_MiKTeX"));
	actionAbout_MiKTeX->setText(QApplication::translate("TeXDocument", "About MiKTeX..."));
        actionAbout_MiKTeX->setMenuRole(QAction::AboutRole);
	connect (actionAbout_MiKTeX, SIGNAL(triggered()), qApp, SLOT(aboutMiKTeX()));
#if 0
	menuHelp->addAction (actionAbout_MiKTeX);
#endif
#if defined(MIKTEX_TODO_PRINT)
	// see http://code.google.com/p/texworks/issues/detail?id=78#c1
	actionPrintSource = new QAction(this);
	actionPrintSource->setIcon(QIcon(":/images/tango/document-print.png"));
	actionPrintSource->setObjectName(QString::fromUtf8("actionPrintPDF"));
	actionPrintSource->setText(QApplication::translate("PDFDocument", "Print..."));
	actionPrintSource->setShortcut (QKeySequence::Print);
	connect (actionPrintSource, SIGNAL(triggered()), this, SLOT(print()));
	menuFile->insertAction (actionClose, actionPrintSource);
	menuFile->insertSeparator (actionClose);
#endif
#endif
}

void TeXDocument::changeEvent(QEvent *event)
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

void TeXDocument::setLangInternal(const QString& lang)
{
	// called internally by the spelling menu actions;
	// not for use from scripts as it won't update the menu
	QTextCodec *spellingCodec;
	Hunhandle* pOldHunspell = pHunspell;
	pHunspell = TWUtils::getDictionary(lang);
	// if the dictionary hasn't change, don't reset the spell checker as that
	// can result in a serious delay for long documents
	// NB: Don't delete the hunspell handles; the pointers are kept by TWUtils
	if (pOldHunspell == pHunspell)
		return;
	
	if (pHunspell != NULL) {
		spellingCodec = QTextCodec::codecForName(Hunspell_get_dic_encoding(pHunspell));
		if (spellingCodec == NULL)
			spellingCodec = QTextCodec::codecForLocale(); // almost certainly wrong, if we couldn't find the actual name!
	}
	else
		spellingCodec = NULL;
	textEdit->setSpellChecker(pHunspell, spellingCodec);
	if (highlighter)
		highlighter->setSpellChecker(pHunspell, spellingCodec);
}

void TeXDocument::setSpellcheckLanguage(const QString& lang)
{
	// this is called by the %!TEX spellcheck... line, or by scripts;
	// it searches the menu for the given language code, and triggers it if available
	
	// Determine all aliases for the specified lang
	QList<QString> langAliases;
	foreach (const QString& dictKey, TWUtils::getDictionaryList()->uniqueKeys()) {
		if(TWUtils::getDictionaryList()->values(dictKey).contains(lang))
			langAliases += TWUtils::getDictionaryList()->values(dictKey);
	}
	langAliases.removeAll(lang);
	langAliases.prepend(lang);
	
	bool found = false;
	if (menuSpelling) {
		QAction *chosen = menuSpelling->actions()[0]; // default is None
		foreach (QAction *act, menuSpelling->actions()) {
			foreach(QString alias, langAliases) {
				if (act->text() == alias || act->text().contains("(" + alias + ")")) {
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

QString TeXDocument::spellcheckLanguage() const
{
	return TWUtils::getLanguageForDictionary(pHunspell);
}

void TeXDocument::reloadSpellcheckerMenu()
{
	Q_ASSERT(menuSpelling != NULL);
	Q_ASSERT(menuSpelling->actions().size() > 0);
	
	QActionGroup * group = menuSpelling->actions()[0]->actionGroup();
	Q_ASSERT(group != NULL);
	
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
	foreach (const QString& dictKey, TWUtils::getDictionaryList()->uniqueKeys()) {
		QAction *act;
		QString dict, label;
		QLocale loc;

		foreach (dict, TWUtils::getDictionaryList()->values(dictKey)) {
			loc = QLocale(dict);
			if (loc.language() != QLocale::C) break;
		}

		if (loc.language() == QLocale::C)
			label = dict;
		else {
			label = QLocale::languageToString(loc.language());
			QLocale::Country country = loc.country();
			if (country != QLocale::AnyCountry)
				label += " - " + QLocale::countryToString(country);
			label += " (" + dict + ")";
		}

		act = new QAction(label, NULL);
		act->setCheckable(true);
		if (!oldSelected.isEmpty() && label == oldSelected)
			act->setChecked(true);
		connect(act, SIGNAL(triggered()), &dictSignalMapper, SLOT(map()));
		dictSignalMapper.setMapping(act, dict);
		group->addAction(act);
		dictActions << act;
	}
	qSort(dictActions.begin(), dictActions.end(), dictActionLessThan);
	foreach (QAction* dictAction, dictActions)
		menuSpelling->addAction(dictAction);
}

void TeXDocument::clipboardChanged()
{
	actionPaste->setEnabled(textEdit->canPaste());
}

void TeXDocument::editMenuAboutToShow()
{
//	undoAction->setText(tr("Undo ") + undoStack->undoText());
//	redoAction->setText(tr("Redo ") + undoStack->redoText());
	actionSelect_All->setEnabled(!textEdit->document()->isEmpty());
}

void TeXDocument::newFile()
{
	TeXDocument *doc = new TeXDocument;
	doc->selectWindow();
	doc->textEdit->updateLineNumberAreaWidth(0);
	doc->runHooks("NewFile");
}

void TeXDocument::newFromTemplate()
{
	QString templateName = TemplateDialog::doTemplateDialog();
	if (!templateName.isEmpty()) {
		TeXDocument *doc = NULL;
		if (isUntitled && textEdit->document()->isEmpty() && !isWindowModified()) {
			loadFile(templateName, true);
			doc = this;
		}
		else {
			doc = new TeXDocument(templateName, true);
		}
		if (doc != NULL) {
			doc->makeUntitled();
			doc->selectWindow();
			doc->textEdit->updateLineNumberAreaWidth(0);
			doc->runHooks("NewFromTemplate");
		}
	}
}

void TeXDocument::makeUntitled()
{
	setCurrentFile("");
	actionRemove_Aux_Files->setEnabled(false);
}

void TeXDocument::open()
{
	QFileDialog::Options options = 0;
#if defined(Q_OS_DARWIN)
		/* use a sheet if we're calling Open from an empty, untitled, untouched window; otherwise use a separate dialog */
	if (!(isUntitled && textEdit->document()->isEmpty() && !isWindowModified()))
		options = QFileDialog::DontUseSheet;
#elif defined(Q_OS_WIN)
	if(TWApp::GetWindowsVersion() < 0x06000000) options |= QFileDialog::DontUseNativeDialog;
#endif
	QSETTINGS_OBJECT(settings);
	QString lastOpenDir = settings.value("openDialogDir").toString();
	if (lastOpenDir.isEmpty())
#if defined(MIKTEX_WINDOWS)
	    lastOpenDir = MiKTeX::Core::Utils::GetFolderPath(CSIDL_MYDOCUMENTS, CSIDL_MYDOCUMENTS, true).GetData();
#else
		lastOpenDir = QDir::homePath();
#endif
	QStringList files = QFileDialog::getOpenFileNames(this, QString(tr("Open File")), lastOpenDir, TWUtils::filterList()->join(";;"), NULL, options);
	foreach (QString fileName, files) {
		if (!fileName.isEmpty()) {
			TWApp::instance()->openFile(fileName); // not TeXDocument::open() - give the app a chance to open as PDF
		}
	}
}

TeXDocument* TeXDocument::open(const QString &fileName)
{
	TeXDocument *doc = NULL;
	if (!fileName.isEmpty()) {
		doc = findDocument(fileName);
		if (doc == NULL) {
			if (isUntitled && textEdit->document()->isEmpty() && !isWindowModified()) {
				loadFile(fileName);
				doc = this;
			}
			else {
				doc = new TeXDocument(fileName);
				if (doc->isUntitled) {
					delete doc;
					doc = NULL;
				}
			}
		}
	}
	if (doc != NULL)
		doc->selectWindow();
	return doc;
}

TeXDocument* TeXDocument::openDocument(const QString &fileName, bool activate, bool raiseWindow, int lineNo, int selStart, int selEnd) // static
{
	TeXDocument *doc = findDocument(fileName);
	if (doc == NULL) {
		if (docList.count() == 1) {
			doc = docList[0];
			doc = doc->open(fileName); // open into existing window if untitled/empty
		}
		else {
			doc = new TeXDocument(fileName);
			if (doc->isUntitled) {
				delete doc;
				doc = NULL;
			}
		}
	}
	if (doc != NULL) {
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

void TeXDocument::closeEvent(QCloseEvent *event)
{
	if (process != NULL) {
		if (QMessageBox::question(this, tr("Abort typesetting?"), tr("A typesetting process is still running and must be stopped before closing this window.\nDo you want to stop it now?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) {
			event->ignore();
			return;
		}
		else
			interrupt();
	}

	if (maybeSave()) {
		event->accept();
		saveRecentFileInfo();
		deleteLater();
	}
	else
		event->ignore();
}

bool TeXDocument::event(QEvent *event) // based on example at doc.trolltech.com/qq/qq18-macfeatures.html
{
	switch (event->type()) {
		case QEvent::IconDrag:
			if (isActiveWindow()) {
				event->accept();
				Qt::KeyboardModifiers mods = qApp->keyboardModifiers();
				if (mods == Qt::NoModifier) {
					QDrag *drag = new QDrag(this);
					QMimeData *data = new QMimeData();
					data->setUrls(QList<QUrl>() << QUrl::fromLocalFile(curFile));
					drag->setMimeData(data);
					QPixmap dragIcon(":/images/images/TeXworks-doc-48.png");
					drag->setPixmap(dragIcon);
					drag->setHotSpot(QPoint(dragIcon.width() - 5, 5));
					drag->start(Qt::LinkAction | Qt::CopyAction);
				}
				else if (mods == Qt::ShiftModifier) {
					QMenu menu(this);
					connect(&menu, SIGNAL(triggered(QAction*)), this, SLOT(openAt(QAction*)));
					QFileInfo info(curFile);
					QAction *action = menu.addAction(info.fileName());
					action->setIcon(QIcon(":/images/images/TeXworks-doc.png"));
					QStringList folders = info.absolutePath().split('/');
					QStringListIterator it(folders);
					it.toBack();
					while (it.hasPrevious()) {
						QString str = it.previous();
						QIcon icon;
						if (!str.isEmpty()) {
							icon = style()->standardIcon(QStyle::SP_DirClosedIcon, 0, this);
						}
						else {
							str = "/";
							icon = style()->standardIcon(QStyle::SP_DriveHDIcon, 0, this);
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

		case QEvent::WindowActivate:
			showFloaters();
			emit activatedWindow(this);
			break;

		default:
			break;
	}
	return QMainWindow::event(event);
}

void TeXDocument::openAt(QAction *action)
{
	QString path = curFile.left(curFile.indexOf(action->text())) + action->text();
	if (path == curFile)
		return;
	QProcess proc;
	proc.start("/usr/bin/open", QStringList() << path, QIODevice::ReadOnly);
	proc.waitForFinished();
}

bool TeXDocument::save()
{
	if (isUntitled)
		return saveAs();
	else
		return saveFile(curFile);
}

bool TeXDocument::saveAll()
{
	bool savedAll = true;
	foreach (TeXDocument* doc, docList) {
		if (doc->textEdit->document()->isModified()) {
			if (!doc->save()) {
				savedAll = false;
			}
		}
	}
	return savedAll;
}

bool TeXDocument::saveAs()
{
	QFileDialog::Options	options = 0;
#if defined(Q_OS_WIN)
	if(TWApp::GetWindowsVersion() < 0x06000000) options |= QFileDialog::DontUseNativeDialog;
#endif
	QString selectedFilter = TWUtils::chooseDefaultFilter(curFile, *(TWUtils::filterList()));;

	// for untitled docs, default to the last dir used, or $HOME if no saved value
	QSETTINGS_OBJECT(settings);
	QString lastSaveDir = settings.value("saveDialogDir").toString();
	if (lastSaveDir.isEmpty() || !QDir(lastSaveDir).exists())
#if defined(MIKTEX_WINDOWS)
	    lastSaveDir = MiKTeX::Core::Utils::GetFolderPath(CSIDL_MYDOCUMENTS, CSIDL_MYDOCUMENTS, true).GetData();
#else
		lastSaveDir = QDir::homePath();
#endif
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
													isUntitled ? lastSaveDir + "/" + curFile : curFile,
													TWUtils::filterList()->join(";;"),
													&selectedFilter, options);
	if (fileName.isEmpty())
		return false;

	// save the old document in "Recent Files"
	saveRecentFileInfo();

	// add extension from the selected filter, if unique and not already present
	QRegExp re("\\(\\*(\\.[^ *]+)\\)");
	if (re.indexIn(selectedFilter) >= 0) {
		QString ext = re.cap(1);
		if (!fileName.endsWith(ext, Qt::CaseInsensitive) && !fileName.endsWith("."))
			fileName.append(ext);
	}
	
	if (fileName != curFile && pdfDoc) {
		// For the pdf, it is as if it's source doc was closed
		// Note that this may result in the pdf being closed!
		pdfDoc->texClosed(this);
		// The pdf connection is no longer (necessarily) valid. Detach it for
		// now (the correct connection will be reestablished on next typeset).
		detachPdf();
	}

	QFileInfo info(fileName);
	settings.setValue("saveDialogDir", info.absolutePath());
	
	return saveFile(fileName);
}

bool TeXDocument::maybeSave()
{
	if (textEdit->document()->isModified()) {
		QMessageBox::StandardButton ret;
		QMessageBox msgBox(QMessageBox::Warning, tr(TEXWORKS_NAME),
						   tr("The document \"%1\" has been modified.\n"
							  "Do you want to save your changes?")
						   .arg(TWUtils::strippedName(curFile)),
						   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
						   this);
		msgBox.button(QMessageBox::Discard)->setShortcut(QKeySequence(tr("Ctrl+D", "shortcut: Don't Save")));
		msgBox.setWindowModality(Qt::WindowModal);
		ret = (QMessageBox::StandardButton)msgBox.exec();
		if (ret == QMessageBox::Save)
			return save();
		else if (ret == QMessageBox::Cancel)
			return false;
	}
	return true;
}

bool TeXDocument::saveFilesHavingRoot(const QString& aRootFile)
{
	foreach (TeXDocument* doc, docList) {
		if (doc->getRootFilePath() == aRootFile) {
			if (doc->textEdit->document()->isModified() && !doc->save())
				return false;
		}
	}
	return true;
}

const QString& TeXDocument::getRootFilePath()
{
	findRootFilePath();
	return rootFilePath;
}

void TeXDocument::revert()
{
	if (!isUntitled) {
		QMessageBox	messageBox(QMessageBox::Warning, tr(TEXWORKS_NAME),
					tr("Do you want to discard all changes to the document \"%1\", and revert to the last saved version?")
					   .arg(TWUtils::strippedName(curFile)), QMessageBox::Cancel, this);
		QAbstractButton *revertButton = messageBox.addButton(tr("Revert"), QMessageBox::DestructiveRole);
		revertButton->setShortcut(QKeySequence(tr("Ctrl+R", "shortcut: Revert")));
		messageBox.setDefaultButton(QMessageBox::Cancel);
		messageBox.setWindowModality(Qt::WindowModal);
		messageBox.exec();
		if (messageBox.clickedButton() == revertButton)
			loadFile(curFile);
	}
}

void TeXDocument::maybeEnableSaveAndRevert(bool modified)
{
	actionSave->setEnabled(modified || isUntitled);
	actionRevert_to_Saved->setEnabled(modified && !isUntitled);
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
	NULL
};

QTextCodec *TeXDocument::scanForEncoding(const QString &peekStr, bool &hasMetadata, QString &reqName)
{
	// peek at the file for %!TEX encoding = ....
	QRegExp re("% *!TEX +encoding *= *([^\\r\\n\\x2029]+)[\\r\\n\\x2029]", Qt::CaseInsensitive);
	int pos = re.indexIn(peekStr);
	QTextCodec *reqCodec = NULL;
	if (pos > -1) {
		hasMetadata = true;
		reqName = re.cap(1).trimmed();
		reqCodec = QTextCodec::codecForName(reqName.toLatin1());
		if (reqCodec == NULL) {
			static QHash<QString,QString> *synonyms = NULL;
			if (synonyms == NULL) {
				synonyms = new QHash<QString,QString>;
				for (int i = 0; texshopSynonyms[i] != NULL; i += 2)
					synonyms->insert(QString(texshopSynonyms[i]).toLower(), texshopSynonyms[i+1]);
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

QString TeXDocument::readFile(const QString &fileName,
							  QTextCodec **codecUsed,
							  int *lineEndings,
							  QTextCodec * forceCodec)
	// reads the text from a file, after checking for %!TEX encoding.... metadata
	// sets codecUsed to the QTextCodec used to read the text
	// returns a null (not just empty) QString on failure
{
	if (lineEndings != NULL) {
		// initialize to default for the platform
#if defined(Q_OS_WIN)
		*lineEndings = kLineEnd_CRLF;
#else
		*lineEndings = kLineEnd_LF;
#endif
	}
	
	utf8BOM = false;
	QFile file(fileName);
	// Not using QFile::Text because this prevents us reading "classic" Mac files
	// with CR-only line endings. See issue #242.
	if (!file.open(QFile::ReadOnly)) {
		QMessageBox::warning(this, tr(TEXWORKS_NAME),
							 tr("Cannot read file \"%1\":\n%2")
							 .arg(fileName)
							 .arg(file.errorString()));
		return QString();
	}

	QByteArray peekBytes(file.peek(PEEK_LENGTH));
	
	QString reqName;
	bool hasMetadata;
	if (forceCodec)
		*codecUsed = forceCodec;
	else {
		*codecUsed = scanForEncoding(QString::fromUtf8(peekBytes), hasMetadata, reqName);
		if (*codecUsed == NULL) {
			*codecUsed = TWApp::instance()->getDefaultCodec();
			if (hasMetadata) {
				if (QMessageBox::warning(this, tr("Unrecognized encoding"),
						tr("The text encoding %1 used in %2 is not supported.\n\n"
						   "It will be interpreted as %3 instead, which may result in incorrect text.")
							.arg(reqName)
							.arg(fileName)
							.arg(QString::fromLatin1((*codecUsed)->name())),
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
	
	if (file.atEnd())
		return QString("");
	else {
		QTextStream in(&file);
		in.setCodec(*codecUsed);
		QString text = in.readAll();

		if (lineEndings != NULL) {
			if (text.contains("\r\n")) {
				text.replace("\r\n", "\n");
				*lineEndings = kLineEnd_CRLF;
			}
			else if (text.contains("\r") && !text.contains("\n")) {
				text.replace("\r", "\n");
				*lineEndings = kLineEnd_CR;
			}
			else
				*lineEndings = kLineEnd_LF;

			if (text.contains("\r")) {
				text.replace("\r", "\n");
				*lineEndings |= kLineEnd_Mixed;
			}
		}

		return text;
	}
}

void TeXDocument::loadFile(const QString &fileName, bool asTemplate /* = false */, bool inBackground /* = false */, bool reload /* = false */, QTextCodec * forceCodec /* = NULL */)
{
	QString fileContents = readFile(fileName, &codec, &lineEndings, forceCodec);
	showLineEndingSetting();
	showEncodingSetting();

	if (fileContents.isNull())
		return;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	deferTagListChanges = true;
	tagListChanged = false;
	textEdit->setPlainText(fileContents);
	deferTagListChanges = false;
	if (tagListChanged)
		emit tagListUpdated();

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
		Q_ASSERT(doc != NULL);
		QAbstractTextDocumentLayout * docLayout = doc->documentLayout();
		Q_ASSERT(docLayout != NULL);

		int tries;
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
			doc->setPlainText(doc->toPlainText());
			QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
		}
		if (tries >= 10) {
			QMessageBox::warning(this, tr("Layout Problem"), tr("A problem occured while laying out the loaded document in the editor. This is caused by an issue in the underlying Qt framework and can cause TeXworks to crash under certain circumstances. The symptoms of this problem are hidden or overlapping lines. To work around this, please try one of the following:\n -) Turn syntax highlighting off and on\n -) Turn line numbers off and on\n -) Resize the window\n\nWe are sorry for the inconvenience."));
		}
	}

	QApplication::restoreOverrideCursor();

	if (asTemplate) {
		lastModified = QDateTime();
	}
	else {
		setCurrentFile(fileName);
		if (!reload) {
			QSETTINGS_OBJECT(settings);
			if (!inBackground && settings.value("openPDFwithTeX", kDefault_OpenPDFwithTeX).toBool()) {
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
			QFileInfo info(fileName);
			settings.setValue("openDialogDir", info.canonicalPath());
		}

		statusBar()->showMessage(tr("File \"%1\" loaded").arg(TWUtils::strippedName(curFile)),
								 kStatusMessageDuration);
		setupFileWatcher();
	}
	maybeEnableSaveAndRevert(false);

	if (!reload) {
		bool autoPlace = true;
		QMap<QString,QVariant> properties = TWApp::instance()->getFileProperties(curFile);
		if (properties.contains("geometry")) {
			restoreGeometry(properties.value("geometry").toByteArray());
			autoPlace = false;
		}
		if (properties.contains("state"))
			restoreState(properties.value("state").toByteArray(), kTeXWindowStateVersion);

		if (properties.contains("selStart")) {
			QTextCursor c(textEdit->document());
			c.setPosition(properties.value("selStart").toInt());
			c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, properties.value("selLength", 0).toInt());
			textEdit->setTextCursor(c);
		}

		if (properties.contains("quotesMode"))
			setSmartQuotesMode(properties.value("quotesMode").toString());
		if (properties.contains("indentMode"))
			setAutoIndentMode(properties.value("indentMode").toString());
		if (properties.contains("syntaxMode"))
			setSyntaxColoringMode(properties.value("syntaxMode").toString());
		if (properties.contains("wrapLines"))
			setWrapLines(properties.value("wrapLines").toBool());
		if (properties.contains("lineNumbers"))
			setLineNumbers(properties.value("lineNumbers").toBool());
	
		if (pdfDoc) {
			if (properties.contains("pdfgeometry")) {
				pdfDoc->restoreGeometry(properties.value("pdfgeometry").toByteArray());
				autoPlace = false;
			}
			if (properties.contains("pdfstate"))
				pdfDoc->restoreState(properties.value("pdfstate").toByteArray(), kPDFWindowStateVersion);
		}

		if (autoPlace)
			sideBySide();

		if (pdfDoc)
			pdfDoc->show();

		selectWindow();
		saveRecentFileInfo();
	}
	
	editor()->updateLineNumberAreaWidth(0);
	
	runHooks("LoadFile");
}

void TeXDocument::delayedInit()
{
	if (!highlighter) {
		QSETTINGS_OBJECT(settings);

		highlighter = new TeXHighlighter(textEdit->document(), this);
		connect(textEdit, SIGNAL(rehighlight()), highlighter, SLOT(rehighlight()));

		// set up syntax highlighting
		// First, use the current file's syntaxMode property (if available)
		QMap<QString,QVariant> properties = TWApp::instance()->getFileProperties(curFile);
		if (properties.contains("syntaxMode"))
			setSyntaxColoringMode(properties.value("syntaxMode").toString());
		// Secondly, try the global settings
		else if (settings.contains("syntaxColoring"))
				setSyntaxColoringMode(settings.value("syntaxColoring").toString());
		// Lastly, use the default setting
		else {
			// This should mimick the code in PrefsDialog::doPrefsDialog()
			QStringList syntaxOptions = TeXHighlighter::syntaxOptions();
			if (kDefault_SyntaxColoring < syntaxOptions.count())
				setSyntaxColoringMode(syntaxOptions[kDefault_SyntaxColoring]);
			else
				setSyntaxColoringMode("");
		}

		// set the default spell checking language
		setSpellcheckLanguage(settings.value("language").toString());

		// contentsChanged() parses the modlines (thus possibly overrinding the spell checking language)
		contentsChanged(0, 0, 0);
	}
}

#define FILE_MODIFICATION_ACCURACY	1000	// in msec
void TeXDocument::reloadIfChangedOnDisk()
{
	if (isUntitled || !lastModified.isValid())
		return;

	QDateTime fileModified = QFileInfo(curFile).lastModified();
	if (!fileModified.isValid() || fileModified == lastModified)
		return;

	clearFileWatcher(); // stop watching until next save or reload
	if (textEdit->document()->isModified()) {
		if (QMessageBox::warning(this, tr("File changed on disk"),
								 tr("%1 has been modified by another program.\n\n"
									"Do you want to discard your current changes, and reload the file from disk?")
								 .arg(curFile),
								 QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Cancel) {
			lastModified = QDateTime();	// invalidate the timestamp
			return;
		}
	}
	// user chose to discard, or there were no local changes
	// save the current cursor position
	QTextCursor cur;
	int oldSelStart, oldSelEnd, oldBlockStart, oldBlockEnd;
	int xPos = 0, yPos = 0;
	QString oldSel;

	// Store the selection (note that oldSelStart == oldSelEnd if there is
	// no selection)
	cur = textEdit->textCursor();
	oldSelStart = cur.selectionStart();
	oldSelEnd = cur.selectionEnd();
	oldSel = cur.selectedText();

	// Get the block number and the offset in the block of the start of the
	// selection
	cur.setPosition(oldSelStart);
	oldBlockStart = cur.blockNumber();
	oldSelStart -= cur.block().position();

	// Get the block number and the offset in the block of the end of the
	// selection
	cur.setPosition(oldSelEnd);
	oldBlockEnd = cur.blockNumber();
	oldSelEnd -= cur.block().position();

	// Get the values of the scroll bars so we can later restore the view
	if (textEdit->horizontalScrollBar())
		xPos = textEdit->horizontalScrollBar()->value();
	if (textEdit->verticalScrollBar())
		yPos = textEdit->verticalScrollBar()->value();

	// Reload the file from the disk
	// Note that the file may change again before the system watcher is enabled
	// again, so we should catch that case (this sometimes occurs with version
	// control systems during commits)
	unsigned int i;
	// Limit this to avoid infinite loops
	for (i = 0; i < 10; ++i) {
		clearFileWatcher(); // stop watching until next save or reload
		// Only reload files at full seconds to avoid problems with limited
		// accuracy of the file system modification timestamps (if the file changes
		// twice in one second, the modification timestamp is not altered and we may
		// miss the second change otherwise)
		while (QDateTime::currentDateTime() <= QFileInfo(curFile).lastModified().addMSecs(FILE_MODIFICATION_ACCURACY))
			; // do nothing
		loadFile(curFile, false, true, true);
		// one final safety check - if the file has not changed, we can safely end this
		if (QDateTime::currentDateTime() > QFileInfo(curFile).lastModified().addMSecs(FILE_MODIFICATION_ACCURACY))
			break;
	}
	if (i == 10) { // the file has been changing constantly - give up and inform the user
		QMessageBox::information(this, tr("File changed on disk"),
								 tr("%1 is constantly being modified by another program.\n\n"
									"Please use \"File > Revert to Saved\" manually when the external process has finished.")
								 .arg(curFile),
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
bool TeXDocument::getPreviewFileName(QString &pdfName)
{
	findRootFilePath();
	if (rootFilePath == "")
		return false;
	QFileInfo fi(rootFilePath);
	pdfName = fi.canonicalPath() + "/" + fi.completeBaseName() + ".pdf";
	fi.setFile(pdfName);
	return fi.exists();
}

bool TeXDocument::openPdfIfAvailable(bool show)
{
	detachPdf();
	actionSide_by_Side->setEnabled(false);
	actionGo_to_Preview->setEnabled(false);

	QString pdfName;
	if (getPreviewFileName(pdfName)) {
		PDFDocument *existingPdf = PDFDocument::findDocument(pdfName);
		if (existingPdf != NULL) {
			pdfDoc = existingPdf;
			pdfDoc->selectWindow();
			pdfDoc->linkToSource(this);
		}
		else {
			pdfDoc = new PDFDocument(pdfName, this);
			if (show)
				pdfDoc->show();
		}
	}

	if (pdfDoc != NULL) {
		actionSide_by_Side->setEnabled(true);
		actionGo_to_Preview->setEnabled(true);
		connect(pdfDoc, SIGNAL(destroyed()), this, SLOT(pdfClosed()));
		connect(this, SIGNAL(destroyed(QObject*)), pdfDoc, SLOT(texClosed(QObject*)));
		return true;
	}
	
	return false;
}

void TeXDocument::pdfClosed()
{
	pdfDoc = NULL;
	actionSide_by_Side->setEnabled(false);
}

bool TeXDocument::saveFile(const QString &fileName)
{
	QFileInfo fileInfo(fileName);
	QDateTime fileModified = fileInfo.lastModified();
	if (fileName == curFile && fileModified.isValid() && fileModified != lastModified) {
		if (QMessageBox::warning(this, tr("File changed on disk"),
								 tr("%1 has been modified by another program.\n\n"
									"Do you want to proceed with saving this file, overwriting the version on disk?")
								 .arg(fileName),
								 QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Cancel) {
			notSaved:
				statusBar()->showMessage(tr("Document \"%1\" was not saved")
										 .arg(TWUtils::strippedName(curFile)),
										 kStatusMessageDuration);
				return false;
		}
	}
	
	QString theText = textEdit->toPlainText();
	switch (lineEndings & kLineEnd_Mask) {
		case kLineEnd_CR:
			theText.replace("\n", "\r");
			break;
		case kLineEnd_LF:
			break;
		case kLineEnd_CRLF:
			theText.replace("\n", "\r\n");
			break;
	}
	
	if (!codec)
		codec = TWApp::instance()->getDefaultCodec();
	if (!codec->canEncode(theText)) {
		if (QMessageBox::warning(this, tr("Text cannot be converted"),
				tr("This document contains characters that cannot be represented in the encoding %1.\n\n"
				   "If you proceed, they will be replaced with default codes. "
				   "Alternatively, you may wish to use a different encoding (such as UTF-8) to avoid loss of data.")
					.arg(QString(codec->name())),
				QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Cancel)
			goto notSaved;
	}

	clearFileWatcher();

	{
		QFile file(fileName);
		if (!file.open(QFile::WriteOnly)) {
			QMessageBox::warning(this, tr(TEXWORKS_NAME),
								 tr("Cannot write file \"%1\":\n%2")
								 .arg(fileName)
								 .arg(file.errorString()));
			setupFileWatcher();
			goto notSaved;
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
			goto notSaved;
		}
		QApplication::restoreOverrideCursor();
	}

	setCurrentFile(fileName);
	statusBar()->showMessage(tr("File \"%1\" saved")
								.arg(TWUtils::strippedName(curFile)),
								kStatusMessageDuration);
	
	QTimer::singleShot(0, this, SLOT(setupFileWatcher()));
	return true;
}

void TeXDocument::clearFileWatcher()
{
	const QStringList files = watcher->files();
	if (files.count() > 0)
		watcher->removePaths(files);	
	const QStringList dirs = watcher->directories();
	if (dirs.count() > 0)
		watcher->removePaths(dirs);	
}

void TeXDocument::setupFileWatcher()
{
	clearFileWatcher();
	if (!isUntitled) {
		QFileInfo info(curFile);
		lastModified = info.lastModified();
		watcher->addPath(curFile);
		watcher->addPath(info.canonicalPath());
	}
}	

void TeXDocument::setCurrentFile(const QString &fileName)
{
	static int sequenceNumber = 1;

	curFile = QFileInfo(fileName).canonicalFilePath();
	isUntitled = curFile.isEmpty();
	if (isUntitled) {
		curFile = tr("untitled-%1.tex").arg(sequenceNumber++);
		setWindowIcon(QApplication::windowIcon());
	}
	else {
		QIcon winIcon;
#if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
		// The Compiz window manager doesn't seem to support icons larger than
		// 128x128, so we add a suitable one first
		winIcon.addFile(":/images/images/TeXworks-doc-128.png");
#endif
		winIcon.addFile(":/images/images/TeXworks-doc.png");
		setWindowIcon(winIcon);
	}

	textEdit->document()->setModified(false);
	setWindowModified(false);

	setWindowTitle(tr("%1[*] - %2").arg(TWUtils::strippedName(curFile)).arg(tr(TEXWORKS_NAME)));

	actionRemove_Aux_Files->setEnabled(!isUntitled);
	
	TWApp::instance()->updateWindowMenus();
}

void TeXDocument::saveRecentFileInfo()
{
	if (isUntitled)
		return;
	
	QMap<QString,QVariant> fileProperties;

	fileProperties.insert("path", curFile);
	fileProperties.insert("geometry", saveGeometry());
	fileProperties.insert("state", saveState(kTeXWindowStateVersion));
	fileProperties.insert("selStart", selectionStart());
	fileProperties.insert("selLength", selectionLength());
	fileProperties.insert("quotesMode", textEdit->getQuotesMode());
	fileProperties.insert("indentMode", textEdit->getIndentMode());
	if (highlighter)
		fileProperties.insert("syntaxMode", highlighter->getSyntaxMode());
	fileProperties.insert("lineNumbers", textEdit->getLineNumbersVisible());
	fileProperties.insert("wrapLines", textEdit->wordWrapMode() == QTextOption::WordWrap);

	if (pdfDoc) {
		fileProperties.insert("pdfgeometry", pdfDoc->saveGeometry());
		fileProperties.insert("pdfstate", pdfDoc->saveState(kPDFWindowStateVersion));
	}

	TWApp::instance()->addToRecentFiles(fileProperties);
}

void TeXDocument::updateRecentFileActions()
{
	TWUtils::updateRecentFileActions(this, recentFileActions, menuOpen_Recent, actionClear_Recent_Files);
}

void TeXDocument::updateWindowMenu()
{
	TWUtils::updateWindowMenu(this, menuWindow);
}

void TeXDocument::updateEngineList()
{
	engine->disconnect(this);
	while (menuRun->actions().count() > 2)
		menuRun->removeAction(menuRun->actions().last());
	while (engineActions->actions().count() > 0)
		engineActions->removeAction(engineActions->actions().last());
	engine->clear();
	foreach (Engine e, TWApp::instance()->getEngineList()) {
		QAction *newAction = new QAction(e.name(), engineActions);
		newAction->setCheckable(true);
		menuRun->addAction(newAction);
		engine->addItem(e.name());
	}
	connect(engine, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(selectedEngine(const QString&)));
	int index = engine->findText(engineName, Qt::MatchFixedString);
	if (index < 0)
		index = engine->findText(TWApp::instance()->getDefaultEngine().name(), Qt::MatchFixedString);
	if (index >= 0)
		engine->setCurrentIndex(index);
}

void TeXDocument::selectedEngine(QAction* engineAction) // sent by actions in menubar menu; update toolbar combo box
{
	engineName = engineAction->text();
	for (int i = 0; i < engine->count(); ++i)
		if (engine->itemText(i) == engineName) {
			engine->setCurrentIndex(i);
			break;
		}
}

void TeXDocument::selectedEngine(const QString& name) // sent by toolbar combo box; need to update menu
{
	engineName = name;
	foreach (QAction *act, engineActions->actions()) {
		if (act->text() == name) {
			act->setChecked(true);
			break;
		}
	}
}

void TeXDocument::showCursorPosition()
{
	QTextCursor cursor = textEdit->textCursor();
	cursor.setPosition(cursor.selectionStart());
	int line = cursor.blockNumber() + 1;
	int total = textEdit->document()->blockCount();
	int col = cursor.position() - textEdit->document()->findBlock(cursor.selectionStart()).position();
	lineNumberLabel->setText(tr("Line %1 of %2; col %3").arg(line).arg(total).arg(col));
	if (actionAuto_Follow_Focus->isChecked())
		emit syncFromSource(curFile, line, col, false);
}

void TeXDocument::showLineEndingSetting()
{
	QString lineEndStr;
	switch (lineEndings & kLineEnd_Mask) {
		case kLineEnd_LF:
			lineEndStr = "LF";
			break;
		case kLineEnd_CRLF:
			lineEndStr = "CRLF";
			break;
		case kLineEnd_CR:
			lineEndStr = "CR";
			break;
	}
	if ((lineEndings & kLineEnd_Mixed) != 0)
		lineEndStr += "*";
	lineEndingLabel->setText(lineEndStr);
}

void TeXDocument::lineEndingPopup(const QPoint loc)
{
	QMenu menu;
	QAction *cr, *lf, *crlf;
	menu.addAction(lf = new QAction("LF (Unix, Mac OS X)", &menu));
	menu.addAction(crlf = new QAction("CRLF (Windows)", &menu));
	menu.addAction(cr = new QAction("CR (Mac Classic)", &menu));
	QAction *result = menu.exec(lineEndingLabel->mapToGlobal(loc));
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

void TeXDocument::showEncodingSetting()
{
	encodingLabel->setText(codec ? codec->name() : "");
}

void TeXDocument::encodingPopup(const QPoint loc)
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
	QAction * a;
	
	if (!isUntitled)
		menu.addAction(reloadAction);
	menu.addAction(BOMAction);
	menu.addSeparator();
	
	foreach (QTextCodec *codec, *TWUtils::findCodecs()) {
		a = new QAction(codec->name(), &menu);
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
										 .arg(QString(codec->name())),
										 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
					return;
				}
			}
			clearFileWatcher(); // stop watching until next save or reload
			loadFile(curFile, false, true, true, codec);
			; // FIXME
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

void TeXDocument::sideBySide()
{
	if (pdfDoc != NULL) {
		TWUtils::sideBySide(this, pdfDoc);
		pdfDoc->selectWindow(false);
		selectWindow();
	}
	else
		placeOnLeft();
}

TeXDocument *TeXDocument::findDocument(const QString &fileName)
{
	QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
	if (canonicalFilePath.isEmpty())
		canonicalFilePath = fileName;
			// file doesn't exist (probably from find-results in a new untitled doc),
			// so just use the name as-is

	foreach (QWidget *widget, qApp->topLevelWidgets()) {
		TeXDocument *theDoc = qobject_cast<TeXDocument*>(widget);
		if (theDoc && theDoc->curFile == canonicalFilePath)
			return theDoc;
	}
	return NULL;
}

void TeXDocument::clear()
{
	textEdit->textCursor().removeSelectedText();
}

QString TeXDocument::getLineText(int lineNo) const
{
	QTextDocument* doc = textEdit->document();
	if (lineNo < 1 || lineNo > doc->blockCount())
		return QString();
#if QT_VERSION >= 0x040400
	return doc->findBlockByNumber(lineNo - 1).text();
#else
	QTextBlock block = doc->findBlock(0);
	while (--lineNo > 0)
		block = block.next();
	return block.text();
#endif
}

void TeXDocument::goToLine(int lineNo, int selStart, int selEnd)
{
	QTextDocument* doc = textEdit->document();
	if (lineNo < 1 || lineNo > doc->blockCount())
		return;
	int oldScrollValue = -1;
	if (textEdit->verticalScrollBar() != NULL)
		oldScrollValue = textEdit->verticalScrollBar()->value();
#if QT_VERSION >= 0x040400
	QTextCursor cursor(doc->findBlockByNumber(lineNo - 1));
#else
	QTextBlock block = doc->findBlock(0);
	while (--lineNo > 0)
		block = block.next();
	QTextCursor cursor(block);
#endif
	if (selStart >= 0 && selEnd >= selStart) {
		cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, selStart);
		cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, selEnd - selStart);
	}
	else
		cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
	textEdit->setTextCursor(cursor);
	maybeCenterSelection(oldScrollValue);
}

void TeXDocument::maybeCenterSelection(int oldScrollValue)
{
	if (oldScrollValue != -1 && textEdit->verticalScrollBar() != NULL) {
		int newScrollValue = textEdit->verticalScrollBar()->value();
		if (newScrollValue != oldScrollValue) {
			int delta = (textEdit->height() - textEdit->cursorRect().height()) / 2;
			if (newScrollValue < oldScrollValue)
				delta = -delta;
			textEdit->verticalScrollBar()->setValue(newScrollValue + delta);
		}
	}
}

void TeXDocument::doFontDialog()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, textEdit->font());
	if (ok) {
		textEdit->setFont(font);
		font.setPointSize(font.pointSize() - 1);
		textEdit_console->setFont(font);
		inputLine->setFont(font);
	}
}

void TeXDocument::doLineDialog()
{
	QTextCursor cursor = textEdit->textCursor();
	cursor.setPosition(cursor.selectionStart());
	bool ok;
	#if QT_VERSION >= 0x050000
	int lineNo = QInputDialog::getInt(this, tr("Go to Line"),
									tr("Line number:"), cursor.blockNumber() + 1,
									1, textEdit->document()->blockCount(), 1, &ok);
	#else
	int lineNo = QInputDialog::getInteger(this, tr("Go to Line"),
									tr("Line number:"), cursor.blockNumber() + 1,
									1, textEdit->document()->blockCount(), 1, &ok);
	#endif
	if (ok)
		goToLine(lineNo);
}

void TeXDocument::doFindDialog()
{
	if (FindDialog::doFindDialog(textEdit) == QDialog::Accepted)
		doFindAgain(true);
}

void TeXDocument::doReplaceDialog()
{
	ReplaceDialog::DialogCode result;
	if ((result = ReplaceDialog::doReplaceDialog(textEdit)) != ReplaceDialog::Cancel)
		doReplace(result);
}

void TeXDocument::prefixLines(const QString &prefix)
{
	QTextCursor cursor = textEdit->textCursor();
	cursor.beginEditBlock();
	int selStart = cursor.selectionStart();
	int selEnd = cursor.selectionEnd();
	cursor.setPosition(selStart);
	if (!cursor.atBlockStart()) {
		cursor.movePosition(QTextCursor::StartOfBlock);
		selStart = cursor.position();
	}
	cursor.setPosition(selEnd);
	if (!cursor.atBlockStart() || selEnd == selStart) {
		cursor.movePosition(QTextCursor::NextBlock);
		selEnd = cursor.position();
	}
	if (selEnd == selStart)
		goto handle_end_of_doc;	// special case - cursor in blank line at end of doc
	if (!cursor.atBlockStart()) {
		cursor.movePosition(QTextCursor::StartOfBlock);
		goto handle_end_of_doc; // special case - unterminated last line
	}
	while (cursor.position() > selStart) {
		cursor.movePosition(QTextCursor::PreviousBlock);
	handle_end_of_doc:
		cursor.insertText(prefix);
		cursor.movePosition(QTextCursor::StartOfBlock);
		selEnd += prefix.length();
	}
	cursor.setPosition(selStart);
	cursor.setPosition(selEnd, QTextCursor::KeepAnchor);
	textEdit->setTextCursor(cursor);
	cursor.endEditBlock();
}

void TeXDocument::doIndent()
{
	prefixLines("\t");
}

void TeXDocument::doComment()
{
	prefixLines("%");
}

void TeXDocument::unPrefixLines(const QString &prefix)
{
	QTextCursor cursor = textEdit->textCursor();
	cursor.beginEditBlock();
	int selStart = cursor.selectionStart();
	int selEnd = cursor.selectionEnd();
	cursor.setPosition(selStart);
	if (!cursor.atBlockStart()) {
		cursor.movePosition(QTextCursor::StartOfBlock);
		selStart = cursor.position();
	}
	cursor.setPosition(selEnd);
	if (!cursor.atBlockStart() || selEnd == selStart) {
		cursor.movePosition(QTextCursor::NextBlock);
		selEnd = cursor.position();
	}
	if (!cursor.atBlockStart()) {
		cursor.movePosition(QTextCursor::StartOfBlock);
		goto handle_end_of_doc; // special case - unterminated last line
	}
	while (cursor.position() > selStart) {
		cursor.movePosition(QTextCursor::PreviousBlock);
	handle_end_of_doc:
		cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
		QString		str = cursor.selectedText();
		if (str == prefix) {
			cursor.removeSelectedText();
			selEnd -= prefix.length();
		}
		else
			cursor.movePosition(QTextCursor::PreviousCharacter);
	}
	cursor.setPosition(selStart);
	cursor.setPosition(selEnd, QTextCursor::KeepAnchor);
	textEdit->setTextCursor(cursor);
	cursor.endEditBlock();
}

void TeXDocument::doUnindent()
{
	unPrefixLines("\t");
}

void TeXDocument::doUncomment()
{
	unPrefixLines("%");
}

void TeXDocument::toUppercase()
{
	replaceSelection(textEdit->textCursor().selectedText().toUpper());
}

void TeXDocument::toLowercase()
{
	replaceSelection(textEdit->textCursor().selectedText().toLower());
}

void TeXDocument::toggleCase()
{
	QString theText = textEdit->textCursor().selectedText();
	for (int i = 0; i < theText.length(); ++i) {
		QCharRef ch = theText[i];
		if (ch.isLower())
			ch = ch.toUpper();
		else
			ch = ch.toLower();
	}
	replaceSelection(theText);
}

void TeXDocument::replaceSelection(const QString& newText)
{
	QTextCursor cursor = textEdit->textCursor();
	int start = cursor.selectionStart();
	cursor.insertText(newText);
	int end = cursor.selectionEnd();
	cursor.setPosition(start);
	cursor.setPosition(end, QTextCursor::KeepAnchor);
	textEdit->setTextCursor(cursor);
}

void TeXDocument::selectRange(int start, int length)
{
	QTextCursor c = textCursor();
	c.setPosition(start);
	c.setPosition(start + length, QTextCursor::KeepAnchor);
	editor()->setTextCursor(c);
}

void TeXDocument::insertText(const QString& text)
{
	textCursor().insertText(text);
}

void TeXDocument::balanceDelimiters()
{
	const QString text = textEdit->toPlainText();
	QTextCursor cursor = textEdit->textCursor();
	int openPos = TWUtils::findOpeningDelim(text, cursor.selectionStart());
	if (openPos >= 0 && openPos < text.length() - 1) {
		do {
			int closePos = TWUtils::balanceDelim(text, openPos + 1, TWUtils::closerMatching(text[openPos]), 1);
			if (closePos < 0)
				break;
			if (closePos >= cursor.selectionEnd()) {
				cursor.setPosition(openPos);
				cursor.setPosition(closePos + 1, QTextCursor::KeepAnchor);
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

void TeXDocument::doHardWrapDialog()
{
	HardWrapDialog dlg(this);
	
	dlg.show();
	if (dlg.exec()) {
		dlg.saveSettings();
		doHardWrap(dlg.mode(), dlg.lineWidth(), dlg.rewrap());
	}
}

void TeXDocument::doHardWrap(int mode, int lineWidth, bool rewrap)
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
	QRegExp breakPattern("\\s+");
	QString newText;
	
	while (!oldText.isEmpty()) {
		int eol = oldText.indexOf(QChar::ParagraphSeparator);
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
				line = line.trimmed().append(QChar(' ')).append(nextLine);
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

		int curLength = 0;
		while (!line.isEmpty()) {
			int breakPoint = line.indexOf(breakPattern);
			int matchLen = breakPattern.matchedLength();
			if (breakPoint == -1) {
				breakPoint = line.length();
				matchLen = 0;
			}
			if (curLength > 0 && curLength + breakPoint >= lineWidth) {
				newText.append(QChar::ParagraphSeparator);
				curLength = 0;
			}
			if (curLength > 0) {
				newText.append(QChar(' '));
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


void TeXDocument::setLineNumbers(bool displayNumbers)
{
	actionLine_Numbers->setChecked(displayNumbers);
	textEdit->setLineNumberDisplay(displayNumbers);
}

void TeXDocument::setWrapLines(bool wrap)
{
	actionWrap_Lines->setChecked(wrap);
	textEdit->setWordWrapMode(wrap ? QTextOption::WordWrap : QTextOption::NoWrap);
}

void TeXDocument::setSyntaxColoring(int index)
{
	if (highlighter)
		highlighter->setActiveIndex(index);
}

void TeXDocument::setSyntaxColoringMode(const QString& mode)
{
	QList<QAction*> actionList = menuSyntax_Coloring->actions();
	
	if (mode == "") {
#if defined(MIKTEX)
		Q_ASSERT(actionSyntaxColoring_None != NULL);
#else
		Q_ASSERT(actionSyntaxColoring != NULL);
#endif
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

void TeXDocument::setSmartQuotesMode(const QString& mode)
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

void TeXDocument::setAutoIndentMode(const QString& mode)
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

void TeXDocument::doFindAgain(bool fromDialog)
{
	QSETTINGS_OBJECT(settings);
	QString	searchText = settings.value("searchText").toString();
	if (searchText.isEmpty())
		return;

	QTextDocument::FindFlags flags = (QTextDocument::FindFlags)settings.value("searchFlags").toInt();

	QRegExp	*regex = NULL;
	if (settings.value("searchRegex").toBool()) {
		regex = new QRegExp(searchText, ((flags & QTextDocument::FindCaseSensitively) != 0)
										? Qt::CaseSensitive : Qt::CaseInsensitive);
		if (!regex->isValid()) {
			qApp->beep();
			statusBar()->showMessage(tr("Invalid regular expression"), kStatusMessageDuration);
			delete regex;
			return;
		}
	}

	if (fromDialog && (settings.value("searchFindAll").toBool() || settings.value("searchAllFiles").toBool())) {
		bool singleFile = true;
		QList<SearchResult> results;
		flags &= ~QTextDocument::FindBackward;
		int docListIndex = 0;
		TeXDocument* theDoc = this;
		while (1) {
			QTextCursor curs(theDoc->textDoc());
			curs.movePosition(QTextCursor::End);
			int rangeStart = 0;
			int rangeEnd = curs.position();
			while (1) {
				curs = doSearch(theDoc->textDoc(), searchText, regex, flags, rangeStart, rangeEnd);
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

			if (settings.value("searchAllFiles").toBool() == false)
				break;
			// go to next document
		next_doc:
			if (docList[docListIndex] == theDoc)
				docListIndex++;
			if (docListIndex == docList.count())
				break;
			theDoc = docList[docListIndex];
			if (theDoc == this)
				goto next_doc;
			singleFile = false;
		}
		
		if (results.count() == 0) {
			qApp->beep();
			statusBar()->showMessage(tr("Not found"), kStatusMessageDuration);
		}
		else {
			SearchResults::presentResults(searchText, results, this, singleFile);
			statusBar()->showMessage(tr("Found %n occurrence(s)", "", results.count()), kStatusMessageDuration);
		}
	}
	else {
		QTextCursor	curs = textEdit->textCursor();
		if (settings.value("searchSelection").toBool() && curs.hasSelection()) {
			int rangeStart = curs.selectionStart();
			int rangeEnd = curs.selectionEnd();
			curs = doSearch(textEdit->document(), searchText, regex, flags, rangeStart, rangeEnd);
		}
		else {
			if ((flags & QTextDocument::FindBackward) != 0) {
				int rangeStart = 0;
				int rangeEnd = curs.selectionStart();
				curs = doSearch(textEdit->document(), searchText, regex, flags, rangeStart, rangeEnd);
				if (curs.isNull() && settings.value("searchWrap").toBool()) {
					curs = QTextCursor(textEdit->document());
					curs.movePosition(QTextCursor::End);
					curs = doSearch(textEdit->document(), searchText, regex, flags, 0, curs.position());
				}
			}
			else {
				int rangeStart = curs.selectionEnd();
				curs.movePosition(QTextCursor::End);
				int rangeEnd = curs.position();
				curs = doSearch(textEdit->document(), searchText, regex, flags, rangeStart, rangeEnd);
				if (curs.isNull() && settings.value("searchWrap").toBool())
					curs = doSearch(textEdit->document(), searchText, regex, flags, 0, rangeEnd);
			}
		}

		if (curs.isNull()) {
			qApp->beep();
			statusBar()->showMessage(tr("Not found"), kStatusMessageDuration);
		}
		else
			textEdit->setTextCursor(curs);
	}

	if (regex != NULL)
		delete regex;
}

void TeXDocument::doReplaceAgain()
{
	doReplace(ReplaceDialog::ReplaceOne);
}

void TeXDocument::doReplace(ReplaceDialog::DialogCode mode)
{
	QSETTINGS_OBJECT(settings);
	
	QString	searchText = settings.value("searchText").toString();
	if (searchText.isEmpty())
		return;
	
	QTextDocument::FindFlags flags = (QTextDocument::FindFlags)settings.value("searchFlags").toInt();

	QRegExp	*regex = NULL;
	if (settings.value("searchRegex").toBool()) {
		regex = new QRegExp(searchText, ((flags & QTextDocument::FindCaseSensitively) != 0)
										? Qt::CaseSensitive : Qt::CaseInsensitive);
		if (!regex->isValid()) {
			qApp->beep();
			statusBar()->showMessage(tr("Invalid regular expression"), kStatusMessageDuration);
			delete regex;
			return;
		}
	}

	QString	replacement = settings.value("replaceText").toString();
	if (regex != NULL) {
		QRegExp escapedChar("\\\\([nt\\\\]|x([0-9A-Fa-f]{4}))");
		int index = -1;
		while ((index = replacement.indexOf(escapedChar, index + 1)) >= 0) {
			QChar ch;
			if (escapedChar.cap(1).length() == 1) {
				// single-char escape code newline/tab/backslash
				ch = escapedChar.cap(1)[0];
				switch (ch.unicode()) {
					case 'n':
						ch = '\n';
						break;
					case 't':
						ch = '\t';
						break;
					case '\\':
						ch = '\\';
						break;
					default:
						// should not happen!
						break;
				}
			}
			else {
				// Unicode char number \xHHHH
				bool ok;
				ch = (QChar)escapedChar.cap(2).toUInt(&ok, 16);
			}
			replacement.replace(index, escapedChar.matchedLength(), ch);
		}
	}
	
	bool allFiles = (mode == ReplaceDialog::ReplaceAll) && settings.value("searchAllFiles").toBool();
	
	bool searchWrap = settings.value("searchWrap").toBool();
	bool searchSel = settings.value("searchSelection").toBool();
	
	int rangeStart, rangeEnd;
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
		QTextCursor curs = doSearch(textEdit->document(), searchText, regex, flags, rangeStart, rangeEnd);
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
			curs = doSearch(textEdit->document(), searchText, regex, flags, rangeStart, rangeEnd);
		}
		if (curs.isNull()) {
			qApp->beep();
			statusBar()->showMessage(tr("Not found"), kStatusMessageDuration);
		}
		else {
			// do replacement
			QString target;
			if (regex != NULL)
				target = textEdit->document()->toPlainText()
							.mid(curs.selectionStart(), curs.selectionEnd() - curs.selectionStart()).replace(*regex, replacement);
			else
				target = replacement;
			curs.insertText(target);
			textEdit->setTextCursor(curs);
		}
	}
	else if (mode == ReplaceDialog::ReplaceAll) {
		if (allFiles) {
			int replacements = 0;
			foreach (TeXDocument* doc, docList)
				replacements += doc->doReplaceAll(searchText, regex, replacement, flags);
			QString numOccurrences = tr("%n occurrence(s)", "", replacements);
			QString numDocuments = tr("%n documents", "", docList.count());
			QString message = tr("Replaced %1 in %2").arg(numOccurrences).arg(numDocuments);
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

	if (regex != NULL)
		delete regex;
}

int TeXDocument::doReplaceAll(const QString& searchText, QRegExp* regex, const QString& replacement,
								QTextDocument::FindFlags flags, int rangeStart, int rangeEnd)
{
	QTextCursor searchRange = textCursor();
	searchRange.select(QTextCursor::Document);
	if (rangeStart < 0)
		rangeStart = searchRange.selectionStart();
	if (rangeEnd < 0)
		rangeEnd = searchRange.selectionEnd();
		
	int replacements = 0;
	bool first = true;
	while (1) {
		QTextCursor curs = doSearch(textEdit->document(), searchText, regex, flags, rangeStart, rangeEnd);
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
		int oldLen = curs.selectionEnd() - curs.selectionStart();
		if (regex != NULL)
			target = textEdit->document()->toPlainText().mid(curs.selectionStart(), oldLen).replace(*regex, replacement);
		else
			target = replacement;
		int newLen = target.length();
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

QTextCursor TeXDocument::doSearch(QTextDocument *theDoc, const QString& searchText, const QRegExp *regex, QTextDocument::FindFlags flags, int s, int e)
{
	QTextCursor curs;
	const QString& docText = theDoc->toPlainText();
	
	if ((flags & QTextDocument::FindBackward) != 0) {
		if (regex != NULL) {
			// this doesn't seem to match \n or even \x2029 for newline
			// curs = theDoc->find(*regex, e, flags);
			int offset = regex->lastIndexIn(docText, e, QRegExp::CaretAtZero);
			while (offset >= s && offset + regex->matchedLength() > e)
				offset = regex->lastIndexIn(docText, offset - 1, QRegExp::CaretAtZero);
			if (offset >= s) {
				curs = QTextCursor(theDoc);
				curs.setPosition(offset);
				curs.setPosition(offset + regex->matchedLength(), QTextCursor::KeepAnchor);
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
		if (regex != NULL) {
			// this doesn't seem to match \n or even \x2029 for newline
			// curs = theDoc->find(*regex, s, flags);
			int offset = regex->indexIn(docText, s, QRegExp::CaretAtZero);
			if (offset >= 0) {
				curs = QTextCursor(theDoc);
				curs.setPosition(offset);
				curs.setPosition(offset + regex->matchedLength(), QTextCursor::KeepAnchor);
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

void TeXDocument::copyToFind()
{
	if (textEdit->textCursor().hasSelection()) {
		QString searchText = textEdit->textCursor().selectedText();
		searchText.replace(QString(0x2029), "\n");
		QSETTINGS_OBJECT(settings);
		// Note: To search for multi-line strings, we currently need regex
		// enabled (since we only have a single search line). If it was not
		// enabled, we also need to ensure that the replaceText is escaped
		// properly
		bool isMultiLine = searchText.contains("\n");
		if (isMultiLine && !settings.value("searchRegex").toBool()) {
			settings.setValue("searchRegex", true);
			settings.setValue("replaceText", QRegExp::escape(settings.value("replaceText").toString()));
		}
		if (settings.value("searchRegex").toBool()) {
			if (isMultiLine)
				settings.setValue("searchText", QRegExp::escape(searchText).replace("\n", "\\n"));
			else
				settings.setValue("searchText", QRegExp::escape(searchText));
		}
		else
			settings.setValue("searchText", searchText);
	}
}

void TeXDocument::copyToReplace()
{
	if (textEdit->textCursor().hasSelection()) {
		QString replaceText = textEdit->textCursor().selectedText();
		replaceText.replace(QString(0x2029), "\n");
		QSETTINGS_OBJECT(settings);
		// Note: To do multi-line replacements, we currently need regex enabled
		// (since we only have a single replace line). If it was not enabled, we
		// also need to ensure that the searchText is escaped properly
		bool isMultiLine = replaceText.contains("\n");
		if (isMultiLine && !settings.value("searchRegex").toBool()) {
			settings.setValue("searchRegex", true);
			settings.setValue("searchText", QRegExp::escape(settings.value("searchText").toString()));
		}
		if (settings.value("searchRegex").toBool()) {
			if (isMultiLine)
				settings.setValue("replaceText", QRegExp::escape(replaceText).replace("\n", "\\n"));
			else
				settings.setValue("replaceText", QRegExp::escape(replaceText));
		}
		else
			settings.setValue("replaceText", replaceText);
	}
}

void TeXDocument::findSelection()
{
	copyToFind();
	doFindAgain();
}

void TeXDocument::showSelection()
{
	int oldScrollValue = -1;
	if (textEdit->verticalScrollBar() != NULL)
		oldScrollValue = textEdit->verticalScrollBar()->value();
	textEdit->ensureCursorVisible();
	maybeCenterSelection(oldScrollValue);
}

void TeXDocument::zoomToLeft(QWidget *otherWindow)
{
	QDesktopWidget *desktop = QApplication::desktop();
	QRect screenRect = desktop->availableGeometry(otherWindow == NULL ? this : otherWindow);
	screenRect.setTop(screenRect.top() + 22);
	screenRect.setLeft(screenRect.left() + 1);
	screenRect.setBottom(screenRect.bottom() - 1);
	screenRect.setRight((screenRect.left() + screenRect.right()) / 2 - 1);
	setGeometry(screenRect);
}

void TeXDocument::typeset()
{
	if (process)
		return;	// this shouldn't happen if we disable the command at the right time

	if (isUntitled || textEdit->document()->isModified())
		if (!save()) {
			statusBar()->showMessage(tr("Cannot process unsaved document"), kStatusMessageDuration);
			return;
		}

	findRootFilePath();
	if (!saveFilesHavingRoot(rootFilePath))
		return;

	QFileInfo fileInfo(rootFilePath);
	if (!fileInfo.isReadable()) {
		statusBar()->showMessage(tr("Root document %1 is not readable").arg(rootFilePath), kStatusMessageDuration);
		return;
	}

	Engine e = TWApp::instance()->getNamedEngine(engine->currentText());
	if (e.program() == "") {
		statusBar()->showMessage(tr("%1 is not properly configured").arg(engine->currentText()), kStatusMessageDuration);
		return;
	}

	process = new QProcess(this);
	updateTypesettingAction();

	QString workingDir = fileInfo.canonicalPath();	// Note that fileInfo refers to the root file
#if defined(Q_OS_WIN)
	// files in the root directory of the current drive have to be handled specially
	// because QFileInfo::canonicalPath() returns a path without trailing slash
	// (i.e., a bare drive letter)
	if (workingDir.length() == 2 && workingDir.endsWith(':'))
		workingDir.append('/');
#endif
	process->setWorkingDirectory(workingDir);

	QStringList env = QProcess::systemEnvironment();
	QStringList binPaths = TWApp::instance()->getBinaryPaths(env);
	
	QString exeFilePath = TWApp::instance()->findProgram(e.program(), binPaths);
	
#if !defined(Q_OS_DARWIN) // not supported on OS X yet :(
	// Add a (customized) TEXEDIT environment variable
	env << QString("TEXEDIT=%1 --position=%d %s").arg(QCoreApplication::applicationFilePath());
	
	#if defined(Q_OS_WIN) // MiKTeX apparently uses it's own variable
	env << QString("MIKTEX_EDITOR=%1 --position=%l \"%f\"").arg(QCoreApplication::applicationFilePath());
	#endif
#endif
	
	if (!exeFilePath.isEmpty()) {
		QStringList args = e.arguments();
		
#if !defined(MIKTEX)
		// for old MikTeX versions: delete $synctexoption if it causes an error
		static bool checkedForSynctex = false;
		static bool synctexSupported = true;
		if (!checkedForSynctex) {
			QString pdftex = TWApp::instance()->findProgram("pdftex", binPaths);
			if (!pdftex.isEmpty()) {
				int result = QProcess::execute(pdftex, QStringList() << "-synctex=1" << "-version");
				synctexSupported = (result == 0);
			}
			checkedForSynctex = true;
		}
		if (!synctexSupported)
			args.removeAll("$synctexoption");
#endif
		args.replaceInStrings("$synctexoption", "-synctex=1");
		args.replaceInStrings("$fullname", fileInfo.fileName());
		args.replaceInStrings("$basename", fileInfo.completeBaseName());
		args.replaceInStrings("$suffix", fileInfo.suffix());
		args.replaceInStrings("$directory", fileInfo.absoluteDir().absolutePath());
		
		textEdit_console->clear();
		if (consoleTabs->isHidden()) {
			keepConsoleOpen = false;
			showConsole();
		}
		else {
			inputLine->show();
		}
		// ensure the window is visible - otherwise we can't see the output
		// panel (and the typeset process appears to hang in case of an error)
		consoleTabs->setCurrentIndex(0);
		raise();
		
		inputLine->setFocus(Qt::OtherFocusReason);
		showPdfWhenFinished = e.showPdf();
		userInterrupt = false;

		process->setEnvironment(env);
		process->setProcessChannelMode(QProcess::MergedChannels);
		
		connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(processStandardOutput()));
		connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
		connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
		
		QString pdfName;
		if (getPreviewFileName(pdfName))
			oldPdfTime = QFileInfo(pdfName).lastModified();
		else
			oldPdfTime = QDateTime();
		
		// Stop watching the pdf document while it is being changed to avoid
		// interference
		if (pdfDoc && pdfDoc->widget())
			pdfDoc->widget()->setWatchForDocumentChangesOnDisk(false);

#if defined(MIKTEX)
		if (!MiKTeX::Core::Session::Get()->UnloadFilenameDatabase())
		{
		  // TODO: log
		}
#endif
		process->start(exeFilePath, args);
	}
	else {
		process->deleteLater();
		process = NULL;
		QMessageBox msgBox(QMessageBox::Critical, tr("Unable to execute %1").arg(e.name()),
							  "<p>" + tr("The program \"%1\" was not found.").arg(e.program()) + "</p>" +
#if defined(Q_OS_WIN)
							  "<p>" + tr("You need a <b>TeX distribution</b> like <a href=\"http://tug.org/texlive/\">TeX Live</a> or <a href=\"http://miktex.org/\">MiKTeX</a> installed on your system to typeset your document.") + "</p>" +
#elif defined(Q_OS_DARWIN)
							  "<p>" + tr("You need a <b>TeX distribution</b> like <a href=\"http://www.tug.org/mactex/\">MacTeX</a> installed on your system to typeset your document.") + "</p>" +
#else // defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
							  "<p>" + tr("You need a <b>TeX distribution</b> like <a href=\"http://tug.org/texlive/\">TeX Live</a> installed on your system to typeset your document. On most systems such a TeX distribution is available as prebuilt package.") + "</p>" +
#endif
							  "<p>" + tr("When a TeX distribution is installed you may need to tell TeXworks where to find it in Edit -> Preferences -> Typesetting.") + "</p>",
							  QMessageBox::Cancel, this);
		msgBox.setDetailedText(
							  tr("Searched in directories:") + "\n" +
							  " * " + binPaths.join("\n * ") + "\n" +
							  tr("Check the configuration of the %1 tool and the path settings in the Preferences dialog.").arg(e.name()));
		msgBox.exec();
		updateTypesettingAction();
	}
}

void TeXDocument::interrupt()
{
	if (process != NULL) {
		userInterrupt = true;
		process->kill();

		// Start watching for changes in the pdf (again)
		if (pdfDoc && pdfDoc->widget())
			pdfDoc->widget()->setWatchForDocumentChangesOnDisk(true);
	}
}

void TeXDocument::updateTypesettingAction()
{
	if (process == NULL) {
		disconnect(actionTypeset, SIGNAL(triggered()), this, SLOT(interrupt()));
		actionTypeset->setIcon(QIcon(":/images/images/runtool.png"));
		actionTypeset->setText(tr("Typeset"));
		connect(actionTypeset, SIGNAL(triggered()), this, SLOT(typeset()));
		if (pdfDoc != NULL)
			pdfDoc->updateTypesettingAction(false);
	}
	else {
		disconnect(actionTypeset, SIGNAL(triggered()), this, SLOT(typeset()));
		actionTypeset->setIcon(QIcon(":/images/tango/process-stop.png"));
		actionTypeset->setText(tr("Abort typesetting"));
		connect(actionTypeset, SIGNAL(triggered()), this, SLOT(interrupt()));
		if (pdfDoc != NULL)
			pdfDoc->updateTypesettingAction(true);
	}
}

void TeXDocument::processStandardOutput()
{
	QByteArray bytes = process->readAllStandardOutput();
	QTextCursor cursor(textEdit_console->document());
	cursor.select(QTextCursor::Document);
	cursor.setPosition(cursor.selectionEnd());
	cursor.insertText(QString::fromUtf8(bytes));
	textEdit_console->setTextCursor(cursor);
}

void TeXDocument::processError(QProcess::ProcessError /*error*/)
{
	if (userInterrupt)
		textEdit_console->append(tr("Process interrupted by user"));
	else
		textEdit_console->append(process->errorString());
	process->kill();
	process->deleteLater();
	process = NULL;
	inputLine->hide();
	updateTypesettingAction();

	// Start watching for changes in the pdf (again)
	if (pdfDoc && pdfDoc->widget())
		pdfDoc->widget()->setWatchForDocumentChangesOnDisk(true);
}

void TeXDocument::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	// Start watching for changes in the pdf (again)
	if (pdfDoc && pdfDoc->widget())
		pdfDoc->widget()->setWatchForDocumentChangesOnDisk(true);

	if (exitStatus != QProcess::CrashExit) {
		QString pdfName;
		if (getPreviewFileName(pdfName)) {
			actionGo_to_Preview->setEnabled(true);
			if (QFileInfo(pdfName).lastModified() != oldPdfTime) {
				// only open/refresh the PDF if it was changed by the typeset process
				if (pdfDoc == NULL || pdfName != pdfDoc->fileName()) {
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
	
	QSETTINGS_OBJECT(settings);
	
	bool shouldHideConsole = false;
	QVariant hideConsoleSetting = settings.value("autoHideConsole", kDefault_HideConsole);
	// Backwards compatibility to Tw 0.4.0 and before
	if (hideConsoleSetting.toString() == "true" || hideConsoleSetting.toString() == "false")
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
	process = NULL;
	updateTypesettingAction();
}

void TeXDocument::executeAfterTypesetHooks()
{
	TWScriptManager * scriptManager = TWApp::instance()->getScriptManager();

	for (int i = consoleTabs->count() - 1; i > 0; --i)
		consoleTabs->removeTab(i);
	
	foreach (TWScript *s, scriptManager->getHookScripts("AfterTypeset")) {
		QVariant result;
		bool success = s->run(this, result);
		if (success && !result.isNull()) {
			QString res = result.toString();
			if (res.startsWith("<html>", Qt::CaseInsensitive)) {
				QTextBrowser *browser = new QTextBrowser(this);
				browser->setOpenLinks(false);
				connect(browser, SIGNAL(anchorClicked(const QUrl&)), this, SLOT(anchorClicked(const QUrl&)));
				browser->setHtml(res);
				consoleTabs->addTab(browser, s->getTitle());
			}
			else {
				QTextEdit *textEdit = new QTextEdit(this);
				textEdit->setPlainText(res);
				textEdit->setReadOnly(true);
				consoleTabs->addTab(textEdit, s->getTitle());
			}
		}
	}
}

void TeXDocument::anchorClicked(const QUrl& url)
{
	if (url.scheme() == "texworks") {
		int line = 0;
		if (url.hasFragment()) {
			line = url.fragment().toLong();
		}
		TeXDocument * target = openDocument(QFileInfo(getRootFilePath()).absoluteDir().filePath(url.path()), true, true, line);
		if (target)
			target->textEdit->setFocus(Qt::OtherFocusReason);
	}
	else {
		TWApp::instance()->openUrl(url);
	}
}

// showConsole() and hideConsole() are used internally to update the visibility;
// they must NOT change the keepConsoleOpen setting that records user choice
void TeXDocument::showConsole()
{
	consoleTabs->show();
	if (process != NULL)
		inputLine->show();
	actionShow_Hide_Console->setText(tr("Hide Console Output"));
}

void TeXDocument::hideConsole()
{
	consoleTabs->hide();
	inputLine->hide();
	actionShow_Hide_Console->setText(tr("Show Console Output"));
}

// this is connected to the user command, so remember the choice
// for when typesetting finishes
void TeXDocument::toggleConsoleVisibility()
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

void TeXDocument::acceptInputLine()
{
	if (process != NULL) {
		QString	str = inputLine->text();
		QTextCursor	curs(textEdit_console->document());
		curs.setPosition(textEdit_console->toPlainText().length());
		textEdit_console->setTextCursor(curs);
		QTextCharFormat	consoleFormat = textEdit_console->currentCharFormat();
		QTextCharFormat inputFormat(consoleFormat);
		inputFormat.setForeground(inputLine->palette().text());
		str.append("\n");
		textEdit_console->insertPlainText(str);
		curs.movePosition(QTextCursor::PreviousCharacter);
		curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, str.length() - 1);
		curs.setCharFormat(inputFormat);
		process->write(str.toUtf8());
		inputLine->clear();
	}
}

void TeXDocument::goToPreview()
{
	if (pdfDoc != NULL)
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

void TeXDocument::syncClick(int lineNo, int col)
{
	if (!isUntitled) {
		// ensure that there is a pdf to receive our signal
		goToPreview();
		emit syncFromSource(curFile, lineNo, col, true);
	}
}

void TeXDocument::contentsChanged(int position, int /*charsRemoved*/, int /*charsAdded*/)
{
	if (position < PEEK_LENGTH) {
		int pos;
		QTextCursor curs(textEdit->document());
		// (begin|end)EditBlock() is a workaround for QTBUG-24718 that causes
		// movePosition() to crash the program under some circumstances.
		// Since we don't change any text in the edit block, it should be a noop
		// in the context of undo/redo.
		curs.beginEditBlock();
		curs.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, PEEK_LENGTH);
		curs.endEditBlock();
		
		QString peekStr = curs.selectedText();
		
		/* Search for engine specification */
		QRegExp re("% *!TEX +(?:TS-)?program *= *([^\\x2029]+)\\x2029", Qt::CaseInsensitive);
		pos = re.indexIn(peekStr);
		if (pos > -1) {
			QString name = re.cap(1).trimmed();
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
		
		/* Search for encoding specification */
		bool hasMetadata;
		QString reqName;
		QTextCodec *newCodec = scanForEncoding(peekStr, hasMetadata, reqName);
		if (newCodec != NULL) {
			codec = newCodec;
			showEncodingSetting();
		}
		
		/* Search for spellcheck specification */
		QRegExp reSpell("% *!TEX +spellcheck *= *([^\\x2029]+)\\x2029", Qt::CaseInsensitive);
		pos = reSpell.indexIn(peekStr);
		if (pos > -1) {
			QString lang = reSpell.cap(1).trimmed();
			setSpellcheckLanguage(lang);
		}
	}
}

void TeXDocument::findRootFilePath()
{
	if (isUntitled) {
		rootFilePath = "";
		return;
	}
	QFileInfo fileInfo(curFile);
	QString rootName;
	QTextCursor curs(textEdit->document());
	curs.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, PEEK_LENGTH);
	QString peekStr = curs.selectedText();
	QRegExp re("% *!TEX +root *= *([^\\x2029]+)\\x2029", Qt::CaseInsensitive);
	int pos = re.indexIn(peekStr);
	if (pos > -1) {
		rootName = re.cap(1).trimmed();
		QFileInfo rootFileInfo(fileInfo.canonicalPath() + "/" + rootName);
		if (rootFileInfo.exists())
			rootFilePath = rootFileInfo.canonicalFilePath();
		else
			rootFilePath = rootFileInfo.filePath();
	}
	else
		rootFilePath = fileInfo.canonicalFilePath();
}

void TeXDocument::addTag(const QTextCursor& cursor, int level, const QString& text)
{
	int index = 0;
	while (index < tags.size()) {
		if (tags[index].cursor.selectionStart() > cursor.selectionStart())
			break;
		++index;
	}
	tags.insert(index, Tag(cursor, level, text));
}

int TeXDocument::removeTags(int offset, int len)
{
	int removed = 0;
	for (int index = tags.count() - 1; index >= 0; --index) {
		if (tags[index].cursor.selectionStart() < offset)
			break;
		if (tags[index].cursor.selectionStart() < offset + len) {
			tags.removeAt(index);
			++removed;
		}
	}
	return removed;
}

void TeXDocument::goToTag(int index)
{
	if (index < tags.count()) {
		textEdit->setTextCursor(tags[index].cursor);
		textEdit->setFocus(Qt::OtherFocusReason);
	}
}

void TeXDocument::tagsChanged()
{
	if (deferTagListChanges)
		tagListChanged = true;
	else
		emit tagListUpdated();
}

void TeXDocument::removeAuxFiles()
{
	findRootFilePath();
	if (rootFilePath.isEmpty())
		return;

	QFileInfo fileInfo(rootFilePath);
	QString jobname = fileInfo.completeBaseName();
	QDir dir(fileInfo.dir());
	
	QStringList filterList = TWUtils::cleanupPatterns().split(QRegExp("\\s+"));
	if (filterList.count() == 0)
		return;
	for (int i = 0; i < filterList.count(); ++i)
		filterList[i].replace("$jobname", jobname);
	
	dir.setNameFilters(filterList);
	QStringList auxFileList = dir.entryList(QDir::Files | QDir::CaseSensitive, QDir::Name);
	if (auxFileList.count() > 0)
		ConfirmDelete::doConfirmDelete(dir, auxFileList);
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

void TeXDocument::dragEnterEvent(QDragEnterEvent *event)
{
	event->ignore();
	if (event->mimeData()->hasUrls()) {
		const QList<QUrl> urls = event->mimeData()->urls();
		foreach (const QUrl& url, urls) {
			if (url.scheme() == "file") {
				event->acceptProposedAction();
				break;
			}
		}
	}
}

void TeXDocument::dragMoveEvent(QDragMoveEvent *event)
{
	if (event->proposedAction() == INSERT_DOCUMENT_TEXT || event->proposedAction() == CREATE_INCLUDE_COMMAND) {
		if (dragSavedCursor.isNull())
			dragSavedCursor = textEdit->textCursor();
		QTextCursor curs = textEdit->cursorForPosition(textEdit->mapFromGlobal(mapToGlobal(event->pos())));
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

void TeXDocument::dragLeaveEvent(QDragLeaveEvent *event)
{
	if (!dragSavedCursor.isNull()) {
		textEdit->setTextCursor(dragSavedCursor);
		dragSavedCursor = QTextCursor();
	}
	event->accept();
}

void TeXDocument::dropEvent(QDropEvent *event)
{
	if (event->mimeData()->hasUrls()) {
		Qt::DropAction action = event->proposedAction();
		const QList<QUrl> urls = event->mimeData()->urls();
		bool editBlockStarted = false;
		QString text;
		QTextCursor curs = textEdit->cursorForPosition(textEdit->mapFromGlobal(mapToGlobal(event->pos())));
		foreach (const QUrl& url, urls) {
			if (url.scheme() == "file") {
				QString fileName = url.toLocalFile();
				switch (action) {
					case OPEN_FILE_IN_NEW_WINDOW:
						if(!TWUtils::isImageFile(fileName)) {
							TWApp::instance()->openFile(fileName);
							break;
						}
						// for graphic files, fall through (there's no point in
						// trying to open binary files as text

					case INSERT_DOCUMENT_TEXT:
						if (!TWUtils::isPDFfile(fileName) && !TWUtils::isImageFile(fileName) && !TWUtils::isPostscriptFile(fileName)) {
							QTextCodec *codecUsed;
							text = readFile(fileName, &codecUsed);
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

					case CREATE_INCLUDE_COMMAND:
						if (!editBlockStarted) {
							curs.beginEditBlock();
							editBlockStarted = true;
						}
						textEdit->setTextCursor(curs);
						if (TWUtils::isPDFfile(fileName))
							text = TWUtils::includePdfCommand();
						else if (TWUtils::isImageFile(fileName))
							text = TWUtils::includeImageCommand();
						else if (TWUtils::isPostscriptFile(fileName))
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

void TeXDocument::detachPdf()
{
	if (pdfDoc != NULL) {
		disconnect(pdfDoc, SIGNAL(destroyed()), this, SLOT(pdfClosed()));
		disconnect(this, SIGNAL(destroyed(QObject*)), pdfDoc, SLOT(texClosed(QObject*)));
		pdfDoc = NULL;
	}
}
#if defined(MIKTEX_TODO_PRINT)
// see http://code.google.com/p/texworks/issues/detail?id=78#c1
void TeXDocument::print()
{
	QPrinter printer(QPrinter::HighResolution);
	QPrintDialog printDlg(&printer, this);

	// Set up some basic information about the document
	printer.setCreator(TEXWORKS_NAME);
	printer.setDocName(QFileInfo(curFile).baseName());

	// do some setup for the print dialog
	// Note: no page range since we don't know how many pages this will be.
	printDlg.setOption(QAbstractPrintDialog::PrintToFile, true);
	printDlg.setOption(QAbstractPrintDialog::PrintSelection, true);
	printDlg.setOption(QAbstractPrintDialog::PrintPageRange, false);
	printDlg.setOption(QAbstractPrintDialog::PrintCollateCopies, true);
	printDlg.setWindowTitle(tr("Print %1").arg(QFileInfo(curFile).fileName()));

	// show the print dialog to the user
	if(printDlg.exec() != QDialog::Accepted) return;
	
	textEdit->print(&printer);
}
#endif
