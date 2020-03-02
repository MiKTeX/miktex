/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2019  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "document/SpellChecker.h"
#include "ui/LineNumberWidget.h"
#include "ui_CompletingEdit.h"

#include <QTextEdit>
#include <QHash>
#include <QTimer>
#include <QDrag>
#include <QMimeData>
#include <QRegularExpression>

class QCompleter;
class QStandardItemModel;
class QTextCodec;

class CompletingEdit : public QTextEdit, private Ui::CompletingEdit
{
	Q_OBJECT

public:
	CompletingEdit(QWidget *parent = nullptr);
	~CompletingEdit() override;

	bool selectWord(QTextCursor& cursor);

	void setLineNumberDisplay(bool displayNumbers);
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

	// Override of QTextEdit's method to reconnect signals
	void setDocument(QTextDocument * document);
	
	static QStringList autoIndentModes();
	static QStringList smartQuotesModes();
	
	static void setHighlightCurrentLine(bool highlight);
	static void setAutocompleteEnabled(bool autocomplete);

	void prefixLines(const QString &prefix);
	void unPrefixLines(const QString &prefix);

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
	void keyPressEvent(QKeyEvent *e) override;
	void focusInEvent(QFocusEvent *e) override;
	void mousePressEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e) override;
	void mouseMoveEvent(QMouseEvent *e) override;
	void mouseDoubleClickEvent(QMouseEvent *e) override;
	void contextMenuEvent(QContextMenuEvent *e) override;
	void dragEnterEvent(QDragEnterEvent *e) override;
	void dropEvent(QDropEvent *e) override;
	void timerEvent(QTimerEvent *e) override;
	bool canInsertFromMimeData(const QMimeData *source) const override;
	void insertFromMimeData(const QMimeData *source) override;
	void resizeEvent(QResizeEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;
	bool event(QEvent *event) override;
	void scrollContentsBy(int dx, int dy) override;

	Tw::Document::SpellChecker::Dictionary * getSpellChecker() const;

private slots:
	void cursorPositionChangedSlot();
	void correction(const QString& suggestion);
	void addToDictionary();
	void ignoreWord();
	void resetExtraSelections();
	void jumpToPdf(QTextCursor pos = {});
	void jumpToPdfFromContextMenu();
	void updateLineNumberArea(const QRect&, int);
	
private:
	void updateColors();

	void setCompleter(QCompleter *c);

	void showCompletion(const QString& completion, int insOffset = -1);
	void showCurrentCompletion();

	void loadCompletionsFromFile(QStandardItemModel *model, const QString& filename);
	void loadCompletionFiles(QCompleter *theCompleter);

	bool handleCompletionShortcut(QKeyEvent *e);
	void handleReturn(QKeyEvent *e);
	void handleBackspace(QKeyEvent *e);
	void handleTab(QKeyEvent * e);
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
	MouseMode mouseMode{none};
	
	QTextCursor dragStartCursor;

	int droppedOffset{-1}, droppedLength{0};
	
	QBasicTimer clickTimer;
	QPoint clickPos;
	int clickCount{0};

	int wheelDelta{0};  // used to accumulate small steps of high-resolution mice
	
	static void loadIndentModes();

	struct IndentMode {
		QString name;
		QRegularExpression regex;
	};
	static QList<IndentMode> *indentModes;
	int autoIndentMode{-1};
	int prefixLength{0};

	static void loadSmartQuotesModes();
	
	typedef QPair<QString,QString> QuotePair;
	typedef QHash<QChar,QuotePair> QuoteMapping;
	struct QuotesMode {
		QString name;
		QuoteMapping mappings;
	};
	static QList<QuotesMode> *quotesModes;
	
	int smartQuotesMode{-1};

	QCompleter * c{nullptr};
	QTextCursor cmpCursor;

	QString prevCompletion; // used with multiple entries for the same key (e.g., "--")
	int itemIndex{0};
	int prevRow{-1};

	QTextCursor currentWord;

	QTextCursor	currentCompletionRange;

	Tw::UI::LineNumberWidget * lineNumberArea;

	static QTextCharFormat	*currentCompletionFormat;
	static QTextCharFormat	*braceMatchingFormat;
	static QTextCharFormat	*currentLineFormat;
	
	static QCompleter	*sharedCompleter;
	
	static bool highlightCurrentLine;
	static bool autocompleteEnabled;
};

#endif // COMPLETING_EDIT_H
