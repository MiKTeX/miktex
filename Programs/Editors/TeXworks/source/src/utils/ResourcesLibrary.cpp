/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2020-2021  Stefan Löffler

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

#include "utils/ResourcesLibrary.h"

#include "TWApp.h" // for PATH_LIST_SEP
#include "utils/FileVersionDatabase.h"
#include "utils/VersionInfo.h"

#include <QDebug>
#include <QDirIterator>
#include <QStandardPaths>

namespace Tw {

namespace Utils {

QString ResourcesLibrary::m_portableLibPath;

#define TEXWORKS_NAME "TeXworks"

#if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
// compile-time default paths - customize by defining in the .pro file
#	ifndef TW_DICPATH
#		define TW_DICPATH "/usr/share/hunspell" PATH_LIST_SEP "/usr/share/myspell/dicts"
#	endif
#endif

// static
const QString ResourcesLibrary::getLibraryRootPath()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
	return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#else
	return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#endif
}

// the return value is sorted from new to old
const QStringList ResourcesLibrary::getLegacyLibraryRootPaths()
{
	QStringList retVal;
#if defined(Q_OS_DARWIN)
	retVal << QDir::homePath() + QLatin1String("/Library/" TEXWORKS_NAME "/");
#elif defined(Q_OS_UNIX) // && !defined(Q_OS_DARWIN)
	retVal << QDir::homePath() + QLatin1String("/." TEXWORKS_NAME "/");
#else // defined(Q_OS_WIN)
	retVal << QDir::homePath() + QLatin1String("/" TEXWORKS_NAME "/");
#endif
	return retVal;
}

// static
bool ResourcesLibrary::shouldMigrateLegacyLibrary()
{
	// We don't migrate old (system) libraries in portable mode
	if (!getPortableLibPath().isEmpty()) {
		return false;
	}
	const QString dst = getLibraryRootPath();
	// We don't migrate if the destination exists already
	if (QDir(dst).exists()) {
		return false;
	}
	for (const QString & src : getLegacyLibraryRootPaths()) {
		if (QDir(src).exists()) {
			return true;
		}
	}
	return false;
}

// static
void ResourcesLibrary::migrateLegacyLibrary()
{
	const QDir dst(getLibraryRootPath());

	for (const QString & srcPath : getLegacyLibraryRootPaths()) {
		QDir src(srcPath);
		if (src.exists()) {
			qDebug() << "Migrating resources library from" << src.absolutePath() << "to" << dst.absolutePath();
			dst.mkpath(QStringLiteral("."));
			QDirIterator it(src, QDirIterator::Subdirectories);
			while (it.hasNext()) {
				it.next();
				const QFileInfo & srcFileInfo(it.fileInfo());
				const QString relativePath(src.relativeFilePath(srcFileInfo.absoluteFilePath()));

				if (srcFileInfo.isSymLink()) {
					QFile::link(srcFileInfo.symLinkTarget(), dst.filePath(relativePath));
				}
				else if (srcFileInfo.isDir()) {
					dst.mkpath(relativePath);
				}
				else {
					QFile f(srcFileInfo.absoluteFilePath());
					f.copy(dst.filePath(relativePath));
				}
			}
		}
	}
}

// static
const QString ResourcesLibrary::getLibraryPath(const QString& subdir, const bool updateOnDisk /* = true */)
{
	QString libRootPath, libPath;

	libRootPath = getPortableLibPath();
	if (libRootPath.isEmpty()) {
#if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
		if (subdir == QLatin1String("dictionaries")) {
			libPath = QString::fromLatin1(TW_DICPATH);
			QString dicPath = QString::fromLocal8Bit(getenv("TW_DICPATH"));
			if (!dicPath.isEmpty())
				libPath = dicPath;
			return libPath; // don't try to create/update the system dicts directory
		}
#endif
		libRootPath = getLibraryRootPath();
		if (shouldMigrateLegacyLibrary()) {
			migrateLegacyLibrary();
		}
	}
	libPath = QDir(libRootPath).absolutePath() + QStringLiteral("/") + subdir;

	if(updateOnDisk)
		updateLibraryResources(QDir(QString::fromLatin1(":/resfiles")), libRootPath, subdir);
	return libPath;
}

// static
const QStringList ResourcesLibrary::getLibraryPaths(const QString & subdir, const bool updateOnDisk)
{
	return getLibraryPath(subdir, updateOnDisk).split(QStringLiteral(PATH_LIST_SEP));
}

// static
void ResourcesLibrary::updateLibraryResources(const QDir& srcRootDir, const QDir& destRootDir, const QString& subdir)
{
	QDir srcDir(srcRootDir);
	QDir destDir(destRootDir.absolutePath() + QDir::separator() + subdir);

	// sanity check
	if (!srcDir.cd(subdir))
		return;

	// make sure the library folder exists - even if the user deleted it;
	// otherwise other parts of the program might fail
	if (!destDir.exists())
		QDir::root().mkpath(destDir.absolutePath());

	if (subdir == QString::fromLatin1("translations")) // don't copy the built-in translations
		return;

	Tw::Utils::FileVersionDatabase fvdb = Tw::Utils::FileVersionDatabase::load(destRootDir.absoluteFilePath(QString::fromLatin1("TwFileVersions.db")));

	QDirIterator iter(srcDir, QDirIterator::Subdirectories);
	while (iter.hasNext()) {
		(void)iter.next();
		// Skip directories (they get created on-the-fly if required for copying files)
		if (iter.fileInfo().isDir())
			continue;

		QString srcPath = iter.fileInfo().filePath();
		QString path = srcRootDir.relativeFilePath(srcPath);
		QFileInfo destPath(destRootDir.filePath(path));

		// Check if the file is in the database
		if (fvdb.hasFileRecord(destPath)) {
			Tw::Utils::FileVersionDatabase::Record rec = fvdb.getFileRecord(destPath);
			// If the file no longer exists on the disk, the user has deleted it
			// Hence we won't recreate it, but we keep the database record to
			// remember that this file was deleted by the user
			if (!QFileInfo(destPath).exists())
				continue;

			QByteArray srcHash = Tw::Utils::FileVersionDatabase::hashForFile(srcPath);
			QByteArray destHash = Tw::Utils::FileVersionDatabase::hashForFile(destPath.filePath());
			// If the file was modified, don't do anything, either
			if (destHash != rec.hash) {
				// The only exception is if the file on the disk matches the
				// new file we would have installed. In this case, we reassume
				// ownership of it. (This is the case if the user deleted the
				// file, but later wants to resurrect it by downloading the
				// latest version from the internet)
				if (destHash != srcHash)
					continue;
				fvdb.addFileRecord(destPath, srcHash, Tw::Utils::VersionInfo::gitCommitHash());
			}
			else {
				// The file matches the record in the database; update it
				// (copying is only necessary if the contents has changed)
				if (srcHash == destHash)
					fvdb.addFileRecord(destPath, srcHash, Tw::Utils::VersionInfo::gitCommitHash());
				else {
					// we have to remove the file first as QFile::copy doesn't
					// overwrite existing files
					QFile::remove(destPath.filePath());
					if(QFile::copy(srcPath, destPath.filePath()))
						fvdb.addFileRecord(destPath, srcHash, Tw::Utils::VersionInfo::gitCommitHash());
				}
			}
		}
		else {
			QByteArray srcHash = Tw::Utils::FileVersionDatabase::hashForFile(srcPath);
			// If the file is not in the database, we add it - unless a file
			// with the name already exists
			if (!QFileInfo(destPath).exists()) {
				// We have to make sure the directory exists - otherwise copying
				// might fail
				destRootDir.mkpath(QFileInfo(destPath).path());
				QFile(srcPath).copy(destPath.filePath());
				fvdb.addFileRecord(destPath, srcHash, Tw::Utils::VersionInfo::gitCommitHash());
			}
			else {
				// If a file with that name already exists, we don't replace it
				// If it happens to be identical with the version we would install
				// we do take ownership, however, and register it in the
				// database so that future updates are applied
				QByteArray destHash = Tw::Utils::FileVersionDatabase::hashForFile(destPath.filePath());
				if (srcHash == destHash)
					fvdb.addFileRecord(destPath, destHash, Tw::Utils::VersionInfo::gitCommitHash());
			}
		}
	}

	// Now, remove all files that are unmodified on disk and were
	// removed upstream
	QMutableListIterator<Tw::Utils::FileVersionDatabase::Record> recIt(fvdb.getFileRecords());
	while (recIt.hasNext()) {
		const Tw::Utils::FileVersionDatabase::Record & rec = recIt.next();

		QString destPath = rec.filePath.filePath();
		QString path = destRootDir.relativeFilePath(destPath);
		QString srcPath = srcRootDir.filePath(path);

		// If the source file still exists there is nothing to do here
		if (QFileInfo(srcPath).exists())
			continue;

		// If the source file no longer exists but the file on disk is up to
		// date, remove it
		if (rec.filePath.exists() && Tw::Utils::FileVersionDatabase::hashForFile(destPath) == rec.hash) {
			QFile(destPath).remove();
			recIt.remove();
		}
	}

	// Finally, save the updated database
	fvdb.save(destRootDir.absoluteFilePath(QString::fromLatin1("TwFileVersions.db")));
}

} // namespace Utils

} // namespace Tw
