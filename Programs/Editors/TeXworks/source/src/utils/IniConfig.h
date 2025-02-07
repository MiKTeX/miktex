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
#ifndef IniConfig_H
#define IniConfig_H

#include <QSettings>

namespace Tw {

namespace Utils {

class IniConfig : public QSettings
{
	Q_OBJECT
public:
	IniConfig(const QString & filename, QObject * parent = nullptr) : QSettings(filename, QSettings::IniFormat, parent) { }
};

} // namespace Utils

} // namespace Tw

#endif // !defined(IniConfig_H)
