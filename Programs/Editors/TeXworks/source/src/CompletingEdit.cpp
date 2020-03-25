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

#include "CompletingEdit.h"
#include "TWUtils.h"
#include "TWApp.h"
#include "Settings.h"
#include "TeXHighlighter.h"
#include "document/TeXDocument.h"

#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QApplication>
#include <QModelIndex>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTextCursor>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QMenu>
#include <QTextStream>
#include <QTextCodec>
#include <QAbstractTextDocumentLayout>
#include <QSignalMapper>
#include <QTextBlock>
#include <QScrollBar>
#include <QTimer>
#include <QPainter>
#include <QClipboard>

CompletingEdit::CompletingEdit(QWidget *parent /* = nullptr */)
	: QTextEdit(parent)
{
	if (!sharedCompleter) { // initialize shared (static) members
		sharedCompleter = new QCompleter(qApp);
		sharedCompleter->setCompletionMode(QCompleter::InlineCompletion);
		sharedCompleter->setCaseSensitivity(Qt::CaseInsensitive);
		loadCompletionFiles(sharedCompleter);

		currentCompletionFormat = new QTextCharFormat;
		braceMatchingFormat = new QTextCharFormat;
		currentLineFormat = new QTextCharFormat;

		Tw::Settings settings;
		highlightCurrentLine = settings.value(QString::fromLatin1("highlightCurrentLine"), true).toBool();
		autocompleteEnabled = settings.value(QString::fromLatin1("autocompleteEnabled"), true).toBool();
	}
		
	loadIndentModes();
	loadSmartQuotesModes();
	
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(cursorPositionChangedSlot()));
	connect(this, SIGNAL(selectionChanged()), this, SLOT(cursorPositionChangedSlot()));

	lineNumberArea = new Tw::UI::LineNumberWidget(this);
	
	// Invoke our setDocument() method to properly set up document-specific
	// connections
	setDocument(document());
	connect(this, SIGNAL(updateRequest(const QRect&, int)), this, SLOT(updateLineNumberArea(const QRect&, int)));
	connect(this, SIGNAL(textChanged()), lineNumberArea, SLOT(update()));

	connect(TWApp::instance(), SIGNAL(highlightLineOptionChanged()), this, SLOT(resetExtraSelections()));

	setupUi(this);
	connect(actionJump_To_PDF, SIGNAL(triggered()), this, SLOT(jumpToPdf()));
	// As these actions are not used in menus/toolbars, we need to manually add
	// them to the widget for TWUtils::installCustomShortcuts to work
	insertActions(nullptr, {actionNext_Completion, actionPrevious_Completion, actionNext_Completion_Placeholder, actionPrevious_Completion_Placeholder, actionJump_To_PDF});

#ifdef Q_OS_DARWIN
	// Backwards compatibility
	// Ctrl+Tab is mapped to Command+Tab on the Mac, which is the standard key
	// sequence for switching applications. Hence that combination is changed to
	// Alt+Tab on the Mac
	if (actionNext_Completion_Placeholder->shortcut() == QKeySequence(Qt::CTRL + Qt::Key_Tab))
		actionNext_Completion_Placeholder->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Tab));
	if (actionPrevious_Completion_Placeholder->shortcut() == QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab))
		actionPrevious_Completion_Placeholder->setShortcut(QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_Tab));
#endif

	cursorPositionChangedSlot();
	updateLineNumberAreaWidth(0);
	updateColors();
	TWUtils::installCustomShortcuts(this);
}

void CompletingEdit::prefixLines(const QString &prefix)
{
	QTextCursor cursor = textCursor();
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
	setTextCursor(cursor);
	cursor.endEditBlock();
}

void CompletingEdit::unPrefixLines(const QString &prefix)
{
	QTextCursor cursor = textCursor();
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
	setTextCursor(cursor);
	cursor.endEditBlock();
}


void CompletingEdit::updateColors()
{
	Q_ASSERT(currentCompletionFormat);
	Q_ASSERT(braceMatchingFormat);
	Q_ASSERT(currentLineFormat);
	Q_ASSERT(lineNumberArea);

	qreal bgR, bgG, bgB;
	qreal fgR, fgG, fgB;

	palette().color(QPalette::Active, QPalette::Base).getRgbF(&bgR, &bgG, &bgB);
	palette().color(QPalette::Active, QPalette::Text).getRgbF(&fgR, &fgG, &fgB);

	currentCompletionFormat->setBackground(QColor::fromRgbF(.75 * bgR + .25 * fgR, .75 * bgG + .25 * fgG, .75 * bgB + .25 * fgB));
	braceMatchingFormat->setBackground(QColor("orange"));

	currentLineFormat->setBackground(QColor::fromRgbF(.9 * bgR + .1 * fgR, .9 * bgG + .1 * fgG, .9 * bgB + .1 * fgB));
	currentLineFormat->setProperty(QTextFormat::FullWidthSelection, true);

	palette().color(QPalette::Window).getRgbF(&bgR, &bgG, &bgB);
	palette().color(QPalette::Text).getRgbF(&fgR, &fgG, &fgB);
	lineNumberArea->setBgColor(QColor::fromRgbF(0.75 * bgR + 0.25 * fgR, 0.75 * bgG + 0.25 * fgG, 0.75 * bgB + 0.25 * fgB));
}

CompletingEdit::~CompletingEdit()
{
	setCompleter(nullptr);
}

void CompletingEdit::setCompleter(QCompleter *completer)
{
	c = completer;
	if (!c)
		return;

	c->setWidget(this);
}

void CompletingEdit::cursorPositionChangedSlot()
{
	setCompleter(nullptr);
	if (!currentCompletionRange.isNull())
		currentCompletionRange = QTextCursor();
	resetExtraSelections();
	prefixLength = 0;
}

void CompletingEdit::mousePressEvent(QMouseEvent *e)
{
	if (e->buttons() != Qt::LeftButton) {
		mouseMode = none;
		QTextEdit::mousePressEvent(e);
		return;
	}

	if (e->modifiers() == Qt::ControlModifier) {
		mouseMode = synctexClick;
		e->accept();
		return;
	}

	int proximityToPrev = (e->pos() - clickPos).manhattanLength();

	mouseMode = normalSelection;
	clickPos = e->pos();
	if (e->modifiers() & Qt::ShiftModifier) {
		mouseMode = extendingSelection;
		clickCount = 1;
		QTextEdit::mousePressEvent(e);
		return;
	}

	if (clickTimer.isActive() && proximityToPrev < qApp->startDragDistance())
		++clickCount;
	else
		clickCount = 1;
	clickTimer.start(qApp->doubleClickInterval(), this);

	QTextCursor	curs;
	switch (clickCount) {
		case 1:
			curs = cursorForPosition(clickPos);
			break;
		case 2:
			curs = wordSelectionForPos(clickPos);
			break;
		default:
			curs = blockSelectionForPos(clickPos);
			break;
	}

	if (clickCount > 1) {
		setTextCursor(curs);
		setSelectionClipboard(curs);
		mouseMode = dragSelecting;
	}
	dragStartCursor = curs;
	e->accept();
}

void CompletingEdit::mouseMoveEvent(QMouseEvent *e)
{
	switch (mouseMode) {
		case none:
			QTextEdit::mouseMoveEvent(e);
			return;
		
		case synctexClick:
		case ignoring:
			e->accept();
			return;

		case extendingSelection:
			QTextEdit::mouseMoveEvent(e);
			return;
			
		case normalSelection:
			if (clickCount == 1
				&& dragStartCursor.position() >= textCursor().selectionStart()
				&& dragStartCursor.position() < textCursor().selectionEnd()) {
				if ((e->pos() - clickPos).manhattanLength() >= qApp->startDragDistance()) {
					int sourceStart = textCursor().selectionStart();
					int sourceEnd = textCursor().selectionEnd();
					QTextCursor source = textCursor();
					QDrag *drag = new QDrag(this);
					drag->setMimeData(createMimeDataFromSelection());
					QTextCursor dropCursor = textCursor();
					textCursor().beginEditBlock();
					Qt::DropAction action = drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::MoveAction);
					if (action != Qt::IgnoreAction) {
						dropCursor.setPosition(droppedOffset);
						dropCursor.setPosition(droppedOffset + droppedLength, QTextCursor::KeepAnchor);
						if (action == Qt::MoveAction) {
							// If we successfully completed a "Move" action, we
							// need to make sure that the source text is removed
							// (inserting the text in the target has already
							// been completed at this point)
							bool insideWindow = (drag->target() && (this == drag->target() || this->isAncestorOf(qobject_cast<QWidget*>(drag->target()))));
							bool insideSelection = (insideWindow && droppedOffset >= sourceStart && droppedOffset <= sourceEnd);
							if (insideSelection) {
								// The text was dropped into the same window at
								// an overlapping position.
								// First, remove everything that is left from
								// the source text *after* the inserted text
								// (note that every position after droppedOffset
								// needs to be translated by droppedLength)
								source.setPosition(droppedOffset + droppedLength);
								source.setPosition(sourceEnd + droppedLength, QTextCursor::KeepAnchor);
								source.removeSelectedText();
								// Second, remove everything that is left from
								// the source text *before* the inserted text
								source.setPosition(sourceStart);
								source.setPosition(droppedOffset, QTextCursor::KeepAnchor);
								source.removeSelectedText();
							}
							else {
								// Otherwise, simply remove the source text
								source.removeSelectedText();
							}
							
							if (!insideWindow) {
								// The selection was moved to a different window,
								// so dropCursor has no sensible data here. Thus,
								// we collapse the cursor to where the selection
								// was before the move action.
								dropCursor.setPosition(sourceStart);
							}
						}
					}
					textCursor().endEditBlock();
					setTextCursor(dropCursor);
					mouseMode = ignoring;
				}
				e->accept();
				return;
			}
			setTextCursor(dragStartCursor);
			mouseMode = dragSelecting;
			// fall through
			// to dragSelecting

		case dragSelecting:
			QPoint pos = e->pos();
			int scrollValue = -1;
			if (verticalScrollBar()) {
				if (pos.y() < frameRect().top())
					verticalScrollBar()->triggerAction(QScrollBar::SliderSingleStepSub);
				else if (pos.y() > frameRect().bottom())
					verticalScrollBar()->triggerAction(QScrollBar::SliderSingleStepAdd);
				scrollValue = verticalScrollBar()->value();
			}
			QTextCursor curs;
			switch (clickCount) {
				case 1:
					curs = cursorForPosition(pos);
					break;
				case 2:
					curs = wordSelectionForPos(pos);
					break;
				default:
					curs = blockSelectionForPos(pos);
					break;
			}
			int start = qMin(dragStartCursor.selectionStart(), curs.selectionStart());
			int end = qMax(dragStartCursor.selectionEnd(), curs.selectionEnd());
			if (dragStartCursor.selectionStart() > curs.selectionStart()) {
				// If the user is selecting from right to left (moving towards
				// smaller positions), we reverse start and end so that the
				// selection anchor is placed at the proper place (the
				// left-most end of the selection).
				qSwap(start, end);
			}
			curs.setPosition(start);
			curs.setPosition(end, QTextCursor::KeepAnchor);
			setTextCursor(curs);
			setSelectionClipboard(curs);
			if (scrollValue != -1)
				verticalScrollBar()->setValue(scrollValue);
			e->accept();
			return;
	}
}

void CompletingEdit::mouseReleaseEvent(QMouseEvent *e)
{
	switch (mouseMode) {
		case none:
			QTextEdit::mouseReleaseEvent(e);
			return;
		case ignoring:
			mouseMode = none;
			e->accept();
			return;
		case synctexClick:
			mouseMode = none;
			{
				QTextCursor curs = cursorForPosition(e->pos());
				emit syncClick(curs.blockNumber() + 1, curs.positionInBlock());
			}
			e->accept();
			return;
		case dragSelecting:
			mouseMode = none;
			e->accept();
			return;
		case normalSelection:
			mouseMode = none;
			setTextCursor(dragStartCursor);
			setSelectionClipboard(dragStartCursor);
			e->accept();
			return;
		case extendingSelection:
			mouseMode = none;
			QTextEdit::mouseReleaseEvent(e);
			return;
	}
}

QTextCursor CompletingEdit::blockSelectionForPos(const QPoint& pos)
{
	QTextCursor curs = cursorForPosition(pos);
	curs.setPosition(curs.block().position());
	curs.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
	return curs;
}

bool CompletingEdit::selectWord(QTextCursor& cursor)
{
	if (cursor.selectionEnd() - cursor.selectionStart() > 1)
		return false;	// actually an error by the caller

	const QTextBlock block = document()->findBlock(cursor.selectionStart());
	const QString text = block.text();
	if (text.length() < 1) // empty line
		return false;

	int start, end;
	bool result = TWUtils::findNextWord(text, cursor.selectionStart() - block.position(), start, end);
	cursor.setPosition(block.position() + start);
	cursor.setPosition(block.position() + end, QTextCursor::KeepAnchor);

	return result;
}

QTextCursor CompletingEdit::wordSelectionForPos(const QPoint& mousePos)
{
	QTextCursor cursor;
	QPoint	pos = mousePos + QPoint(horizontalScrollBar()->value(), verticalScrollBar()->value());
	int cursorPos = document()->documentLayout()->hitTest(pos, Qt::FuzzyHit);
	if (cursorPos == -1)
		return cursor;

	cursor = QTextCursor(document());
	cursor.setPosition(cursorPos);
	cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

	// check if click was within the char to the right of cursor; if so we select forwards
	QRect r = cursorRect(cursor);
	if (r.contains(pos)) {
		// Currently I don't seem to be getting a useful answer from cursorRect(), it's always zero-width :-(
		// and so this path will not be used, but leaving it here in hopes of fixing it some day
		//			QString s = cursor.selectedText();
		//			if (isPairedChar(s)) ...
		(void)selectWord(cursor);
		return cursor;
	}

	if (cursorPos > 0) {
		cursorPos -= 1;
		cursor.setPosition(cursorPos);
		cursor.setPosition(cursorPos + 1, QTextCursor::KeepAnchor);
		// don't test because the rect will be zero width (see above)!
		//		r = cursorRect(cursor);
		//		if (r.contains(pos)) {
		const QString plainText = toPlainText();
		QChar curChr = plainText[cursorPos];
		QChar c;
		if (!(c = TWUtils::closerMatching(curChr)).isNull()) {
			int balancePos = TWUtils::balanceDelim(plainText, cursorPos + 1, c, 1);
			if (balancePos < 0)
				QApplication::beep();
			else
				cursor.setPosition(balancePos + 1, QTextCursor::KeepAnchor);
		}
		else if (!(c = TWUtils::openerMatching(curChr)).isNull()) {
			int balancePos = TWUtils::balanceDelim(plainText, cursorPos - 1, c, -1);
			if (balancePos < 0)
				QApplication::beep();
			else {
				cursor.setPosition(balancePos);
				cursor.setPosition(cursorPos + 1, QTextCursor::KeepAnchor);
			}
		}
		else
			(void)selectWord(cursor);
	//		}
	}
	return cursor;
}

void CompletingEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
	if (e->modifiers() == Qt::ControlModifier)
		e->accept();
	else if (e->modifiers() != Qt::NoModifier)
		QTextEdit::mouseDoubleClickEvent(e);
	else
		mousePressEvent(e); // don't like QTextEdit's selection behavior, so we try to improve it
}

void
CompletingEdit::setSelectionClipboard(const QTextCursor& curs)
{
	if (!curs.hasSelection())
		return;
	QClipboard *c = QApplication::clipboard();
	if (!c->supportsSelection())
		return;
	c->setText(curs.selectedText().replace(QChar(0x2019), QChar::fromLatin1('\n')),
		QClipboard::Selection);
}

void CompletingEdit::timerEvent(QTimerEvent *e)
{
	if (e->timerId() == clickTimer.timerId()) {
		clickTimer.stop();
		e->accept();
	}
	else
		QTextEdit::timerEvent(e);
}
	
void CompletingEdit::focusInEvent(QFocusEvent *e)
{
	if (c)
		c->setWidget(this);
	QTextEdit::focusInEvent(e);
}

void CompletingEdit::resetExtraSelections()
{
	QList<ExtraSelection> selections;
	if (highlightCurrentLine && !textCursor().hasSelection()) {
		ExtraSelection sel;
		sel.format = *currentLineFormat;
		sel.cursor = textCursor();
		selections.append(sel);
	}
	if (!currentCompletionRange.isNull()) {
		ExtraSelection sel;
		sel.cursor = currentCompletionRange;
		sel.format = *currentCompletionFormat;
		selections.append(sel);
	}
	setExtraSelections(selections);
}

void CompletingEdit::keyPressEvent(QKeyEvent *e)
{
	if (autocompleteEnabled) {
		QKeySequence seq(static_cast<int>(e->modifiers()) | e->key());
		if (seq == actionNext_Completion->shortcut() || seq == actionPrevious_Completion->shortcut() || seq == actionNext_Completion_Placeholder->shortcut() || seq == actionPrevious_Completion_Placeholder->shortcut()) {
			if (handleCompletionShortcut(e))
				return;
		}
	}

	if (!e->text().isEmpty())
		cmpCursor = QTextCursor();

	switch (e->key()) {
		case Qt::Key_Return:
			handleReturn(e);
			break;

		case Qt::Key_Tab:
		case Qt::Key_Backtab:
			handleTab(e);
			break;

		case Qt::Key_Backspace:
			handleBackspace(e);
			break;

		default:
			handleOtherKey(e);
			break;
	}
}

void CompletingEdit::handleReturn(QKeyEvent *e)
{
	QString prefix;
	// Check if auto indent is on and applicable
	if (autoIndentMode >= 0 && autoIndentMode < indentModes->count() && e->modifiers() == Qt::NoModifier) {
		const QRegularExpression &re = (*indentModes)[autoIndentMode].regex;
		// Only apply prefix recognition to characters in front of the cursor.
		// Otherwise, we would accumulate characters if the cursor is inside the
		// region matched by the regexp.
		QTextCursor curs = textCursor();
		// Collaps the selection (if any) in the direction of the beginning of
		// the line (as the selection gets replaced by \n, there's no point in
		// including it in the prefix calculation)
		if (curs.hasSelection())
			curs.setPosition(curs.selectionStart());
		// Get the (possible) prefix text to check
		curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
		QString blockText = curs.selectedText();
		// Check if the prefix matches the regexp of the current auto indent mode
		QRegularExpressionMatch m = re.match(blockText);
		if (m.capturedStart() == 0 && m.capturedLength() > 0)
			prefix = m.captured();
	}
	// Propagate the key press event to the base class so that the text is
	// actually modified
	QTextEdit::keyPressEvent(e);
	// Insert the appropriate prefix if necessary
	if (!prefix.isEmpty()) {
		insertPlainText(prefix);
		prefixLength = prefix.length();
	}
}

void CompletingEdit::handleBackspace(QKeyEvent *e)
{
	QTextCursor curs = textCursor();
	if (e->modifiers() == Qt::NoModifier && prefixLength > 0 && !curs.hasSelection()) {
		curs.beginEditBlock();
		// note that prefixLength will get reset on the first deletion,
		// so it is important that the loop counts down rather than up!
		for (int i = prefixLength; i > 0; --i)
			curs.deletePreviousChar();
		curs.endEditBlock();
	}
	else
		QTextEdit::keyPressEvent(e);

	// Without the following, when pressing Up or Down immediately
	// after Backspace the cursor ends up in the wrong column.
	curs.setPosition(curs.position());
	setTextCursor(curs);
}

void CompletingEdit::handleOtherKey(QKeyEvent *e)
{
	int pos = textCursor().selectionStart(); // remember cursor before the keystroke
	int end = textCursor().selectionEnd();
	QTextEdit::keyPressEvent(e);
	QTextCursor cursor = textCursor();
	bool arrowKey = false;
	if (e->key() == Qt::Key_Left || e->key() == Qt::Key_Right) {
		arrowKey = true;
		if (pos < end && !cursor.hasSelection()) { // collapsed selection
			if (cursor.position() == end + 1)
				pos = end;
			cursor.setPosition(pos);
			setTextCursor(cursor);
		}
	}
	if ((e->modifiers() & Qt::ControlModifier) == 0) {
		// not a command key - maybe do brace matching or smart quotes
		if (!cursor.hasSelection()) {
			if (!arrowKey && cursor.selectionStart() == pos + 1) {
				if (smartQuotesMode >= 0)
					maybeSmartenQuote(cursor.position() - 1);
			}
			if (cursor.selectionStart() == pos + 1 || cursor.selectionStart() == pos - 1) {
				if (cursor.selectionStart() == pos - 1) // we moved backward, set pos to look at the char we just passed over
					--pos;
				const QString text = document()->toPlainText();
				int match = -2;
				QChar c;
				if (pos > 0 && pos < text.length() && !(c = TWUtils::openerMatching(text[pos])).isNull())
					match = TWUtils::balanceDelim(text, pos - 1, c, -1);
				else if (pos < text.length() - 1 && !(c = TWUtils::closerMatching(text[pos])).isNull())
					match = TWUtils::balanceDelim(text, pos + 1, c, 1);
				if (match >= 0) {
					QList<ExtraSelection> selList = extraSelections();
					ExtraSelection	sel;
					sel.cursor = QTextCursor(document());
					sel.cursor.setPosition(match);
					sel.cursor.setPosition(match + 1, QTextCursor::KeepAnchor);
					sel.format = *braceMatchingFormat;
					selList.append(sel);
					setExtraSelections(selList);
					QTimer::singleShot(250, this, SLOT(resetExtraSelections()));
				}
			}
		}
	}	
}

void CompletingEdit::setSmartQuotesMode(int index)
{
	smartQuotesMode = (index >= 0 && index < quotesModes->count()) ? index : -1;
}

QStringList CompletingEdit::smartQuotesModes()
{
	loadSmartQuotesModes();
	
	QStringList modes;
	foreach (const QuotesMode& mode, *quotesModes)
		modes << mode.name;
	return modes;
}

void CompletingEdit::loadSmartQuotesModes()
{
	if (!quotesModes) {
		QDir configDir(TWUtils::getLibraryPath(QString::fromLatin1("configuration")));
		quotesModes = new QList<QuotesMode>;
		QFile quotesModesFile(configDir.filePath(QString::fromLatin1("smart-quotes-modes.txt")));
		if (quotesModesFile.open(QIODevice::ReadOnly)) {
			QRegularExpression modeName(QStringLiteral("^\\[([^]]+)\\]$"));
			QRegularExpression quoteLine(QStringLiteral("^([^ \\t])\\s+([^ \\t]+)\\s+([^ \\t]+)$"));
			QuotesMode newMode;
			while (true) {
				QByteArray ba = quotesModesFile.readLine();
				if (ba.size() == 0)
					break;
				if (ba[0] == '#' || ba[0] == '\n')
					continue;
				QString line = QString::fromUtf8(ba.data(), ba.size()).trimmed();
				QRegularExpressionMatch modeNameMatch = modeName.match(line);
				if (modeNameMatch.hasMatch()) {
					if (newMode.mappings.count() > 0) {
						quotesModes->append(newMode);
						newMode.mappings.clear();
					}
					newMode.name = modeNameMatch.captured(1);
					continue;
				}
				QRegularExpressionMatch quoteLineMatch = quoteLine.match(line);
				if (quoteLineMatch.hasMatch() && newMode.name.length() > 0) {
					QChar key = quoteLineMatch.captured(1)[0];
					const QString& open = quoteLineMatch.captured(2);
					const QString& close = quoteLineMatch.captured(3);
					newMode.mappings[key] = QuotePair(open,close);
					continue;
				}
			}
			if (newMode.mappings.count() > 0) {
				quotesModes->append(newMode);
			}
		}
	}
}

void CompletingEdit::maybeSmartenQuote(int offset)
{
	if (smartQuotesMode < 0 || smartQuotesMode >= quotesModes->count())
		return;
	const QuoteMapping& mappings = quotesModes->at(smartQuotesMode).mappings;
	QString replacement;

	const QString& text = document()->toPlainText();
	if (offset < 0 || offset >= text.length())
		return;
	QTextCursor cursor(document());
	cursor.setPosition(offset);
	cursor.setPosition(offset + 1, QTextCursor::KeepAnchor);

	QChar keyChar = cursor.selectedText()[0];
	QuoteMapping::const_iterator iter = mappings.find(keyChar);
	if (iter == mappings.end())
		return;
	
	replacement = iter.value().second;
	if (offset == 0) {
		// always use opening quotes at the beginning of the document
		replacement = iter.value().first;
	}
	else {
		if (text[offset - 1].isSpace())
			replacement = iter.value().first;
		
		// after opening brackets, also use opening quotes
		if (text[offset - 1] == QChar::fromLatin1('{') || text[offset - 1] == QChar::fromLatin1('[') || text[offset - 1] == QChar::fromLatin1('('))
			replacement = iter.value().first;
	}
	
	cursor.insertText(replacement);
}

void CompletingEdit::smartenQuotes()
{
	if (smartQuotesMode < 0 || smartQuotesMode >= quotesModes->count())
		return;
	const QuoteMapping& mappings = quotesModes->at(smartQuotesMode).mappings;

	const QString& text = document()->toPlainText();

	QTextCursor curs = textCursor();
	int selStart = curs.selectionStart();
	int selEnd = curs.selectionEnd();
	bool changed = false;
	for (int offset = selEnd; offset > selStart; ) {
		--offset;
		QChar ch = text[offset];
		QuoteMapping::const_iterator iter = mappings.find(ch);
		if (iter == mappings.end())
			continue;

		if (!changed) {
			curs.beginEditBlock();
			changed = true;
		}
		curs.setPosition(offset, QTextCursor::MoveAnchor);
		curs.setPosition(offset + 1, QTextCursor::KeepAnchor);
		const QString& replacement((offset == 0 || text[offset - 1].isSpace()) ?
								   iter.value().first : iter.value().second);
		curs.insertText(replacement);
		selEnd += replacement.length() - 1;
	}
	if (changed) {
		curs.endEditBlock();
		curs.setPosition(selStart, QTextCursor::MoveAnchor);
		curs.setPosition(selEnd, QTextCursor::KeepAnchor);
		setTextCursor(curs);
	}
}

// \returns true if shortcut was handled, false otherwise
bool CompletingEdit::handleCompletionShortcut(QKeyEvent *e)
{
	QKeySequence seq(static_cast<int>(e->modifiers()) | e->key());
	if (seq == actionNext_Completion_Placeholder->shortcut() || seq == actionPrevious_Completion_Placeholder->shortcut())
	{
		if (!find(QString(0x2022), (seq == actionPrevious_Completion_Placeholder->shortcut())
									? QTextDocument::FindBackward : QTextDocument::FindFlags()))
			QApplication::beep();
		return true;
	}

	// if we are at the beginning of the line (i.e., only whitespaces before a
	// caret cursor), insert a tab (for indentation) instead of doing completion
	bool atLineStart = false;

	QTextCursor lineStartCursor = textCursor();
	bool noSelection = lineStartCursor.selectionEnd() == lineStartCursor.selectionStart();

	if (noSelection) {
		lineStartCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
		if(lineStartCursor.selectedText().trimmed().isEmpty())
			atLineStart = true;
	}
	
	if (!c && !atLineStart) {
		cmpCursor = textCursor();
		if (!selectWord(cmpCursor) && textCursor().selectionStart() > 0) {
			cmpCursor.setPosition(textCursor().selectionStart() - 1);
			selectWord(cmpCursor);
		}
		// check if the word is preceded by open-brace; if so try with that included
		int start = cmpCursor.selectionStart();
		int end = cmpCursor.selectionEnd();
		if (start > 0) { // special cases: possibly look back to include brace or hyphen(s)
			if (cmpCursor.selectedText() == QLatin1String("-")) {
				QTextCursor hyphCursor(cmpCursor);
				int hyphPos = start;
				while (hyphPos > 0) {
					hyphCursor.setPosition(hyphPos - 1);
					hyphCursor.setPosition(hyphPos, QTextCursor::KeepAnchor);
					if (hyphCursor.selectedText() != QLatin1String("-"))
						break;
					--hyphPos;
				}
				cmpCursor.setPosition(hyphPos);
				cmpCursor.setPosition(end, QTextCursor::KeepAnchor);
			}
			else if (cmpCursor.selectedText() != QLatin1String("{")) {
				QTextCursor braceCursor(cmpCursor);
				braceCursor.setPosition(start - 1);
				braceCursor.setPosition(start, QTextCursor::KeepAnchor);
				if (braceCursor.selectedText() == QLatin1String("{")) {
					cmpCursor.setPosition(start - 1);
					cmpCursor.setPosition(end, QTextCursor::KeepAnchor);
				}
			}
		}
		
		while (true) {
			QString completionPrefix = cmpCursor.selectedText();
			if (!completionPrefix.isEmpty()) {
				setCompleter(sharedCompleter);
				c->setCompletionPrefix(completionPrefix);
				if (c->completionCount() == 0) {
					if (cmpCursor.selectionStart() < start) {
						// we must have included a preceding brace or hyphen; now try without it
						cmpCursor.setPosition(start);
						cmpCursor.setPosition(end, QTextCursor::KeepAnchor);
						continue;
					}
					setCompleter(nullptr);
				}
				else {
					if (seq == actionPrevious_Completion->shortcut())
						c->setCurrentRow(c->completionCount() - 1);
					showCurrentCompletion();
					return true;
				}
			}
			break;
		}
	}
	
	if (c && c->completionCount() > 0) {
		if (seq == actionPrevious_Completion->shortcut()) {
			if (c->currentRow() == 0) {
				showCompletion(c->completionPrefix());
				setCompleter(nullptr);
			}
			else {
				c->setCurrentRow(c->currentRow() - 1);
				showCurrentCompletion();
			}
		}
		else {
			if (c->currentRow() == c->completionCount() - 1) {
				showCompletion(c->completionPrefix());
				setCompleter(nullptr);
			}
			else {
				c->setCurrentRow(c->currentRow() + 1);
				showCurrentCompletion();
			}
		}
		return true;
	}
	return false;
}

void CompletingEdit::handleTab(QKeyEvent * e)
{
	if (textCursor().hasSelection()) {
		if(e->modifiers() == Qt::ShiftModifier) {
			unPrefixLines(QString::fromLatin1("\t"));
		} else {
			prefixLines(QString::fromLatin1("\t"));
		}
	} else {
		QTextEdit::keyPressEvent(e);
	}
}

void CompletingEdit::showCompletion(const QString& completion, int insOffset)
{
	disconnect(this, SIGNAL(cursorPositionChanged()), this, SLOT(cursorPositionChangedSlot()));

	if (c->widget() != this)
		return;

	QTextCursor tc = cmpCursor;
	if (tc.isNull()) {
		tc = textCursor();
		tc.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, c->completionPrefix().length());
	}

	tc.insertText(completion);
	cmpCursor = tc;
	cmpCursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, completion.length());

	if (insOffset != -1)
		tc.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, completion.length() - insOffset);
	setTextCursor(tc);

	currentCompletionRange = cmpCursor;
	resetExtraSelections();

	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(cursorPositionChangedSlot()));
}

void CompletingEdit::showCurrentCompletion()
{
	if (c->widget() != this)
		return;

	QStandardItemModel *model = qobject_cast<QStandardItemModel*>(c->model());
	QList<QStandardItem*> items = model->findItems(c->currentCompletion());

	if (items.count() > 1) {
		if (c->currentCompletion() == prevCompletion) {
			if (c->currentIndex().row() > prevRow)
				++itemIndex;
			else
				--itemIndex;
			if (itemIndex < 0)
				itemIndex = items.count() - 1;
			else if (itemIndex > items.count() - 1)
				itemIndex = 0;
		}
		else
			itemIndex = 0;
		prevRow = c->currentIndex().row();
		prevCompletion = c->currentCompletion();
	}
	else {
		prevCompletion = QString();
		itemIndex = 0;
	}

	QString completion = model->item(items[itemIndex]->row(), 1)->text();
	
	int insOffset = completion.indexOf(QLatin1String("#INS#"));
	if (insOffset != -1)
		completion.replace(QLatin1String("#INS#"), QLatin1String(""));

	showCompletion(completion, insOffset);
}

void CompletingEdit::loadCompletionsFromFile(QStandardItemModel *model, const QString& filename)
{
	QFile	completionFile(filename);
	if (completionFile.exists() && completionFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream in(&completionFile);
		in.setCodec("UTF-8");
		in.setAutoDetectUnicode(true);
		QList<QStandardItem*> row;
		while (true) {
			QString	line = in.readLine();
			if (line.isNull())
				break;
			if (line[0] == QChar::fromLatin1('%'))
				continue;
			line.replace(QLatin1String("#RET#"), QLatin1String("\n"));
			QStringList parts = line.split(QString::fromLatin1(":="));
			if (parts.count() > 2)
				continue;
			if (parts.count() == 1)
				parts.append(parts[0]);
			parts[0].replace(QLatin1String("#INS#"), QLatin1String(""));
			row.append(new QStandardItem(parts[0]));
			row.append(new QStandardItem(parts[1]));
			model->appendRow(row);
			row.clear();
		}
		completionFile.close();
	}
}

void CompletingEdit::loadCompletionFiles(QCompleter *theCompleter)
{
	QStandardItemModel *model = new QStandardItemModel(0, 2, theCompleter); // columns are abbrev, expansion

	QDir completionDir(TWUtils::getLibraryPath(QString::fromLatin1("completion")));
	foreach (QFileInfo fileInfo, completionDir.entryInfoList(QDir::Files | QDir::Readable, QDir::Name)) {
		loadCompletionsFromFile(model, fileInfo.canonicalFilePath());
	}

	theCompleter->setModel(model);
}

void CompletingEdit::jumpToPdf(QTextCursor pos)
{
	if (pos.isNull())
		pos = textCursor();
	emit syncClick(pos.blockNumber() + 1, pos.positionInBlock());
}

void CompletingEdit::jumpToPdfFromContextMenu()
{
	QAction *act = qobject_cast<QAction*>(sender());
	if (act) {
		QPoint pt = act->data().toPoint();
		emit syncClick(pt.y(), pt.x());
	}
}

void CompletingEdit::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu *menu = createStandardContextMenu();
	QAction *defaultAction = nullptr;
	QAction *act = new QAction(tr("Jump to PDF"), menu);
	QTextCursor cur = cursorForPosition(event->pos());

	act->setData(QVariant(QPoint(cur.positionInBlock(), cur.blockNumber() + 1)));
	connect(act, SIGNAL(triggered()), this, SLOT(jumpToPdfFromContextMenu()));
	menu->insertSeparator(menu->actions().first());
	menu->insertAction(menu->actions().first(), act);
	
	const Tw::Document::SpellChecker::Dictionary * dictionary = getSpellChecker();
	if (dictionary) {
		currentWord = cursorForPosition(event->pos());
		currentWord.setPosition(currentWord.position());
		if (selectWord(currentWord)) {
			if (!dictionary->isWordCorrect(currentWord.selectedText())) {
				QAction *sep = menu->insertSeparator(menu->actions().first());

				QList<QString> suggestions = dictionary->suggestionsForWord(currentWord.selectedText());
				if (suggestions.size() == 0)
					menu->insertAction(sep, new QAction(tr("No suggestions"), menu));
				else {
					QSignalMapper *mapper = new QSignalMapper(menu);
					foreach(const QString & str, suggestions) {
						act = new QAction(str, menu);
						connect(act, SIGNAL(triggered()), mapper, SLOT(map()));
						mapper->setMapping(act, str);
						menu->insertAction(sep, act);
						if (!defaultAction)
							defaultAction = act;
					}
					connect(mapper, SIGNAL(mapped(const QString&)), this, SLOT(correction(const QString&)));
				}
				sep = menu->insertSeparator(menu->actions().first());
//				QAction *add = new QAction(tr("Add to dictionary"), menu);
//				connect(add, SIGNAL(triggered()), this, SLOT(addToDictionary()));
//				menu->insertAction(sep, add);
				QAction *ignore = new QAction(tr("Ignore word"), menu);
				connect(ignore, SIGNAL(triggered()), this, SLOT(ignoreWord()));
				menu->insertAction(sep, ignore);
			}
		}
	}
	
	menu->exec(event->globalPos(), defaultAction);
	delete menu;
}

void CompletingEdit::setAutoIndentMode(int index)
{
	autoIndentMode = (index >= 0 && index < indentModes->count()) ? index : -1;
}

void CompletingEdit::correction(const QString& suggestion)
{
	currentWord.insertText(suggestion);
}

void CompletingEdit::addToDictionary()
{
	// For this to be useful, we need to be able to store a user dictionary (per language).
	// Prefer to switch to Enchant first, before looking into this further.
}

void CompletingEdit::ignoreWord()
{
	Tw::Document::SpellChecker::Dictionary * dictionary = getSpellChecker();
	if (dictionary == nullptr)
		return;
	// note that this is not persistent after quitting TW
	dictionary->ignoreWord(currentWord.selectedText());
	emit rehighlight();
}

void CompletingEdit::loadIndentModes()
{
	if (!indentModes) {
		QDir configDir(TWUtils::getLibraryPath(QString::fromLatin1("configuration")));
		indentModes = new QList<IndentMode>;
		QFile indentPatternFile(configDir.filePath(QString::fromLatin1("auto-indent-patterns.txt")));
		if (indentPatternFile.open(QIODevice::ReadOnly)) {
			QRegularExpression re(QStringLiteral("^\"([^\"]+)\"\\s+(.+)$"));
			while (true) {
				QByteArray ba = indentPatternFile.readLine();
				if (ba.size() == 0)
					break;
				if (ba[0] == '#' || ba[0] == '\n')
					continue;
				QString line = QString::fromUtf8(ba.data(), ba.size()).trimmed();
				QRegularExpressionMatch m = re.match(line);
				if (m.hasMatch()) {
					IndentMode mode;
					mode.name = m.captured(1);
					mode.regex = QRegularExpression(m.captured(2).trimmed());
					if (!mode.name.isEmpty() && mode.regex.isValid())
						indentModes->append(mode);
				}
			}
		}
	}
}

QStringList CompletingEdit::autoIndentModes()
{
	loadIndentModes();
	
	QStringList modes;
	foreach (const IndentMode& mode, *indentModes)
		modes << mode.name;
	return modes;
}

void CompletingEdit::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasUrls())
		event->ignore();
	else
		QTextEdit::dragEnterEvent(event);
}

void CompletingEdit::dropEvent(QDropEvent *event)
{
	QTextCursor dropCursor = cursorForPosition(event->pos());
	if (!dropCursor.isNull()) {
		droppedOffset = dropCursor.position();
		droppedLength = event->mimeData()->text().length();
	}
	else
		droppedOffset = -1;

	int scrollX = horizontalScrollBar()->value();
	int scrollY = verticalScrollBar()->value();
	QTextEdit::dropEvent(event);
	verticalScrollBar()->setValue(scrollY);
	horizontalScrollBar()->setValue(scrollX);
}

void CompletingEdit::insertFromMimeData(const QMimeData *source)
{
	if (source->hasText()) {
		QTextCursor curs = textCursor();
		curs.insertText(source->text());
	}
}

bool CompletingEdit::canInsertFromMimeData(const QMimeData *source) const
{
	return source->hasText();
}

// support for the line-number area
// from Qt tutorial "Code Editor"

void CompletingEdit::setLineNumberDisplay(bool displayNumbers)
{
	lineNumberArea->setVisible(displayNumbers);
	updateLineNumberAreaWidth(0);
}

bool CompletingEdit::getLineNumbersVisible() const
{
	return lineNumberArea->isVisible();
}

void CompletingEdit::updateLineNumberAreaWidth(int /* newBlockCount */)
{
	if (lineNumberArea->isVisible()) {
		setViewportMargins(lineNumberArea->sizeHint().width(), 0, 0, 0);
		lineNumberArea->update();
	}
	else {
		setViewportMargins(0, 0, 0, 0);
	}
}

void CompletingEdit::updateLineNumberArea(const QRect &rect, int dy)
{
	if (dy)
		lineNumberArea->scroll(0, dy);
	else
		lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
	
	if (rect.contains(viewport()->rect()))
		updateLineNumberAreaWidth(0);
}

void CompletingEdit::resizeEvent(QResizeEvent *e)
{
	QTextEdit::resizeEvent(e);
	
	QRect cr = contentsRect();
	lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberArea->sizeHint().width(), cr.height()));
}

void CompletingEdit::wheelEvent(QWheelEvent *e)
{
	if (e->modifiers() & Qt::ControlModifier)
	{
		wheelDelta += e->delta();  // accumulate wheelDelta for high-resolution mice, which might pass small values.
		int sign = (wheelDelta < 0) ? -1 : 1;
		const int stepSize = 120;  // according to Qt docs a standard wheel step corresponds to a delta of 120.
		int steps = (sign * wheelDelta) / stepSize;  // abs value to guarantee rounding towards 0.
		if (steps > 0) {
			QFont ft = font();
			const int minFontSize = 4;
			ft.setPointSize(qMax(ft.pointSize() + sign * steps, minFontSize));
			setFont(ft);
			wheelDelta = 0;
		}
		e->accept();
		return;
	}

	QTextEdit::wheelEvent(e);
}

void CompletingEdit::setTextCursor(const QTextCursor & cursor)
{
	// QTextEdit::setTextCursor only scrolls to cursor.position(). If 
	// position() > anchor(), the two are on different lines, and the view has
	// to scroll up, this means that not the whole selection is visible.
	// By manually setting the cursor to anchor() first, we ensure that the
	// anchor is visible. Only then we set the final cursor (which may scroll to
	// position() as it should, but at least a large part of the selection will
	// be visible).
	QTextCursor c(cursor);
	c.setPosition(c.anchor());
	QTextEdit::setTextCursor(c);
	QTextEdit::setTextCursor(cursor);
}

void CompletingEdit::setDocument(QTextDocument * document)
{
	disconnect(this, SLOT(updateLineNumberAreaWidth(int)));
	QTextEdit::setDocument(document);
	connect(document, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
}

bool CompletingEdit::event(QEvent *e)
{
	if (e->type() == QEvent::UpdateRequest) {
		// should be limiting the rect to what actually needs updating
		// but don't know how to get that from the event :(
		emit updateRequest(viewport()->rect(), 0);
	}
	// Alternatively, we could use QEvent::ApplicationPaletteChange if we'd
	// derive the colors from the application's palette
	if (e->type() == QEvent::PaletteChange)
		updateColors();
	if (e->type() == QEvent::ShortcutOverride) {
		auto ke = reinterpret_cast<QKeyEvent*>(e);
		QKeySequence seq(static_cast<int>(ke->modifiers()) | ke->key());
		if (seq == actionNext_Completion->shortcut() ||
			seq == actionPrevious_Completion->shortcut() ||
			seq == actionNext_Completion_Placeholder->shortcut() ||
			seq == actionPrevious_Completion_Placeholder->shortcut())
		{
			// If the key press corresponds to a completion shortcut, accept the
			// event to tell Qt not to treat it as a shortcut but to send it to
			// the focused widget normally (thereby invoking the keyPressEvent
			// handler)
			e->accept();
		}
	}

	return QTextEdit::event(e);
}

void CompletingEdit::scrollContentsBy(int dx, int dy)
{
	if (dy != 0) {
		emit updateRequest(viewport()->rect(), dy);
	}
	QTextEdit::scrollContentsBy(dx, dy);
}

Tw::Document::SpellChecker::Dictionary * CompletingEdit::getSpellChecker() const
{
	Tw::Document::TeXDocument * doc = qobject_cast<Tw::Document::TeXDocument *>(document());
	if (doc == nullptr)
		return nullptr;
	TeXHighlighter * highlighter = doc->getHighlighter();
	if (highlighter == nullptr)
		return nullptr;
	return highlighter->getSpellChecker();
}

void CompletingEdit::setHighlightCurrentLine(bool highlight)
{
	if (highlight != highlightCurrentLine) {
		highlightCurrentLine = highlight;
		TWApp::instance()->emitHighlightLineOptionChanged();
	}
}

void CompletingEdit::setAutocompleteEnabled(bool autocomplete)
{
	if (autocomplete != autocompleteEnabled) {
		autocompleteEnabled = autocomplete;
	}
}

void CompletingEdit::setFont(const QFont & font)
{
	QTextEdit::setFont(font);
	updateLineNumberAreaWidth((document() ? document()->blockCount() : 0));
}

void CompletingEdit::setFontFamily(const QString & fontFamily)
{
	QTextEdit::setFontFamily(fontFamily);
	updateLineNumberAreaWidth((document() ? document()->blockCount() : 0));
}

void CompletingEdit::setFontItalic(bool italic)
{
	QTextEdit::setFontItalic(italic);
	updateLineNumberAreaWidth((document() ? document()->blockCount() : 0));
}

void CompletingEdit::setFontPointSize(qreal s)
{
	QTextEdit::setFontPointSize(s);
	updateLineNumberAreaWidth((document() ? document()->blockCount() : 0));
}

void CompletingEdit::setFontWeight(int weight)
{
	QTextEdit::setFontWeight(weight);
	updateLineNumberAreaWidth((document() ? document()->blockCount() : 0));
}


QTextCharFormat	*CompletingEdit::currentCompletionFormat = nullptr;
QTextCharFormat	*CompletingEdit::braceMatchingFormat = nullptr;
QTextCharFormat	*CompletingEdit::currentLineFormat = nullptr;
bool CompletingEdit::highlightCurrentLine = true;
bool CompletingEdit::autocompleteEnabled = true;

QCompleter	*CompletingEdit::sharedCompleter = nullptr;

QList<CompletingEdit::IndentMode> *CompletingEdit::indentModes = nullptr;
QList<CompletingEdit::QuotesMode> *CompletingEdit::quotesModes = nullptr;
