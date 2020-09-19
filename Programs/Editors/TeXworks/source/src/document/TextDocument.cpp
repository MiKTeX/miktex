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

#include "document/TextDocument.h"

namespace Tw {
namespace Document {

TextDocument::TextDocument(QObject * parent) : QTextDocument(parent) { }

TextDocument::TextDocument(const QString & text, QObject * parent) : QTextDocument(text, parent) { }

void TextDocument::addTag(const QTextCursor & cursor, const unsigned int level, const QString & text)
{
	QList<Tag>::iterator it;

	for (it = _tags.begin(); it != _tags.end(); ++it) {
		if (it->cursor.selectionStart() > cursor.selectionStart())
			break;
	}
	_tags.insert(it, {cursor, level, text});
	emit tagsChanged();
}

unsigned int TextDocument::removeTags(int offset, int len)
{
	unsigned int removed = 0;
	QList<Tag>::iterator start, end;

	for (start = _tags.begin(); start != _tags.end(); ++start) {
		if (start->cursor.selectionStart() >= offset)
			break;
	}
	for (end = start; end != _tags.end(); ++end) {
		if (end->cursor.selectionStart() < offset + len)
			++removed;
		else
			break;
	}
	if (removed > 0) {
		_tags.erase(start, end);
		emit tagsChanged();
	}
	return removed;
}

} // namespace Document
} // namespace Tw
