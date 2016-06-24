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

#ifndef COMPLETING_EDIT_H
#define COMPLETING_EDIT_H

#include <QTextEdit>
#include <QHash>
#include <QTimer>
#include <QDrag>
#include <QMimeData>

#include <hunspell.h>

class QCompleter;
class QStandardItemModel;
class QTextCodec;
class LineNumberArea;

class CompletingEdit : public QTextEdit
{
	Q_OBJECT

public:
	CompletingEdit(QWidget *parent = 0);
	~CompletingEdit();

	void setSpellChecker(Hunhandle *h, QTextCodec *codec);

	bool selectWord(QTextCursor& cursor);

	void setLineNumberDisplay(bool displayNumbers);
	void lineNumberAreaPaintEvent(QPaintEvent *event);
	int lineNumberAreaWidth();

	bool getLineNumbersVisible() const;

	QString getIndentMode() const {
		return autoIndentMode >= 0 && autoIndentMode < autoIndentModes().size() ?
			autoIndentModes().at(autoIndentMode) : QString();
	}
	QString getQuotesMode() const {
		return smartQuotesMode >= 0 && smartQuotesMode < smartQuotesModes().size() ?
			smartQuotesModes().at(smartQuotesMode) : QString();
	}
	
	// Override of QTextEdit's method to properly handle scrolling for multiline
	// cursors
	void setTextCursor(const QTextCursor & cursor);
	
	static QStringList autoIndentModes();
	static QStringList smartQuotesModes();
	
	static void setHighlightCurrentLine(bool highlight);
	static void setAutocompleteEnabled(bool autocomplete);

public slots:
	void setAutoIndentMode(int index);
	void setSmartQuotesMode(int index);
	void smartenQuotes();
	void updateLineNumberAreaWidth(int newBlockCount);
	void setFont(const QFont & font);
	void setFontFamily(const QString & fontFamily);
	void setFontItalic(bool italic);
	void setFontPointSize(qreal s);
	void setFontWeight(int weight);
	
signals:
	void syncClick(int line, int col);
	void rehighlight();
	void updateRequest(const QRect& rect, int dy);

protected:
	virtual void keyPressEvent(QKeyEvent *e);
	virtual void focusInEvent(QFocusEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mouseDoubleClickEvent(QMouseEvent *e);
	virtual void contextMenuEvent(QContextMenuEvent *e);
	virtual void dragEnterEvent(QDragEnterEvent *e);
	virtual void dropEvent(QDropEvent *e);
	virtual void timerEvent(QTimerEvent *e);
	virtual bool canInsertFromMimeData(const QMimeData *source) const;
	virtual void insertFromMimeData(const QMimeData *source);
	virtual void resizeEvent(QResizeEvent *event);
	virtual bool event(QEvent *event);	
	virtual void scrollContentsBy(int dx, int dy);
	
private slots:
	void cursorPositionChangedSlot();
	void correction(const QString& suggestion);
	void addToDictionary();
	void ignoreWord();
	void resetExtraSelections();
	void jumpToPdf();
	void updateLineNumberArea(const QRect&, int);
	
private:
	void updateColors();

	void setCompleter(QCompleter *c);

	void showCompletion(const QString& completion, int insOffset = -1);
	void showCurrentCompletion();

	void loadCompletionsFromFile(QStandardItemModel *model, const QString& filename);
	void loadCompletionFiles(QCompleter *theCompleter);

	void handleCompletionShortcut(QKeyEvent *e);
	void handleReturn(QKeyEvent *e);
	void handleBackspace(QKeyEvent *e);
	void handleOtherKey(QKeyEvent *e);
	void maybeSmartenQuote(int offset);

	void setSelectionClipboard(const QTextCursor& curs);

	QTextCursor wordSelectionForPos(const QPoint& pos);
	QTextCursor blockSelectionForPos(const QPoint& pos);
	
	enum MouseMode {
		none,
		ignoring,
		synctexClick,
		normalSelection,
		extendingSelection,
		dragSelecting
	};
	MouseMode mouseMode;
	
	QTextCursor dragStartCursor;

	int droppedOffset, droppedLength;
	
	QBasicTimer clickTimer;
	QPoint clickPos;
	int clickCount;
	
	static void loadIndentModes();

	struct IndentMode {
		QString	name;
		QRegExp	regex;
	};
	static QList<IndentMode> *indentModes;
	int autoIndentMode;
	int prefixLength;

    static void loadSmartQuotesModes();
	
	typedef QPair<QString,QString> QuotePair;
	typedef QHash<QChar,QuotePair> QuoteMapping;
	struct QuotesMode {
		QString name;
		QuoteMapping mappings;
	};
	static QList<QuotesMode> *quotesModes;
	
	int smartQuotesMode;

	QCompleter *c;
	QTextCursor cmpCursor;

	QString prevCompletion; // used with multiple entries for the same key (e.g., "--")
	int itemIndex;
	int prevRow;

	QTextCursor currentWord;
	Hunhandle *pHunspell;
	QTextCodec *spellingCodec;

	QTextCursor	currentCompletionRange;

	LineNumberArea *lineNumberArea;

	static QTextCharFormat	*currentCompletionFormat;
	static QTextCharFormat	*braceMatchingFormat;
	static QTextCharFormat	*currentLineFormat;
	
	static QCompleter	*sharedCompleter;
	
	static bool highlightCurrentLine;
	static bool autocompleteEnabled;
};

class LineNumberArea : public QWidget
{
public:
	LineNumberArea(CompletingEdit *e)
		: QWidget(e)
	{
		editor = e;
	}
	
	QSize sizeHint() const {
		return QSize(editor->lineNumberAreaWidth(), 0);
	}
	
	QColor bgColor() const { return _bgColor; }
	void setBgColor(const QColor color) { _bgColor = color; }
	
protected:
	void paintEvent(QPaintEvent *event) {
		editor->lineNumberAreaPaintEvent(event);
	}
	
private:
	CompletingEdit *editor;
	QColor _bgColor;
};

#endif // COMPLETING_EDIT_H
