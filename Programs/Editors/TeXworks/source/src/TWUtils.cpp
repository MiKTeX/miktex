/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2021  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#if defined(MIKTEX)
#if defined(MIKTEX_WINDOWS)
#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>
#endif
#include <miktex/miktex-texworks.hpp>
#include <miktex/Trace/StopWatch>
#endif
#include "TWUtils.h"

#include "GitRev.h"
#include "PDFDocumentWindow.h"
#include "Settings.h"
#include "TWApp.h"
#include "TeXDocumentWindow.h"
#include "utils/FileVersionDatabase.h"
#include "utils/ResourcesLibrary.h"
#include "utils/VersionInfo.h"

#include <QAction>
#include <QCompleter>
#include <QDateTime>
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QDesktopWidget>
#endif
#include <QDirIterator>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QSignalMapper>
#include <QString>
#include <QStringList>
#include <QTextCodec>

#if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
#ifndef TW_HELPPATH
#define TW_HELPPATH "/usr/local/share/texworks-help"
#endif
#endif

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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			titleStream.setCodec("UTF-8");
#endif
			title = titleStream.readLine();
		}
		QAction* action = new QAction(title, helpMenu);
		mapper->setMapping(action, fi.canonicalFilePath());
		QObject::connect(action, &QAction::triggered, mapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
		helpMenu->insertAction(before, action);
		return 1;
	}
	return 0;
}

void TWUtils::insertHelpMenuItems(QMenu* helpMenu)
{
	QSignalMapper* mapper = new QSignalMapper(helpMenu);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	QObject::connect(mapper, static_cast<void (QSignalMapper::*)(const QString&)>(&QSignalMapper::mapped), TWApp::instance(), &TWApp::openHelpFile);
#else
	QObject::connect(mapper, &QSignalMapper::mappedString, TWApp::instance(), &TWApp::openHelpFile);
#endif

	QAction* before = nullptr;
	int i{0}, firstSeparator = 0;
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
        std::shared_ptr<MiKTeX::Core::Session> session = MIKTEX_SESSION();
        MiKTeX::Util::PathName path = session->GetSpecialPath(MiKTeX::Configuration::SpecialPath::DistRoot) / MiKTeX::Util::PathName("doc/texworks/help");
        if (MiKTeX::Core::Directory::Exists(path))
        {
          helpDir = QString::fromUtf8(path.GetData());
        }
        else
        {
          helpDir = QCoreApplication::applicationDirPath() + QStringLiteral("/texworks-help");
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

	foreach (QByteArray name, QTextCodec::availableCodecs()) {
		QTextCodec * codec = QTextCodec::codecForName(name);
		QString sortKey = QString::fromUtf8(codec->name().constData()).toUpper();
		QRegularExpressionMatch iso8859Match = iso8859RegExp.match(sortKey);
		int rank{5};
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
		sortKey.prepend(QChar('0' + rank));
		// Add the codec if it is not already in the list
		// (NB: QTextCodec::availableCodecs() lists all aliases separately)
		if (!codecMap.contains(sortKey)) {
			codecMap.insert(sortKey, codec);
		}
	}
	*codecList = codecMap.values();
	return codecList;
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
		if (dir.isRoot()) {
			// If we moved up to the root directory, there is no point in going
			// any further; particularly on Windows, going further may produce
			// invalid paths (such as C:\.. which make no sense and can result
			// in infinite loops in constructUniqueFileLabels()
			return fullFileName;
		}
		// NB: dir.cdUp() would be more logical, but fails if the resulting
		// path does not exist
		// NB: QDir::cleanPath resolves .. such as the one we deliberately
		// introduce using string operations (i.e., without file system access)
		// Avoiding file system access is important in case the path refers to
		// a slow (or non-existent) network share, which would cause the program
		// to hang until the file system access times out (which can accumulate
		// to a very long time if this function has to be called repeatedly)
		dir.setPath(QDir::cleanPath(dir.path() + QString::fromLatin1("/..")));
	}
	return dir.relativeFilePath(fullFileName);
}

QStringList TWUtils::constructUniqueFileLabels(const QStringList & fileList)
{
	QStringList labelList;

	for (const QString & file : fileList) {
		labelList.append(strippedName(file));
	}

	// Make label list unique, i.e. while labels are not unique, add
	// directory components
	bool done{false};
	for (unsigned int dirComponents = 1; !done; ++dirComponents) {
		QList<bool> isDuplicate;
		for (const QString & label : labelList) {
			isDuplicate.append(labelList.count(label) > 1);
		}
		if (!isDuplicate.contains(true))
			break;

		done = true;
		for (int i = 0; i < labelList.size(); ++i) {
			if (!isDuplicate[i])
				continue;
			const QString newName = strippedName(fileList[i], dirComponents);
			if (labelList[i] != newName) {
				labelList[i] = newName;
				done = false;
			}
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
		QObject::connect(act, &QAction::triggered, TWApp::instance(), &TWApp::openRecentFile);
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

		actions[i]->setText(QDir::toNativeSeparators(labelList[i]));
		actions[i]->setData(fileList[i]);
		actions[i]->setToolTip(QDir::toNativeSeparators(fileList[i]));
		actions[i]->setVisible(true);
	}
	menu->setToolTipsVisible(true);

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
		// Don't use a direct connection as triggered has a boolean argument
		// (checked) which would get forwarded to selectWindow's "activate",
		// which doesn't make sense.
		QObject::connect(selWin, &SelWinAction::triggered, texDoc, [texDoc](){ texDoc->selectWindow(); });
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
		QObject::connect(selWin, &SelWinAction::triggered, pdfDoc, &PDFDocumentWindow::selectWindow);
		menu->addAction(selWin);
	}
}

void TWUtils::ensureOnScreen(QWidget *window)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
	QDesktopWidget *desktop = QApplication::desktop();
	QRect screenRect = desktop->availableGeometry(window);
#else
	QRect screenRect = window->screen()->availableGeometry();
#endif
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
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
	QDesktopWidget *desktop = QApplication::desktop();
	QRect screenRect = desktop->availableGeometry(window);
#else
	QRect screenRect = window->screen()->availableGeometry();
#endif
	screenRect.setTop(screenRect.top() + window->geometry().y() - window->y());
	window->setGeometry(screenRect);
}

void TWUtils::zoomToHalfScreen(QWidget *window, bool rhs)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
	QDesktopWidget *desktop = QApplication::desktop();
	QRect r = desktop->availableGeometry(window);
#else
	QRect r = window->screen()->availableGeometry();
#endif
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
	// if the windows reside on the same screen zoom each so that it occupies
	// half of that screen
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
	QDesktopWidget *desktop = QApplication::desktop();
	if (desktop->screenNumber(window1) == desktop->screenNumber(window2)) {
#else
	if (window1->screen() == window2->screen()) {
#endif
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

	QDir configDir(Tw::Utils::ResourcesLibrary::getLibraryPath(QStringLiteral("configuration")));
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
		QString filename = QDir(Tw::Utils::ResourcesLibrary::getLibraryPath(QStringLiteral("configuration"))).absoluteFilePath(QString::fromLatin1("shortcuts.ini"));
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

