/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2008-2019  Stefan Löffler

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

#ifndef InterProcessCommunicator_H
#define InterProcessCommunicator_H

#include <QObject>

namespace Tw {

class InterProcessCommunicatorPrivate;

class InterProcessCommunicator : public QObject
{
	Q_OBJECT
#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0)
	Q_DECLARE_PRIVATE_D(_private.data(), InterProcessCommunicator)
#else
	Q_DECLARE_PRIVATE_D(_private, InterProcessCommunicator)
#endif
	bool _isFirstInstance{true};
	QScopedPointer<InterProcessCommunicatorPrivate> _private;

public:
	InterProcessCommunicator();
	~InterProcessCommunicator() override;

	bool isFirstInstance() const { return _isFirstInstance; }

	void sendBringToFront();
	void sendOpenFile(const QString & path, const int position = -1);

signals:
	void receivedBringToFront();
	void receivedOpenFile(const QString & path, const int position);
};

} // namespace Tw

#endif // !defined(InterProcessCommunicator_H)
