/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2019-2024  Stefan Löffler

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
#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

namespace Tw {

class Settings
{
	QSettings m_s;
public:
#if defined(MIKTEX)
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
	using KeyType = QString;
#else
	using KeyType = QAnyStringView;
#endif
#else
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	using KeyType = QString;
#else
	using KeyType = QAnyStringView;
#endif
#endif

#if defined(MIKTEX)
	Settings();
#else
	Settings() = default;
#endif

	bool contains(KeyType key) const;
	void remove(KeyType key);
	void setValue(KeyType key, const QVariant & value);
	QVariant value(KeyType key, const QVariant & defaultValue = QVariant()) const;

	QString fileName() const;

	static void setPortableIniPath(const QString & iniPath);
#if defined(Q_OS_WIN)
	bool isStoredInRegistry();
#endif
};

} // namespace Tw

#endif // SETTINGS_H
