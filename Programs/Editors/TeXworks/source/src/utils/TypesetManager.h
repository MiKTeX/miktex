/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2022  Stefan Löffler

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
#ifndef TYPESETMANAGER_H
#define TYPESETMANAGER_H

#include <QMap>
#include <QObject>
#include <QString>

namespace Tw {
namespace Utils {

// Class that keeps track of all running typesetting processes and who started/
// owns them.
// This helps avoid running multiple processes on the same input file (which
// would wreak havoc in the auxiliary and output files) and provides information
// in which object (window) information about a currently running typesetting
// process for a given input (root) file can be found
class TypesetManager : public QObject
{
	Q_OBJECT
public:
	explicit TypesetManager(QObject * parent = nullptr) : QObject(parent) { }

	// In practice, the returned object should be a TeXDocumentWindow; to avoid
	// interdependencies of headers (and to enable other types as owners in the
	// future) we use a generic QObject* here instead
	QObject * getOwnerForRootFile(const QString & rootFile) const;
	bool isFileBeingTypeset(const QString & rootFile) const { return getOwnerForRootFile(rootFile) != nullptr; }

public slots:
	// Returns true if it is safe to start typesetting, false if typesetting
	// should not be started (e.g. because another owner is already typesetting
	// the specified root file)
	// The root file should always be a canonical file path
	bool startTypesetting(const QString & rootFile, QObject * const owner);
	void stopTypesetting(QObject * const owner);

signals:
	void typesettingStarted(const QString rootFile);
	void typesettingStopped(const QString rootFile);

private:
	QMap<QString, QObject*> m_running;
};

} // namespace Utils
} // namespace Tw

#endif // TYPESETMANAGER_H
