/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2020  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef FullscreenManager_H
#define FullscreenManager_H

#include <QMainWindow>
#include <QMap>
#include <QShortcut>
#include <QTimer>

namespace Tw {
namespace Utils {

class FullscreenManager : public QObject
{
	Q_OBJECT
public:
	FullscreenManager(QMainWindow * parent);
	~FullscreenManager() override;

	bool isFullscreen() const;
	void mouseMoveEvent(QMouseEvent * event);

	void addShortcut(QAction * action, const char * member);
	void addShortcut(const QKeySequence & key, const char * member, QAction * action = nullptr);

public slots:
	void setFullscreen(const bool fullscreen = true);
	void toggleFullscreen();

signals:
	void fullscreenChanged(bool fullscreen);

private slots:
	void showMenuBar() { setMenuBarVisible(true); }
	void hideMenuBar() { setMenuBarVisible(false); }
	void actionDeleted(QObject * obj);

protected:
	void setMenuBarVisible(const bool visible = true);

	struct shortcut_info {
		QShortcut * shortcut;
		QAction * action;
	};

	QList<shortcut_info> _shortcuts;
	QMap<QWidget*, bool> _normalVisibility;
	QMainWindow * _parent;
	QTimer _menuBarTimer;
};

} // namespace Utils
} // namespace Tw

#endif // !defined(FullscreenManager)
