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

#if defined(MIKTEX_WINDOWS)
#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>
#endif
#include "TWUtils.h"
#include "TWApp.h"
#include "TeXDocumentWindow.h"
#include "PDFDocumentWindow.h"
#include "TWVersion.h"
#if !defined(MIKTEX)
#include "GitRev.h"
#endif
#include "Settings.h"
#include "utils/FileVersionDatabase.h"

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
#include <QDateTime>

#if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
// compile-time default paths - customize by defining in the .pro file
#ifndef TW_DICPATH
#define TW_DICPATH "/usr/share/hunspell" PATH_LIST_SEP "/usr/share/myspell/dicts"
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
		libRootPath = QDir::homePath() + QLatin1String("/Library/" TEXWORKS_NAME "/");
#elif defined(Q_OS_UNIX) // && !defined(Q_OS_DARWIN)
		if (subdir == QLatin1String("dictionaries")) {
			libPath = QString::fromLatin1(TW_DICPATH);
			QString dicPath = QString::fromLocal8Bit(getenv("TW_DICPATH"));
			if (!dicPath.isEmpty())
				libPath = dicPath;
			return libPath; // don't try to create/update the system dicts directory
		}
		libRootPath = QDir::homePath() + QLatin1String("/." TEXWORKS_NAME "/");
#else // defined(Q_OS_WIN)
#if defined(MIKTEX)
                {
                  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();
                  MiKTeX::Core::PathName dir;
                  dir = session->GetSpecialPath(MiKTeX::Core::SpecialPath::DataRoot);
                  dir /= TEXWORKS_NAME;
		  dir /= std::to_string(VER_MAJOR) + "." + std::to_string(VER_MINOR);
                  libRootPath = QString::fromUtf8(dir.GetData());
                }
#else
		libRootPath = QDir::homePath() + QLatin1String("/" TEXWORKS_NAME "/");
#endif
#endif
	}
	libPath = QDir(libRootPath).absolutePath() + QDir::separator() + subdir;

	if(updateOnDisk)
		updateLibraryResources(QDir(QString::fromLatin1(":/resfiles")), libRootPath, subdir);
	return libPath;
}

const QStringList TWUtils::getLibraryPaths(const QString & subdir, const bool updateOnDisk)
{
	return getLibraryPath(subdir, updateOnDisk).split(QStringLiteral(PATH_LIST_SEP));
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
	
	if (subdir == QString::fromLatin1("translations")) // don't copy the built-in translations
		return;
	
	Tw::Utils::FileVersionDatabase fvdb = Tw::Utils::FileVersionDatabase::load(destRootDir.absoluteFilePath(QString::fromLatin1("TwFileVersions.db")));
	
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
			Tw::Utils::FileVersionDatabase::Record rec = fvdb.getFileRecord(destPath);
			// If the file no longer exists on the disk, the user has deleted it
			// Hence we won't recreate it, but we keep the database record to
			// remember that this file was deleted by the user
			if (!QFileInfo(destPath).exists())
				continue;
			
			QByteArray srcHash = Tw::Utils::FileVersionDatabase::hashForFile(srcPath);
			QByteArray destHash = Tw::Utils::FileVersionDatabase::hashForFile(destPath);
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
			QByteArray srcHash = Tw::Utils::FileVersionDatabase::hashForFile(srcPath);
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
				QByteArray destHash = Tw::Utils::FileVersionDatabase::hashForFile(destPath);
				if (srcHash == destHash)
					fvdb.addFileRecord(destPath, destHash, gitCommitHash());
			}
		}
	}

	// Now, remove all files that are unmodified on disk and were
	// removed upstream
	QMutableListIterator<Tw::Utils::FileVersionDatabase::Record> recIt(fvdb.getFileRecords());
	while (recIt.hasNext()) {
		const Tw::Utils::FileVersionDatabase::Record & rec = recIt.next();

		QString destPath = rec.filePath.filePath();
		QString path = destRootDir.relativeFilePath(destPath);
		QString srcPath = srcRootDir.filePath(path);
		
		// If the source file still exists there is nothing to do here
		if (QFileInfo(srcPath).exists())
			continue;
		
		// If the source file no longer exists but the file on disk is up to
		// date, remove it
		if (rec.filePath.exists() && Tw::Utils::FileVersionDatabase::hashForFile(destPath) == rec.hash) {
			QFile(destPath).remove();
			recIt.remove();
		}
	}

	// Finally, save the updated database
	fvdb.save(destRootDir.absoluteFilePath(QString::fromLatin1("TwFileVersions.db")));
}

static int
insertItemIfPresent(QFileInfo& fi, QMenu* helpMenu, QAction* before, QSignalMapper* mapper, QString title)
{
	QFileInfo indexFile(fi.absoluteFilePath(), QString::fromLatin1("index.html"));
	if (indexFile.exists()) {
		QFileInfo titlefileInfo(fi.absoluteFilePath(), QString::fromLatin1("tw-help-title.txt"));
		if (titlefileInfo.exists() && titlefileInfo.isReadable()) {
			QFile titleFile(titlefileInfo.absoluteFilePath());
			titleFile.open(QIODevice::ReadOnly | QIODevice::Text);
			QTextStream titleStream(&titleFile);
			titleStream.setCodec("UTF-8");
			title = titleStream.readLine();
		}
		QAction* action = new QAction(title, helpMenu);
		mapper->setMapping(action, fi.canonicalFilePath());
		QObject::connect(action, SIGNAL(triggered()), mapper, SLOT(map()));
		helpMenu->insertAction(before, action);
		return 1;
	}
	return 0;
}

void TWUtils::insertHelpMenuItems(QMenu* helpMenu)
{
	QSignalMapper* mapper = new QSignalMapper(helpMenu);
	QObject::connect(mapper, SIGNAL(mapped(const QString&)), TWApp::instance(), SLOT(openHelpFile(const QString&)));

	QAction* before = nullptr;
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
	QDir helpDir(QCoreApplication::applicationDirPath() + QLatin1String("/../texworks-help"));
#else
#if defined(MIKTEX)
        // TODO: code review
        QDir helpDir;
        std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();
        MiKTeX::Core::PathName path = session->GetSpecialPath(MiKTeX::Core::SpecialPath::DistRoot);
        path /= "doc/texworks/help";
        if (MiKTeX::Core::Directory::Exists(path))
        {
          helpDir = QString::fromUtf8(path.GetData());
        }
        else
        {
          helpDir = QCoreApplication::applicationDirPath() + "/texworks-help";
        }
#else
	QDir helpDir(QCoreApplication::applicationDirPath() + QLatin1String("/texworks-help"));
#endif
#if defined(Q_OS_UNIX) // && !defined(Q_OS_DARWIN)
	if (!helpDir.exists())
		helpDir.cd(QString::fromLatin1(TW_HELPPATH));
#endif
#endif
#if defined(MIKTEX_WINDOWS)
        QString helpPath = QString::fromUtf8(getenv("TW_HELPPATH"));
#else
	QString helpPath = QString::fromLocal8Bit(getenv("TW_HELPPATH"));
#endif
	if (!helpPath.isEmpty())
		helpDir.cd(QString(helpPath));

	Tw::Settings settings;
	QString loc = settings.value(QString::fromLatin1("locale")).toString();
	if (loc.isEmpty())
		loc = QLocale::system().name();
	
	QDirIterator iter(helpDir);
	int inserted = 0;
	while (iter.hasNext()) {
		(void)iter.next();
		if (!iter.fileInfo().isDir())
			continue;
		QString name(iter.fileInfo().fileName());
		if (name == QLatin1String(".") || name == QLatin1String(".."))
			continue;
		QDir subDir(iter.filePath());
		// try for localized content first
		QFileInfo fi(subDir, loc);
		if (fi.exists() && fi.isDir() && fi.isReadable()) {
			inserted += insertItemIfPresent(fi, helpMenu, before, mapper, name);
			continue;
		}
		fi.setFile(subDir.absolutePath() + QChar::fromLatin1('/') + loc.left(2));
		if (fi.exists() && fi.isDir() && fi.isReadable()) {
			inserted += insertItemIfPresent(fi, helpMenu, before, mapper, name);
			continue;
		}
		fi.setFile(subDir.absolutePath() + QString::fromLatin1("/en"));
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

QList<QTextCodec*> *TWUtils::codecList = nullptr;

QList<QTextCodec*> *TWUtils::findCodecs()
{
	if (codecList)
		return codecList;

	codecList = new QList<QTextCodec*>;
	QMap<QString, QTextCodec*> codecMap;
	QRegularExpression iso8859RegExp(QStringLiteral("^ISO[- ]8859-([0-9]+)"));
	foreach (int mib, QTextCodec::availableMibs()) {
		QTextCodec *codec = QTextCodec::codecForMib(mib);
		QString sortKey = QString::fromUtf8(codec->name().constData()).toUpper();
		QRegularExpressionMatch iso8859Match = iso8859RegExp.match(sortKey);
		int rank;
		if (sortKey.startsWith(QLatin1String("UTF-8")))
			rank = 1;
		else if (sortKey.startsWith(QLatin1String("UTF-16")))
			rank = 2;
		else if (iso8859Match.hasMatch()) {
			if (iso8859Match.captured(1).size() == 1)
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

QStringList* TWUtils::translationList = nullptr;

QStringList* TWUtils::getTranslationList()
{
	if (translationList)
		return translationList;

	translationList = new QStringList;
	
	QDir transDir(QString::fromLatin1(":/resfiles/translations"));
	foreach (QFileInfo qmFileInfo, transDir.entryInfoList(QStringList(QString::fromLatin1(TEXWORKS_NAME "_*.qm")),
														  QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase)) {
		QString locName = qmFileInfo.completeBaseName();
		locName.remove(QString::fromLatin1(TEXWORKS_NAME "_"));
		*translationList << locName;
	}
	
	transDir = QDir(TWUtils::getLibraryPath(QString::fromLatin1("translations")));
	foreach (QFileInfo qmFileInfo, transDir.entryInfoList(QStringList(QString::fromLatin1(TEXWORKS_NAME "_*.qm")),
				QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase)) {
		QString locName = qmFileInfo.completeBaseName();
		locName.remove(QString::fromLatin1(TEXWORKS_NAME "_"));
		if (!translationList->contains(locName, Qt::CaseInsensitive))
			*translationList << locName;
	}
	
	// English is always available, and it has to be the first item
	translationList->removeAll(QString::fromLatin1("en"));
	translationList->prepend(QString::fromLatin1("en"));
	
	return translationList;
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
	*filters << QObject::tr("All files") + QLatin1String(" (*)"); // this must not be "*.*", which causes an extension ".*" to be added on some systems
}

/*static*/
QString TWUtils::chooseDefaultFilter(const QString & filename, const QStringList & filters)
{
	QString extension = QFileInfo(filename).completeSuffix();

	if (extension.isEmpty())
		return filters[0];

	foreach (QString filter, filters) {
		// return filter if it corresponds to the given extension
		// note that the extension must be the first one in the list to match;
		// otherwise, the file dialog would replace the actual extension by the
		// first one in the list, thereby altering it without cause
		if (filter.contains(QString::fromLatin1("(*.%1").arg(extension)))
			return filter;
	}
	// if no filter matched, return the last one (which should be "All files")
	return filters.last();
}

QString TWUtils::strippedName(const QString &fullFileName, const unsigned int dirComponents /* = 0 */)
{
	QDir dir(QFileInfo(fullFileName).dir());
	for (unsigned int i = 0; i < dirComponents; ++i) {
		// NB: dir.cdUp() would be more logical, but fails if the resulting
		// path does not exist
		dir.setPath(dir.path() + QString::fromLatin1("/.."));
	}
	return dir.relativeFilePath(fullFileName);
}

QStringList TWUtils::constructUniqueFileLabels(const QStringList & fileList)
{
	QStringList labelList;

	Q_FOREACH (QString file, fileList)
		labelList.append(strippedName(file));

	// Make label list unique, i.e. while labels are not unique, add
	// directory components
	for (unsigned int dirComponents = 1; ; ++dirComponents) {
		QList<bool> isDuplicate;
		Q_FOREACH(QString label, labelList)
			isDuplicate.append(labelList.count(label) > 1);
		if (!isDuplicate.contains(true))
			break;

		for (int i = 0; i < labelList.size(); ++i) {
			if (!isDuplicate[i])
				continue;
			labelList[i] = strippedName(fileList[i], dirComponents);
		}
	}
	return labelList;
}

void TWUtils::updateRecentFileActions(QObject *parent, QList<QAction*> &actions, QMenu *menu, QAction * clearAction) /* static */
{
	Tw::Settings settings;
	QStringList fileList, labelList;
	if (settings.contains(QString::fromLatin1("recentFiles"))) {
		QList<QVariant> files = settings.value(QString::fromLatin1("recentFiles")).toList();
		foreach (const QVariant& v, files) {
			QMap<QString,QVariant> map = v.toMap();
			if (map.contains(QString::fromLatin1("path")))
				fileList.append(map.value(QString::fromLatin1("path")).toString());
		}
	}
	else {
		// check for an old "recentFilesList" entry, and migrate it
		if (settings.contains(QString::fromLatin1("recentFileList"))) {
			fileList = settings.value(QString::fromLatin1("recentFileList")).toStringList();
			QList<QVariant> files;
			foreach (const QString& path, fileList) {
				QMap<QString,QVariant> map;
				map.insert(QString::fromLatin1("path"), path);
				files.append(QVariant(map));
			}
			settings.remove(QString::fromLatin1("recentFileList"));
			settings.setValue(QString::fromLatin1("recentFiles"), files);
		}
	}

	// Generate label list (list of filenames without directory components)
	labelList = constructUniqueFileLabels(fileList);

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
		// a "&" inside a menu label is considered a mnemonic, thus, we need to escape them
		labelList[i].replace(QString::fromLatin1("&"), QString::fromLatin1("&&"));

		actions[i]->setText(labelList[i]);
		actions[i]->setData(fileList[i]);
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
	
	QList<TeXDocumentWindow *> texDocList;
	QStringList fileList, labelList;
	Q_FOREACH(TeXDocumentWindow * texDoc, TeXDocumentWindow::documentList()) {
		texDocList.append(texDoc);
		fileList.append(texDoc->fileName());
	}
	labelList = constructUniqueFileLabels(fileList);

	// append an item for each TeXDocument
	bool first = true;
	for (int i = 0; i < texDocList.size(); ++i) {
		TeXDocumentWindow * texDoc = texDocList[i];
		if (first && !menu->actions().isEmpty())
			menu->addSeparator();
		first = false;
		SelWinAction *selWin = new SelWinAction(menu, fileList[i], labelList[i]);
		if (texDoc->isModified()) {
			QFont f(selWin->font());
			f.setItalic(true);
			selWin->setFont(f);
		}
		if (texDoc == qobject_cast<TeXDocumentWindow*>(window)) {
			selWin->setCheckable(true);
			selWin->setChecked(true);
		}
		QObject::connect(selWin, SIGNAL(triggered()), texDoc, SLOT(selectWindow()));
		menu->addAction(selWin);
	}

	QList<PDFDocumentWindow *> pdfDocList;
	fileList.clear();
	labelList.clear();
	Q_FOREACH(PDFDocumentWindow * pdfDoc, PDFDocumentWindow::documentList()) {
		pdfDocList.append(pdfDoc);
		fileList.append(pdfDoc->fileName());
	}
	labelList = constructUniqueFileLabels(fileList);

	// append an item for each PDFDocument
	first = true;
	for (int i = 0; i < pdfDocList.size(); ++i) {
		PDFDocumentWindow * pdfDoc = pdfDocList[i];
		if (first && !menu->actions().isEmpty())
			menu->addSeparator();
		first = false;
		SelWinAction *selWin = new SelWinAction(menu, fileList[i], labelList[i]);
		if (pdfDoc == qobject_cast<PDFDocumentWindow*>(window)) {
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
			Tw::Settings settings;
			wDiff = qMax(0, settings.value(QString::fromLatin1("windowWDiff"), 0).toInt());
			hDiff = qMax(0, settings.value(QString::fromLatin1("windowHDiff"), 0).toInt());
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
	
	// Ensure the window is not maximized, otherwise some window managers might
	// react strangely to resizing
	window->showNormal();
	if (rhs) {
		r.setLeft((r.left() + r.right()) / 2);
		window->move(r.left(), r.top());
		window->resize(r.width() - wDiff, r.height() - hDiff);
	}
	else {
		r.setRight((r.left() + r.right()) / 2 - 1);
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
		if (theToolBar) {
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

#define IS_WORD_FORMING(ch) ((ch).isLetter() || (ch).isMark())

	if (IS_WORD_FORMING(ch) || ch == QChar::fromLatin1('@') /* || ch == QChar::fromLatin1('\'') || ch == 0x2019 */) {
		bool isControlSeq = false; // becomes true if we include an @ sign or a leading backslash
		bool includesApos = false; // becomes true if we include an apostrophe
		if (ch == QChar::fromLatin1('@'))
			isControlSeq = true;
		//else if (ch == QChar::fromLatin1('\'') || ch == 0x2019)
		//	includesApos = true;
		while (start > 0) {
			--start;
			ch = text.at(start);
			if (IS_WORD_FORMING(ch))
				continue;
			if (!includesApos && ch == QChar::fromLatin1('@')) {
				isControlSeq = true;
				continue;
			}
			if (!isControlSeq && (ch == QChar::fromLatin1('\'') || ch == QChar(0x2019)) && start > 0 && IS_WORD_FORMING(text.at(start - 1))) {
				includesApos = true;
				continue;
			}
			++start;
			break;
		}
		if (start > 0 && text.at(start - 1) == QChar::fromLatin1('\\')) {
			isControlSeq = true;
			--start;
		}
		while (++end < text.length()) {
			ch = text.at(end);
			if (IS_WORD_FORMING(ch))
				continue;
			if (!includesApos && ch == QChar::fromLatin1('@')) {
				isControlSeq = true;
				continue;
			}
			if (!isControlSeq && (ch == QChar::fromLatin1('\'') || ch == QChar(0x2019)) && end < text.length() - 1 && IS_WORD_FORMING(text.at(end + 1))) {
				includesApos = true;
				continue;
			}
			break;
		}
		return !isControlSeq;
	}
	
	if (index > 0 && text.at(index - 1) == QChar::fromLatin1('\\')) {
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
	
	if (ch == QChar::fromLatin1(' ') || ch == QChar::fromLatin1('\t')) {
		while (start > 0) {
			--start;
			ch = text.at(start);
			if (!(ch == QChar::fromLatin1(' ') || ch == QChar::fromLatin1('\t'))) {
				++start;
				break;
			}
		}
		while (++end < text.length()) {
			ch = text.at(end);
			if (!(ch == QChar::fromLatin1(' ') || ch == QChar::fromLatin1('\t')))
				break;
		}
		return false;
	}
	
	if (ch == QChar::fromLatin1('\\')) {
		if (++end < text.length()) {
			ch = text.at(end);
			if (IS_WORD_FORMING(ch) || ch == QChar::fromLatin1('@'))
				while (++end < text.length()) {
					ch = text.at(end);
					if (IS_WORD_FORMING(ch) || ch == QChar::fromLatin1('@'))
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

	QDir configDir(TWUtils::getLibraryPath(QString::fromLatin1("configuration")));
	QRegularExpression pair(QString::fromLatin1("^([^\\s])\\s+([^\\s])\\s*(?:#.*)?$"));

	QFile pairsFile(configDir.filePath(QString::fromLatin1("delimiter-pairs.txt")));
	if (pairsFile.open(QIODevice::ReadOnly)) {
		while (true) {
			QByteArray ba = pairsFile.readLine();
			if (ba.size() == 0)
				break;
			if (ba[0] == '#' || ba[0] == '\n')
				continue;
			ba.chop(1);
			QString line = QString::fromUtf8(ba.data(), ba.size());
			QRegularExpressionMatch pairMatch = pair.match(line);
			if (pairMatch.hasMatch()) {
				pairClosers[pairMatch.captured(1).at(0)] = pairMatch.captured(2).at(0);
				pairOpeners[pairMatch.captured(2).at(0)] = pairMatch.captured(1).at(0);
			}
		}
	}

	// defaults in case config file not found
	sIncludeTextCommand			= QString::fromLatin1("\\include{%1}\n");
	sIncludePdfCommand			= QString::fromLatin1("\\includegraphics[]{%1}\n");
	sIncludeImageCommand		= QString::fromLatin1("\\includegraphics[]{%1}\n");
	sIncludePostscriptCommand	= QString::fromLatin1("\\includegraphics[]{%1}\n");

	sCleanupPatterns = QString::fromLatin1("*.aux $jobname.log $jobname.lof $jobname.lot $jobname.toc");

	filters = new QStringList;
	
	QFile configFile(configDir.filePath(QString::fromLatin1("texworks-config.txt")));
	if (configFile.open(QIODevice::ReadOnly)) {
		QRegularExpression keyVal(QStringLiteral("^([-a-z]+):\\s*([^ \\t].+)$"));
			// looking for keyword, colon, optional whitespace, value
		while (true) {
			QByteArray ba = configFile.readLine();
			if (ba.size() == 0)
				break;
			if (ba[0] == '#' || ba[0] == '\n')
				continue;
			QString	line = QString::fromUtf8(ba.data(), ba.size());
			QRegularExpressionMatch keyValMatch = keyVal.match(line);
			if (keyValMatch.hasMatch()) {
				// if that matched, keyVal.cap(1) is the keyword, cap(2) is the value
				const QString& keyword = keyValMatch.captured(1);
				QString value = keyValMatch.captured(2).trimmed();
				if (keyword == QString::fromLatin1("include-text")) {
					sIncludeTextCommand = value.replace(QString::fromLatin1("#RET#"), QChar::fromLatin1('\n'));
					continue;
				}
				if (keyword == QString::fromLatin1("include-pdf")) {
					sIncludePdfCommand = value.replace(QString::fromLatin1("#RET#"), QChar::fromLatin1('\n'));
					continue;
				}
				if (keyword == QString::fromLatin1("include-image")) {
					sIncludeImageCommand = value.replace(QString::fromLatin1("#RET#"), QChar::fromLatin1('\n'));
					continue;
				}
				if (keyword == QString::fromLatin1("include-postscript")) {
					sIncludePostscriptCommand = value.replace(QString::fromLatin1("#RET#"), QChar::fromLatin1('\n'));
					continue;
				}
				if (keyword == QString::fromLatin1("cleanup-patterns")) {
					static bool first = true;
					if (first) {
						sCleanupPatterns = value;
						first = false;
					}
					else {
						sCleanupPatterns += QChar::fromLatin1(' ');
						sCleanupPatterns += value;
					}
					continue;
				}
				if (keyword == QString::fromLatin1("file-open-filter")) {
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
		if (!openerMatching(c).isNull())
			pos = (direction < 0) ? balanceDelim(text, pos - 1, openerMatching(c), -1) : -1;
		else if (!closerMatching(c).isNull())
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
		if (!closerMatching(c).isNull())
			return pos;
	}
	return -1;
}

void TWUtils::installCustomShortcuts(QWidget * widget, bool recursive /* = true */, QSettings * map /* = nullptr */)
{
	bool deleteMap = false;
	
	if (!widget)
		return;

	if (!map) {
		QString filename = QDir(TWUtils::getLibraryPath(QString::fromLatin1("configuration"))).absoluteFilePath(QString::fromLatin1("shortcuts.ini"));
		if (filename.isEmpty() || !QFileInfo(filename).exists())
			return;

		map = new QSettings(filename, QSettings::IniFormat);
		if (map->status() != QSettings::NoError) {
			delete map;
			return;
		}
		deleteMap = true;
	}

	foreach (QAction * act, widget->actions()) {
		if (act->objectName().isEmpty())
			continue;
		if (map->contains(act->objectName()))
			act->setShortcut(QKeySequence(map->value(act->objectName()).toString()));
	}
	
	if (recursive) {
		foreach (QObject * obj, widget->children()) {
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
#if defined(MIKTEX)
  return false;
#else
	return (!QString::fromLatin1(GIT_COMMIT_HASH).startsWith(QString::fromLatin1("$Format:")) && !QString::fromLatin1(GIT_COMMIT_DATE).startsWith(QString::fromLatin1("$Format:")));
#endif
}

// static
QString TWUtils::gitCommitHash()
{
#if defined(MIKTEX)
  return "deadbeaf";
#else
	if(QString::fromLatin1(GIT_COMMIT_HASH).startsWith(QString::fromLatin1("$Format:")))
		return QString();
	return QString::fromLatin1(GIT_COMMIT_HASH);
#endif
}

// static
QDateTime TWUtils::gitCommitDate()
{
#if defined(MIKTEX)
  MIKTEX_UNEXPECTED();
#else
  if (QString::fromLatin1(GIT_COMMIT_DATE).startsWith(QString::fromLatin1("$Format:")))
		return QDateTime();
	return QDateTime::fromString(QString::fromLatin1(GIT_COMMIT_DATE), Qt::ISODate).toUTC();
#endif
}

// action subclass used for dynamic window-selection items in the Window menu

SelWinAction::SelWinAction(QObject *parent, const QString &fileName, const QString &label)
	: QAction(parent)
{
	setText(label);
	setData(fileName);
}

// on OS X only, the singleton CmdKeyFilter object is attached to all TeXDocument editor widgets
// to stop Command-keys getting inserted into edit text items

CmdKeyFilter *CmdKeyFilter::filterObj = nullptr;

CmdKeyFilter *CmdKeyFilter::filter()
{
	if (!filterObj)
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

