/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2013-2020  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "ClickableLabel.h"

#include <QApplication>

namespace Tw {
namespace UI {

ClickableLabel::ClickableLabel(QWidget * parent, Qt::WindowFlags f)
: QLabel(parent, f)
{
}

ClickableLabel::ClickableLabel(const QString & text, QWidget * parent, Qt::WindowFlags f)
: QLabel(text, parent, f)
{
}

// virtual
void ClickableLabel::mouseDoubleClickEvent(QMouseEvent * event)
{
	emit mouseDoubleClick(event);
}

// virtual
void ClickableLabel::mousePressEvent(QMouseEvent * event)
{
	mouseStartPoint = event->pos();
}

// virtual
void ClickableLabel::mouseReleaseEvent(QMouseEvent * event)
{
	if ((event->pos() - mouseStartPoint).manhattanLength() < QApplication::startDragDistance()) {
		switch (event->button()) {
			case Qt::LeftButton:
				emit mouseLeftClick(event);
				break;
			case Qt::RightButton:
				emit mouseRightClick(event);
				break;
			case Qt::MiddleButton:
				emit mouseMiddleClick(event);
				break;
			default:
				break;
		}
	}
}

} // namespace UI
} // namespace Tw
