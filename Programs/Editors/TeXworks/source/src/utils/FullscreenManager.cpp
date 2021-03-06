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

#include "utils/FullscreenManager.h"

#include <QMenuBar>
#include <QMouseEvent>
#include <QStatusBar>
#include <QToolBar>

namespace Tw {
namespace Utils {

FullscreenManager::FullscreenManager(QMainWindow * parent)
	: _parent(parent)
{
	if (parent) {
		QShortcut * esc = new QShortcut(QKeySequence(Qt::Key_Escape), parent);
		Q_ASSERT(esc != nullptr);
		esc->setEnabled(false);
		connect(esc, &QShortcut::activated, this, &FullscreenManager::toggleFullscreen);
		_shortcuts << shortcut_info{esc, nullptr};
	}

	_menuBarTimer.setSingleShot(true);
	_menuBarTimer.setInterval(500);
	connect(&_menuBarTimer, &QTimer::timeout, this, &FullscreenManager::showMenuBar);
}

//virtual
FullscreenManager::~FullscreenManager()
{
	foreach (const shortcut_info & sci, _shortcuts)
		delete sci.shortcut;
}

void FullscreenManager::setFullscreen(const bool fullscreen /* = true */)
{
	if (!_parent) return;
	if (fullscreen == isFullscreen()) return;

	if (fullscreen) {
		// entering full-screen mode

		// store the visibilities of important widgets such as the menu bar
		// before hiding them (so we can restore them when exiting fullscreen
		// mode)
		_normalVisibility.clear();
		if (_parent->menuBar()) {
			_normalVisibility[_parent->menuBar()] = _parent->menuBar()->isVisible();
			hideMenuBar();
		}
		if (_parent->statusBar()) {
			_normalVisibility[_parent->statusBar()] = _parent->statusBar()->isVisible();
			_parent->statusBar()->hide();
		}
		foreach (QToolBar * tb, _parent->findChildren<QToolBar*>()) {
			_normalVisibility[tb] = tb->isVisible();
			tb->hide();
		}

		_parent->showFullScreen();

		// Enable custom shortcuts
		foreach (const shortcut_info & sci, _shortcuts) {
			// Skip shortcuts that are associated with a menu QAction; those are
			// enabled/disabled when the menubar is hidden/shown
			if (sci.action) continue;
			sci.shortcut->setEnabled(fullscreen);
		}
	}
	else {
		// exiting full-screen mode
		// stop the timer, just in case
		_menuBarTimer.stop();

		_parent->showNormal();

		// restore visibilities
		foreach (QWidget * w, _normalVisibility.keys())
			w->setVisible(_normalVisibility[w]);

		// Disable custom shortcuts
		foreach (const shortcut_info & sci, _shortcuts)
			sci.shortcut->setEnabled(false);
	}

	// Enable/disable mouse tracking (which is required for getting mouse move
	// events when no mouse button is pressed) to be able to show/hide the
	// menu bar depending on the mouse position
	_parent->setMouseTracking(fullscreen);

	emit fullscreenChanged(fullscreen);
}

bool FullscreenManager::isFullscreen() const
{
	if (!_parent) return false;
	return _parent->windowState().testFlag(Qt::WindowFullScreen);
}

void FullscreenManager::toggleFullscreen()
{
	setFullscreen(!isFullscreen());
}

void FullscreenManager::mouseMoveEvent(QMouseEvent * event)
{
	if (!_parent || !_parent->menuBar() || !isFullscreen()) return;
	const int thresholdHeight = 10;

	// The menu bar is shown when the mouse stays within thresholdHeight
	// from the top of the screen for _menuBarTimer.interval()
	// When the mouse moves in(to) that area and the timer is not running,
	// start it
	if (!_parent->menuBar()->isVisible()) {
		if (event->pos().y() <= thresholdHeight && !_menuBarTimer.isActive()) _menuBarTimer.start();
		// When the mouse moves out(side) of that area and the timer is running,
		// stop it
		else if (event->pos().y() > thresholdHeight && _menuBarTimer.isActive()) _menuBarTimer.stop();
	}
	// The menu bar is hidden whenever the mouse moves off of the menu bar
	// (Note: when opening a menu, that menu intercepts all mouse events so we
	// don't need to worry about the menu bar disappearing while the user
	// browses through the menus.
	else if (event->pos().y() > _parent->menuBar()->height())
		hideMenuBar();
}

void FullscreenManager::addShortcut(QAction * action, const char * member)
{
	addShortcut(action->shortcut(), member, action);
}

void FullscreenManager::addShortcut(const QKeySequence & key, const char * member, QAction * action /* = nullptr */)
{
	if (!_parent) {
		return;
	}

	shortcut_info sci;
	sci.shortcut = new QShortcut(key, _parent, member);
	sci.shortcut->setEnabled(false);
	sci.action = action;
	if (action)
		connect(action, &QAction::destroyed, this, &FullscreenManager::actionDeleted);
	_shortcuts << sci;
}

void FullscreenManager::actionDeleted(QObject * obj)
{
	// Do not try to qobject_cast obj to QAction* as the object is already
	// (mostly) destroyed, so the cast would fail
	for (int i = 0; i < _shortcuts.size(); ) {
		if (_shortcuts[i].action == obj) {
			delete _shortcuts[i].shortcut;
			_shortcuts.removeAt(i);
		}
		else
			++i;
	}
}

void FullscreenManager::setMenuBarVisible(const bool visible /* = true */)
{
	if (!_parent || !_parent->menuBar()) return;
	if (visible == _parent->menuBar()->isVisible()) return;

	_parent->menuBar()->setVisible(visible);

	// Enable our shortcut overrides when the menubar is hidden (to ensure that
	// the most important shortcuts are available even when the corresponding
	// QActions are hidden and disabled)
	foreach (const shortcut_info & sci, _shortcuts) {
		// Skip shortcuts that are not associated with any menu QAction
		if (!sci.action) continue;

		// If the shortcut gets enabled and corresponds to a valid, named
		// QAction, update it to the QAction's key sequence (in case that
		// got changed in the meantime)
		if (!visible)
			sci.shortcut->setKey(sci.action->shortcut());
		sci.shortcut->setEnabled(!visible);
	}
}

} // namespace Utils
} // namespace Tw
