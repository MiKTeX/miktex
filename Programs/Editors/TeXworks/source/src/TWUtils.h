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

#ifndef TWUtils_H
#define TWUtils_H

#include <QAction>
#include <QDateTime>
#include <QDir>
#include <QList>
#include <QMap>
#include <QPair>
#include <QSettings>
#include <QString>
#include <QTextCodec>
#if defined(MIKTEX)
#  include <miktex/Core/Directory>
#  include <miktex/Core/Paths>
#  include <miktex/Core/Session>
#  include <miktex/Core/Utils>
#  if defined(MIKTEX_WINDOWS)
#    include <shlobj.h>
#  endif
#endif

#define TEXWORKS_NAME "TeXworks" /* app name, for use in menus, messages, etc */

class QCompleter;
class QMainWindow;
class PDFDocumentWindow;
class TeXDocumentWindow;

// static utility methods
class TWUtils
{
public:
	static void insertHelpMenuItems(QMenu* helpMenu);

	// return a sorted list of all the available text codecs
	static QList<QTextCodec*> *findCodecs();

	// list of filename filters for the Open/Save dialogs
	static QStringList* filterList();
	static void setDefaultFilters();
	static QString chooseDefaultFilter(const QString & filename, const QStringList & filters);

	// perform the updates to a menu; used by the documents to update their own menus
	static void updateRecentFileActions(QObject *parent, QList<QAction*> &actions, QMenu *menu, QAction * clearAction);

	// update the SelWinActions in a menu, used by the documents
	static void updateWindowMenu(QWidget *window, QMenu *menu);

	// return just the filename from a full pathname, suitable for UI display
	static QString strippedName(const QString &fullFileName, const unsigned int dirComponents = 0);

	// return a list of file labels suitable for UI display that uniquely
	// describe the given filenames
	static QStringList constructUniqueFileLabels(const QStringList & fileList);

	static void tileWindowsInRect(const QWidgetList& windows, const QRect& bounds);
	static void stackWindowsInRect(const QWidgetList& windows, const QRect& bounds);

	static void zoomToScreen(QWidget *window);
	static void zoomToHalfScreen(QWidget *window, bool rhs = false);
	static void sideBySide(QWidget *window1, QWidget *window2);
	static void ensureOnScreen(QWidget *window);
	static void applyToolbarOptions(QMainWindow *theWindow, int iconSize, bool showText);

	static QChar closerMatching(QChar c);
	static QChar openerMatching(QChar c);
	static void readConfig();

	static int balanceDelim(const QString& text, int pos, QChar delim, int direction);
	static int findOpeningDelim(const QString& text, int pos);

	static const QString& includeTextCommand();
	static const QString& includePdfCommand();
	static const QString& includeImageCommand();
	static const QString& includePostscriptCommand();

	static const QString& cleanupPatterns();

	static void installCustomShortcuts(QWidget * widget, bool recursive = true, QSettings * map = nullptr);

private:
	TWUtils();

	static QList<QTextCodec*>		*codecList;
	static QStringList				*translationList;

	static QStringList			*filters;

	static QMap<QChar,QChar>	pairOpeners;
	static QMap<QChar,QChar>	pairClosers;

	static QString sIncludeTextCommand;
	static QString sIncludePdfCommand;
	static QString sIncludeImageCommand;
	static QString sIncludePostscriptCommand;
	static QString sCleanupPatterns;
};

// this special QAction class is used in Window menus, so that it's easy to recognize the dynamically-created items
class SelWinAction : public QAction
{
	Q_OBJECT

public:
	SelWinAction(QObject *parent, const QString & fileName, const QString &label);
};

// filter used to stop Command-keys getting inserted into edit text items
// (only used on Mac OS X)
class CmdKeyFilter: public QObject
{
	Q_OBJECT

public:
	static CmdKeyFilter *filter();

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;

private:
	static CmdKeyFilter *filterObj;
};


#endif
