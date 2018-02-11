/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2016  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "TWUtils.h"
#include "TWApp.h"
#include "TeXDocument.h"
#include "PDFDocument.h"
#include "TWVersion.h"
#include "GitRev.h"

#include <QFileDialog>
#include <QString>
#include <QMenu>
#include <QAction>
#include <QStringList>
#include <QEvent>
#include <QKeyEvent>
#include <QDesktopWidget>
#include <QCompleter>
#include <QTextCodec>
#include <QFile>
#include <QDirIterator>
#include <QSignalMapper>
#include <QCryptographicHash>
#include <QTextStream>
#include <QDateTime>

#include <hunspell.h>

#pragma mark === TWUtils ===

#if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
// compile-time default paths - customize by defining in the .pro file
#ifndef TW_DICPATH
#define TW_DICPATH "/usr/share/myspell/dicts"
#endif
#ifndef TW_HELPPATH
#define TW_HELPPATH "/usr/local/share/texworks-help"
#endif
#endif

bool TWUtils::isPDFfile(const QString& fileName)
{
	QFile theFile(fileName);
	if (theFile.open(QIODevice::ReadOnly)) {
		QByteArray ba = theFile.peek(8);
		if (ba.startsWith("%PDF-1."))
			return true;
	}
	return false;
}

bool TWUtils::isImageFile(const QString& fileName)
{
	QImage	image(fileName);
	return !image.isNull();
}

bool TWUtils::isPostscriptFile(const QString& fileName)
{
	QFile theFile(fileName);
	if (theFile.open(QIODevice::ReadOnly)) {
		QByteArray ba = theFile.peek(4);
		if (ba.startsWith("%!PS"))
			return true;
	}
	return false;
}

const QString TWUtils::getLibraryPath(const QString& subdir, const bool updateOnDisk /* = true */)
{
	QString libRootPath, libPath;
	
	libRootPath = TWApp::instance()->getPortableLibPath();
	if (libRootPath.isEmpty()) {
#if defined(Q_OS_DARWIN)
		libRootPath = QDir::homePath() + "/Library/" + TEXWORKS_NAME + "/";
#elif defined(Q_OS_UNIX) // && !defined(Q_OS_DARWIN)
		if (subdir == "dictionaries") {
			libPath = TW_DICPATH;
			QString dicPath = QString::fromLocal8Bit(getenv("TW_DICPATH"));
			if (!dicPath.isEmpty())
				libPath = dicPath;
			return libPath; // don't try to create/update the system dicts directory
		}
		else
			libRootPath = QDir::homePath() + "/." + TEXWORKS_NAME + "/";
#else // defined(Q_OS_WIN)
#if defined(MIKTEX)
		{
                  std::shared_ptr<MiKTeX::Core::Session> pSession = MiKTeX::Core::Session::Get();
		  MiKTeX::Core::PathName dir;
		  dir = pSession->GetSpecialPath(MiKTeX::Core::SpecialPath::UserDataRoot);
		  dir /= TEXWORKS_NAME;
		  std::string subdir (std::to_string(VER_MAJOR));
		  subdir += ".";
		  subdir += std::to_string(VER_MINOR);
		  dir /= subdir;
		  libRootPath = dir.GetData();
                  if (!pSession->UnloadFilenameDatabase())
                  {
                    //TODO: log
                  }
		}
#else
		libRootPath = QDir::homePath() + "/" + TEXWORKS_NAME + "/";
#endif
#endif
	}
	libPath = QDir(libRootPath).absolutePath() + QDir::separator() + subdir;

	if(updateOnDisk)
		updateLibraryResources(QDir(":/resfiles"), libRootPath, subdir);
	return libPath;
}

/*static*/
void TWUtils::updateLibraryResources(const QDir& srcRootDir, const QDir& destRootDir, const QString& subdir)
{
	QDir srcDir(srcRootDir);
	QDir destDir(destRootDir.absolutePath() + QDir::separator() + subdir);
	
	// sanity check
	if (!srcDir.cd(subdir))
		return;
	
	// make sure the library folder exists - even if the user deleted it;
	// otherwise other parts of the program might fail
	if (!destDir.exists())
		QDir::root().mkpath(destDir.absolutePath());
	
	if (subdir == "translations") // don't copy the built-in translations
		return;
	
	FileVersionDatabase fvdb = FileVersionDatabase::load(destRootDir.absoluteFilePath("TwFileVersions.db"));
	
	QDirIterator iter(srcDir, QDirIterator::Subdirectories);
	while (iter.hasNext()) {
		(void)iter.next();
		// Skip directories (they get created on-the-fly if required for copying files)
		if (iter.fileInfo().isDir())
			continue;

		QString srcPath = iter.fileInfo().filePath();
		QString path = srcRootDir.relativeFilePath(srcPath);
		QString destPath = destRootDir.filePath(path);

		// Check if the file is in the database
		if (fvdb.hasFileRecord(destPath)) {
			FileVersionDatabase::Record rec = fvdb.getFileRecord(destPath);
			// If the file no longer exists on the disk, the user has deleted it
			// Hence we won't recreate it, but we keep the database record to
			// remember that this file was deleted by the user
			if (!QFileInfo(destPath).exists())
				continue;
			
			QByteArray srcHash = FileVersionDatabase::hashForFile(srcPath);
			QByteArray destHash = FileVersionDatabase::hashForFile(destPath);
			// If the file was modified, don't do anything, either
			if (destHash != rec.hash) {
				// The only exception is if the file on the disk matches the
				// new file we would have installed. In this case, we reassume
				// ownership of it. (This is the case if the user deleted the
				// file, but later wants to resurrect it by downloading the
				// latest version from the internet)
				if (destHash != srcHash)
					continue;
				fvdb.addFileRecord(destPath, srcHash, gitCommitHash());
			}
			else {
				// The file matches the record in the database; update it
				// (copying is only necessary if the contents has changed)
				if (srcHash == destHash)
					fvdb.addFileRecord(destPath, srcHash, gitCommitHash());
				else {
					// we have to remove the file first as QFile::copy doesn't
					// overwrite existing files
					QFile::remove(destPath);
					if(QFile::copy(srcPath, destPath))
						fvdb.addFileRecord(destPath, srcHash, gitCommitHash());
				}
			}
		}
		else {
			QByteArray srcHash = FileVersionDatabase::hashForFile(srcPath);
			// If the file is not in the database, we add it - unless a file
			// with the name already exists
			if (!QFileInfo(destPath).exists()) {
				// We have to make sure the directory exists - otherwise copying
				// might fail
				destRootDir.mkpath(QFileInfo(destPath).path());
				QFile(srcPath).copy(destPath);
				fvdb.addFileRecord(destPath, srcHash, gitCommitHash());
			}
			else {
				// If a file with that name already exists, we don't replace it
				// If it happens to be identical with the version we would install
				// we do take ownership, however, and register it in the
				// database so that future updates are applied
				QByteArray destHash = FileVersionDatabase::hashForFile(destPath);
				if (srcHash == destHash)
					fvdb.addFileRecord(destPath, destHash, gitCommitHash());
			}
		}
	}

	// Now, remove all files that are unmodified on disk and were
	// removed upstream
	QMutableListIterator<FileVersionDatabase::Record> recIt(fvdb.getFileRecords());
	while (recIt.hasNext()) {
		const FileVersionDatabase::Record & rec = recIt.next();

		QString destPath = rec.filePath.filePath();
		QString path = destRootDir.relativeFilePath(destPath);
		QString srcPath = srcRootDir.filePath(path);
		
		// If the source file still exists there is nothing to do here
		if (QFileInfo(srcPath).exists())
			continue;
		
		// If the source file no longer exists but the file on disk is up to
		// date, remove it
		if (rec.filePath.exists() && FileVersionDatabase::hashForFile(destPath) == rec.hash) {
			QFile(destPath).remove();
			recIt.remove();
		}
	}

	// Finally, save the updated database
	fvdb.save(destRootDir.absoluteFilePath("TwFileVersions.db"));
}

static int
insertItemIfPresent(QFileInfo& fi, QMenu* helpMenu, QAction* before, QSignalMapper* mapper, QString title)
{
	QFileInfo indexFile(fi.absoluteFilePath(), "index.html");
	if (indexFile.exists()) {
		QFileInfo titlefileInfo(fi.absoluteFilePath(), "tw-help-title.txt");
		if (titlefileInfo.exists() && titlefileInfo.isReadable()) {
			QFile titleFile(titlefileInfo.absoluteFilePath());
			titleFile.open(QIODevice::ReadOnly | QIODevice::Text);
			QTextStream titleStream(&titleFile);
			titleStream.setCodec("UTF-8");
			title = titleStream.readLine();
		}
		QAction* action = new QAction(title, helpMenu);
		mapper->setMapping(action, fi.canonicalFilePath());
		action->connect(action, SIGNAL(triggered()), mapper, SLOT(map()));
		helpMenu->insertAction(before, action);
		return 1;
	}
	return 0;
}

void TWUtils::insertHelpMenuItems(QMenu* helpMenu)
{
	QSignalMapper* mapper = new QSignalMapper(helpMenu);
	mapper->connect(mapper, SIGNAL(mapped(const QString&)), TWApp::instance(), SLOT(openHelpFile(const QString&)));

	QAction* before = NULL;
	int i, firstSeparator = 0;
	QList<QAction*> actions = helpMenu->actions();
	for (i = 0; i < actions.count(); ++i) {
		if (actions[i]->isSeparator() && !firstSeparator)
			firstSeparator = i;
		if (actions[i]->menuRole() == QAction::AboutRole) {
			before = actions[i];
			break;
		}
	}
	while (--i > firstSeparator) {
		helpMenu->removeAction(actions[i]);
		delete actions[i];
	}

#if defined(Q_OS_DARWIN)
	QDir helpDir(QCoreApplication::applicationDirPath() + "/../texworks-help");
#else
#if defined(MIKTEX)
	QDir helpDir;
        std::shared_ptr<MiKTeX::Core::Session> pSession = MiKTeX::Core::Session::Get();
	MiKTeX::Core::PathName path = pSession->GetSpecialPath(MiKTeX::Core::SpecialPath::UserInstallRoot);
	path /= "doc/texworks/help";
	if (MiKTeX::Core::Directory::Exists(path))
	{
	  helpDir = path.GetData();
	}
	else
	{
	  path = pSession->GetSpecialPath(MiKTeX::Core::SpecialPath::CommonInstallRoot);
	  path /= "doc/texworks/help";
	  if (MiKTeX::Core::Directory::Exists(path))
	  {
	    helpDir = path.GetData();
	  }
	  else
	  {
	    helpDir = QCoreApplication::applicationDirPath() + "/texworks-help";
	  }
	}
        if (!pSession->UnloadFilenameDatabase())
        {
          //TODO: log
        }
#else
	QDir helpDir(QCoreApplication::applicationDirPath() + "/texworks-help");
#endif
#if defined(Q_OS_UNIX) // && !defined(Q_OS_DARWIN)
	if (!helpDir.exists())
		helpDir.cd(TW_HELPPATH);
#endif
#endif
#if defined(MIKTEX_WINDOWS)
	QString helpPath = QString::fromUtf8(getenv("TW_HELPPATH"));
#else
	QString helpPath = QString::fromLocal8Bit(getenv("TW_HELPPATH"));
#endif
	if (!helpPath.isEmpty())
		helpDir.cd(QString(helpPath));

	QSETTINGS_OBJECT(settings);
	QString loc = settings.value("locale").toString();
	if (loc.isEmpty())
		loc = QLocale::system().name();
	
	QDirIterator iter(helpDir);
	int inserted = 0;
	while (iter.hasNext()) {
		(void)iter.next();
		if (!iter.fileInfo().isDir())
			continue;
		QString name(iter.fileInfo().fileName());
		if (name == "." || name == "..")
			continue;
		QDir subDir(iter.filePath());
		// try for localized content first
		QFileInfo fi(subDir, loc);
		if (fi.exists() && fi.isDir() && fi.isReadable()) {
			inserted += insertItemIfPresent(fi, helpMenu, before, mapper, name);
			continue;
		}
		fi.setFile(subDir.absolutePath() + "/" + loc.left(2));
		if (fi.exists() && fi.isDir() && fi.isReadable()) {
			inserted += insertItemIfPresent(fi, helpMenu, before, mapper, name);
			continue;
		}
		fi.setFile(subDir.absolutePath() + "/en");
		if (fi.exists() && fi.isDir() && fi.isReadable()) {
			inserted += insertItemIfPresent(fi, helpMenu, before, mapper, name);
			continue;
		}
		fi.setFile(subDir.absolutePath());
		inserted += insertItemIfPresent(fi, helpMenu, before, mapper, name);
	}

	if (inserted > 0) {
		QAction* sep = new QAction(helpMenu);
		sep->setSeparator(true);
		helpMenu->insertAction(before, sep);
	}
}

QList<QTextCodec*> *TWUtils::codecList = NULL;

QList<QTextCodec*> *TWUtils::findCodecs()
{
	if (codecList != NULL)
		return codecList;

	codecList = new QList<QTextCodec*>;
	QMap<QString, QTextCodec*> codecMap;
	QRegExp iso8859RegExp("ISO[- ]8859-([0-9]+).*");
	foreach (int mib, QTextCodec::availableMibs()) {
		QTextCodec *codec = QTextCodec::codecForMib(mib);
		QString sortKey = codec->name().toUpper();
		int rank;
		if (sortKey.startsWith("UTF-8"))
			rank = 1;
		else if (sortKey.startsWith("UTF-16"))
			rank = 2;
		else if (iso8859RegExp.exactMatch(sortKey)) {
			if (iso8859RegExp.cap(1).size() == 1)
				rank = 3;
			else
				rank = 4;
		}
		else
			rank = 5;
		sortKey.prepend(QChar('0' + rank));
		codecMap.insert(sortKey, codec);
	}
	*codecList = codecMap.values();
	return codecList;
}

QStringList* TWUtils::translationList = NULL;

QStringList* TWUtils::getTranslationList()
{
	if (translationList != NULL)
		return translationList;

	translationList = new QStringList;
	
	QDir transDir(":/resfiles/translations");
	foreach (QFileInfo qmFileInfo, transDir.entryInfoList(QStringList(TEXWORKS_NAME "_*.qm"),
														  QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase)) {
		QString locName = qmFileInfo.completeBaseName();
		locName.remove(TEXWORKS_NAME "_");
		*translationList << locName;
	}
	
	transDir = QDir(TWUtils::getLibraryPath("translations"));
	foreach (QFileInfo qmFileInfo, transDir.entryInfoList(QStringList(TEXWORKS_NAME "_*.qm"),
				QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase)) {
		QString locName = qmFileInfo.completeBaseName();
		locName.remove(TEXWORKS_NAME "_");
		if (!translationList->contains(locName, Qt::CaseInsensitive))
			*translationList << locName;
	}
	
	// English is always available, and it has to be the first item
	translationList->removeAll("en");
	translationList->prepend("en");
	
	return translationList;
}

QHash<QString, QString>* TWUtils::dictionaryList = NULL;

QHash<QString, QString>* TWUtils::getDictionaryList(const bool forceReload /* = false */)
{
	if (dictionaryList != NULL) {
		if (!forceReload)
			return dictionaryList;
		delete dictionaryList;
	}

	dictionaryList = new QHash<QString, QString>();
	QDir dicDir(TWUtils::getLibraryPath("dictionaries"));
	foreach (QFileInfo dicFileInfo, dicDir.entryInfoList(QStringList("*.dic"),
				QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase)) {
		QFileInfo affFileInfo(dicFileInfo.dir(), dicFileInfo.completeBaseName() + ".aff");
		if (affFileInfo.isReadable())
			dictionaryList->insertMulti(dicFileInfo.canonicalFilePath(), dicFileInfo.completeBaseName());
	}
#if defined(MIKTEX)
        std::shared_ptr<MiKTeX::Core::Session> pSession = MiKTeX::Core::Session::Get();
	for (unsigned r = 0; r < pSession->GetNumberOfTEXMFRoots(); ++ r)
	{
	  MiKTeX::Core::PathName dicPath = pSession->GetRootDirectoryPath(r);
	  dicPath /= MIKTEX_PATH_HUNSPELL_DICT_DIR;
	  QDir dicDir (dicPath.GetData());
	  foreach (QFileInfo affFileInfo, dicDir.entryInfoList(QStringList("*.aff"),
	    QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase))
	  {
	    QFileInfo dicFileInfo (affFileInfo.dir(), affFileInfo.completeBaseName() + ".dic");
	    QString lang = dicFileInfo.completeBaseName();
	    if (dicFileInfo.isReadable() && ! dictionaryList->contains(lang))
	    {
	      dictionaryList->insertMulti(affFileInfo.canonicalFilePath(), affFileInfo.completeBaseName());
	    }
	  }
	}
        if (!pSession->UnloadFilenameDatabase())
        {
          //TODO: log
        }
#endif
	
	TWApp::instance()->notifyDictionaryListChanged();
	return dictionaryList;
}

QHash<const QString,Hunhandle*> *TWUtils::dictionaries = NULL;

Hunhandle* TWUtils::getDictionary(const QString& language)
{
	if (language.isEmpty())
		return NULL;
	
	if (dictionaries == NULL)
		dictionaries = new QHash<const QString,Hunhandle*>;
	
	if (dictionaries->contains(language))
		return dictionaries->value(language);
	
	Hunhandle *h = NULL;
	const QString dictPath = getLibraryPath("dictionaries");
	QFileInfo affFile(dictPath + "/" + language + ".aff");
	QFileInfo dicFile(dictPath + "/" + language + ".dic");
	if (affFile.isReadable() && dicFile.isReadable()) {
#if defined(MIKTEX_WINDOWS)
	  h = Hunspell_create(affFile.canonicalFilePath().toUtf8().data(),
	    dicFile.canonicalFilePath().toUtf8().data());
#else
		h = Hunspell_create(affFile.canonicalFilePath().toLocal8Bit().data(),
							dicFile.canonicalFilePath().toLocal8Bit().data());
#endif
		(*dictionaries)[language] = h;
	}
#if defined(MIKTEX)
        std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();
        for (unsigned r = 0; r < session->GetNumberOfTEXMFRoots(); ++r)
        {
          MiKTeX::Core::PathName dicPath = session->GetRootDirectoryPath(r);
          dicPath /= MIKTEX_PATH_HUNSPELL_DICT_DIR;
          const QString dictPath = QString::fromStdWString(dicPath.ToWideCharString());
          QFileInfo affFile(dictPath + "/" + language + ".aff");
          QFileInfo dicFile(dictPath + "/" + language + ".dic");
          if (affFile.isReadable() && dicFile.isReadable())
          {
            h = Hunspell_create(affFile.canonicalFilePath().toUtf8().data(),
              dicFile.canonicalFilePath().toUtf8().data());
            (*dictionaries)[language] = h;
            break;
          }
        }
        if (!session->UnloadFilenameDatabase())
        {
          //TODO: log
        }
#endif
	return h;
}

QString TWUtils::getLanguageForDictionary(const Hunhandle * pHunspell)
{
	if (!pHunspell || !dictionaries)
		return QString();
	
	for (QHash<const QString,Hunhandle*>::const_iterator it = dictionaries->begin(); it != dictionaries->end(); ++it) {
		if (it.value() == pHunspell)
			return it.key();
	}
	return QString();
}

void TWUtils::clearDictionaries()
{
	if (!dictionaries)
		return;
	
	for (QHash<const QString,Hunhandle*>::iterator it = dictionaries->begin(); it != dictionaries->end(); ++it) {
		if (it.value())
			Hunspell_destroy(it.value());
	}
	delete dictionaries;
	dictionaries = NULL;
}

QStringList* TWUtils::filters;
QStringList* TWUtils::filterList()
{
	return filters;
}

void TWUtils::setDefaultFilters()
{
	*filters << QObject::tr("TeX documents (*.tex)");
	*filters << QObject::tr("LaTeX documents (*.ltx)");
	*filters << QObject::tr("Log files (*.log *.blg)");
	*filters << QObject::tr("BibTeX databases (*.bib)");
	*filters << QObject::tr("Style files (*.sty)");
	*filters << QObject::tr("Class files (*.cls)");
	*filters << QObject::tr("Documented macros (*.dtx)");
	*filters << QObject::tr("Auxiliary files (*.aux *.toc *.lot *.lof *.nav *.out *.snm *.ind *.idx *.bbl *.brf)");
	*filters << QObject::tr("Text files (*.txt)");
	*filters << QObject::tr("PDF documents (*.pdf)");
	*filters << QObject::tr("All files") + " (*)"; // this must not be "*.*", which causes an extension ".*" to be added on some systems
}

/*static*/
QString TWUtils::chooseDefaultFilter(const QString & filename, const QStringList & filters)
{
	QString extension = QFileInfo(filename).completeSuffix();

	if (extension.isEmpty())
		return filters[0];
	
	QRegExp re("\\*\\." + QRegExp::escape(extension));
	foreach (QString filter, filters) {
		// return filter if it corresponds to the given extension
		// note that the extension must be the first one in the list to match;
		// otherwise, the file dialog would replace the actual extension by the
		// first one in the list, thereby altering it without cause
		if (filter.contains(QString("(*.%1").arg(extension)))
			return filter;
	}
	// if no filter matched, return the last one (which should be "All files")
	return filters.last();
}

QString TWUtils::strippedName(const QString &fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}

void TWUtils::updateRecentFileActions(QObject *parent, QList<QAction*> &actions, QMenu *menu, QAction * clearAction) /* static */
{
	QSETTINGS_OBJECT(settings);
	QStringList fileList;
	if (settings.contains("recentFiles")) {
		QList<QVariant> files = settings.value("recentFiles").toList();
		foreach (const QVariant& v, files) {
			QMap<QString,QVariant> map = v.toMap();
			if (map.contains("path"))
				fileList.append(map.value("path").toString());
		}
	}
	else {
		// check for an old "recentFilesList" entry, and migrate it
		if (settings.contains("recentFileList")) {
			fileList = settings.value("recentFileList").toStringList();
			QList<QVariant> files;
			foreach (const QString& path, fileList) {
				QMap<QString,QVariant> map;
				map.insert("path", path);
				files.append(QVariant(map));
			}
			settings.remove("recentFileList");
			settings.setValue("recentFiles", files);
		}
	}
	
	int numRecentFiles = fileList.size();
	
	foreach(QAction * sep, menu->actions()) {
		if (sep->isSeparator())
			delete sep;
	}

	while (actions.size() < numRecentFiles) {
		QAction *act = new QAction(parent);
		act->setVisible(false);
		QObject::connect(act, SIGNAL(triggered()), qApp, SLOT(openRecentFile()));
		actions.append(act);
		menu->insertAction(clearAction, act);
	}

	while (actions.size() > numRecentFiles) {
		QAction *act = actions.takeLast();
		delete act;
	}

	for (int i = 0; i < numRecentFiles; ++i) {
		QString path = fileList[i];
		QString text = TWUtils::strippedName(path);
		actions[i]->setText(text);
		actions[i]->setData(path);
		actions[i]->setVisible(true);
	}
	
	if (numRecentFiles > 0)
		menu->insertSeparator(clearAction);
	if (clearAction)
		clearAction->setEnabled(numRecentFiles > 0);
}

void TWUtils::updateWindowMenu(QWidget *window, QMenu *menu) /* static */
{
	// shorten the menu by removing everything from the first "selectWindow" action onwards
	QList<QAction*> actions = menu->actions();
	for (QList<QAction*>::iterator i = actions.begin(); i != actions.end(); ++i) {
		SelWinAction *selWin = qobject_cast<SelWinAction*>(*i);
		if (selWin)
			menu->removeAction(*i);
	}
	while (!menu->actions().isEmpty() && menu->actions().last()->isSeparator())
		menu->removeAction(menu->actions().last());
	
	// append an item for each TeXDocument
	bool first = true;
	foreach (TeXDocument *texDoc, TeXDocument::documentList()) {
		if (first && !menu->actions().isEmpty())
			menu->addSeparator();
		first = false;
		QString label = texDoc->fileName();
		SelWinAction *selWin = new SelWinAction(menu, label);
		if (texDoc->isModified()) {
			QFont f(selWin->font());
			f.setItalic(true);
			selWin->setFont(f);
		}
		if (texDoc == qobject_cast<TeXDocument*>(window)) {
			selWin->setCheckable(true);
			selWin->setChecked(true);
		}
		QObject::connect(selWin, SIGNAL(triggered()), texDoc, SLOT(selectWindow()));
		menu->addAction(selWin);
	}
	
	// append an item for each PDFDocument
	first = true;
	foreach (PDFDocument *pdfDoc, PDFDocument::documentList()) {
		if (first && !menu->actions().isEmpty())
			menu->addSeparator();
		first = false;
		SelWinAction *selWin = new SelWinAction(menu, pdfDoc->fileName());
		if (pdfDoc == qobject_cast<PDFDocument*>(window)) {
			selWin->setCheckable(true);
			selWin->setChecked(true);
		}
		QObject::connect(selWin, SIGNAL(triggered()), pdfDoc, SLOT(selectWindow()));
		menu->addAction(selWin);
	}
}

void TWUtils::ensureOnScreen(QWidget *window)
{
	QDesktopWidget *desktop = QApplication::desktop();
	QRect screenRect = desktop->availableGeometry(window);
	QRect adjustedFrame = window->frameGeometry();
	if (adjustedFrame.width() > screenRect.width())
		adjustedFrame.setWidth(screenRect.width());
	if (adjustedFrame.height() > screenRect.height())
		adjustedFrame.setHeight(screenRect.height());
	if (adjustedFrame.left() < screenRect.left())
		adjustedFrame.moveLeft(screenRect.left());
	else if (adjustedFrame.right() > screenRect.right())
		adjustedFrame.moveRight(screenRect.right());
	if (adjustedFrame.top() < screenRect.top())
		adjustedFrame.moveTop(screenRect.top());
	else if (adjustedFrame.bottom() > screenRect.bottom())
		adjustedFrame.moveBottom(screenRect.bottom());
	if (adjustedFrame != window->frameGeometry())
		window->setGeometry(adjustedFrame.adjusted(window->geometry().left() - window->frameGeometry().left(),
													window->geometry().top() - window->frameGeometry().top(),
													window->frameGeometry().right() - window->geometry().right(),
													window->frameGeometry().bottom() - window->geometry().bottom()
												));
}

void TWUtils::zoomToScreen(QWidget *window)
{
	QDesktopWidget *desktop = QApplication::desktop();
	QRect screenRect = desktop->availableGeometry(window);
	screenRect.setTop(screenRect.top() + window->geometry().y() - window->y());
	window->setGeometry(screenRect);
}

void TWUtils::zoomToHalfScreen(QWidget *window, bool rhs)
{
	QDesktopWidget *desktop = QApplication::desktop();
	QRect r = desktop->availableGeometry(window);
	int wDiff = window->frameGeometry().width() - window->width();
	int hDiff = window->frameGeometry().height() - window->height();

	if (hDiff == 0 && wDiff == 0) {
		// window may not be decorated yet, so we don't know how large
		// the title bar etc. is. Try to extrapolate from other top-level
		// windows (if some are available). We assume that if either
		// hDiff or wDiff is non-zero, we have found a decorated window
		// and can use its values.
		foreach (QWidget * widget, QApplication::topLevelWidgets()) {
			if (!qobject_cast<QMainWindow*>(widget))
				continue;
			hDiff = widget->frameGeometry().height() - widget->height();
			wDiff = widget->frameGeometry().width() - widget->width();
			if (hDiff != 0 || wDiff != 0)
				break;
		}
		if (hDiff == 0 && wDiff == 0) {
			// Give the user the possibility to specify his own values by
			// hacking the config files.
			// (Note: this should only be necessary in some special cases, e.g.
			// on X11 systems with special effects enabled)
			QSETTINGS_OBJECT(settings);
			wDiff = qMax(0, settings.value("windowWDiff", 0).toInt());
			hDiff = qMax(0, settings.value("windowHDiff", 0).toInt());
		}
		// If we still have no valid value for hDiff/wDiff, just guess (on some
		// platforms)
		if (hDiff == 0 && wDiff == 0) {
#if defined(Q_OS_WIN)
			// (these values were determined on WinXP with default theme)
			hDiff = 34;
			wDiff = 8;
#endif
		}
	}
	
	if (rhs) {
		r.setLeft(r.left() + r.right() / 2);
		window->move(r.left(), r.top());
		window->resize(r.width() - wDiff, r.height() - hDiff);
	}
	else {
		r.setRight(r.left() + r.right() / 2 - 1);
		window->move(r.left(), r.top());
		window->resize(r.width() - wDiff, r.height() - hDiff);
	}
}

void TWUtils::sideBySide(QWidget *window1, QWidget *window2)
{
	QDesktopWidget *desktop = QApplication::desktop();

	// if the windows reside on the same screen zoom each so that it occupies 
	// half of that screen
	if (desktop->screenNumber(window1) == desktop->screenNumber(window2)) {
		zoomToHalfScreen(window1, false);
		zoomToHalfScreen(window2, true);
	}
	// if the windows reside on different screens zoom each so that it uses
	// its whole screen
	else {
		zoomToScreen(window1);
		zoomToScreen(window2);
	}
}

void TWUtils::tileWindowsInRect(const QWidgetList& windows, const QRect& bounds)
{
	int numWindows = windows.count();
	int rows = 1, cols = 1;
	while (rows * cols < numWindows)
		if (rows == cols)
			++cols;
		else
			++rows;
	QRect r;
	r.setWidth(bounds.width() / cols);
	r.setHeight(bounds.height() / rows);
	r.moveLeft(bounds.left());
	r.moveTop(bounds.top());
	int x = 0, y = 0;
	foreach (QWidget* window, windows) {
		int wDiff = window->frameGeometry().width() - window->width();
		int hDiff = window->frameGeometry().height() - window->height();
		window->move(r.left(), r.top());
		window->resize(r.width() - wDiff, r.height() - hDiff);
		if (window->isMinimized())
			window->showNormal();
		if (++x == cols) {
			x = 0;
			++y;
			r.moveLeft(bounds.left());
			r.moveTop(bounds.top() + (bounds.height() * y) / rows);
		}
		else
			r.moveLeft(bounds.left() + (bounds.width() * x) / cols); 
	}
}

void TWUtils::stackWindowsInRect(const QWidgetList& windows, const QRect& bounds)
{
	const int kStackingOffset = 20;
	QRect r(bounds);
	r.setWidth(r.width() / 2);
	int index = 0;
	foreach (QWidget* window, windows) {
		int wDiff = window->frameGeometry().width() - window->width();
		int hDiff = window->frameGeometry().height() - window->height();
		window->move(r.left(), r.top());
		window->resize(r.width() - wDiff, r.height() - hDiff);
		if (window->isMinimized())
			window->showNormal();
		r.moveLeft(r.left() + kStackingOffset);
		if (r.right() > bounds.right()) {
			r = bounds;
			r.setWidth(r.width() / 2);
			index = 0;
		}
		else if (++index == 10) {
			r.setTop(bounds.top());
			index = 0;
		}
		else {
			r.setTop(r.top() + kStackingOffset);
			if (r.height() < bounds.height() / 2) {
				r.setTop(bounds.top());
				index = 0;
			}
		}
	}
}

void TWUtils::applyToolbarOptions(QMainWindow *theWindow, int iconSize, bool showText)
{
	iconSize = iconSize * 8 + 8;	// convert 1,2,3 to 16,24,32
	foreach (QObject *object, theWindow->children()) {
		QToolBar *theToolBar = qobject_cast<QToolBar*>(object);
		if (theToolBar != NULL) {
			theToolBar->setToolButtonStyle(showText ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);
			theToolBar->setIconSize(QSize(iconSize, iconSize));
		}
	}
}

bool TWUtils::findNextWord(const QString& text, int index, int& start, int& end)
{
	// try to do a sensible "word" selection for TeX documents, taking account of the form of control sequences:
	// given an index representing a caret,
	// if current char (following caret) is letter, apostrophe, or '@', extend in both directions
	//    include apostrophe if surrounded by letters
	//    include preceding backslash if any, unless word contains apostrophe
	// if preceding char is backslash, extend to include backslash only
	// if current char is number, extend in both directions
	// if current char is space or tab, extend in both directions to include all spaces or tabs
	// if current char is backslash, include next char; if letter or '@', extend to include all following letters or '@'
	// else select single char following index
	// returns TRUE if the resulting selection consists of word-forming chars

	start = end = index;

	if (text.length() < 1) // empty
		return false;

	if (index >= text.length()) // end of line
		return false;
	QChar	ch = text.at(index);

#define IS_WORD_FORMING(ch) (ch.isLetter() || ch.isMark())

	bool isControlSeq = false; // becomes true if we include an @ sign or a leading backslash
	bool includesApos = false; // becomes true if we include an apostrophe
	if (IS_WORD_FORMING(ch) || ch == '@' /* || ch == '\'' || ch == 0x2019 */) {
		if (ch == '@')
			isControlSeq = true;
		//else if (ch == '\'' || ch == 0x2019)
		//	includesApos = true;
		while (start > 0) {
			--start;
			ch = text.at(start);
			if (IS_WORD_FORMING(ch))
				continue;
			if (!includesApos && ch == '@') {
				isControlSeq = true;
				continue;
			}
			if (!isControlSeq && (ch == '\'' || ch == 0x2019) && start > 0 && IS_WORD_FORMING(text.at(start - 1))) {
				includesApos = true;
				continue;
			}
			++start;
			break;
		}
		if (start > 0 && text.at(start - 1) == '\\') {
			isControlSeq = true;
			--start;
		}
		while (++end < text.length()) {
			ch = text.at(end);
			if (IS_WORD_FORMING(ch))
				continue;
			if (!includesApos && ch == '@') {
				isControlSeq = true;
				continue;
			}
			if (!isControlSeq && (ch == '\'' || ch == 0x2019) && end < text.length() - 1 && IS_WORD_FORMING(text.at(end + 1))) {
				includesApos = true;
				continue;
			}
			break;
		}
		return !isControlSeq;
	}
	
	if (index > 0 && text.at(index - 1) == '\\') {
		start = index - 1;
		end = index + 1;
		return false;
	}
	
	if (ch.isNumber()) {
		// TODO: handle decimals, leading signs
		while (start > 0) {
			--start;
			ch = text.at(start);
			if (ch.isNumber())
				continue;
			++start;
			break;
		}
		while (++end < text.length()) {
			ch = text.at(end);
			if (ch.isNumber())
				continue;
			break;
		}
		return false;
	}
	
	if (ch == ' ' || ch == '\t') {
		while (start > 0) {
			--start;
			ch = text.at(start);
			if (!(ch == ' ' || ch == '\t')) {
				++start;
				break;
			}
		}
		while (++end < text.length()) {
			ch = text.at(end);
			if (!(ch == ' ' || ch == '\t'))
				break;
		}
		return false;
	}
	
	if (ch == '\\') {
		if (++end < text.length()) {
			ch = text.at(end);
			if (IS_WORD_FORMING(ch) || ch == '@')
				while (++end < text.length()) {
					ch = text.at(end);
					if (IS_WORD_FORMING(ch) || ch == '@')
						continue;
					break;
				}
			else
				++end;
		}
		return false;
	}

	// else the character is selected in isolation
	end = index + 1;
	return false;
}

QMap<QChar,QChar> TWUtils::pairOpeners;
QMap<QChar,QChar> TWUtils::pairClosers;

QChar TWUtils::closerMatching(QChar c)
{
	return pairClosers.value(c);
}

QChar TWUtils::openerMatching(QChar c)
{
	return pairOpeners.value(c);
}

QString TWUtils::sIncludePdfCommand;
QString TWUtils::sIncludeImageCommand;
QString TWUtils::sIncludePostscriptCommand;
QString TWUtils::sIncludeTextCommand;
QString TWUtils::sCleanupPatterns;

const QString& TWUtils::includePdfCommand()
{
	return sIncludePdfCommand;
}

const QString& TWUtils::includeImageCommand()
{
	return sIncludeImageCommand;
}

const QString& TWUtils::includePostscriptCommand()
{
	return sIncludePostscriptCommand;
}

const QString& TWUtils::includeTextCommand()
{
	return sIncludeTextCommand;
}

const QString& TWUtils::cleanupPatterns()
{
	return sCleanupPatterns;
}

void TWUtils::readConfig()
{
	pairOpeners.clear();
	pairClosers.clear();

	QDir configDir(TWUtils::getLibraryPath("configuration"));
	QRegExp pair("([^\\s])\\s+([^\\s])\\s*(?:#.*)?");

	QFile pairsFile(configDir.filePath("delimiter-pairs.txt"));
	if (pairsFile.open(QIODevice::ReadOnly)) {
		while (1) {
			QByteArray ba = pairsFile.readLine();
			if (ba.size() == 0)
				break;
			if (ba[0] == '#' || ba[0] == '\n')
				continue;
			ba.chop(1);
			QString line = QString::fromUtf8(ba.data(), ba.size());
			if (pair.exactMatch(line)) {
				pairClosers[pair.cap(1).at(0)] = pair.cap(2).at(0);
				pairOpeners[pair.cap(2).at(0)] = pair.cap(1).at(0);
			}
		}
	}

	// defaults in case config file not found
	sIncludeTextCommand			= "\\include{%1}\n";
	sIncludePdfCommand			= "\\includegraphics[]{%1}\n";
	sIncludeImageCommand		= "\\includegraphics[]{%1}\n";
	sIncludePostscriptCommand	= "\\includegraphics[]{%1}\n";

	sCleanupPatterns = "*.aux $jobname.log $jobname.lof $jobname.lot $jobname.toc";

	filters = new QStringList;
	
	QFile configFile(configDir.filePath("texworks-config.txt"));
	if (configFile.open(QIODevice::ReadOnly)) {
		QRegExp keyVal("([-a-z]+):\\s*([^ \\t].+)");
			// looking for keyword, colon, optional whitespace, value
		while (1) {
			QByteArray ba = configFile.readLine();
			if (ba.size() == 0)
				break;
			if (ba[0] == '#' || ba[0] == '\n')
				continue;
			QString	line = QString::fromUtf8(ba.data(), ba.size());
			if (keyVal.exactMatch(line)) {
				// if that matched, keyVal.cap(1) is the keyword, cap(2) is the value
				const QString& keyword = keyVal.cap(1);
				QString value = keyVal.cap(2).trimmed();
				if (keyword == "include-text") {
					sIncludeTextCommand = value.replace("#RET#", "\n");
					continue;
				}
				if (keyword == "include-pdf") {
					sIncludePdfCommand = value.replace("#RET#", "\n");
					continue;
				}
				if (keyword == "include-image") {
					sIncludeImageCommand = value.replace("#RET#", "\n");
					continue;
				}
				if (keyword == "include-postscript") {
					sIncludePostscriptCommand = value.replace("#RET#", "\n");
					continue;
				}
				if (keyword == "cleanup-patterns") {
					static bool first = true;
					if (first) {
						sCleanupPatterns = value;
						first = false;
					}
					else {
						sCleanupPatterns += " ";
						sCleanupPatterns += value;
					}
					continue;
				}
				if (keyword == "file-open-filter") {
					*filters << value;
				}
			}
		}
	}
	
	if (filters->count() == 0)
		setDefaultFilters();
}

int TWUtils::balanceDelim(const QString& text, int pos, QChar delim, int direction)
{
	int len = text.length();
	QChar c;
	while ((c = text[pos]) != delim) {
		if (openerMatching(c) != 0)
			pos = (direction < 0) ? balanceDelim(text, pos - 1, openerMatching(c), -1) : -1;
		else if (closerMatching(c) != 0)
			pos = (direction > 0) ? balanceDelim(text, pos + 1, closerMatching(c), 1) : -1;
		if (pos < 0)
			return -1;
		pos += direction;
		if (pos < 0 || pos >= len)
			return -1;
	}
	return pos;
}

int TWUtils::findOpeningDelim(const QString& text, int pos)
	// find the first opening delimiter before offset /pos/
{
	while (--pos >= 0) {
		QChar c = text[pos];
		if (closerMatching(c) != 0)
			return pos;
	}
	return -1;
}

void TWUtils::installCustomShortcuts(QWidget * widget, bool recursive /* = true */, QSettings * map /* = NULL */)
{
	bool deleteMap = false;
	
	if (widget == NULL)
		return;

	if (!map) {
		QString filename = QDir(TWUtils::getLibraryPath("configuration")).absoluteFilePath("shortcuts.ini");
		if (filename.isEmpty() || !QFileInfo(filename).exists())
			return;

		map = new QSettings(filename, QSettings::IniFormat);
		if (map->status() != QSettings::NoError) {
			delete map;
			return;
		}
		deleteMap = true;
	}

	QAction * act;
	foreach (act, widget->actions()) {
		if (act->objectName().isEmpty())
			continue;
		if (map->contains(act->objectName()))
			act->setShortcut(QKeySequence(map->value(act->objectName()).toString()));
	}
	
	if (recursive) {
		QObject * obj;
		foreach (obj, widget->children()) {
			QWidget * child = qobject_cast<QWidget*>(obj);
			if (child)
				installCustomShortcuts(child, true, map);
		}
	}
	
	if (deleteMap)
		delete map;
}

// static
bool TWUtils::isGitInfoAvailable()
{
	return (!QString::fromLatin1(GIT_COMMIT_HASH).startsWith("$Format:") && !QString::fromLatin1(GIT_COMMIT_DATE).startsWith("$Format:"));
}

// static
QString TWUtils::gitCommitHash()
{
	if(QString::fromLatin1(GIT_COMMIT_HASH).startsWith("$Format:"))
		return QString();
	return GIT_COMMIT_HASH;
}

// static
QDateTime TWUtils::gitCommitDate()
{
	if (QString::fromLatin1(GIT_COMMIT_DATE).startsWith("$Format:"))
		return QDateTime();
	return QDateTime::fromString(GIT_COMMIT_DATE, Qt::ISODate).toUTC();
}

#pragma mark === SelWinAction ===

// action subclass used for dynamic window-selection items in the Window menu

SelWinAction::SelWinAction(QObject *parent, const QString &fileName)
	: QAction(parent)
{
	setText(TWUtils::strippedName(fileName));
	setData(fileName);
}

#pragma mark === CmdKeyFilter ===

// on OS X only, the singleton CmdKeyFilter object is attached to all TeXDocument editor widgets
// to stop Command-keys getting inserted into edit text items

CmdKeyFilter *CmdKeyFilter::filterObj = NULL;

CmdKeyFilter *CmdKeyFilter::filter()
{
	if (filterObj == NULL)
		filterObj = new CmdKeyFilter;
	return filterObj;
}

bool CmdKeyFilter::eventFilter(QObject *obj, QEvent *event)
{
#if defined(Q_OS_DARWIN)
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
		if ((keyEvent->modifiers() & Qt::ControlModifier) != 0) {
			if (keyEvent->key() <= 0x0ff
				&& keyEvent->key() != Qt::Key_Z
				&& keyEvent->key() != Qt::Key_X
				&& keyEvent->key() != Qt::Key_C
				&& keyEvent->key() != Qt::Key_V
				&& keyEvent->key() != Qt::Key_A)
				return true;
		}
	}
#endif
	return QObject::eventFilter(obj, event);
}

#pragma mark === Engine ===

Engine::Engine()
	: QObject()
	, f_showPdf(false)
{
}

Engine::Engine(const QString& name, const QString& program, const QStringList arguments, bool showPdf)
	: QObject(), f_name(name), f_program(program), f_arguments(arguments), f_showPdf(showPdf)
{
}

Engine::Engine(const Engine& orig)
	: QObject(), f_name(orig.f_name), f_program(orig.f_program), f_arguments(orig.f_arguments), f_showPdf(orig.f_showPdf)
{
}

Engine& Engine::operator=(const Engine& rhs)
{
	f_name = rhs.f_name;
	f_program = rhs.f_program;
	f_arguments = rhs.f_arguments;
	f_showPdf = rhs.f_showPdf;
	return *this;
}

const QString Engine::name() const
{
	return f_name;
}

const QString Engine::program() const
{
	return f_program;
}

const QStringList Engine::arguments() const
{
	return f_arguments;
}

bool Engine::showPdf() const
{
	return f_showPdf;
}

void Engine::setName(const QString& name)
{
	f_name = name;
}

void Engine::setProgram(const QString& program)
{
	f_program = program;
}

void Engine::setArguments(const QStringList& arguments)
{
	f_arguments = arguments;
}

void Engine::setShowPdf(bool showPdf)
{
	f_showPdf = showPdf;
}

/*static*/
FileVersionDatabase FileVersionDatabase::load(const QString & path)
{
	QFile fin(path);
	FileVersionDatabase retVal;
	QDir rootDir(QFileInfo(path).absoluteDir());
	
	if (!fin.open(QIODevice::ReadOnly | QIODevice::Text))
		return retVal;

	QTextStream strm(&fin);
	
	while (!strm.atEnd()) {
		FileVersionDatabase::Record rec;
		QString line = strm.readLine().trimmed();
		
		// ignore comments
		if (line.startsWith('#')) continue;
		
		rec.version = line.section(' ', 0, 0);
		rec.hash = QByteArray::fromHex(line.section(' ', 1, 1).toLatin1());
		rec.filePath = line.section(' ', 2).trimmed();
		rec.filePath = rootDir.absoluteFilePath(rec.filePath.filePath());
		retVal.m_records.append(rec);
	}
	
	fin.close();
	
	return retVal;
}

bool FileVersionDatabase::save(const QString & path) const
{
	QFile fout(path);
	QDir rootDir(QFileInfo(path).absoluteDir());
	
	if (!fout.open(QIODevice::WriteOnly | QIODevice::Text))
		return false;
	
	QTextStream strm(&fout);

	foreach (FileVersionDatabase::Record rec, m_records) {
		QString filePath = rec.filePath.absoluteFilePath();
		strm << rec.version << " " << rec.hash.toHex() << " " << rootDir.relativeFilePath(filePath) << endl;
	}
	
	fout.close();
	return true;
}

void FileVersionDatabase::addFileRecord(const QFileInfo & file, const QByteArray & md5Hash, const QString version)
{
	// remove all existing entries for this file
	QMutableListIterator<FileVersionDatabase::Record> it(m_records);
	
	while (it.hasNext()) {
		const FileVersionDatabase::Record rec = it.next();
		if (file.absoluteFilePath() == rec.filePath.absoluteFilePath()) {
			it.remove();
		}
	}
	
	// add the new data
	FileVersionDatabase::Record rec;
	rec.filePath = file;
	rec.version = version;
	rec.hash = md5Hash;
	m_records.append(rec);
}

bool FileVersionDatabase::hasFileRecord(const QFileInfo & file) const
{
	QListIterator<FileVersionDatabase::Record> it(m_records);
	
	while (it.hasNext()) {
		const FileVersionDatabase::Record rec = it.next();
		if (file.filePath() == rec.filePath.filePath())
			return true;
	}
	return false;
}

FileVersionDatabase::Record FileVersionDatabase::getFileRecord(const QFileInfo & file) const
{
	QListIterator<FileVersionDatabase::Record> it(m_records);
	
	while (it.hasNext()) {
		const FileVersionDatabase::Record rec = it.next();
		if (file == rec.filePath)
			return rec;
	}

	FileVersionDatabase::Record retVal;
	retVal.version = QString();
	retVal.hash = QByteArray::fromHex("d41d8cd98f00b204e9800998ecf8427e"); // hash for the zero-length string
	return retVal;
}

/*static*/
QByteArray FileVersionDatabase::hashForFile(const QString & path)
{
	QByteArray retVal = QByteArray::fromHex("d41d8cd98f00b204e9800998ecf8427e"); // hash for the zero-length string;
	QFile fin(path);
	
	if (!fin.open(QIODevice::ReadOnly))
		return retVal;
	
	retVal = QCryptographicHash::hash(fin.readAll(), QCryptographicHash::Md5);
	fin.close();
	return retVal;
}
