/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2012-2020  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "ui/ClosableTabWidget.h"

#include <QStyle>
#include <QTabBar>

namespace Tw {
namespace UI {

ClosableTabWidget::ClosableTabWidget(QWidget * parent /* = nullptr */)
 : QTabWidget(parent)
{
	_closeButton = new QToolButton(this);
	Q_ASSERT(_closeButton);
	_closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
	_closeButton->setCursor(Qt::ArrowCursor);
//	_closeButton->setStyleSheet(QString::fromUtf8("QToolButton { border: none; padding: 0px; }"));
	connect(_closeButton, &QToolButton::clicked, this, &ClosableTabWidget::requestClose);
}

void ClosableTabWidget::resizeEvent(QResizeEvent * e)
{
	// Let the QTabWidget handle proper resizing of the widgets inside the tabs
	QTabWidget::resizeEvent(e);

	// Position the close button on the right
	Q_ASSERT(_closeButton);
	QSize b = _closeButton->sizeHint();
	_closeButton->move(rect().right() - b.width(), 0);

	// Ensure that the tab bar is small enough not to overlap the close button
	Q_ASSERT(tabBar());
	tabBar()->setMaximumWidth(rect().right() - b.width());
}

} // namespace UI
} // namespace Tw
