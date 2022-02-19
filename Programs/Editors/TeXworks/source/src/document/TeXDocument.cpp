/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2019-2022  Stefan Löffler

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

#include "document/TeXDocument.h"
#include "TeXHighlighter.h"

namespace Tw {
namespace Document {

TeXDocument::TeXDocument(QObject * parent) : TextDocument(parent)
{
	connect(this, &TeXDocument::contentsChange, this, &TeXDocument::maybeUpdateModeLines);
}

TeXDocument::TeXDocument(const QString & text, QObject * parent) : TextDocument(text, parent)
{
	connect(this, &TeXDocument::contentsChange, this, &TeXDocument::maybeUpdateModeLines);
	parseModeLines();
}

TeXHighlighter * TeXDocument::getHighlighter() const
{
	return findChild<TeXHighlighter*>();
}

void TeXDocument::parseModeLines()
{
	QMap<QString, QString> newModeLines;

	QRegularExpression re(QStringLiteral(u"%(?:\\^\\^A)?\\s*!TEX\\s+(?:TS-)?(\\w+)\\s*=\\s*([^\r\n\x2029]+)[\r\n\x2029]"), QRegularExpression::CaseInsensitiveOption);

	QTextCursor curs(this);
	// (begin|end)EditBlock() is a workaround for QTBUG-24718 that causes
	// movePosition() to crash the program under some circumstances.
	// Since we don't change any text in the edit block, it should be a noop
	// in the context of undo/redo.
	curs.beginEditBlock();
	curs.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, PeekLength);
	curs.endEditBlock();

	QRegularExpressionMatchIterator it = re.globalMatch(curs.selectedText());

	while (it.hasNext()) {
		QRegularExpressionMatch m = it.next();
		newModeLines.insert(m.captured(1).trimmed().toLower(), m.captured(2).trimmed());
	}

	if (_modelines != newModeLines) {
		QStringList changedKeys;
		QStringList removedKeys;

		Q_FOREACH(QString key, _modelines.keys()) {
			if (!newModeLines.contains(key)) {
				removedKeys.append(key);
			}
		}
		Q_FOREACH(QString key, newModeLines.keys()) {
			if (!_modelines.contains(key) || _modelines.value(key) != newModeLines.value(key)) {
				changedKeys.append(key);
			}
		}

		_modelines = newModeLines;
		emit modelinesChanged(changedKeys, removedKeys);
	}
}

void TeXDocument::maybeUpdateModeLines(int position, int charsRemoved, int charsAdded)
{
	Q_UNUSED(charsRemoved)
	Q_UNUSED(charsAdded)

	if (position < PeekLength)
		parseModeLines();
}

// static
bool TeXDocument::findNextWord(const QString & text, int index, int & start, int & end)
{
	// try to do a sensible "word" selection for TeX documents, taking into
	// account the form of control sequences:
	// given an index representing a caret,
	// - if current char (following caret) is a letter, apostrophe, or '@',
	//   extend in both directions
	//   - include apostrophe if surrounded by letters
	//   - include preceding backslash if any, unless word contains apostrophe
	// - if preceeding char is a \, extend to include \ only
	// - if current char is a number, extend in both directions
	// - if current char is a space or tab, extend in both directions to include
	//   all spaces or tabs
	// - if current char is a \, include next char; if letter or '@', extend to
	//   include all following letters or '@'
	// - else select single char following index
	// returns TRUE if the resulting selection consists of word-forming chars

	start = end = index;

	if (text.length() < 1) // empty
		return false;
	if (index >= text.length()) // end of line
		return false;

	QChar ch = text.at(index);

	auto isWordForming = [](const QChar & c) { return c.isLetter() || c.isMark(); };

	if (isWordForming(ch) || ch == QChar::fromLatin1('@') /* || ch == QChar::fromLatin1('\'') || ch == 0x2019 */) {
		bool isControlSeq{false}; // becomes true if we include an @ sign or a leading backslash
		bool includesApos{false}; // becomes true if we include an apostrophe
		if (ch == QChar::fromLatin1('@'))
			isControlSeq = true;
		//else if (ch == QChar::fromLatin1('\'') || ch == 0x2019)
		//	includesApos = true;
		while (start > 0) {
			--start;
			ch = text.at(start);
			if (isWordForming(ch))
				continue;
			if (!includesApos && ch == QChar::fromLatin1('@')) {
				isControlSeq = true;
				continue;
			}
			if (!isControlSeq && (ch == QChar::fromLatin1('\'') || ch == QChar(0x2019)) && start > 0 && isWordForming(text.at(start - 1))) {
				includesApos = true;
				continue;
			}
			++start;
			break;
		}
		if (start > 0 && text.at(start - 1) == QChar::fromLatin1('\\')) {
			isControlSeq = true;
			--start;
		}
		while (++end < text.length()) {
			ch = text.at(end);
			if (isWordForming(ch))
				continue;
			if (!includesApos && ch == QChar::fromLatin1('@')) {
				isControlSeq = true;
				continue;
			}
			if (!isControlSeq && (ch == QChar::fromLatin1('\'') || ch == QChar(0x2019)) && end < text.length() - 1 && isWordForming(text.at(end + 1))) {
				includesApos = true;
				continue;
			}
			break;
		}
		return !isControlSeq;
	}

	if (index > 0 && text.at(index - 1) == QChar::fromLatin1('\\')) {
		start = index - 1;
		end = index + 1;
		return false;
	}

	if (ch.isNumber()) {
		// TODO: handle decimals, leading signs
		while (start > 0) {
			--start;
			ch = text.at(start);
			if (ch.isNumber())
				continue;
			++start;
			break;
		}
		while (++end < text.length()) {
			ch = text.at(end);
			if (ch.isNumber())
				continue;
			break;
		}
		return false;
	}

	if (ch == QChar::fromLatin1(' ') || ch == QChar::fromLatin1('\t')) {
		while (start > 0) {
			--start;
			ch = text.at(start);
			if (!(ch == QChar::fromLatin1(' ') || ch == QChar::fromLatin1('\t'))) {
				++start;
				break;
			}
		}
		while (++end < text.length()) {
			ch = text.at(end);
			if (!(ch == QChar::fromLatin1(' ') || ch == QChar::fromLatin1('\t')))
				break;
		}
		return false;
	}

	if (ch == QChar::fromLatin1('\\')) {
		if (++end < text.length()) {
			ch = text.at(end);
			if (isWordForming(ch) || ch == QChar::fromLatin1('@'))
				while (++end < text.length()) {
					ch = text.at(end);
					if (isWordForming(ch) || ch == QChar::fromLatin1('@'))
						continue;
					break;
				}
			else
				++end;
		}
		return false;
	}

	// else the character is selected in isolation
	end = index + 1;
	return false;
}

} // namespace Document
} // namespace Tw
