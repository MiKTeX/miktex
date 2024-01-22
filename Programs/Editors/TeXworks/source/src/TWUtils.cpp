/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2022  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "PDFDocumentWindow.h"
#include "Settings.h"
#include "TWApp.h"
#include "TeXDocumentWindow.h"
#include "utils/ResourcesLibrary.h"
#include "utils/WindowManager.h"

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
        MiKTeX::Util::PathName path = session->GetSpecialPath(MiKTeX::Configuration::SpecialPath::DistRoot) / "doc" / "texworks" / "help";
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
#ifdef Q_OS_WIN
	// It seems (contrary to documentation) that on Windows, this has to be
	// *.* to allow saving files with non-standard extensions (though *.* still
	// does not allow to save without any extension at all, see below)
	const QString allFilesFilter = QStringLiteral("*.*");
//	const QString allFilesFilter = QStringLiteral("*");
#else
	// On other systems, *.* might require a . in the filename, which would
	// preclude filenames without extension. In line with the documentation, *
	// should be used in those cases
	const QString allFilesFilter = QStringLiteral("*");
#endif
	*filters << QObject::tr("All files") + QStringLiteral(" (%1)").arg(allFilesFilter);
}

/*static*/
QString TWUtils::chooseDefaultFilter(const QString & filename, const QStringList & filters)
{
	QString extension = QFileInfo(filename).completeSuffix();

	if (extension.isEmpty())
		return filters.last();

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
	labelList = Tw::Utils::WindowManager::constructUniqueFileLabels(fileList);

	QStringList::size_type numRecentFiles = fileList.size();

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

	for (QStringList::size_type i = 0; i < numRecentFiles; ++i) {
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

QString::size_type TWUtils::balanceDelim(const QString& text, QString::size_type pos, QChar delim, int direction)
{
	QString::size_type len = text.length();
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

QString::size_type TWUtils::findOpeningDelim(const QString& text, QString::size_type pos)
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
