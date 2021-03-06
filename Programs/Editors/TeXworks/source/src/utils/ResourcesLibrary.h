/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2008-2021  Stefan Löffler

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
#ifndef ResourcesLibrary_H
#define ResourcesLibrary_H

#include <QString>
#include <QDir>

namespace Tw {

namespace Utils {

class ResourcesLibrary
{
	static QString m_portableLibPath;
public:
	// return the path to our "library" folder for resources like templates, completion lists, etc
	static const QString getLibraryPath(const QString& subdir, const bool updateOnDisk = true);

	// same as getLibraryPath(), but splits the return value by PATH_LIST_SEP
	static const QStringList getLibraryPaths(const QString& subdir, const bool updateOnDisk = true);

	static void updateLibraryResources(const QDir& srcRootDir, const QDir& destRootDir, const QString& libPath);

	static QString getPortableLibPath() { return m_portableLibPath; }
	static void setPortableLibPath(const QString & path) { m_portableLibPath = path; }

private:
	static const QString getLibraryRootPath();
	// the return value is sorted from new to old
	static const QStringList getLegacyLibraryRootPaths();
	static bool shouldMigrateLegacyLibrary();
	static void migrateLegacyLibrary();
};

} // namespace Utils

} // namespace Tw

#endif // !defined(ResourcesLibrary)
