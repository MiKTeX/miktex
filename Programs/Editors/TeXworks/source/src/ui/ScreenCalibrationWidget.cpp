/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2016-2020  Stefan Löffler

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

#include "ui/ScreenCalibrationWidget.h"

#include <QApplication>
#include <QDoubleSpinBox>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>

namespace Tw {
namespace UI {

ScreenCalibrationWidget::ScreenCalibrationWidget(QWidget * parent)
	: QWidget(parent)
	, _contextMenuActionGroup(this)
{
	_sbDPI = new QDoubleSpinBox(this);
	_sbDPI->setRange(0, 9999);
	_sbDPI->setValue(physicalDpiX());
	_sbDPI->installEventFilter(this);
	connect(_sbDPI, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, static_cast<void (ScreenCalibrationWidget::*)()>(&ScreenCalibrationWidget::repaint));
	connect(_sbDPI, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &ScreenCalibrationWidget::dpiChanged);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	connect(&_unitSignalMapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped), this, &ScreenCalibrationWidget::setUnit);
#else
	connect(&_unitSignalMapper, &QSignalMapper::mappedInt, this, &ScreenCalibrationWidget::setUnit);
#endif

	setToolTip(tr("Drag the ruler or change the value to match real world lengths.\nCommon paper sizes are marked as well (you may need to resize the dialog window to see them).\nUse the context menu to change the units."));

	switch(locale().measurementSystem()) {
		case QLocale::ImperialUSSystem:
		case QLocale::ImperialUKSystem:
			_curUnit = 1;
			break;
		default:
			_curUnit = 0;
	}

	retranslate();
	recalculateSizes();
}

void ScreenCalibrationWidget::recalculateSizes()
{
	_majorTickHeight = qRound(1.2 * fontMetrics().lineSpacing());
	_mediumTickHeight = qRound(0.5 * _majorTickHeight);
	_minorTickHeight = qRound(0.25 * _majorTickHeight);
	_paperTickHeight = qRound(2.2 * fontMetrics().lineSpacing());
	_hSpace = style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);
	if (_hSpace < 0)
		_hSpace = style()->layoutSpacing(QSizePolicy::SpinBox, QSizePolicy::DefaultType, Qt::Horizontal);
	setMinimumHeight(static_cast<int>(_paperTickHeight) + qRound(0.2 * fontMetrics().lineSpacing()));
}

void ScreenCalibrationWidget::retranslate()
{
	Q_ASSERT(_sbDPI);

	//: dots per inch
	_sbDPI->setSuffix(tr(" dpi"));

	_paperSizes.clear();
	_paperSizes.append({tr("DIN A4"), QSizeF(8.26772, 11.69291), Qt::blue, Qt::AlignRight, true});
	_paperSizes.append({tr("Letter"), QSizeF(8.5, 11), Qt::red, Qt::AlignLeft, true});

	_units.clear();
	//: this refers to the length unit of centimeters
	_units.append({tr("cm"), 0.393701f});
	//: this refers to the length unit of inches
	_units.append({tr("in"), 1});

	_contextMenu.clear();
	QAction * a{nullptr};
	a = _contextMenu.addAction(tr("cm"));
	a->setCheckable(true);
	a->setChecked(_curUnit == 0);
	_unitSignalMapper.setMapping(a, 0);
	connect(a, &QAction::triggered, &_unitSignalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
	_contextMenuActionGroup.addAction(a);
	a = _contextMenu.addAction(tr("in"));
	a->setCheckable(true);
	a->setChecked(_curUnit == 1);
	_unitSignalMapper.setMapping(a, 1);
	connect(a, &QAction::triggered, &_unitSignalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
	_contextMenuActionGroup.addAction(a);
}

void ScreenCalibrationWidget::repositionSpinBox()
{
	Q_ASSERT(_sbDPI);

	// Find the layout direction (search recursively through the widget
	// hierarchy)
	Qt::LayoutDirection layoutDirection{Qt::LayoutDirectionAuto};
	QWidget * w = this;
	while (layoutDirection == Qt::LayoutDirectionAuto && w) {
		layoutDirection = w->layoutDirection();
		w = w->parentWidget();
	}
	// If none of the widgets had a specific layout direction, fall back to the
	// QApplication value
	if (layoutDirection == Qt::LayoutDirectionAuto)
		layoutDirection = QApplication::layoutDirection();

	_sbDPI->ensurePolished();
	switch (layoutDirection) {
		case Qt::LeftToRight:
			// Ensure the spin box is positioned on the left side and at the top
			_sbDPI->move(0, 0);
			_rulerRect = QRect(QPoint(_sbDPI->width() + _hSpace, 0), QPoint(width() - 2, height() - 2));
			break;
		case Qt::RightToLeft:
			// Ensure the spin box is positioned on the right side and at the top
			_sbDPI->move(width() - _sbDPI->width(), 0);
			_rulerRect = QRect(QPoint(0, 0), QPoint(width() - _sbDPI->width() - _hSpace - 2, height() - 2));
			break;
		case Qt::LayoutDirectionAuto:
			// This should not happen as we resolved the layout direction above
			break;
	}
}

double ScreenCalibrationWidget::dpi() const
{
	Q_ASSERT(_sbDPI);
	return _sbDPI->value();
}

void ScreenCalibrationWidget::setDpi(const double dpi)
{
	Q_ASSERT(_sbDPI);
	_sbDPI->setValue(dpi);
}

void ScreenCalibrationWidget::setUnit(const int unitIdx)
{
	if (unitIdx < 0 || unitIdx >= _units.size())
		return;
	_curUnit = unitIdx;
	repaint();
}

void ScreenCalibrationWidget::paintEvent(QPaintEvent * event)
{
	Q_UNUSED(event)
	Q_ASSERT(_sbDPI);

	double dpi = _sbDPI->value(); // dots per inch
	double dpu = dpi * static_cast<double>(_units[_curUnit].unitsPerInch); // dots per unit

	int y = _rulerRect.top();

	QPainter painter(this);

	// Draw ruler
	painter.fillRect(_rulerRect, Qt::white);
	painter.drawRect(_rulerRect);
	painter.setClipRect(_rulerRect);

	// Print unit label
	{
		double x = _rulerRect.left() + 2;
		painter.drawText(QPointF(x, y + _paperTickHeight), _units[_curUnit].label);
	}

	// Draw tick marks
	for (int majorTick = 0; majorTick * dpu < _rulerRect.width(); ++majorTick) {
		double x = majorTick * dpu + _rulerRect.left();
		painter.drawLine(QPointF(x, y), QPointF(x, y + _majorTickHeight));

		painter.drawText(QPointF(x + 2, y + _majorTickHeight), QString::number(majorTick));

		for (int minorTick = 1; minorTick < 10 && x + minorTick * dpu / 10. < _rulerRect.right(); ++minorTick) {
			painter.drawLine(QPointF(x + minorTick * dpu / 10., y), QPointF(x + minorTick * dpu / 10., y + (minorTick == 5 ? _mediumTickHeight : _minorTickHeight)));
		}
	}

	// Draw extra ticks for common paper sizes
	foreach(paperSize ps, _paperSizes) {
		if (!ps.visible)
			continue;
		double x = ps.size.width() * dpi + _rulerRect.left();
		painter.setPen(ps.col);
		painter.drawLine(QPointF(x, y), QPointF(x, y + _paperTickHeight));
		if (ps.alignment.testFlag(Qt::AlignRight))
#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
			x -= 2 + painter.fontMetrics().width(ps.name);
#else
			x -= 2 + painter.fontMetrics().horizontalAdvance(ps.name);
#endif
		else
			x += 2;
		painter.drawText(QPointF(x, y + _paperTickHeight), ps.name);
	}
}

void ScreenCalibrationWidget::resizeEvent(QResizeEvent * event)
{
	QWidget::resizeEvent(event);
	repositionSpinBox();
}

void ScreenCalibrationWidget::mousePressEvent(QMouseEvent * event)
{
	Q_ASSERT(_sbDPI);
	QWidget::mousePressEvent(event);

	if (event->buttons() == Qt::LeftButton && _rulerRect.contains(event->pos(), true)) {
		_mouseDownPos = event->pos();
		double dpi = _sbDPI->value();
		_mouseDownInches = (event->pos().x() - _rulerRect.left()) / dpi;
	}
}

void ScreenCalibrationWidget::mouseMoveEvent(QMouseEvent * event)
{
	Q_ASSERT(_sbDPI);
	QWidget::mouseMoveEvent(event);

	if (!_isDragging && event->buttons() == Qt::LeftButton && (event->pos() - _mouseDownPos).manhattanLength() >= QApplication::startDragDistance())
		_isDragging = true;
	if (_isDragging) {
		int px = event->pos().x() - _rulerRect.left();
		if (px <= 0)
			px = _mouseDownPos.x() - _rulerRect.left();
		double dpi = px / _mouseDownInches;
		_sbDPI->setValue(dpi);
	}
}

void ScreenCalibrationWidget::mouseReleaseEvent(QMouseEvent * event)
{
	QWidget::mouseReleaseEvent(event);

	if (_isDragging && event->button() == Qt::LeftButton)
		_isDragging = false;
}

void ScreenCalibrationWidget::changeEvent(QEvent * event)
{
	Q_ASSERT(event);
	QWidget::changeEvent(event);

	switch (event->type()) {
		case QEvent::FontChange:
			recalculateSizes();
			repaint();
			break;
		case QEvent::LanguageChange:
			retranslate();
			repaint();
			break;
		default:
			break;
	}
}

void ScreenCalibrationWidget::contextMenuEvent(QContextMenuEvent * event)
{
	QWidget::contextMenuEvent(event);

	if (event->reason() == QContextMenuEvent::Mouse && !_rulerRect.contains(event->pos()))
		return;

	_contextMenu.popup(event->globalPos());
}

bool ScreenCalibrationWidget::eventFilter(QObject * object, QEvent * event)
{
	Q_ASSERT(_sbDPI);

	if (object == _sbDPI && event->type() == QEvent::Resize)
		repositionSpinBox();

	return QWidget::eventFilter(object, event);
}

} // namespace UI
} // namespace Tw
