/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2020  Stefan Löffler

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

#include "utils/VersionInfo.h"

// Explicitly specify ../src/ to make unit-tests compile.
// GitRev.h is created automatically in the TeXworks binary directory (i.e.,
// <build>/src), but that is generally different from the unit-test binary
// directory (i.e., <build>/unit-tests)
#if defined(MIKTEX)
#include "GitRev.h"
#else
#include "../src/GitRev.h"
#endif
#include "TWVersion.h"

#ifndef TW_BUILD_ID
#define TW_BUILD_ID unknown build
#endif
#define STRINGIFY_2(s) #s
#define STRINGIFY(s) STRINGIFY_2(s)
#define TW_BUILD_ID_STR STRINGIFY(TW_BUILD_ID)

namespace Tw {

namespace Utils {

// static
bool VersionInfo::isGitInfoAvailable()
{
	return (!QStringLiteral(GIT_COMMIT_HASH).startsWith(QStringLiteral("$Format:")) && !QStringLiteral(GIT_COMMIT_DATE).startsWith(QStringLiteral("$Format:")));
}

// static
QString VersionInfo::gitCommitHash()
{
	const QString & hashStr = QStringLiteral(GIT_COMMIT_HASH);
	if(hashStr.startsWith(QStringLiteral("$Format:")))
		return QString();
	return hashStr;
}

// static
QDateTime VersionInfo::gitCommitDate()
{
	const QString & dateStr = QStringLiteral(GIT_COMMIT_DATE);
	if (dateStr.startsWith(QStringLiteral("$Format:")))
		return QDateTime();
	return QDateTime::fromString(dateStr, Qt::ISODate).toUTC();
}

// static
QString VersionInfo::versionString()
{
	return QStringLiteral(TEXWORKS_VERSION);
}

QString VersionInfo::buildIdString()
{
	return QStringLiteral(TW_BUILD_ID_STR);
}

// static
int VersionInfo::getVersion()
{
	return (VER_MAJOR << 16) | (VER_MINOR << 8) | VER_BUGFIX;
}

// static
int VersionInfo::getVersionMajor()
{
	return VER_MAJOR;
}

// static
int VersionInfo::getVersionMinor()
{
	return VER_MINOR;
}

// static
int VersionInfo::getVersionBugfix()
{
	return VER_BUGFIX;
}

} // namespace Utils

} // namespace Tw

