/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2018  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef TeXDocument_H
#define TeXDocument_H

#include "TWScriptable.h"

#include <QList>
#include <QRegExp>
#include <QProcess>
#include <QDateTime>
#include <QSignalMapper>

#include "ui_TeXDocument.h"

#include "FindDialog.h"
#include "TWApp.h"
#include "ClickableLabel.h"

#include <hunspell.h>

class QAction;
class QMenu;
class QTextEdit;
class QToolBar;
class QLabel;
class QComboBox;
class QActionGroup;
class QTextCodec;
class QFileSystemWatcher;

class TeXHighlighter;
class PDFDocument;

const int kTeXWindowStateVersion = 1; // increment this if we add toolbars/docks/etc

class TeXDocument : public TWScriptable, private Ui::TeXDocument
{
	Q_OBJECT

public:
	TeXDocument();
	TeXDocument(const QString &fileName, bool asTemplate = false);

	virtual ~TeXDocument();

	static TeXDocument *findDocument(const QString &fileName);
	static QList<TeXDocument*> documentList()
		{
			return docList;
		}
	static TeXDocument *openDocument(const QString &fileName, bool activate = true, bool raiseWindow = true,
									 int lineNo = 0, int selStart = -1, int selEnd = -1);

	TeXDocument *open(const QString &fileName);
	void makeUntitled();
	bool untitled()
		{ return isUntitled; }
	QString fileName() const
		{ return curFile; }
	QTextCursor textCursor() const
		{ return textEdit->textCursor(); }
	QTextDocument* textDoc()
		{ return textEdit->document(); }
	QString getLineText(int lineNo) const;
	CompletingEdit* editor()
		{ return textEdit; }
	int cursorPosition() const { return textCursor().position(); }
	int selectionStart() const { return textCursor().selectionStart(); }
	int selectionLength() const { return textCursor().selectionEnd() - textCursor().selectionStart(); }
	QString getCurrentCodecName() const { return (codec ? QString::fromUtf8(codec->name().constData()) : QString()); }
	bool getUTF8BOM() const { return utf8BOM; }
	
	QString spellcheckLanguage() const;

	PDFDocument* pdfDocument()
		{ return pdfDoc; }

	void addTag(const QTextCursor& cursor, int level, const QString& text);
	int removeTags(int offset, int len);
	void goToTag(int index);
	void tagsChanged();

	bool isModified() const { return textEdit->document()->isModified(); }
	void setModified(const bool m = true) { textEdit->document()->setModified(m); }

	class Tag {
	public:
		QTextCursor	cursor;
		int			level;
		QString		text;
		Tag(const QTextCursor& curs, int lvl, const QString& txt)
			: cursor(curs), level(lvl), text(txt) { };
	};
	const QList<Tag> getTags() const
		{ return tags; }

	Q_PROPERTY(int cursorPosition READ cursorPosition STORED false)
	Q_PROPERTY(QString selection READ selectedText STORED false)
	Q_PROPERTY(int selectionStart READ selectionStart STORED false)
	Q_PROPERTY(int selectionLength READ selectionLength STORED false)
	Q_PROPERTY(QString consoleOutput READ consoleText STORED false)
	Q_PROPERTY(QString text READ text STORED false)
    Q_PROPERTY(QString fileName READ fileName)
	Q_PROPERTY(QString rootFileName READ getRootFilePath STORED false)
	Q_PROPERTY(bool untitled READ untitled STORED false)
	Q_PROPERTY(bool modified READ isModified WRITE setModified STORED false)
	Q_PROPERTY(QString spellcheckLanguage READ spellcheckLanguage WRITE setSpellcheckLanguage STORED false)
	Q_PROPERTY(QString currentCodecName READ getCurrentCodecName STORED false)
	Q_PROPERTY(bool writeUTF8BOM READ getUTF8BOM STORED false)
	
signals:
	void syncFromSource(const QString& sourceFile, int lineNo, int col, bool activatePreview);
	void activatedWindow(QWidget*);
	void tagListUpdated();
	void asyncFlashStatusBarMessage(const QString & msg, const int timeout = 0);

protected:
	virtual void changeEvent(QEvent *event);
	virtual void closeEvent(QCloseEvent *event);
	virtual bool event(QEvent *event);
	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dragMoveEvent(QDragMoveEvent *event);
	virtual void dragLeaveEvent(QDragLeaveEvent *event);
	virtual void dropEvent(QDropEvent *event);

public slots:
	void typeset();
	void interrupt();
	void newFile();
	void newFromTemplate();
	void open();
	bool save();
	bool saveAll();
	bool saveAs();
	void revert();
	void clear();
	void doFontDialog();
	void doLineDialog();
	void doFindDialog();
	void doFindAgain(bool fromDialog = false);
	void doReplaceDialog();
	void doReplaceAgain();
	void doIndent();
	void doUnindent();
	void doComment();
	void doUncomment();
	void toUppercase();
	void toLowercase();
	void toggleCase();
	void balanceDelimiters();
	void doHardWrapDialog();
	void doInsertCitationsDialog();
	void setLineNumbers(bool displayNumbers);
	void setWrapLines(bool wrap);
	void setSyntaxColoring(int index);
	void copyToFind();
	void copyToReplace();
	void findSelection();
	void showSelection();
	void toggleConsoleVisibility();
	void goToPreview();
	void syncClick(int lineNo, int col);
	void openAt(QAction *action);
	void sideBySide();
	void removeAuxFiles();
	void setSpellcheckLanguage(const QString& lang);
	void reloadSpellcheckerMenu();
	void selectRange(int start, int length = 0);
	void insertText(const QString& text);
	void selectAll() { textEdit->selectAll(); }
 	void setWindowModified(bool modified) {
		QMainWindow::setWindowModified(modified);
		TWApp::instance()->updateWindowMenus();
	}
	void setSmartQuotesMode(const QString& mode);
	void setAutoIndentMode(const QString& mode);
	void setSyntaxColoringMode(const QString& mode);
	
private slots:
	void setLangInternal(const QString& lang);
	void maybeEnableSaveAndRevert(bool modified);
	void clipboardChanged();
	void doReplace(ReplaceDialog::DialogCode mode);
	void pdfClosed();
	void updateRecentFileActions();
	void updateWindowMenu();
	void updateEngineList();
	void showCursorPosition();
	void editMenuAboutToShow();
	void processStandardOutput();
	void processError(QProcess::ProcessError error);
	void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void acceptInputLine();
	void selectedEngine(QAction* engineAction);
	void selectedEngine(const QString& name);
	void contentsChanged(int position, int charsRemoved, int charsAdded);
	void reloadIfChangedOnDisk();
	void setupFileWatcher();
	void lineEndingPopup(const QPoint loc);
	void encodingPopup(const QPoint loc);
	void lineEndingLabelClick(QMouseEvent * event) { lineEndingPopup(event->pos()); }
	void encodingLabelClick(QMouseEvent * event) { encodingPopup(event->pos()); }
	void anchorClicked(const QUrl& url);
	void delayedInit();
#if defined(MIKTEX_TODO_PRINT)
        void print();
#endif

private:
	void init();
	bool maybeSave();
	void detachPdf();
	bool saveFilesHavingRoot(const QString& aRootFile);
	void clearFileWatcher();
	QTextCodec *scanForEncoding(const QString &peekStr, bool &hasMetadata, QString &reqName);
	QString readFile(const QString &fileName, QTextCodec **codecUsed, int *lineEndings = NULL, QTextCodec * forceCodec = NULL);
	void loadFile(const QString &fileName, bool asTemplate = false, bool inBackground = false, bool reload = false, QTextCodec * forceCodec = NULL);
	bool saveFile(const QString &fileName);
	void setCurrentFile(const QString &fileName);
	void saveRecentFileInfo();
	bool getPreviewFileName(QString &pdfName);
	bool openPdfIfAvailable(bool show);
	void replaceSelection(const QString& newText);
	void doHardWrap(int mode, int lineWidth, bool rewrap);
	void zoomToLeft(QWidget *otherWindow);
	QTextCursor doSearch(QTextDocument *theDoc, const QString& searchText, const QRegExp *regex,
						 QTextDocument::FindFlags flags, int rangeStart, int rangeEnd);
	int doReplaceAll(const QString& searchText, QRegExp* regex, const QString& replacement,
						QTextDocument::FindFlags flags, int rangeStart = -1, int rangeEnd = -1);
	void executeAfterTypesetHooks();
	void showConsole();
	void hideConsole();
	void goToLine(int lineNo, int selStart = -1, int selEnd = -1);
	void updateTypesettingAction();
	void findRootFilePath();
	const QString& getRootFilePath();
	void maybeCenterSelection(int oldScrollValue = -1);
	void presentResults(const QList<SearchResult>& results);
	void showLineEndingSetting();
	void showEncodingSetting();
	
	QString selectedText() { return textCursor().selectedText().replace(QChar(QChar::ParagraphSeparator), QChar::fromLatin1('\n')); }
	QString consoleText() { return textEdit_console->toPlainText(); }
	QString text() { return textEdit->toPlainText(); }
	
	TeXHighlighter *highlighter;
	PDFDocument *pdfDoc;

	QTextCodec *codec;
	// When using the UTF-8 codec, byte order marks (BOMs) are ignored during 
	// reading and not produced when writing. To keep them in files that have
	// them, we need to keep track of them ourselves.
	bool utf8BOM;
	int lineEndings;
	QString curFile;
	QString rootFilePath;
	bool isUntitled;
	QDateTime lastModified;

	ClickableLabel *lineNumberLabel;
	ClickableLabel *encodingLabel;
	ClickableLabel *lineEndingLabel;

	QActionGroup *engineActions;
	QString engineName;
	
	QSignalMapper dictSignalMapper;

	QComboBox *engine;
	QProcess *process;
	bool keepConsoleOpen;
	bool showPdfWhenFinished;
	bool userInterrupt;
	QDateTime oldPdfTime;

	QList<QAction*> recentFileActions;

	Hunhandle *pHunspell;

	QFileSystemWatcher *watcher;
	
	QList<Tag>	tags;
	bool deferTagListChanges;
	bool tagListChanged;

	QTextCursor	dragSavedCursor;

	static QList<TeXDocument*> docList;
#if defined(MIKTEX)
    QAction* actionAbout_MiKTeX;
#if defined(MIKTEX_TODO_PRINT)
    QAction* actionPrintSource;
#endif
#endif
};

#endif
