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

#if defined(MIKTEX_WINDOWS)
#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>
#endif
#include "TWApp.h"
#include "TWUtils.h"
#include "TeXDocumentWindow.h"
#include "PDFDocumentWindow.h"
#include "PrefsDialog.h"
#include "DefaultPrefs.h"
#include "TemplateDialog.h"
#include "TWSystemCmd.h"
#include "Settings.h"
#include "document/SpellChecker.h"
#include "scripting/ScriptAPI.h"

#include "TWVersion.h"
#include "ResourcesDialog.h"
#include "TWTextCodecs.h"

#if !defined(MIKTEX)
#if defined(Q_OS_WIN)
#include "DefaultBinaryPathsWin.h"
#else
#include "DefaultBinaryPaths.h"
#endif
#endif

#include <QMessageBox>
#include <QFileDialog>
#include <QString>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QStringList>
#include <QEvent>
#include <QKeyEvent>
#include <QKeySequence>
#include <QDesktopWidget>
#include <QTextCodec>
#include <QLocale>
#include <QTranslator>
#include <QUrl>
#include <QDesktopServices>
#include <QLibraryInfo>

#if defined(Q_OS_DARWIN)
#include <CoreServices/CoreServices.h>
#endif

#if defined(Q_OS_WIN)
#include <windows.h>
#ifndef VER_SUITE_WH_SERVER /* not defined in my mingw system */
#define VER_SUITE_WH_SERVER 0x00008000
#endif
#endif

#define SETUP_FILE_NAME "texworks-setup.ini"

const int kDefaultMaxRecentFiles = 20;

TWApp *TWApp::theAppInstance = nullptr;

const QEvent::Type TWDocumentOpenEvent::type = static_cast<QEvent::Type>(QEvent::registerEventType());


TWApp::TWApp(int &argc, char **argv)
	: QApplication(argc, argv)
	, recentFilesLimit(kDefaultMaxRecentFiles)
	, defaultCodec(nullptr)
	, binaryPaths(nullptr)
	, defaultBinPaths(nullptr)
	, engineList(nullptr)
	, defaultEngineIndex(0)
	, scriptManager(nullptr)
{
	init();
}

TWApp::~TWApp()
{
	if (scriptManager) {
		scriptManager->saveDisabledList();
		delete scriptManager;
	}
}

void TWApp::init()
{
	customTextCodecs << new MacCentralEurRomanCodec();

	QIcon appIcon;
#if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
	// The Compiz window manager doesn't seem to support icons larger than
	// 128x128, so we add a suitable one first
	appIcon.addFile(QString::fromLatin1(":/images/images/TeXworks-128.png"));
#endif
	appIcon.addFile(QString::fromLatin1(":/images/images/TeXworks.png"));
	setWindowIcon(appIcon);
	
	setOrganizationName(QString::fromLatin1("TUG"));
	setOrganizationDomain(QString::fromLatin1("tug.org"));
	setApplicationName(QString::fromLatin1(TEXWORKS_NAME));
	
	// <Check for portable mode>
#if defined(Q_OS_DARWIN)
	QDir appDir(applicationDirPath() + QLatin1String("/../../..")); // move up to dir containing the .app package
#else
	QDir appDir(applicationDirPath());
#endif
	QDir iniPath(appDir.absolutePath());
	QDir libPath(appDir.absolutePath());
	if (appDir.exists(QString::fromLatin1(SETUP_FILE_NAME))) {
		QSettings portable(appDir.filePath(QString::fromLatin1(SETUP_FILE_NAME)), QSettings::IniFormat);
		if (portable.contains(QString::fromLatin1("inipath"))) {
			if (iniPath.cd(portable.value(QString::fromLatin1("inipath")).toString())) {
				Tw::Settings::setDefaultFormat(QSettings::IniFormat);
				Tw::Settings::setPath(QSettings::IniFormat, QSettings::UserScope, iniPath.absolutePath());
			}
		}
		if (portable.contains(QString::fromLatin1("libpath"))) {
			if (libPath.cd(portable.value(QString::fromLatin1("libpath")).toString())) {
				portableLibPath = libPath.absolutePath();
			}
		}
		if (portable.contains(QString::fromLatin1("defaultbinpaths"))) {
			defaultBinPaths = new QStringList;
			*defaultBinPaths = portable.value(QString::fromLatin1("defaultbinpaths")).toString().split(QString::fromLatin1(PATH_LIST_SEP), QString::SkipEmptyParts);
		}
	}
#if defined(MIKTEX_WINDOWS)
        QString envPath = QString::fromUtf8(getenv("TW_INIPATH"));
#else
	QString envPath = QString::fromLocal8Bit(getenv("TW_INIPATH"));
#endif
	if (!envPath.isNull() && iniPath.cd(envPath)) {
		Tw::Settings::setDefaultFormat(QSettings::IniFormat);
		Tw::Settings::setPath(QSettings::IniFormat, QSettings::UserScope, iniPath.absolutePath());
	}
#if defined(MIKTEX)
        else
        {
          auto session = MiKTeX::Core::Session::Get();
          if (!session->IsAdminMode())
          {
            if (session->IsMiKTeXPortable())
            {
              Tw::Settings::setDefaultFormat(QSettings::IniFormat);
              MiKTeX::Util::PathName path = session->GetSpecialPath(MiKTeX::Configuration::SpecialPath::UserConfigRoot);
              Tw::Settings::setPath(QSettings::IniFormat, QSettings::UserScope, QString::fromUtf8(path.GetData()));
            }
          }
        }
#endif
#if defined(MIKTEX)
        envPath = QString::fromUtf8(getenv("TW_LIBPATH"));
#else
	envPath = QString::fromLocal8Bit(getenv("TW_LIBPATH"));
#endif
	if (!envPath.isNull() && libPath.cd(envPath)) {
		portableLibPath = libPath.absolutePath();
	}
	// </Check for portable mode>

	// Required for TWUtils::getLibraryPath()
	theAppInstance = this;

	Tw::Settings settings;
	
	QString locale = settings.value(QString::fromLatin1("locale"), QLocale::system().name()).toString();
	applyTranslation(locale);

	recentFilesLimit = settings.value(QString::fromLatin1("maxRecentFiles"), kDefaultMaxRecentFiles).toInt();

	QString codecName = settings.value(QString::fromLatin1("defaultEncoding"), QString::fromLatin1("UTF-8")).toString();
	defaultCodec = QTextCodec::codecForName(codecName.toLatin1());
	if (!defaultCodec)
		defaultCodec = QTextCodec::codecForName("UTF-8");

	TWUtils::readConfig();

	scriptManager = new TWScriptManager;

#if defined(Q_OS_DARWIN)
	setQuitOnLastWindowClosed(false);
	setAttribute(Qt::AA_DontShowIconsInMenus);

	menuBar = new QMenuBar;

	menuFile = menuBar->addMenu(tr("File"));

	actionNew = new QAction(tr("New"), this);
	actionNew->setIcon(QIcon(QString::fromLatin1(":/images/tango/document-new.png")));
	menuFile->addAction(actionNew);
	connect(actionNew, SIGNAL(triggered()), this, SLOT(newFile()));

	actionNew_from_Template = new QAction(tr("New from Template..."), this);
	menuFile->addAction(actionNew_from_Template);
	connect(actionNew_from_Template, SIGNAL(triggered()), this, SLOT(newFromTemplate()));

	actionPreferences = new QAction(tr("Preferences..."), this);
	actionPreferences->setIcon(QIcon(QString::fromLatin1(":/images/tango/preferences-system.png")));
	actionPreferences->setMenuRole(QAction::PreferencesRole);
	menuFile->addAction(actionPreferences);
	connect(actionPreferences, SIGNAL(triggered()), this, SLOT(preferences()));

	actionOpen = new QAction(tr("Open..."), this);
	actionOpen->setIcon(QIcon(QString::fromLatin1(":/images/tango/document-open.png")));
	menuFile->addAction(actionOpen);
	connect(actionOpen, SIGNAL(triggered()), this, SLOT(open()));

	menuRecent = new QMenu(tr("Open Recent"));
	actionClear_Recent_Files = menuRecent->addAction(tr("Clear Recent Files"));
	actionClear_Recent_Files->setEnabled(false);
	connect(actionClear_Recent_Files, SIGNAL(triggered()), this, SLOT(clearRecentFiles()));
	updateRecentFileActions();
	menuFile->addMenu(menuRecent);

	actionQuit = new QAction(tr("Quit TeXworks"), this);
	actionQuit->setMenuRole(QAction::QuitRole);
	menuFile->addAction(actionQuit);
	connect(actionQuit, SIGNAL(triggered()), this, SLOT(quit()));

	menuHelp = menuBar->addMenu(tr("Help"));

	homePageAction = new QAction(tr("Go to TeXworks home page"), this);
	menuHelp->addAction(homePageAction);
	connect(homePageAction, SIGNAL(triggered()), this, SLOT(goToHomePage()));
	mailingListAction = new QAction(tr("Email to the mailing list"), this);
	menuHelp->addAction(mailingListAction);
	connect(mailingListAction, SIGNAL(triggered()), this, SLOT(writeToMailingList()));
	QAction* sep = new QAction(this);
	sep->setSeparator(true);
	menuHelp->addAction(sep);
	aboutAction = new QAction(tr("About " TEXWORKS_NAME "..."), this);
	aboutAction->setMenuRole(QAction::AboutRole);
	menuHelp->addAction(aboutAction);
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
	
	TWUtils::insertHelpMenuItems(menuHelp);

	connect(this, SIGNAL(updatedTranslators()), this, SLOT(changeLanguage()));
	changeLanguage();
#endif
#if defined(MIKTEX)
        QTimer* timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(UnloadFileNameDatabase()));
        timer->start(5000);
#endif
}

void TWApp::maybeQuit()
{
#if defined(Q_OS_DARWIN)
	setQuitOnLastWindowClosed(true);
#endif
	closeAllWindows();
#if defined(Q_OS_DARWIN)
	setQuitOnLastWindowClosed(false);
#endif
}

void TWApp::changeLanguage()
{
#if defined(Q_OS_DARWIN)
	menuFile->setTitle(tr("File"));
	actionNew->setText(tr("New"));
	actionNew->setShortcut(QKeySequence(tr("Ctrl+N")));
	actionNew_from_Template->setText(tr("New from Template..."));
	actionNew_from_Template->setShortcut(QKeySequence(tr("Ctrl+Shift+N")));
	actionOpen->setText(tr("Open..."));
	actionOpen->setShortcut(QKeySequence(tr("Ctrl+O")));
	actionQuit->setText(tr("Quit TeXworks"));
	actionQuit->setShortcut(QKeySequence(QKeySequence::Quit));

	menuRecent->setTitle(tr("Open Recent"));

	menuHelp->setTitle(tr("Help"));
	aboutAction->setText(tr("About " TEXWORKS_NAME "..."));
	homePageAction->setText(tr("Go to TeXworks home page"));
	mailingListAction->setText(tr("Email to the mailing list"));
	TWUtils::insertHelpMenuItems(menuHelp);
#endif
}

void TWApp::about()
{
	QString aboutText = tr("<p>%1 is a simple environment for editing, typesetting, and previewing TeX documents.</p>").arg(QString::fromLatin1(TEXWORKS_NAME));
	aboutText += QLatin1String("<small>");
	aboutText += QLatin1String("<p>&#xA9; 2007-2020  Jonathan Kew, Stefan L&#xF6;ffler, Charlie Sharpsteen");
#if defined(MIKTEX)
	aboutText += tr("<br>Version %1 (%2) [r.%3, %4]").arg(QString::fromLatin1(TEXWORKS_VERSION), QString::fromUtf8(MiKTeX::Core::Utils::GetMiKTeXBannerString().c_str()), TWUtils::gitCommitHash(), TWUtils::gitCommitDate().toLocalTime().toString(Qt::SystemLocaleShortDate));
#else
	if (TWUtils::isGitInfoAvailable())
		aboutText += tr("<br>Version %1 (%2) [r.%3, %4]").arg(QString::fromLatin1(TEXWORKS_VERSION), QString::fromLatin1(TW_BUILD_ID_STR), TWUtils::gitCommitHash(), TWUtils::gitCommitDate().toLocalTime().toString(Qt::SystemLocaleShortDate));
	else
		aboutText += tr("<br>Version %1 (%2)").arg(QString::fromLatin1(TEXWORKS_VERSION), QString::fromLatin1(TW_BUILD_ID_STR));
#endif
	aboutText += tr("<p>Distributed under the <a href=\"http://www.gnu.org/licenses/gpl-2.0.html\">GNU General Public License</a>, version 2 or (at your option) any later version.");
	aboutText += tr("<p><a href=\"http://www.qt.io/\">Qt application framework</a> v%1 by The Qt Company.").arg(QString::fromLatin1(qVersion()));
	aboutText += tr("<br><a href=\"http://poppler.freedesktop.org/\">Poppler</a> PDF rendering library by Kristian H&#xF8;gsberg, Albert Astals Cid and others.");
	aboutText += tr("<br><a href=\"http://hunspell.github.io/\">Hunspell</a> spell checker by L&#xE1;szl&#xF3; N&#xE9;meth.");
	aboutText += tr("<br>Concept and resources from <a href=\"https://pages.uoregon.edu/koch/texshop/\">TeXShop</a> by Richard Koch.");
	aboutText += tr("<br><a href=\"http://itexmac.sourceforge.net/SyncTeX.html\">SyncTeX</a> technology by J&#xE9;r&#xF4;me Laurens.");
	aboutText += tr("<br>Some icons used are from the <a href=\"http://tango.freedesktop.org/\">Tango Desktop Project</a>.");
	QString trText = tr("<p>%1 translation kindly contributed by %2.").arg(tr("[language name]"), tr("[translator's name/email]"));
	if (!trText.contains(QString::fromLatin1("[language name]")))
		aboutText += trText;	// omit this if it hasn't been translated!
	aboutText += QLatin1String("</small>");
	QMessageBox::about(nullptr, tr("About %1").arg(QString::fromLatin1(TEXWORKS_NAME)), aboutText);
}

void TWApp::openUrl(const QUrl& url)
{
	if (!QDesktopServices::openUrl(url))
		QMessageBox::warning(nullptr, QString::fromLatin1(TEXWORKS_NAME),
							 tr("Unable to access \"%1\"; perhaps your browser or mail application is not properly configured?")
							 .arg(url.toString()));
}

void TWApp::goToHomePage()
{
	openUrl(QUrl(QString::fromLatin1("http://www.tug.org/texworks/")));
}

#if defined(Q_OS_WIN)
/* based on MSDN sample code from http://msdn.microsoft.com/en-us/library/ms724429(VS.85).aspx */
typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);

#if !defined(MIKTEX)
QString TWApp::GetWindowsVersionString()
{
	OSVERSIONINFOEXA osvi;
	SYSTEM_INFO si;
	PGNSI pGNSI;
	BOOL bOsVersionInfoEx;
	QString result = QLatin1String("(unknown version)");
	
	memset(&si, 0, sizeof(SYSTEM_INFO));
	memset(&osvi, 0, sizeof(OSVERSIONINFOEXA));
	
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
	if ( !(bOsVersionInfoEx = GetVersionExA ((OSVERSIONINFOA *) &osvi)) )
		return result;
	
	// Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
	pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
	if (pGNSI)
		pGNSI(&si);
	else
		GetSystemInfo(&si);
	
	if ( VER_PLATFORM_WIN32_NT == osvi.dwPlatformId && osvi.dwMajorVersion > 4 ) {
		if ( osvi.dwMajorVersion == 10 ) {
			if ( osvi.dwMinorVersion == 0 ) {
				if ( osvi.wProductType == VER_NT_WORKSTATION )
					result = QLatin1String("10");
				else {
					if (osvi.dwBuildNumber >= 17623)
						result = QLatin1String("Server 2019");
					else
						result = QLatin1String("Server 2016");
				}
			}
		}
		else if ( osvi.dwMajorVersion == 6 ) {
			if ( osvi.dwMinorVersion == 0 ) {
				if ( osvi.wProductType == VER_NT_WORKSTATION )
					result = QLatin1String("Vista");
				else
					result = QLatin1String("Server 2008");
			}
			else if ( osvi.dwMinorVersion == 1 ) {
				if( osvi.wProductType == VER_NT_WORKSTATION )
					result = QLatin1String("7");
				else
					result = QLatin1String("Server 2008 R2");
			}
			else if ( osvi.dwMinorVersion == 2 ) {
				if( osvi.wProductType == VER_NT_WORKSTATION )
					result = QLatin1String("8");
				else
					result = QLatin1String("Server 2012");
			}
			else if ( osvi.dwMinorVersion == 3 ) {
				if( osvi.wProductType == VER_NT_WORKSTATION )
					result = QLatin1String("8.1");
				else
					result = QLatin1String("Server 2012 R2");
			}
		}
		else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 ) {
			if ( GetSystemMetrics(SM_SERVERR2) )
				result = QLatin1String("Server 2003 R2");
			else if ( osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER )
				result = QLatin1String("Storage Server 2003");
			else if ( osvi.wSuiteMask & VER_SUITE_WH_SERVER )
				result = QLatin1String("Home Server");
			else if ( osvi.wProductType == VER_NT_WORKSTATION &&
					si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
				result = QLatin1String("XP Professional x64 Edition");
			else
				result = QLatin1String("Server 2003");
		}
		else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 ) {
			result = QLatin1String("XP ");
			if ( osvi.wSuiteMask & VER_SUITE_PERSONAL )
				result += QLatin1String("Home Edition");
			else
				result += QLatin1String("Professional");
		}
		else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 ) {
			result = QLatin1String("2000 ");
			
			if ( osvi.wProductType == VER_NT_WORKSTATION ) {
				result += QLatin1String("Professional");
			}
			else {
				if ( osvi.wSuiteMask & VER_SUITE_DATACENTER )
					result += QLatin1String("Datacenter Server");
				else if ( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
					result += QLatin1String("Advanced Server");
				else
					result += QLatin1String("Server");
			}
		}
		
		if ( strlen(osvi.szCSDVersion) > 0 ) {
			result += QChar::fromLatin1(' ');
			result += QLatin1String(osvi.szCSDVersion);
		}
		
		if ( osvi.dwMajorVersion >= 6 ) {
			if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
				result += QLatin1String(", 64-bit");
			else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL )
				result += QLatin1String(", 32-bit");
		}
	}
	
	return result;
}
#endif

unsigned int TWApp::GetWindowsVersion()
{
	OSVERSIONINFOEXA osvi;
	
	memset(&osvi, 0, sizeof(OSVERSIONINFOEXA));
	
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
	if ( !GetVersionExA ((OSVERSIONINFOA *) &osvi) )
		return 0;
	
	return (osvi.dwMajorVersion << 24) | (osvi.dwMinorVersion << 16) | (osvi.wServicePackMajor << 8) | (osvi.wServicePackMinor << 0);
}
#endif

const QStringList TWApp::getBinaryPaths(QStringList& systemEnvironment)
{
#if defined(Q_OS_WIN)
#define PATH_CASE_SENSITIVE	Qt::CaseInsensitive
#else
#define PATH_CASE_SENSITIVE	Qt::CaseSensitive
#endif
	QStringList binPaths = getPrefsBinaryPaths();
	QMutableStringListIterator envIter(systemEnvironment);
	while (envIter.hasNext()) {
		QString& envVar = envIter.next();
		if (envVar.startsWith(QLatin1String("PATH="), PATH_CASE_SENSITIVE)) {
			foreach (const QString& s, envVar.mid(5).split(QString::fromLatin1(PATH_LIST_SEP), QString::SkipEmptyParts)) {
				if (!binPaths.contains(s)) {
					binPaths.append(s);
				}
			}
			envVar = envVar.left(5) + binPaths.join(QString::fromLatin1(PATH_LIST_SEP));
			break;
		}
	}
	return binPaths;
}

QString TWApp::findProgram(const QString& program, const QStringList& binPaths)
{
	QStringListIterator pathIter(binPaths);
	bool found = false;
	QFileInfo fileInfo;
#if defined(Q_OS_WIN)
	QStringList executableTypes = QStringList() << QString::fromLatin1("exe") << QString::fromLatin1("com") << QString::fromLatin1("cmd") << QString::fromLatin1("bat");
#endif
	while (pathIter.hasNext() && !found) {
		QString path = pathIter.next();
		fileInfo = QFileInfo(path, program);
		found = fileInfo.exists() && fileInfo.isExecutable();
#if defined(Q_OS_WIN)
		// try adding common executable extensions, if one was not already present
		if (!found && !executableTypes.contains(fileInfo.suffix())) {
			QStringListIterator extensions(executableTypes);
			while (extensions.hasNext() && !found) {
				fileInfo = QFileInfo(path, program + QChar::fromLatin1('.') + extensions.next());
				found = fileInfo.exists() && fileInfo.isExecutable();
			}
		}
#endif
	}
	return found ? fileInfo.absoluteFilePath() : QString();
}

void TWApp::writeToMailingList()
{
	// The strings here are deliberately NOT localizable!
	QString address(QLatin1String("texworks@tug.org"));
	QString body(QLatin1String("Thank you for taking the time to write an email to the TeXworks mailing list. Please read the instructions below carefully as following them will greatly facilitate the communication.\n\nInstructions:\n-) Please write your message in English (it's in your own best interest; otherwise, many people will not be able to understand it and therefore will not answer).\n\n-) Please type something meaningful in the subject line.\n\n-) If you are having a problem, please describe it step-by-step in detail.\n\n-) After reading, please delete these instructions (up to the \"configuration info\" below which we may need to find the source of problems).\n\n\n\n----- configuration info -----\n"));

#if defined(MIKTEX)
	body += QStringLiteral("TeXworks version : " TEXWORKS_VERSION "r.") + TWUtils::gitCommitHash() + QChar::fromLatin1('\n');
	body += QStringLiteral("MiKTeX version   : ") + QString::fromUtf8(MiKTeX::Core::Utils::GetMiKTeXBannerString().c_str()) + QChar::fromLatin1('\n');
#else
	body += QLatin1String("TeXworks version : " TEXWORKS_VERSION "r.") + TWUtils::gitCommitHash() + QLatin1String(" (" TW_BUILD_ID_STR ")\n");
#endif
#if defined(Q_OS_DARWIN)
	body += QLatin1String("Install location : ") + QDir(applicationDirPath() + QLatin1String("/../..")).absolutePath() + QChar::fromLatin1('\n');
#else
	body += QLatin1String("Install location : ") + applicationFilePath() + QChar::fromLatin1('\n');
#endif
	body += QLatin1String("Library path     : ") + TWUtils::getLibraryPath(QString()) + QChar::fromLatin1('\n');

	QStringList sysEnv(QProcess::systemEnvironment());
	const QStringList binPaths = getBinaryPaths(sysEnv);
	QString pdftex = findProgram(QString::fromLatin1("pdftex"), binPaths);
	if (pdftex.isEmpty())
		pdftex = QLatin1String("not found");
	else {
		QFileInfo info(pdftex);
		pdftex = info.canonicalFilePath();
	}
	
	body += QLatin1String("pdfTeX location  : ") + pdftex + QChar::fromLatin1('\n');
	
	body += QLatin1String("Operating system : ");
#if defined(MIKTEX)
        body += QString::fromUtf8(MiKTeX::Core::Utils::GetOSVersionString().c_str()) + QStringLiteral("\n");
#else
#if defined(Q_OS_WIN)
	body += QLatin1String("Windows ") + GetWindowsVersionString() + QChar::fromLatin1('\n');
#else
#if defined(Q_OS_DARWIN)
#define UNAME_CMDLINE "uname -v"
#else
#define UNAME_CMDLINE "uname -a"
#endif
	QString unameResult(QLatin1String("unknown"));
	TWSystemCmd unameCmd(this, true);
	unameCmd.setProcessChannelMode(QProcess::MergedChannels);
	unameCmd.start(QString::fromLatin1(UNAME_CMDLINE));
	if (unameCmd.waitForStarted(1000) && unameCmd.waitForFinished(1000))
		unameResult = unameCmd.getResult().trimmed();
#if defined(Q_OS_DARWIN)
	SInt32 major = 0, minor = 0, bugfix = 0;
	Gestalt(gestaltSystemVersionMajor, &major);
	Gestalt(gestaltSystemVersionMinor, &minor);
	Gestalt(gestaltSystemVersionBugFix, &bugfix);
	body += QString::fromLatin1("Mac OS X %1.%2.%3").arg(major).arg(minor).arg(bugfix);
	body += QLatin1String(" (") + unameResult + QLatin1String(")\n");
#else
	body += unameResult + QChar::fromLatin1('\n');
#endif
#endif
#endif

	body += QLatin1String("Qt version       : " QT_VERSION_STR " (build) / ");
	body += QLatin1String(qVersion());
	body += QLatin1String(" (runtime)\n");
	body += QLatin1String("------------------------------\n");

#if defined(Q_OS_WIN)
	body.replace(QChar::fromLatin1('\n'), QLatin1String("\r\n"));
#endif

#if defined(MIKTEX)
        openUrl(QUrl(QStringLiteral("mailto:%1?body=%2").arg(address).arg(QString(QUrl::toPercentEncoding(body)))));
#else
	openUrl(QUrl(QString::fromLatin1("mailto:%1?subject=&body=%2").arg(address, QString::fromLatin1(QUrl::toPercentEncoding(body).constData()))));
#endif
}

void TWApp::launchAction()
{
	scriptManager->runHooks(QString::fromLatin1("TeXworksLaunched"));

	if (!TeXDocumentWindow::documentList().empty() || !PDFDocumentWindow::documentList().empty())
		return;

	Tw::Settings settings;
	int launchOption = settings.value(QString::fromLatin1("launchOption"), 1).toInt();
	switch (launchOption) {
		case 1: // Blank document
			newFile();
			break;
		case 2: // New from Template
			newFromTemplate();
			break;
		case 3: // Open File
			open();
			break;
	}
#if !defined(Q_OS_DARWIN)
	// on Mac OS, it's OK to end up with no document (we still have the app menu bar)
	// but on W32 and X11 we need a window otherwise the user can't interact at all
	if (TeXDocumentWindow::documentList().empty() && PDFDocumentWindow::documentList().empty()) {
		newFile();
		if (TeXDocumentWindow::documentList().empty()) {
			// something went wrong, give up!
			(void)QMessageBox::critical(nullptr, tr("Unable to create window"),
					tr("Something is badly wrong; %1 was unable to create a document window. "
			           "The application will now quit.").arg(QString::fromLatin1(TEXWORKS_NAME)),
					QMessageBox::Close, QMessageBox::Close);
			quit();
		}
	}
#endif
}

QObject * TWApp::newFile() const
{
	TeXDocumentWindow *doc = new TeXDocumentWindow;
	doc->show();
	doc->editor()->updateLineNumberAreaWidth(0);
	doc->runHooks(QString::fromLatin1("NewFile"));
	return doc;
}

QObject * TWApp::newFromTemplate() const
{
	QString templateName = TemplateDialog::doTemplateDialog();
	if (!templateName.isEmpty()) {
		TeXDocumentWindow *doc = new TeXDocumentWindow(templateName, true);
		if (doc) {
			doc->makeUntitled();
			doc->selectWindow();
			doc->editor()->updateLineNumberAreaWidth(0);
			doc->runHooks(QString::fromLatin1("NewFromTemplate"));
			return doc;
		}
	}
	return nullptr;
}

void TWApp::openRecentFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action)
		openFile(action->data().toString());
}

QStringList TWApp::getOpenFileNames(QString selectedFilter)
{
	QFileDialog::Options options;
#if defined(Q_OS_WIN)
	if(TWApp::GetWindowsVersion() < 0x06000000) options |= QFileDialog::DontUseNativeDialog;
#endif
	Tw::Settings settings;
	QString lastOpenDir = settings.value(QString::fromLatin1("openDialogDir")).toString();
	QStringList filters = *TWUtils::filterList();
	if (!selectedFilter.isNull() && !filters.contains(selectedFilter))
		filters.prepend(selectedFilter);
	return QFileDialog::getOpenFileNames(nullptr, QString(tr("Open File")), lastOpenDir,
	                                     filters.join(QLatin1String(";;")), &selectedFilter, options);
}

QString TWApp::getOpenFileName(QString selectedFilter)
{
	QFileDialog::Options options;
#if defined(Q_OS_WIN)
	if(TWApp::GetWindowsVersion() < 0x06000000) options |= QFileDialog::DontUseNativeDialog;
#endif
	Tw::Settings settings;
	QString lastOpenDir = settings.value(QString::fromLatin1("openDialogDir")).toString();
	QStringList filters = *TWUtils::filterList();
	if (!selectedFilter.isNull() && !filters.contains(selectedFilter))
		filters.prepend(selectedFilter);
#if defined(MIKTEX_WINDOWS)
        if (lastOpenDir.isEmpty())
        {
          lastOpenDir = QString::fromUtf8(MiKTeX::Core::Utils::GetFolderPath(CSIDL_MYDOCUMENTS, CSIDL_MYDOCUMENTS, true).GetData());
        }
#endif
	return QFileDialog::getOpenFileName(nullptr, QString(tr("Open File")), lastOpenDir,
	                                    filters.join(QLatin1String(";;")), &selectedFilter, options);
}

void TWApp::open()
{
	Tw::Settings settings;
	QStringList files = getOpenFileNames();
	foreach (QString fileName, files) {
		if (!fileName.isEmpty()) {
			QFileInfo info(fileName);
			settings.setValue(QString::fromLatin1("openDialogDir"), info.canonicalPath());
			openFile(fileName);
		}
	}
}

QObject* TWApp::openFile(const QString &fileName, int pos /* = 0 */)
{
	if (TWUtils::isPDFfile(fileName)) {
		PDFDocumentWindow *doc = PDFDocumentWindow::findDocument(fileName);
		if (!doc)
			doc = new PDFDocumentWindow(fileName);
		if (doc) {
			if (pos > 0)
				doc->widget()->goToPage(pos - 1);
			doc->selectWindow();
			return doc;
		}
		return nullptr;
	}
	return TeXDocumentWindow::openDocument(fileName, true, true, pos, 0, 0);
}

void TWApp::preferences()
{
	PrefsDialog::doPrefsDialog(activeWindow());
}

void TWApp::emitHighlightLineOptionChanged()
{
	emit highlightLineOptionChanged();
}

int TWApp::maxRecentFiles() const
{
	return recentFilesLimit;
}

void TWApp::setMaxRecentFiles(int value)
{
	if (value < 1)
		value = 1;
	else if (value > 100)
		value = 100;

	if (value != recentFilesLimit) {
		recentFilesLimit = value;

		Tw::Settings settings;
		settings.setValue(QString::fromLatin1("maxRecentFiles"), value);

		updateRecentFileActions();
	}
}

void TWApp::updateRecentFileActions()
{
#if defined(Q_OS_DARWIN)
	TWUtils::updateRecentFileActions(this, recentFileActions, menuRecent, actionClear_Recent_Files);
#endif
	emit recentFileActionsChanged();
}

void TWApp::updateWindowMenus()
{
	emit windowListChanged();
}

void TWApp::stackWindows()
{
	arrangeWindows(TWUtils::stackWindowsInRect);
}

void TWApp::tileWindows()
{
	arrangeWindows(TWUtils::tileWindowsInRect);
}

void TWApp::arrangeWindows(WindowArrangementFunction func)
{
	QDesktopWidget *desktop = QApplication::desktop();
	for (int screenIndex = 0; screenIndex < desktop->numScreens(); ++screenIndex) {
		QWidgetList windows;
		foreach (TeXDocumentWindow* texDoc, TeXDocumentWindow::documentList())
			if (desktop->screenNumber(texDoc) == screenIndex)
				windows << texDoc;
		foreach (PDFDocumentWindow* pdfDoc, PDFDocumentWindow::documentList())
			if (desktop->screenNumber(pdfDoc) == screenIndex)
				windows << pdfDoc;
		if (!windows.empty())
			(*func)(windows, desktop->availableGeometry(screenIndex));
	}
}

bool TWApp::event(QEvent *event)
{
	if (event->type() == TWDocumentOpenEvent::type) {
		TWDocumentOpenEvent * e = dynamic_cast<TWDocumentOpenEvent*>(event);
		openFile(e->filename, e->pos);
		return true;
	}
	switch (event->type()) {
		case QEvent::FileOpen:
			openFile(dynamic_cast<QFileOpenEvent *>(event)->file());
			return true;
		default:
			return QApplication::event(event);
	}
}

void TWApp::setDefaultPaths()
{
	QDir appDir(applicationDirPath());
	if (!binaryPaths)
		binaryPaths = new QStringList;
	else
		binaryPaths->clear();
	if (defaultBinPaths)
		*binaryPaths = *defaultBinPaths;
#if !defined(Q_OS_DARWIN)
	// on OS X, this will be the path to {TW_APP_PACKAGE}/Contents/MacOS/
	// which doesn't make any sense as a search dir for TeX binaries
	if (!binaryPaths->contains(appDir.absolutePath()))
		binaryPaths->append(appDir.absolutePath());
#endif
#if defined(MIKTEX_WINDOWS)
        QString envPath = QString::fromUtf8(getenv("PATH"));
#else
	QString envPath = QString::fromLocal8Bit(getenv("PATH"));
#endif
	if (!envPath.isEmpty())
		foreach (const QString& s, envPath.split(QString::fromLatin1(PATH_LIST_SEP), QString::SkipEmptyParts))
		if (!binaryPaths->contains(s))
			binaryPaths->append(s);
	if (!defaultBinPaths) {
#if defined(MIKTEX)
          {
            auto session = MiKTeX::Core::Session::Get();
            MiKTeX::Util::PathName d = session->GetSpecialPath(MiKTeX::Configuration::SpecialPath::CommonInstallRoot) / MiKTeX::Util::PathName(MIKTEX_PATH_BIN_DIR);
            QString dir = QString::fromUtf8(d.GetData());
            if (!binaryPaths->contains(dir))
            {
              binaryPaths->prepend(dir);
            }
          }
#else
		foreach (const QString& s, QString::fromUtf8(DEFAULT_BIN_PATHS).split(QString::fromLatin1(PATH_LIST_SEP), QString::SkipEmptyParts)) {
			if (!binaryPaths->contains(s))
				binaryPaths->append(s);
		}
#endif
	}
	for (int i = binaryPaths->count() - 1; i >= 0; --i) {
		QDir dir(binaryPaths->at(i));
		if (!dir.exists())
			binaryPaths->removeAt(i);
	}
	if (binaryPaths->count() == 0) {
		QMessageBox::warning(nullptr, tr("No default binary directory found"),
			tr("None of the predefined directories for TeX-related programs could be found."
				"<p><small>To run any processes, you will need to set the binaries directory (or directories) "
				"for your TeX distribution using the Typesetting tab of the Preferences dialog.</small>"));
	}
}

const QStringList TWApp::getPrefsBinaryPaths()
{
	if (!binaryPaths) {
		binaryPaths = new QStringList;
		Tw::Settings settings;
		if (settings.contains(QString::fromLatin1("binaryPaths")))
			*binaryPaths = settings.value(QString::fromLatin1("binaryPaths")).toStringList();
		else
			setDefaultPaths();
	}
	return *binaryPaths;
}

void TWApp::setBinaryPaths(const QStringList& paths)
{
	if (!binaryPaths)
		binaryPaths = new QStringList;
	*binaryPaths = paths;
	Tw::Settings settings;
	settings.setValue(QString::fromLatin1("binaryPaths"), paths);
}

void TWApp::setDefaultEngineList()
{
	if (!engineList)
		engineList = new QList<Engine>;
	else
		engineList->clear();
#if defined(MIKTEX)
        *engineList
          << Engine(QStringLiteral("pdfTeX"), QStringLiteral(MIKTEX_PDFTEX_EXE), QStringList(QStringLiteral("$synctexoption")) << QStringLiteral("$fullname"), true)
          << Engine(QStringLiteral("pdfLaTeX"), QStringLiteral(MIKTEX_PDFTEX_EXE), QStringList(QStringLiteral("$synctexoption")) << QStringLiteral("-undump=pdflatex") << QStringLiteral("$fullname"), true)
          << Engine(QStringLiteral("pdfLaTeX+MakeIndex+BibTeX"), QStringLiteral(MIKTEX_TEXIFY_EXE), QStringList(QStringLiteral("--pdf")) << QStringLiteral("--synctex=1") << QStringLiteral("--clean") << QStringLiteral("$fullname"), true)
          << Engine(QStringLiteral("LuaTeX"), QStringLiteral(MIKTEX_LUATEX_EXE), QStringList(QStringLiteral("$synctexoption")) << QStringLiteral("$fullname"), true)
          << Engine(QStringLiteral("LuaLaTeX"), QStringLiteral(MIKTEX_LUAHBTEX_EXE), QStringList(QStringLiteral("$synctexoption")) << QStringLiteral("--fmt=lualatex") << QStringLiteral("$fullname"), true)
          << Engine(QStringLiteral("LuaLaTeX+MakeIndex+BibTeX"), QStringLiteral(MIKTEX_TEXIFY_EXE), QStringList(QStringLiteral("--pdf")) << QStringLiteral("--engine=luahbtex") << QStringLiteral("--synctex=1") << QStringLiteral("--clean") << QStringLiteral("$fullname"), true)
          << Engine(QStringLiteral("XeTeX"), QStringLiteral(MIKTEX_XETEX_EXE), QStringList(QStringLiteral("$synctexoption")) << QStringLiteral("$fullname"), true)
          << Engine(QStringLiteral("XeLaTeX"), QStringLiteral(MIKTEX_XETEX_EXE), QStringList(QStringLiteral("$synctexoption")) << QStringLiteral("-undump=xelatex") << QStringLiteral("$fullname"), true)
          << Engine(QStringLiteral("XeLaTeX+MakeIndex+BibTeX"), QStringLiteral(MIKTEX_TEXIFY_EXE), QStringList(QStringLiteral("--pdf")) << QStringLiteral("--engine=xetex") << QStringLiteral("--synctex=1") << QStringLiteral("--clean") << QStringLiteral("$fullname"), true)
          << Engine(QStringLiteral("ConTeXt (LuaTeX)"), QStringLiteral("context" MIKTEX_EXE_FILE_SUFFIX), QStringList(QStringLiteral("--synctex")) << QStringLiteral("$fullname"), true)
          << Engine(QStringLiteral("ConTeXt (pdfTeX)"), QStringLiteral("texexec" MIKTEX_EXE_FILE_SUFFIX), QStringList(QStringLiteral("--synctex")) << QStringLiteral("$fullname"), true)
          << Engine(QStringLiteral("ConTeXt (XeTeX)"), QStringLiteral("texexec" MIKTEX_EXE_FILE_SUFFIX), QStringList(QStringLiteral("--synctex")) << QStringLiteral("--xtx") << QStringLiteral("$fullname"), true)
          << Engine(QStringLiteral("BibTeX"), QStringLiteral(MIKTEX_BIBTEX_EXE), QStringList(QStringLiteral("$basename")), false)
          << Engine(QStringLiteral("Biber"), QStringLiteral("biber" MIKTEX_EXE_FILE_SUFFIX), QStringList(QStringLiteral("$basename")), false)
          << Engine(QStringLiteral("MakeIndex"), QStringLiteral(MIKTEX_MAKEINDEX_EXE), QStringList(QStringLiteral("$basename")), false);
        defaultEngineIndex = 2;
#else
	*engineList
//		<< Engine("LaTeXmk", "latexmk" EXE, QStringList("-e") << 
//				  "$pdflatex=q/pdflatex -synctex=1 %O %S/" << "-pdf" << "$fullname", true)
	    << Engine(QString::fromLatin1("pdfTeX"), QString::fromLatin1("pdftex" EXE), QStringList(QString::fromLatin1("$synctexoption")) << QString::fromLatin1("$fullname"), true)
	    << Engine(QString::fromLatin1("pdfLaTeX"), QString::fromLatin1("pdflatex" EXE), QStringList(QString::fromLatin1("$synctexoption")) << QString::fromLatin1("$fullname"), true)
	    << Engine(QString::fromLatin1("LuaTeX"), QString::fromLatin1("luatex" EXE), QStringList(QString::fromLatin1("$synctexoption")) << QString::fromLatin1("$fullname"), true)
	    << Engine(QString::fromLatin1("LuaLaTeX"), QString::fromLatin1("lualatex" EXE), QStringList(QString::fromLatin1("$synctexoption")) << QString::fromLatin1("$fullname"), true)
	    << Engine(QString::fromLatin1("XeTeX"), QString::fromLatin1("xetex" EXE), QStringList(QString::fromLatin1("$synctexoption")) << QString::fromLatin1("$fullname"), true)
	    << Engine(QString::fromLatin1("XeLaTeX"), QString::fromLatin1("xelatex" EXE), QStringList(QString::fromLatin1("$synctexoption")) << QString::fromLatin1("$fullname"), true)
	    << Engine(QString::fromLatin1("ConTeXt (LuaTeX)"), QString::fromLatin1("context" EXE), QStringList(QString::fromLatin1("--synctex")) << QString::fromLatin1("$fullname"), true)
	    << Engine(QString::fromLatin1("ConTeXt (pdfTeX)"), QString::fromLatin1("texexec" EXE), QStringList(QString::fromLatin1("--synctex")) << QString::fromLatin1("$fullname"), true)
	    << Engine(QString::fromLatin1("ConTeXt (XeTeX)"), QString::fromLatin1("texexec" EXE), QStringList(QString::fromLatin1("--synctex")) << QString::fromLatin1("--xtx") << QString::fromLatin1("$fullname"), true)
	    << Engine(QString::fromLatin1("BibTeX"), QString::fromLatin1("bibtex" EXE), QStringList(QString::fromLatin1("$basename")), false)
	    << Engine(QString::fromLatin1("Biber"), QString::fromLatin1("biber" EXE), QStringList(QString::fromLatin1("$basename")), false)
	    << Engine(QString::fromLatin1("MakeIndex"), QString::fromLatin1("makeindex" EXE), QStringList(QString::fromLatin1("$basename")), false);
	defaultEngineIndex = 1;
#endif
}

const QList<Engine> TWApp::getEngineList()
{
	if (!engineList) {
		engineList = new QList<Engine>;
		bool foundList = false;
		// check for old engine list in Preferences
		Tw::Settings settings;
		int count = settings.beginReadArray(QString::fromLatin1("engines"));
		if (count > 0) {
			for (int i = 0; i < count; ++i) {
				settings.setArrayIndex(i);
				Engine eng;
				eng.setName(settings.value(QString::fromLatin1("name")).toString());
				eng.setProgram(settings.value(QString::fromLatin1("program")).toString());
				eng.setArguments(settings.value(QString::fromLatin1("arguments")).toStringList());
				eng.setShowPdf(settings.value(QString::fromLatin1("showPdf")).toBool());
				engineList->append(eng);
				settings.remove(QString());
			}
			foundList = true;
			saveEngineList();
		}
		settings.endArray();
		settings.remove(QString::fromLatin1("engines"));

		if (!foundList) { // read engine list from config file
			QDir configDir(TWUtils::getLibraryPath(QString::fromLatin1("configuration")));
			QFile toolsFile(configDir.filePath(QString::fromLatin1("tools.ini")));
			if (toolsFile.exists()) {
				QSettings toolsSettings(toolsFile.fileName(), QSettings::IniFormat);
				QStringList toolNames = toolsSettings.childGroups();
				foreach (const QString& n, toolNames) {
					toolsSettings.beginGroup(n);
					Engine eng;
					eng.setName(toolsSettings.value(QString::fromLatin1("name")).toString());
					eng.setProgram(toolsSettings.value(QString::fromLatin1("program")).toString());
					eng.setArguments(toolsSettings.value(QString::fromLatin1("arguments")).toStringList());
					eng.setShowPdf(toolsSettings.value(QString::fromLatin1("showPdf")).toBool());
					engineList->append(eng);
					toolsSettings.endGroup();
				}
				foundList = true;
			}
		}

		if (!foundList)
			setDefaultEngineList();
#if defined(MIKTEX)
                if (!settings.value("defaultEngine").toString().isEmpty())
                {
                  setDefaultEngine(settings.value(QStringLiteral("defaultEngine")).toString());
                }
#else
		setDefaultEngine(settings.value(QString::fromLatin1("defaultEngine"), QString::fromUtf8(DEFAULT_ENGINE_NAME)).toString());
#endif
	}
	return *engineList;
}

void TWApp::saveEngineList()
{
	QDir configDir(TWUtils::getLibraryPath(QString::fromLatin1("configuration")));
	QFile toolsFile(configDir.filePath(QString::fromLatin1("tools.ini")));
	QSettings toolsSettings(toolsFile.fileName(), QSettings::IniFormat);
	toolsSettings.clear();
	int n = 0;
	foreach (const Engine& e, *engineList) {
		toolsSettings.beginGroup(QString::fromLatin1("%1").arg(++n, 3, 10, QChar::fromLatin1('0')));
		toolsSettings.setValue(QString::fromLatin1("name"), e.name());
		toolsSettings.setValue(QString::fromLatin1("program"), e.program());
		toolsSettings.setValue(QString::fromLatin1("arguments"), e.arguments());
		toolsSettings.setValue(QString::fromLatin1("showPdf"), e.showPdf());
		toolsSettings.endGroup();
	}
}

void TWApp::setEngineList(const QList<Engine>& engines)
{
	if (!engineList)
		engineList = new QList<Engine>;
	*engineList = engines;
	saveEngineList();
	Tw::Settings settings;
	settings.setValue(QString::fromLatin1("defaultEngine"), getDefaultEngine().name());
	emit engineListChanged();
}

const Engine TWApp::getDefaultEngine()
{
	const QList<Engine> engines = getEngineList();
	if (defaultEngineIndex < engines.count())
		return engines[defaultEngineIndex];
	defaultEngineIndex = 0;
	if (engines.empty())
		return Engine();
	return engines[0];
}

void TWApp::setDefaultEngine(const QString& name)
{
	const QList<Engine> engines = getEngineList();
	int i;
	for (i = 0; i < engines.count(); ++i) {
		if (engines[i].name() == name) {
			Tw::Settings settings;
			settings.setValue(QString::fromLatin1("defaultEngine"), name);
			break;
		}
	}
	// If the engine was not found (e.g., if it has been deleted)
	// try the DEFAULT_ENGINE_NAME instead (should not happen, unless the config
	// was edited manually (or by an updater, copy/paste'ing, etc.)
	if (i == engines.count() && name != QString::fromUtf8(DEFAULT_ENGINE_NAME)) {
		for (i = 0; i < engines.count(); ++i) {
			if (engines[i].name() == QString::fromUtf8(DEFAULT_ENGINE_NAME))
				break;
		}
	}
	// if neither the passed engine name nor DEFAULT_ENGINE_NAME was found,
	// fall back to selecting the first engine
	if (i == engines.count())
		i = 0;

	defaultEngineIndex = i;
}

const Engine TWApp::getNamedEngine(const QString& name)
{
	const QList<Engine> engines = getEngineList();
	foreach (const Engine& e, engines) {
		if (e.name().compare(name, Qt::CaseInsensitive) == 0)
			return e;
	}
	return Engine();
}

QTextCodec *TWApp::getDefaultCodec()
{
	return defaultCodec;
}

void TWApp::setDefaultCodec(QTextCodec *codec)
{
	if (!codec)
		return;

	if (codec != defaultCodec) {
		defaultCodec = codec;
		Tw::Settings settings;
		settings.setValue(QString::fromLatin1("defaultEncoding"), codec->name());
	}
}

void TWApp::activatedWindow(QWidget* theWindow)
{
	emit hideFloatersExcept(theWindow);
}

void TWApp::applyTranslation(const QString& locale)
{
	foreach (QTranslator* t, translators) {
		removeTranslator(t);
		delete t;
	}
	translators.clear();

	if (!locale.isEmpty()) {
		// According to the Qt docs, translators are searched in reverse order
		// (the last installed one is tried first). Here, we use the following
		// search order (1. is tried first):
		// 1. The user's files in <resources>/translations
		// 2. The system-wide translation
		// 3. The bundled translation
		// Note that the bundled translations are not copied to <resources>, so
		// this search order is not messed up.
		QStringList names, directories;
		names << QString::fromLatin1("qt_") + locale \
					<< QString::fromLatin1("QtPDF_") + locale \
					<< QString::fromLatin1(TEXWORKS_NAME) + QString::fromLatin1("_") + locale;
		directories << QString::fromLatin1(":/resfiles/translations") \
								<< QLibraryInfo::location(QLibraryInfo::TranslationsPath) \
		                        << TWUtils::getLibraryPath(QString::fromLatin1("translations"));

		foreach (QString name, names) {
			foreach (QString dir, directories) {
				QTranslator * t = new QTranslator(this);
				if (t->load(name, dir)) {
					installTranslator(t);
					translators.append(t);
				}
				else
					delete t;
			}
		}
	}

	emit updatedTranslators();
}

void TWApp::addToRecentFiles(const QMap<QString,QVariant>& fileProperties)
{
	Tw::Settings settings;

	QString fileName = fileProperties.value(QString::fromLatin1("path")).toString();
	if (fileName.isEmpty())
		return;
	
	QList<QVariant> fileList = settings.value(QString::fromLatin1("recentFiles")).toList();
	QList<QVariant>::iterator i = fileList.begin();
	while (i != fileList.end()) {
		QMap<QString,QVariant> h = i->toMap();
		if (h.value(QString::fromLatin1("path")).toString() == fileName)
			i = fileList.erase(i);
		else
			++i;
	}

	fileList.prepend(fileProperties);

	while (fileList.size() > maxRecentFiles())
		fileList.removeLast();

	settings.setValue(QString::fromLatin1("recentFiles"), QVariant::fromValue(fileList));

	updateRecentFileActions();
}

void TWApp::clearRecentFiles()
{
	Tw::Settings settings;
	QList<QVariant> fileList;
	settings.setValue(QString::fromLatin1("recentFiles"), QVariant::fromValue(fileList));
	updateRecentFileActions();
}

QMap<QString,QVariant> TWApp::getFileProperties(const QString& path)
{
	Tw::Settings settings;
	QList<QVariant> fileList = settings.value(QString::fromLatin1("recentFiles")).toList();
	QList<QVariant>::iterator i = fileList.begin();
	while (i != fileList.end()) {
		QMap<QString,QVariant> h = i->toMap();
		if (h.value(QString::fromLatin1("path")).toString() == path)
			return h;
		++i;
	}
	return QMap<QString,QVariant>();
}

void TWApp::openHelpFile(const QString& helpDirName)
{
	QDir helpDir(helpDirName);
	if (helpDir.exists(QString::fromLatin1("index.html")))
		openUrl(QUrl::fromLocalFile(helpDir.absoluteFilePath(QString::fromLatin1("index.html"))));
	else
		QMessageBox::warning(nullptr, QString::fromLatin1(TEXWORKS_NAME), tr("Unable to find help file."));
}

void TWApp::updateScriptsList()
{
	scriptManager->reloadScripts();

	emit scriptListChanged();
}

void TWApp::showScriptsFolder()
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(TWUtils::getLibraryPath(QString::fromLatin1("scripts"))));
}

void TWApp::bringToFront()
{
	foreach (QWidget* widget, topLevelWidgets()) {
		QMainWindow* window = qobject_cast<QMainWindow*>(widget);
		if (window) {
			window->raise();
			window->activateWindow();
		}
	}
}

QList<QVariant> TWApp::getOpenWindows() const
{
	QList<QVariant> result;
	
	foreach (QWidget *widget, QApplication::topLevelWidgets()) {
		if (qobject_cast<TWScriptableWindow*>(widget))
			result << QVariant::fromValue(qobject_cast<QObject*>(widget));
	}
	return result;
}

void TWApp::setGlobal(const QString& key, const QVariant& val)
{
	QVariant v = val;
	
	if (key.isEmpty())
		return;
	
	// For objects on the heap make sure we are notified when their lifetimes
	// end so that we can remove them from our hash accordingly
	switch (static_cast<QMetaType::Type>(val.type())) {
		case QMetaType::QObjectStar:
			connect(v.value<QObject*>(), SIGNAL(destroyed(QObject*)), this, SLOT(globalDestroyed(QObject*)));
			break;
		default: break;
	}
	m_globals[key] = v;
}

void TWApp::globalDestroyed(QObject * obj)
{
	QHash<QString, QVariant>::iterator i = m_globals.begin();
	
	while (i != m_globals.end()) {
		switch (static_cast<QMetaType::Type>(i.value().type())) {
			case QMetaType::QObjectStar:
				if (i.value().value<QObject*>() == obj)
					i = m_globals.erase(i);
				else
					++i;
				break;
			default:
				++i;
				break;
		}
	}
}

/*Q_INVOKABLE static*/
int TWApp::getVersion()
{
	return (VER_MAJOR << 16) | (VER_MINOR << 8) | VER_BUGFIX;
}

//Q_INVOKABLE
QMap<QString, QVariant> TWApp::openFileFromScript(const QString& fileName, QObject * scriptApiObj, const int pos /* = -1 */, const bool askUser /* = false */)
{
	Tw::Settings settings;
	QMap<QString, QVariant> retVal;
	QObject * doc = nullptr;
	QFileInfo fi(fileName);
	Tw::Scripting::ScriptAPI * scriptApi = qobject_cast<Tw::Scripting::ScriptAPI*>(scriptApiObj);

	retVal[QString::fromLatin1("status")] = Tw::Scripting::ScriptAPI::SystemAccess_PermissionDenied;

	// for absolute paths and full reading permissions, we don't have to care
	// about peculiarities of the script; in that case, this even succeeds
	// if no valid scriptApi is passed; otherwise, we need to investigate further
	if (fi.isRelative() || !settings.value(QString::fromLatin1("allowScriptFileReading"), kDefault_AllowScriptFileReading).toBool()) {
		if (!scriptApi)
			return retVal;
		Tw::Scripting::Script * script = qobject_cast<Tw::Scripting::Script*>(scriptApi->GetScript());
		if (!script)
			return retVal; // this should never happen
	
		// relative paths are taken to be relative to the folder containing the
		// executing script's file
		QDir scriptDir(QFileInfo(script->getFilename()).dir());
		QString path = scriptDir.absoluteFilePath(fileName);
	
		if (!scriptApi->mayReadFile(path, scriptApi->GetTarget())) {
			// Possibly ask user to override the permissions
			if (!askUser)
				return retVal;
			if (QMessageBox::warning(qobject_cast<QWidget*>(scriptApi->GetTarget()), 
				tr("Permission request"),
				tr("The script \"%1\" is trying to open the file \"%2\" without sufficient permissions. Do you want to open the file?")\
					.arg(script->getTitle(), path),
				QMessageBox::Yes | QMessageBox::No, QMessageBox::No
			) != QMessageBox::Yes)
				return retVal;
		}
	}
	doc = openFile(fileName, pos);
	retVal[QString::fromLatin1("result")] = QVariant::fromValue(doc);
	retVal[QString::fromLatin1("status")] = (doc ? Tw::Scripting::ScriptAPI::SystemAccess_OK : Tw::Scripting::ScriptAPI::SystemAccess_Failed);
	return retVal;
}

void TWApp::doResourcesDialog() const
{
	ResourcesDialog::doResourcesDialog(nullptr);
}

void TWApp::reloadSpellchecker()
{
	// save the current language and deactivate the spell checker for all open
	// TeXDocument windows
	QHash<TeXDocumentWindow*, QString> oldLangs;
	foreach (QWidget *widget, QApplication::topLevelWidgets()) {
		TeXDocumentWindow * texDoc = qobject_cast<TeXDocumentWindow*>(widget);
		if (texDoc) {
			oldLangs[texDoc] = texDoc->spellcheckLanguage();
			texDoc->setSpellcheckLanguage(QString());
		}
	}
	
	// reset dictionaries (getDictionaryList(true) automatically updates all
	// spell checker menus)
	Tw::Document::SpellChecker::clearDictionaries();
	Tw::Document::SpellChecker::getDictionaryList(true);
	
	// reenable spell checker
	for (QHash<TeXDocumentWindow*, QString>::iterator it = oldLangs.begin(); it != oldLangs.end(); ++it) {
		it.key()->setSpellcheckLanguage(it.value());
	}
}

#if defined(MIKTEX)
void TWApp::aboutMiKTeX()
{
  QIcon oldIcon = windowIcon();
  setWindowIcon(QIcon(QStringLiteral(":/MiKTeX/miktex32x32.png")));
  QString aboutText = tr("<p>MiKTeX is a modern TeX distribution.</p>");
  aboutText += tr("<p>Please visit the <a href=\"https://miktex.org/\">MiKTeX Project Page</a>.</p>");
  QMessageBox::about(nullptr, tr("About MiKTeX"), aboutText);
  setWindowIcon(oldIcon);
}

void TWApp::UnloadFileNameDatabase()
{
  auto session = MiKTeX::Core::Session::Get();
  session->UnloadFilenameDatabase(std::chrono::seconds(1));
}
#endif
