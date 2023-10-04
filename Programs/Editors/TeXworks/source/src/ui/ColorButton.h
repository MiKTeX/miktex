#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QToolButton>

namespace Tw {
namespace UI {

class ColorButton : public QToolButton
{
	Q_OBJECT
	QColor m_color;
public:
	ColorButton(const QColor & initialColor, QWidget *parent = nullptr);
	ColorButton(QWidget *parent = nullptr) : ColorButton(Qt::white, parent) { }

	QColor color() const { return m_color; }
	void setColor(const QColor & newColor);

	void chooseColor();
signals:
	void colorChanged(QColor newColor);
};

} // namespace UI
} // namespace Tw

#endif // COLORBUTTON_H
