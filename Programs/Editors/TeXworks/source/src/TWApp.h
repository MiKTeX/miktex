/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2018  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef TWApp_H
#define TWApp_H

#include <QApplication>
#include <QList>
#include <QAction>
#include <QSettings>
#include <QClipboard>
#include <QVariant>
#include <QHash>

#include "TWUtils.h"
#include "Engine.h"
#include "TWScriptable.h"
#include "ConfigurableApp.h"
#include "TWScriptAPI.h"

#if defined(Q_OS_WIN)
#define PATH_LIST_SEP   ";"
#define EXE             ".exe"
#else
#define PATH_LIST_SEP   ":"
#define EXE
#endif

#ifndef TW_BUILD_ID
#define TW_BUILD_ID unknown build
#endif
#define STRINGIFY_2(s) #s
#define STRINGIFY(s) STRINGIFY_2(s)
#if defined(MIKTEX)
#undef TW_BUILD_ID
#define TW_BUILD_ID_STR MIKTEX_BANNER_STR
#else
#define TW_BUILD_ID_STR STRINGIFY(TW_BUILD_ID)
#endif
#define DEFAULT_ENGINE_NAME "pdfLaTeX"

class QString;
class QMenu;
class QMenuBar;

// general constants used by multiple document types
const int kStatusMessageDuration = 3000;
const int kNewWindowOffset = 32;

#if defined(Q_OS_WIN) // for communication with the original instance
#if !defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0500
	#define _WIN32_WINNT			0x0500	// for HWND_MESSAGE
#endif
#include <windows.h>
#define TW_HIDDEN_WINDOW_CLASS	TEXWORKS_NAME ":MessageTarget"
#define TW_OPEN_FILE_MSG		(('T' << 8) + 'W')	// just a small sanity check for the receiver
#endif

class TWApp : public ConfigurableApp
{
	Q_OBJECT

public:
	TWApp(int &argc, char **argv);
	virtual ~TWApp();

	int maxRecentFiles() const;
	void setMaxRecentFiles(int value);
	void addToRecentFiles(const QMap<QString,QVariant>& fileProperties);

	void emitHighlightLineOptionChanged();
	
	QMap<QString,QVariant> getFileProperties(const QString& path);
	
	void setBinaryPaths(const QStringList& paths);
	void setEngineList(const QList<Engine>& engines);

	const QStringList getBinaryPaths(QStringList& sysEnv);
	// runtime paths, including $PATH;
	// also modifies passed-in sysEnv to include paths from prefs
	QString findProgram(const QString& program, const QStringList& binPaths);

	const QStringList getPrefsBinaryPaths(); // only paths from prefs
	const QList<Engine> getEngineList();
	void saveEngineList();
	
	const Engine getNamedEngine(const QString& name);
	const Engine getDefaultEngine();
	void setDefaultEngine(const QString& name);

	void setDefaultPaths();
	void setDefaultEngineList();
	
	QTextCodec *getDefaultCodec();
	void setDefaultCodec(QTextCodec *codec);

	void openUrl(const QUrl& url);

	static TWApp *instance();
	
	QString getPortableLibPath() const { return portableLibPath; }

	TWScriptManager* getScriptManager() { return scriptManager; }
	
	void notifyDictionaryListChanged() const { emit dictionaryListChanged(); }

#if defined(Q_OS_WIN)
	void createMessageTarget(QWidget* aWindow);
	static QString GetWindowsVersionString();
	static unsigned int GetWindowsVersion();
#endif
	void bringToFront();

	QObject* openFile(const QString& fileName, const int pos = -1);
	Q_INVOKABLE
	QMap<QString, QVariant> openFileFromScript(const QString& fileName, QObject * scriptApiObj, const int pos = -1, const bool askUser = false);

	Q_INVOKABLE QList<QVariant> getOpenWindows() const;
	
	// return the version of Tw (0xMMNNPP)
	Q_INVOKABLE
	static int getVersion();

	Q_PROPERTY(QString clipboard READ clipboardText WRITE setClipboardText)

	Q_INVOKABLE QString clipboardText(QClipboard::Mode mode = QClipboard::Clipboard) const {
		return clipboard()->text(mode);
	}

	// USE CAREFULLY
	// i.e., never put new text into the clipboard without notifying the user
	Q_INVOKABLE void setClipboardText(const QString& str, QClipboard::Mode mode = QClipboard::Clipboard) {
		clipboard()->setText(str, mode);
	}

	Q_INVOKABLE void setGlobal(const QString& key, const QVariant& val);
	Q_INVOKABLE void unsetGlobal(const QString& key) { m_globals.remove(key); }
	Q_INVOKABLE bool hasGlobal(const QString& key) const { return m_globals.contains(key); }
	Q_INVOKABLE QVariant getGlobal(const QString& key) const { return m_globals[key]; }
	
#if defined(Q_OS_DARWIN)
private:
	// on the Mac only, we have a top-level app menu bar, including its own copy of the recent files menu
	QMenuBar *menuBar;

	QMenu *menuFile;
	QAction *actionNew;
	QAction *actionNew_from_Template;
	QAction *actionOpen;
	QAction *actionPreferences;
	QAction *actionQuit;

	QMenu *menuRecent;
	QAction *actionClear_Recent_Files;
	QList<QAction*> recentFileActions;

	QMenu *menuHelp;
	QAction *aboutAction;
	QAction *homePageAction;
	QAction *mailingListAction;
#endif

public slots:
	// called by documents when they load a file
	void updateRecentFileActions();

	// called by windows when they open/close/change name
	void updateWindowMenus();

	void reloadSpellchecker();

	// called once when the app is first launched
	void launchAction();

	void activatedWindow(QWidget* theWindow);

	void goToHomePage();
	void writeToMailingList();
	void openHelpFile(const QString& helpDirName);

	void applyTranslation(const QString& locale);
	
	void maybeQuit();

	void updateScriptsList();
	void showScriptsFolder();

	void about();
	void doResourcesDialog() const;
	QObject * newFile() const;
	void open();
	void stackWindows();
	void tileWindows();

	QString getOpenFileName(QString selectedFilter = QString());
	QStringList getOpenFileNames(QString selectedFilter = QString());
	QString getSaveFileName(const QString& defaultName);
#if defined(MIKTEX)
        void aboutMiKTeX();
        void UnloadFileNameDatabase();
#endif
	
signals:
	// emitted in response to updateRecentFileActions(); documents can listen to this if they have a recent files menu
	void recentFileActionsChanged();

	// emitted when the window list may have changed, so documents can update their window menu
	void windowListChanged();
	
	// emitted when the engine list is changed from Preferences, so docs can update their menus
	void engineListChanged();
	
	void scriptListChanged();
	
	// emitted when TWUtils::getDictionaryList reloads the dictionary list;
	// windows can connect to it to rebuild, e.g., a spellchecking menu
	void dictionaryListChanged() const;
	
	void syncPdf(const QString& sourceFile, int lineNo, int col, bool activatePreview);

	void hideFloatersExcept(QWidget* theWindow);

	void updatedTranslators();

	void highlightLineOptionChanged();

private slots:
	void clearRecentFiles();
	QObject * newFromTemplate() const;
	void openRecentFile();
	void preferences();

	void changeLanguage();

	void globalDestroyed(QObject * obj);

protected:
	virtual bool event(QEvent *);

private:
	void init();
	
	void arrangeWindows(TWUtils::WindowArrangementFunction func);

	int recentFilesLimit;

	QTextCodec *defaultCodec;

	QStringList *binaryPaths;
	QStringList *defaultBinPaths;
	QList<Engine> *engineList;
	int defaultEngineIndex;
	QString portableLibPath;

	QList<QTranslator*> translators;
	
	TWScriptManager *scriptManager;

	QHash<QString, QVariant> m_globals;
	QList<QTextCodec*> customTextCodecs;
	
#if defined(Q_OS_WIN)
	HWND messageTargetWindow;
#endif

	static TWApp *theAppInstance;
};

inline TWApp *TWApp::instance()
{
	return theAppInstance;
}


class TWDocumentOpenEvent : public QEvent
{
public:
	static const QEvent::Type type;
	TWDocumentOpenEvent(const QString & filename, const int pos = 0) : QEvent(type), filename(filename), pos(pos) { }
	
	QString filename;
	int pos;
};


#ifdef QT_DBUS_LIB
#include <QtDBus>

#define TW_SERVICE_NAME 	"org.tug.texworks.application"
#define TW_APP_PATH		"/org/tug/texworks/application"
#define TW_INTERFACE_NAME	"org.tug.texworks.application"

class TWAdaptor: public QDBusAbstractAdaptor
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.tug.texworks.application") // using the #define here fails :(

private:
	TWApp *app;

public:
	TWAdaptor(TWApp *application)
		: QDBusAbstractAdaptor(application), app(application)
		{ }
	
public slots:
	Q_NOREPLY void openFile(const QString& fileName, const int position = -1)
		{ app->openFile(fileName, position); }
	Q_NOREPLY void bringToFront()
		{ app->bringToFront(); }
};
#endif	// defined(QT_DBUS_LIB)

#endif	// TWApp_H

