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
#include "InterProcessCommunicator.h"
#include <QtDBus>

namespace Tw {

#define TW_SERVICE_NAME 	"org.tug.texworks.application"
#define TW_APP_PATH		"/org/tug/texworks/application"
#define TW_INTERFACE_NAME	"org.tug.texworks.application"

class InterProcessCommunicatorPrivate : public QDBusAbstractAdaptor {
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.tug.texworks.application") // using the #define here fails :(
	Q_DISABLE_COPY(InterProcessCommunicatorPrivate)
	Q_DECLARE_PUBLIC(InterProcessCommunicator)

	InterProcessCommunicator * q_ptr;
	bool serviceRegistered;
	bool objectRegistered;
	QDBusInterface * interface;

	explicit InterProcessCommunicatorPrivate(InterProcessCommunicator * q) : QDBusAbstractAdaptor(q), q_ptr(q), serviceRegistered(false), objectRegistered(false), interface(nullptr) { }
public:
	~InterProcessCommunicatorPrivate() override = default;

public slots:
	Q_NOREPLY void openFile(QString fileName, int position = -1) {
		Q_Q(InterProcessCommunicator);
		emit q->receivedOpenFile(fileName, position);
	}
	Q_NOREPLY void bringToFront() {
		Q_Q(InterProcessCommunicator);
		emit q->receivedBringToFront();
	}
};

InterProcessCommunicator::InterProcessCommunicator()
	: _private(new InterProcessCommunicatorPrivate(this))
{
	Q_D(InterProcessCommunicator);

	d->serviceRegistered = QDBusConnection::sessionBus().registerService(QStringLiteral(TW_SERVICE_NAME));
	if (d->serviceRegistered) {
		_isFirstInstance = true;
	}
	else {
		d->interface = new QDBusInterface(QString::fromLatin1(TW_SERVICE_NAME), QString::fromLatin1(TW_APP_PATH), QString::fromLatin1(TW_INTERFACE_NAME));
		if (d->interface->isValid())
			_isFirstInstance = false;
		else {
			// We could not register the service, but couldn't connect to an
			// already registered one, either. This can mean that something is
			// seriously wrong, we've met some race condition, or the dbus
			// service is not running. Let's assume the best (dbus not running)
			// and continue as a multiple-instance app instead
			_isFirstInstance = true;
			return;
		}
	}
	if (_isFirstInstance) {
		d->objectRegistered = QDBusConnection::sessionBus().registerObject(QStringLiteral(TW_APP_PATH), this);

		// failed to register the application object, so unregister our service
		// and continue as a multiple-instance app instead
		if (!d->objectRegistered) {
			QDBusConnection::sessionBus().unregisterService(QStringLiteral(TW_SERVICE_NAME));
			d->serviceRegistered = false;
			_isFirstInstance = false;
		}
	}
}

InterProcessCommunicator::~InterProcessCommunicator()
{
	Q_D(InterProcessCommunicator);
	delete d->interface;
	d->interface = nullptr;
	if (d->objectRegistered) {
		QDBusConnection::sessionBus().unregisterObject(QStringLiteral(TW_APP_PATH));
		d->objectRegistered = false;
	}
	if (d->serviceRegistered) {
		QDBusConnection::sessionBus().unregisterService(QStringLiteral(TW_SERVICE_NAME));
		d->serviceRegistered = false;
	}
}

void InterProcessCommunicator::sendBringToFront()
{
	Q_D(InterProcessCommunicator);
	if (!d->interface || !d->interface->isValid())
		return;
	d->interface->call(QStringLiteral("bringToFront"));
}

void InterProcessCommunicator::sendOpenFile(const QString & path, const int position)
{
	Q_D(InterProcessCommunicator);
	if (!d->interface || !d->interface->isValid())
		return;
	d->interface->call(QStringLiteral("openFile"), path, position);
}


} // namespace Tw

#include "InterProcessCommunicatorDBus.moc"
