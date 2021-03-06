/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2017-2020  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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
#ifndef BIBTEXFILE_H
#define BIBTEXFILE_H

#include <QList>
#include <QMap>
#include <QString>
#include <QTextCodec>

class BibTeXFile
{
public:
	class Entry {
		friend BibTeXFile;
	public:
		enum Type { NORMAL, COMMENT, PREAMBLE, STRING };

		explicit Entry(BibTeXFile * parent) : _parent(parent) { _cache.valid = false; }
		Type type() const;
		QString value(const QString & key) const;
		bool hasField(const QString & key) const;
		QString title() const { return (_cache.valid ? _cache.title : value(QString::fromLatin1("title"))); }
		QString author() const { return (_cache.valid ? _cache.author : value(QString::fromLatin1("author"))); }
		QString year() const { return (_cache.valid ? _cache.year : value(QString::fromLatin1("year"))); }
		QString howPublished() const;
		QString typeString() const { return _type; }
		QString key() const { return _key; }

	protected:
		void updateCache();

		QString _type;
		QString _key;
		// Use a cache for common values to avoid having to search through all
		// fields each time
		struct {
			QString title, author, year, howPublished;
			bool valid;
		} _cache;
		QMap<QString, QString> _fields;
		BibTeXFile * _parent;
	};

	BibTeXFile() = default;
	explicit BibTeXFile(const QString & filename) : BibTeXFile() { load(filename); }

	unsigned int numEntries() const;
	const Entry & entry(const unsigned int idx) const;

	bool load(const QString & filename);
protected:
	static int readEntry(Entry & e, const QByteArray & content, int curPos, const QTextCodec * codec);
	static void parseEntry(Entry & e, const QString & block);

	QList<Entry> _entries;
};

#endif // BIBTEXFILE_H
