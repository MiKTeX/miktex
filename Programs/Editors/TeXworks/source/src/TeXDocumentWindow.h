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

#ifndef TeXDocumentWindow_H
#define TeXDocumentWindow_H

#include "TWScriptableWindow.h"
#include "document/SpellChecker.h"
#include "document/TeXDocument.h"

#include <QList>
#include <QRegularExpression>
#include <QProcess>
#include <QDateTime>
#include <QSignalMapper>
#include <QMouseEvent>

#include "ui_TeXDocumentWindow.h"

#include "FindDialog.h"

class QAction;
class QMenu;
class QTextEdit;
class QToolBar;
class QLabel;
class QComboBox;
class QActionGroup;
class QTextCodec;
class QFileSystemWatcher;

class PDFDocumentWindow;
class ClickableLabel;

const int kTeXWindowStateVersion = 1; // increment this if we add toolbars/docks/etc

#define kLineEnd_Mask   0x00FF
#define kLineEnd_LF     0x0000
#define kLineEnd_CRLF   0x0001
#define kLineEnd_CR     0x0002

#define kLineEnd_Flags_Mask  0xFF00
#define kLineEnd_Mixed       0x0100

class TeXDocumentWindow : public TWScriptableWindow, private Ui::TeXDocumentWindow
{
	Q_OBJECT

public:
	explicit TeXDocumentWindow();
	explicit TeXDocumentWindow(const QString & fileName, bool asTemplate = false);

	~TeXDocumentWindow() override;

	static TeXDocumentWindow *findDocument(const QString &fileName);
	static QList<TeXDocumentWindow*> documentList()
		{
			return docList;
		}
	static TeXDocumentWindow *openDocument(const QString &fileName, bool activate = true, bool raiseWindow = true,
									 int lineNo = 0, int selStart = -1, int selEnd = -1);

	TeXDocumentWindow *open(const QString &fileName);
	void makeUntitled();
	bool untitled() const
		{ return !textDoc()->isStoredInFilesystem(); }
	QString fileName() const
		{ return textDoc()->getFileInfo().filePath(); }
	QTextCursor textCursor() const
		{ return textEdit->textCursor(); }
	Tw::Document::TeXDocument* textDoc()
		{ return _texDoc; }
	const Tw::Document::TeXDocument* textDoc() const
		{ return _texDoc; }
	QString getLineText(int lineNo) const;
	CompletingEdit* editor()
		{ return textEdit; }
	int cursorPosition() const { return textCursor().position(); }
	int selectionStart() const { return textCursor().selectionStart(); }
	int selectionLength() const { return textCursor().selectionEnd() - textCursor().selectionStart(); }
	QString getCurrentCodecName() const { return (codec ? QString::fromUtf8(codec->name().constData()) : QString()); }
	bool getUTF8BOM() const { return utf8BOM; }
	
	QString spellcheckLanguage() const;

	PDFDocumentWindow* pdfDocument()
		{ return pdfDoc; }

	void goToTag(int index);

	bool isModified() const { return textEdit->document()->isModified(); }
	void setModified(const bool m = true) { textEdit->document()->setModified(m); }

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
	void asyncFlashStatusBarMessage(const QString & msg, const int timeout = 0);

protected:
	void changeEvent(QEvent *event) override;
	void closeEvent(QCloseEvent *event) override;
	bool event(QEvent *event) override;
	void dragEnterEvent(QDragEnterEvent *event) override;
	void dragMoveEvent(QDragMoveEvent *event) override;
	void dragLeaveEvent(QDragLeaveEvent *event) override;
	void dropEvent(QDropEvent *event) override;

	QString scriptContext() const override { return QStringLiteral("TeXDocument"); }

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
	void setLineSpacing(qreal percent);
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
	void setWindowModified(bool modified);
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
	void handleModelineChange(QStringList changedKeys, QStringList removedKeys);
	void reloadIfChangedOnDisk();
	void setupFileWatcher();
	void lineEndingPopup(const QPoint loc);
	void encodingPopup(const QPoint loc);
	void lineEndingLabelClick(QMouseEvent * event) { lineEndingPopup(event->pos()); }
	void encodingLabelClick(QMouseEvent * event) { encodingPopup(event->pos()); }
	void anchorClicked(const QUrl& url);
	void delayedInit();

private:
	void init();
	bool maybeSave();
	void detachPdf();
	bool saveFilesHavingRoot(const QString& aRootFile);
	void clearFileWatcher();
	QTextCodec *scanForEncoding(const QString &peekStr, bool &hasMetadata, QString &reqName);
	QString readFile(const QFileInfo & fileInfo, QTextCodec **codecUsed, int *lineEndings = nullptr, QTextCodec * forceCodec = nullptr);
	void loadFile(const QFileInfo & fileInfo, bool asTemplate = false, bool inBackground = false, bool reload = false, QTextCodec * forceCodec = nullptr);
	bool saveFile(const QFileInfo & fileInfo);
	void setCurrentFile(const QFileInfo & fileInfo);
	void saveRecentFileInfo();
	bool getPreviewFileName(QString &pdfName);
	bool openPdfIfAvailable(bool show);
	void replaceSelection(const QString& newText);
	void doHardWrap(int mode, int lineWidth, bool rewrap);
	void zoomToLeft(QWidget *otherWindow);
	QTextCursor doSearch(QTextDocument *theDoc, const QString& searchText, const QRegularExpression *regex,
						 QTextDocument::FindFlags flags, int rangeStart, int rangeEnd);
	int doReplaceAll(const QString& searchText, QRegularExpression* regex, const QString& replacement,
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
	
	Tw::Document::TeXDocument * _texDoc;
	PDFDocumentWindow * pdfDoc{nullptr};

	QTextCodec * codec{nullptr};
	// When using the UTF-8 codec, byte order marks (BOMs) are ignored during 
	// reading and not produced when writing. To keep them in files that have
	// them, we need to keep track of them ourselves.
	bool utf8BOM{false};
	int lineEndings{kLineEnd_LF};
	QString rootFilePath;
	QDateTime lastModified;

	ClickableLabel * lineNumberLabel{nullptr};
	ClickableLabel * encodingLabel{nullptr};
	ClickableLabel * lineEndingLabel{nullptr};

	QActionGroup *engineActions{nullptr};
	QString engineName;
	
	QSignalMapper dictSignalMapper;

	QComboBox * engine{nullptr};
	QProcess * process{nullptr};
	bool keepConsoleOpen{false};
	bool showPdfWhenFinished{true};
	bool userInterrupt{false};
	QDateTime oldPdfTime;

	QList<QAction*> recentFileActions;

	QFileSystemWatcher * watcher{nullptr};

	QTextCursor	dragSavedCursor;

	static QList<TeXDocumentWindow*> docList;
#if defined(MIKTEX)
        QAction* actionAbout_MiKTeX;
#endif
};

#endif // !defined(TeXDocumentWindow_H)
