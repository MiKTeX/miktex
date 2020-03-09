/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2019  Stefan Löffler

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
	connect(this, SIGNAL(contentsChange(int, int, int)), this, SLOT(maybeUpdateModeLines(int, int, int)));
}

TeXDocument::TeXDocument(const QString & text, QObject * parent) : TextDocument(text, parent)
{
	connect(this, SIGNAL(contentsChange(int, int, int)), this, SLOT(maybeUpdateModeLines(int, int, int)));
	parseModeLines();
}

TeXHighlighter * TeXDocument::getHighlighter() const
{
	return findChild<TeXHighlighter*>();
}

void TeXDocument::parseModeLines()
{
	QMap<QString, QString> newModeLines;

	QRegularExpression re(QStringLiteral(u"%\\s*!TEX\\s+(?:TS-)?(\\w+)\\s*=\\s*([^\r\n\x2029]+)[\r\n\x2029]"), QRegularExpression::CaseInsensitiveOption);

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

} // namespace Document
} // namespace Tw
