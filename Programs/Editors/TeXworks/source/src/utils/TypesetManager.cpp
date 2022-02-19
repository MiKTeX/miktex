/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2008-2022  Stefan Löffler

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
#include "TypesetManager.h"

namespace Tw {
namespace Utils {

QObject * TypesetManager::getOwnerForRootFile(const QString & rootFile) const
{
	return m_running.value(rootFile, nullptr);
}

bool TypesetManager::startTypesetting(const QString & rootFile, QObject * const owner)
{
	if (rootFile.isEmpty() || owner == nullptr || m_running.contains(rootFile)) {
		return false;
	}
	m_running.insert(rootFile, owner);
	connect(owner, &QObject::destroyed, this, &TypesetManager::stopTypesetting);
	emit typesettingStarted(rootFile);
	return true;
}

void TypesetManager::stopTypesetting(QObject * const owner)
{
	Q_FOREACH(const QString & rootFile, m_running.keys(owner)) {
		m_running.remove(rootFile);
		emit typesettingStopped(rootFile);
	}
}

} // namespace Utils
} // namespace Tw
