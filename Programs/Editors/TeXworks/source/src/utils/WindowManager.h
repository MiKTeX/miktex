/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2022  Jonathan Kew, Stefan Löffler

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

#ifndef utils_WindowManager_H
#define utils_WindowManager_H

#include <QMenu>
#include <QString>
#include <QWidget>

class TeXDocumentWindow;
class PDFDocumentWindow;

namespace Tw {

namespace Utils {

class WindowManager
{
public:
	// return just the filename from a full pathname, suitable for UI display
	static QString strippedName(const QString &fullFileName, const unsigned int dirComponents = 0);

	// update the internal list of open windows (and their unique labels)
	static void updateWindowList(const QList<TeXDocumentWindow*> & texDocs, const QList<PDFDocumentWindow*> & pdfDocs);

	// update the SelWinActions in a menu, used by the documents
	static void updateWindowMenu(QWidget *window, QMenu *menu);

	// return a list of file labels suitable for UI display that uniquely
	// describe the given filenames
	static QStringList constructUniqueFileLabels(const QStringList & fileList);

	static QString uniqueLabelForFile(const QString & filename);

	static void tileWindowsInRect(const QWidgetList& windows, const QRect& bounds);
	static void stackWindowsInRect(const QWidgetList& windows, const QRect& bounds);

	static void zoomToScreen(QWidget *window);
	static void zoomToHalfScreen(QWidget *window, bool rhs = false);
	static void sideBySide(QWidget *window1, QWidget *window2);
	static void ensureOnScreen(QWidget *window);
};

} // namespace Utils

} // namespace Tw

#endif // !defined(utils_WindowManager_H)
