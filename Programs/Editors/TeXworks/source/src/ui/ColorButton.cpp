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
