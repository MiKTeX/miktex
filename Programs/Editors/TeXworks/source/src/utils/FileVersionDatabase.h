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
#ifndef FileVersionDatabase_H
#define FileVersionDatabase_H

#include <QFileInfo>

namespace Tw {
namespace Utils {


class FileVersionDatabase
{
public:
	struct Record {
		QFileInfo filePath;
		QString version;
		QByteArray hash;
	};

	FileVersionDatabase() = default;
	virtual ~FileVersionDatabase() = default;

	static QByteArray hashForFile(const QString & path);

	static FileVersionDatabase load(const QString & path);
	bool save(const QString & path) const;

	void addFileRecord(const QFileInfo & file, const QByteArray & hash, const QString & version);
	bool hasFileRecord(const QFileInfo & file) const;
	Record getFileRecord(const QFileInfo & file) const;
	const QList<Record> & getFileRecords() const { return m_records; }
	QList<Record> & getFileRecords() { return m_records; }

private:
	QList<Record> m_records;
};

} // namespace Utils
} // namespace Tw

#endif // !defined(FileVersionDatabase_H)
