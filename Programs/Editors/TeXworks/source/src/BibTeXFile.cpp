/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2017-2018  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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
#include "BibTeXFile.h"

#include <QFile>
#include <QTextCodec>

BibTeXFile::Entry::Type BibTeXFile::Entry::type() const
{
	if (_type.toLower() == QString::fromLatin1("comment"))
		return COMMENT;
	else if (_type.toLower() == QString::fromLatin1("preamble"))
		return PREAMBLE;
	else if (_type.toLower() == QString::fromLatin1("string"))
		return STRING;
	return NORMAL;
}

QString BibTeXFile::Entry::howPublished() const
{
	if (_cache.valid) return _cache.howPublished;
	if (hasField(QString::fromLatin1("howpublished")))
		return value(QString::fromLatin1("howpublished"));
	return value(QString::fromLatin1("journal"));
}

QString BibTeXFile::Entry::value(const QString & key) const
{
	QString retVal;
	for (QMap<QString, QString>::const_iterator it = _fields.constBegin(); it != _fields.constEnd(); ++it) {
		if (QString::compare(key, it.key(), Qt::CaseInsensitive) == 0) {
			retVal = it.value();
			break;
		}
	}
	// strip surrounding {} (if any)
	if (retVal.startsWith(QLatin1Char('{')) && retVal.endsWith(QLatin1Char('}')))
		retVal = retVal.mid(1, retVal.length() - 2);
	// or surrounding "" (if any)
	else if (retVal.startsWith(QLatin1Char('"')) && retVal.endsWith(QLatin1Char('"')))
		retVal = retVal.mid(1, retVal.length() - 2);
	return retVal;
}

bool BibTeXFile::Entry::hasField(const QString & key) const
{
	for (QMap<QString, QString>::const_iterator it = _fields.constBegin(); it != _fields.constEnd(); ++it) {
		if (QString::compare(key, it.key(), Qt::CaseInsensitive) == 0)
			return true;
	}
	return false;
}

void BibTeXFile::Entry::updateCache()
{
	// invalidate cache so that all subsequent method calls actually search the
	// fields
	_cache.valid = false;
	_cache.author = author();
	_cache.howPublished = howPublished();
	_cache.title = title();
	_cache.year = year();
	_cache.valid = true;
}

BibTeXFile::BibTeXFile()
{

}

bool BibTeXFile::load(const QString & filename)
{
	QFile file(filename);
	QByteArray content;
	QTextCodec * codec = QTextCodec::codecForName("utf-8");
	int curPos = 0;
	int start;

	_entries.clear();

	if (!file.open(QFile::ReadOnly | QFile::Text))
		return false;

	content = file.readAll();
	file.close();

	// FIXME: Encoding detection
	if (!codec)
		return false;

	do {
		Entry e(this);
		curPos = readEntry(e, content, curPos, codec);
		if (curPos > 0) {
			e.updateCache();
			_entries.append(e);
		}
		// DEBUG
	} while(curPos > 0);

	return true;
}

template <class S, class C> int findBlock(const S & content, int from, const C & startDelim, const C & endDelim, const C & escapeChar)
{
	// Blocks are enclosed in {}.
	if (content[from] != startDelim)
		return -1;

	int open = 1;
	int i;
	bool escaped = false;
	for (i = from + 1; i < content.size() && open > 0; ++i) {
		if (escaped) escaped = false;
		else if (content[i] == escapeChar) escaped = true;
		// put endDelim check before startDelim check to ensure proper handling
		// of startDelim == endDelim (e.g., '"')
		else if (content[i] == endDelim) --open;
		else if (content[i] == startDelim) ++open;
	}
	if (open == 0) return i - 1;
	else return -1;
}

inline int findBlock(const QByteArray & content, int from, char startDelim = '{', char endDelim = '}', char escapeChar = 0)
{
	return findBlock<QByteArray, char>(content, from, startDelim, endDelim, escapeChar);
}

inline int findBlock(const QString & content, int from, const QChar & startDelim = QChar::fromLatin1('{'), const QChar & endDelim = QChar::fromLatin1('}'), const QChar & escapeChar = QChar())
{
	return findBlock<QString, QChar>(content, from, startDelim, endDelim, escapeChar);
}

// static
int BibTeXFile::readEntry(Entry & e, const QByteArray & content, int curPos, const QTextCodec * codec)
{
	QList<QString> delims;
	curPos = content.indexOf('@', curPos);
	if (curPos < 0)
		return -1;
	++curPos;
	int start = content.indexOf('{', curPos);
	if (start < 0)
		return -1;
	e._type = codec->toUnicode(content.mid(curPos, start - curPos));

	int end = findBlock(content, start);
	if (end < 0) return -1;
	++start;
	QByteArray block = content.mid(start, end - start);

	switch (e.type()) {
	case Entry::COMMENT:
		e._key = codec->toUnicode(block);
		break;
	case Entry::PREAMBLE:
		e._key = codec->toUnicode(block);
		break;
	case Entry::STRING:
		// FIXME
		e._key = codec->toUnicode(block);
		break;
	case Entry::NORMAL:
		parseEntry(e, codec->toUnicode(block));
		break;
	}

	return end + 1;
}

//static
void BibTeXFile::parseEntry(Entry & e, const QString & block)
{
	int pos = block.indexOf(QChar::fromLatin1(','));
	e._key = block.mid(0, pos).trimmed();
	if (pos == -1) return;

	int i;
	QChar startDelim, endDelim;

	do {
		int start = pos + 1;
		pos = block.indexOf(QChar::fromLatin1('='), start);
		if (pos < 0) break;
		QString key = block.mid(start, pos - start).trimmed();
		QString val;

		start = -1;

		// Skip initial whitespace
		for (i = pos + 1; i < block.size() && start < 0; ++i) {
			if (!block[i].isSpace()) start = i;
		}

		for (i = start; i < block.size(); ++i) {
			if (block[i] == QChar::fromLatin1(',')) break;
			else if (block[i] == QChar::fromLatin1('{')) {
				startDelim = QChar::fromLatin1('{');
				endDelim = QChar::fromLatin1('}');
			}
			else if (block[i] == QChar::fromLatin1('"')) {
				startDelim = QChar::fromLatin1('"');
				endDelim = QChar::fromLatin1('"');
			}
			else {
				val += block[i];
				continue;
			}

			int end = findBlock(block, i, startDelim, endDelim);
			if (end < 0) {
				val += block.mid(i);
				i = block.size();
			}
			else {
				val += block.mid(i, end - i + 1);
				i = end;
			}
		}
		e._fields[key] = val.trimmed();
		pos = i;
	} while (pos >= 0 && pos + 1 < block.size());
}

unsigned int BibTeXFile::numEntries() const
{
	// Only count "normal" entries
	unsigned int retVal = 0;
	for (unsigned int i = 0; i < _entries.size(); ++i) {
		if (_entries[i].type() == Entry::NORMAL) ++retVal;
	}
	return retVal;
}

const BibTeXFile::Entry & BibTeXFile::entry(const unsigned int idx) const
{
	unsigned int j = 0;
	for (unsigned int i = 0; i < _entries.size(); ++i) {
		if (_entries[i].type() != Entry::NORMAL) continue;
		if (j == idx) return _entries[i];
		++j;
	}
	// We should never get here
	static BibTeXFile::Entry e(NULL);
	return e;
}
