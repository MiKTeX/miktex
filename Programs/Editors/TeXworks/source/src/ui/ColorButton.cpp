/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2022  Stefan Löffler

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
#include "ColorButton.h"

#include <QColorDialog>
#include <QStyle>

namespace Tw {
namespace UI {

ColorButton::ColorButton(const QColor &initialColor, QWidget *parent)
	: QToolButton(parent)
{
	setColor(initialColor);
	connect(this, &QToolButton::clicked, this, &ColorButton::chooseColor);
}

void ColorButton::setColor(const QColor &newColor)
{
	if (!newColor.isValid()) {
		return;
	}
	m_color = newColor;

	const int iconSize = style()->pixelMetric(QStyle::PM_LargeIconSize);

	QPixmap pixmap{QSize(iconSize, iconSize)};
	pixmap.fill(newColor);

	setIcon(QIcon(pixmap));

	emit colorChanged(newColor);
}

void ColorButton::chooseColor()
{
	QColor color = QColorDialog::getColor(m_color, this);
	if (color.isValid()) {
		setColor(color);
	}
}

} // namespace UI
} // namespace Tw
