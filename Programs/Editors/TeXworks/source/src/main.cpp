/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2020  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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
#include "TWApp.h"
#include "utils/CommandlineParser.h"
#include "utils/VersionInfo.h"

#include <QFileInfo>
#include <QTextCodec>
#include <QTimer>
#if defined(MIKTEX)
#include <miktex/Core/Utils>
#include "miktex/miktex-texworks.hpp"
#endif

#if defined(STATIC_QT5) && defined(Q_OS_WIN)
  #include <QtPlugin>
  Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#	if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#	endif
#endif
	TWApp app(argc, argv);
	Tw::InterProcessCommunicator IPC;

	Tw::Utils::CommandlineParser clp;
	QList<fileToOpenStruct> filesToOpen;
	fileToOpenStruct fileToOpen = {QString(), -1};

	clp.registerSwitch(QString::fromLatin1("help"), TWApp::tr("Display this message"), QString::fromLatin1("?"));
	clp.registerOption(QString::fromLatin1("position"), TWApp::tr("Open the following file at the given position (line or page)"), QString::fromLatin1("p"));
	clp.registerSwitch(QString::fromLatin1("version"), TWApp::tr("Display version information"), QString::fromLatin1("v"));

	bool launchApp = true;
	if (clp.parse()) {
		int i{-1}, numArgs{0};
		while ((i = clp.getNextArgument()) >= 0) {
			++numArgs;
			int j = clp.getPrevOption(QString::fromLatin1("position"), i);
			int pos = -1;
			if (j >= 0) {
				pos = clp.at(j).value.toInt();
				clp.at(j).processed = true;
			}
			Tw::Utils::CommandlineParser::CommandlineItem & item = clp.at(i);
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
#if defined(MIKTEX)
			QString::fromUtf8("TeXworks %1 (%2) [r.%3, %4]\n\n").arg(Tw::Utils::VersionInfo::versionString(), QString::fromUtf8(MiKTeX::Core::Utils::GetMiKTeXBannerString().c_str()), Tw::Utils::VersionInfo::gitCommitHash(), Tw::Utils::VersionInfo::gitCommitHash(), QLocale::system().toString(Tw::Utils::VersionInfo::gitCommitDate().toLocalTime(), QLocale::ShortFormat));
#else
			if (Tw::Utils::VersionInfo::isGitInfoAvailable())
				strm << QString::fromUtf8("TeXworks %1 (%2) [r.%3, %4]\n\n").arg(Tw::Utils::VersionInfo::versionString(), Tw::Utils::VersionInfo::buildIdString(), Tw::Utils::VersionInfo::gitCommitHash(), QLocale::system().toString(Tw::Utils::VersionInfo::gitCommitDate().toLocalTime(), QLocale::ShortFormat));
			else
				strm << QString::fromUtf8("TeXworks %1 (%2)\n\n").arg(Tw::Utils::VersionInfo::versionString(), Tw::Utils::VersionInfo::buildIdString());
#endif
			strm << QString::fromUtf8("\
Copyright (C) %1  %2\n\
License GPLv2+: GNU GPL (version 2 or later) <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\n").arg(QString::fromLatin1("2007-2022"), QString::fromUtf8("Jonathan Kew, Stefan Löffler, Charlie Sharpsteen"));
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

	if (IPC.isFirstInstance()) {
		QObject::connect(&IPC, &Tw::InterProcessCommunicator::receivedBringToFront, &app, &TWApp::bringToFront);
		QObject::connect(&IPC, &Tw::InterProcessCommunicator::receivedOpenFile, &app, &TWApp::openFile);
	}
	else {
		IPC.sendBringToFront();
		foreach(fileToOpen, filesToOpen) {
			QFileInfo fi(fileToOpen.filename);
			if (!fi.exists())
				continue;
			IPC.sendOpenFile(fi.absoluteFilePath(), fileToOpen.position);
		}
		return 0;
	}

	int rval = 0;
	if (launchApp) {
		// If a document is opened during the startup of Tw, the QApplication
		// may not be properly initialized yet. Therefore, defer the opening to
		// the event loop.
		foreach (fileToOpen, filesToOpen) {
			QCoreApplication::postEvent(&app, new TWDocumentOpenEvent(fileToOpen.filename, fileToOpen.position));
		}

		QTimer::singleShot(1, &app, SLOT(launchAction()));
		rval = TWApp::exec();
	}
	return rval;
}
#if defined(MIKTEX)
#undef main
int main(int argc, char* argv[])
{
  MiKTeX::TeXworks::Wrapper wrapper;
  return wrapper.Run(Main, argc, argv);
}
#endif
