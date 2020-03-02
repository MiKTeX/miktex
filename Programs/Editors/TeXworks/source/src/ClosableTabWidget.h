/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2008-2019  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef __CLOSABLE_TAB_WIDGET_H
#define __CLOSABLE_TAB_WIDGET_H

#include <QTabWidget>
#include <QToolButton>

// The ClosableTabWidget class is adapted from code presented by Girish
// Ramakrishnan in a Qt Labs post:
//
//   http://labs.qt.nokia.com/2007/06/06/lineedit-with-a-clear-button
class ClosableTabWidget : public QTabWidget
{
	Q_OBJECT
public:
	ClosableTabWidget(QWidget * parent = nullptr);
	~ClosableTabWidget() override = default;

signals:
	void requestClose();

protected:
	void resizeEvent(QResizeEvent * e) override;

	QToolButton * _closeButton;
};

#endif // !defined(__CLOSABLE_TAB_WIDGET_H)
