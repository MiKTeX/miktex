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

#include "WindowManager.h"

#include "PDFDocumentWindow.h"
#include "TeXDocumentWindow.h"
#include "Settings.h"

#include <QDir>
#include <utility>

namespace {

static QList< std::pair<TeXDocumentWindow*, QString> > texFileLabels;
static QList< std::pair<PDFDocumentWindow*, QString> > pdfFileLabels;

// this special QAction class is used in Window menus, so that it's easy to recognize the dynamically-created items
class SelWinAction : public QAction
{
	Q_OBJECT
public:
	SelWinAction(QObject *parent, const QString & fileName, const QString &label)
		: QAction(parent)
	{
		setText(label);
		setData(fileName);
	}
};

}

namespace Tw {

namespace Utils {

QString WindowManager::strippedName(const QString &fullFileName, const unsigned int dirComponents /* = 0 */)
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

void WindowManager::updateWindowList(const QList<TeXDocumentWindow*> & texDocs, const QList<PDFDocumentWindow*> & pdfDocs)
{
	{
		QStringList fileNames;

		for(TeXDocumentWindow * doc : texDocs) {
			fileNames.append(doc->fileName());
		}
		const QStringList fileLabels = constructUniqueFileLabels(fileNames);

		texFileLabels.clear();
		for (int i = 0; i < fileNames.size(); ++i) {
			texFileLabels.push_back(std::make_pair(texDocs[i], fileLabels[i]));
		}
	}
	{
		QStringList fileNames;

		for(PDFDocumentWindow * doc : pdfDocs) {
			fileNames.append(doc->fileName());
		}
		const QStringList fileLabels = constructUniqueFileLabels(fileNames);

		pdfFileLabels.clear();
		for (int i = 0; i < fileNames.size(); ++i) {
			pdfFileLabels.push_back(std::make_pair(pdfDocs[i], fileLabels[i]));
		}
	}
}

QStringList WindowManager::constructUniqueFileLabels(const QStringList & fileList)
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

QString WindowManager::uniqueLabelForFile(const QString &filename)
{
	for (const auto & p : texFileLabels) {
		const TeXDocumentWindow * doc = p.first;
		const QString & label = p.second;
		if (doc && doc->fileName() == filename) {
			return label;
		}
	}
	for (const auto & p : pdfFileLabels) {
		const PDFDocumentWindow * doc = p.first;
		const QString & label = p.second;
		if (doc && doc->fileName() == filename) {
			return label;
		}
	}
	return {};
}

void WindowManager::updateWindowMenu(QWidget *window, QMenu *menu) /* static */
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
	for (const auto & p : texFileLabels) {
		TeXDocumentWindow * texDoc = p.first;
		const QString & label = p.second;

		if (!texDoc) {
			continue;
		}
		if (first && !menu->actions().isEmpty()) {
			menu->addSeparator();
			first = false;
		}
		SelWinAction *selWin = new SelWinAction(menu, texDoc->fileName(), label);
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

	// append an item for each PDFDocument
	first = true;
	for (const auto & p : pdfFileLabels) {
		PDFDocumentWindow * pdfDoc = p.first;
		const QString & label = p.second;

		if (!pdfDoc) {
			continue;
		}
		if (first && !menu->actions().isEmpty()) {
			menu->addSeparator();
			first = false;
		}
		SelWinAction *selWin = new SelWinAction(menu, pdfDoc->fileName(), label);
		if (pdfDoc == qobject_cast<PDFDocumentWindow*>(window)) {
			selWin->setCheckable(true);
			selWin->setChecked(true);
		}
		// Don't use a direct connection as triggered has a boolean argument
		// (checked) which would get forwarded to selectWindow's "activate",
		// which doesn't make sense.
		QObject::connect(selWin, &SelWinAction::triggered, pdfDoc, [pdfDoc]() { pdfDoc->selectWindow(); });
		menu->addAction(selWin);
	}
}

void WindowManager::ensureOnScreen(QWidget *window)
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

void WindowManager::zoomToScreen(QWidget *window)
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

void WindowManager::zoomToHalfScreen(QWidget *window, bool rhs)
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

void WindowManager::sideBySide(QWidget *window1, QWidget *window2)
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

void WindowManager::tileWindowsInRect(const QWidgetList& windows, const QRect& bounds)
{
	QWidgetList::size_type numWindows = windows.count();
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

void WindowManager::stackWindowsInRect(const QWidgetList& windows, const QRect& bounds)
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

} // namespace Utils

} // namespace Tw

#include "WindowManager.moc"
