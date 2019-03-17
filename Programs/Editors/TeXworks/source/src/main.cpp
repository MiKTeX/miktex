/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2019  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "TWApp.h"
#include "TWVersion.h"
#include "CommandlineParser.h"

#include <QTimer>
#include <QTextCodec>
#if defined(MIKTEX)
#include "miktex/miktex-texworks.hpp"
#endif

#if defined(STATIC_QT5) && defined(Q_OS_WIN)
  #include <QtPlugin>
  Q_IMPORT_PLUGIN (QWindowsIntegrationPlugin);
#endif

#if defined(Q_OS_WIN)
BOOL CALLBACK enumThreadWindowProc(HWND hWnd, LPARAM /*lParam*/)
{
	if (IsWindowVisible(hWnd))
		SetForegroundWindow(hWnd);
	return true;
}
#endif

struct fileToOpenStruct{
	QString filename;
	int position;
};

#if defined(MIKTEX)
#  define main Main
#endif
int main(int argc, char *argv[])
{
	TWApp app(argc, argv);

	CommandlineParser clp;
	QList<fileToOpenStruct> filesToOpen;
	fileToOpenStruct fileToOpen = {QString(), -1};
	
	clp.registerSwitch(QString::fromLatin1("help"), TWApp::tr("Display this message"), QString::fromLatin1("?"));
	clp.registerOption(QString::fromLatin1("position"), TWApp::tr("Open the following file at the given position (line or page)"), QString::fromLatin1("p"));
	clp.registerSwitch(QString::fromLatin1("version"), TWApp::tr("Display version information"), QString::fromLatin1("v"));

	bool launchApp = true;
	if (clp.parse()) {
		int i, numArgs = 0;
		while ((i = clp.getNextArgument()) >= 0) {
			++numArgs;
			int j, pos = -1;
			if ((j = clp.getPrevOption(QString::fromLatin1("position"), i)) >= 0) {
				pos = clp.at(j).value.toInt();
				clp.at(j).processed = true;
			}
			CommandlineParser::CommandlineItem & item = clp.at(i);
			item.processed = true;

			fileToOpen.filename = item.value.toString();
			fileToOpen.position = pos;
			filesToOpen << fileToOpen;
		}
		if ((i = clp.getNextSwitch(QString::fromLatin1("version"))) >= 0) {
			if (numArgs == 0)
				launchApp = false;
			clp.at(i).processed = true;
			QTextStream strm(stdout);
			if (TWUtils::isGitInfoAvailable())
				strm << QString::fromUtf8("TeXworks %1 (%2) [r.%3, %4]\n\n").arg(QString::fromLatin1(TEXWORKS_VERSION)).arg(QString::fromLatin1(TW_BUILD_ID_STR)).arg(TWUtils::gitCommitHash()).arg(TWUtils::gitCommitDate().toLocalTime().toString(Qt::SystemLocaleShortDate));
			else
				strm << QString::fromUtf8("TeXworks %1 (%2)\n\n").arg(QString::fromLatin1(TEXWORKS_VERSION)).arg(QString::fromLatin1(TW_BUILD_ID_STR));
			strm << QString::fromUtf8("\
Copyright (C) %1  %2\n\
License GPLv2+: GNU GPL (version 2 or later) <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\n").arg(QString::fromLatin1("2007-2019"), QString::fromUtf8("Jonathan Kew, Stefan Löffler, Charlie Sharpsteen"));
			strm.flush();
		}
		if ((i = clp.getNextSwitch(QString::fromLatin1("help"))) >= 0) {
			if (numArgs == 0)
				launchApp = false;
			clp.at(i).processed = true;
			QTextStream strm(stdout);
			clp.printUsage(strm);
		}
	}

#if defined(Q_OS_WIN) // single-instance code for Windows
#define TW_MUTEX_NAME		"org.tug.texworks-" TEXWORKS_VERSION
	HANDLE hMutex = CreateMutexA(NULL, FALSE, TW_MUTEX_NAME);
	if (hMutex == NULL)
		return 0;	// failure
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		// this is a second instance: bring the original instance to the top
		for (int retry = 0; retry < 100; ++retry) {
			HWND hWnd = FindWindowExA(HWND_MESSAGE, NULL, TW_HIDDEN_WINDOW_CLASS, NULL);
			if (hWnd) {
				// pull the app's (visible) windows to the foreground
				DWORD thread = GetWindowThreadProcessId(hWnd, NULL);
				(void)EnumThreadWindows(thread, &enumThreadWindowProc, 0);
				// send each cmd-line arg as a WM_COPYDATA message to load a file
				foreach(fileToOpen, filesToOpen) {
					QFileInfo fi(fileToOpen.filename);
					if (!fi.exists())
						continue;
					QByteArray ba = fi.absoluteFilePath().toUtf8() + "\n" + QByteArray::number(fileToOpen.position);
					COPYDATASTRUCT cds;
					cds.dwData = TW_OPEN_FILE_MSG;
					cds.cbData = ba.length();
					cds.lpData = ba.data();
					SendMessageA(hWnd, WM_COPYDATA, 0, (LPARAM)&cds);
				}
				break;
			}
			// couldn't find the other instance; not ready yet?
			// sleep for 50ms and then retry
			Sleep(50);
		}
		CloseHandle(hMutex);	// close our handle to the mutex
		return 0;
	}
#endif

#ifdef QT_DBUS_LIB
	if (QDBusConnection::sessionBus().registerService(QString::fromLatin1(TW_SERVICE_NAME)) == false) {
		QDBusInterface interface(QString::fromLatin1(TW_SERVICE_NAME), QString::fromLatin1(TW_APP_PATH), QString::fromLatin1(TW_INTERFACE_NAME));
		if (interface.isValid()) {
			interface.call(QString::fromLatin1("bringToFront"));
			foreach(fileToOpen, filesToOpen) {
				QFileInfo fi(fileToOpen.filename);
				if (!fi.exists())
					continue;
				interface.call(QString::fromLatin1("openFile"), fi.absoluteFilePath(), fileToOpen.position);
			}
			return 0;
		}
		else {
			// We could not register the service, but couldn't connect to an
			// already registered one, either. This can mean that something is
			// seriously wrong, we've met some race condition, or the dbus
			// service is not running. Let's assume the best (dbus not running)
			// and continue as a multiple-instance app instead
		}
	}

	new TWAdaptor(&app);
	if (QDBusConnection::sessionBus().registerObject(QString::fromLatin1(TW_APP_PATH), &app) == false) {
		// failed to register the application object, so unregister our service
		// and continue as a multiple-instance app instead
		(void)QDBusConnection::sessionBus().unregisterService(QString::fromLatin1(TW_SERVICE_NAME));
	}
#endif // defined(QT_DBUS_LIB)

	int rval = 0;
	if (launchApp) {
		// If a document is opened during the startup of Tw, the QApplication
		// may not be properly initialized yet. Therefore, defer the opening to
		// the event loop.
		foreach (fileToOpen, filesToOpen) {
			QCoreApplication::postEvent(&app, new TWDocumentOpenEvent(fileToOpen.filename, fileToOpen.position));
		}

		QTimer::singleShot(1, &app, SLOT(launchAction()));
		rval = app.exec();
	}

#if defined(Q_OS_WIN)
	CloseHandle(hMutex);
#endif

	return rval;
}
#if defined(MIKTEX)
#undef main
int main(int argc, char* argv[])
{
  MiKTeX_TeXworks wrapper;
  return wrapper.Run(Main, argc, argv);
}
#endif
