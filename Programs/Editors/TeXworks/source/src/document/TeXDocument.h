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
#ifndef Document_TeXDocument_H
#define Document_TeXDocument_H

#include "document/TextDocument.h"

#include <QMap>

class TeXHighlighter;

namespace Tw {
namespace Document {

class TeXDocument : public TextDocument
{
	Q_OBJECT
public:
	explicit TeXDocument(QObject * parent = nullptr);
	explicit TeXDocument(const QString & text, QObject * parent = nullptr);

	TeXHighlighter * getHighlighter() const;

	void parseModeLines();
	bool hasModeLine(const QString & key) const { return _modelines.contains(key); }
	QMap<QString, QString> getModeLines() const { return _modelines; }
	QString getModeLineValue(const QString & key) const { return _modelines.value(key); }

signals:
	void modelinesChanged(QStringList changedKeys, QStringList removedKeys);

protected slots:
	void maybeUpdateModeLines(int position, int charsRemoved, int charsAdded);

protected:
	static constexpr int PeekLength = 1024;
	QMap<QString, QString> _modelines;
};

} // namespace Document
} // namespace Tw

#endif // !defined(Document_TeXDocument_H)
