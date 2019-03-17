#include "ScreenCalibrationWidget.h"
#include <QPainter>
#include <QStyle>
#include <QApplication>

ScreenCalibrationWidget::ScreenCalibrationWidget(QWidget *parent) : QWidget(parent), _contextMenuActionGroup(this)
{
	_isDragging = false;

	_sbDPI = new QDoubleSpinBox(this);
	_sbDPI->setRange(0, 9999);
	_sbDPI->setValue(physicalDpiX());
	_sbDPI->installEventFilter(this);
	connect(_sbDPI, SIGNAL(valueChanged(double)), this, SLOT(repaint()));
	connect(_sbDPI, SIGNAL(valueChanged(double)), this, SIGNAL(dpiChanged(double)));

	connect(&_unitSignalMapper, SIGNAL(mapped(int)), this, SLOT(setUnit(int)));

	setToolTip(tr("Drag the ruler or change the value to match real world lengths.\nCommon paper sizes are marked as well (you may need to resize the dialog window to see them).\nUse the context menu to change the units."));

	switch(locale().measurementSystem()) {
#if QT_VERSION < 0x050000
		case QLocale::ImperialSystem:
#else
		case QLocale::ImperialUSSystem:
		case QLocale::ImperialUKSystem:
#endif
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
	_majorTickHeight = 1.2 * fontMetrics().lineSpacing();
	_mediumTickHeight = 0.5 * _majorTickHeight;
	_minorTickHeight = 0.25 * _majorTickHeight;
	_paperTickHeight = 2.2 * fontMetrics().lineSpacing();
	_hSpace = style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);
	if (_hSpace < 0)
		_hSpace = style()->layoutSpacing(QSizePolicy::SpinBox, QSizePolicy::DefaultType, Qt::Horizontal);
	setMinimumHeight(_paperTickHeight + 0.2 * fontMetrics().lineSpacing());
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
#if defined(MIKTEX)
        _units.append({ tr("cm"), (float)0.393701 });
#else
	_units.append({tr("cm"), 0.393701});
#endif
	//: this refers to the length unit of inches
	_units.append({tr("in"), 1});

	_contextMenu.clear();
	QAction * a;
	a = _contextMenu.addAction(tr("cm"));
	a->setCheckable(true);
	a->setChecked(_curUnit == 0);
	_unitSignalMapper.setMapping(a, 0);
	connect(a, SIGNAL(triggered()), &_unitSignalMapper, SLOT(map()));
	_contextMenuActionGroup.addAction(a);
	a = _contextMenu.addAction(tr("in"));
	a->setCheckable(true);
	a->setChecked(_curUnit == 1);
	_unitSignalMapper.setMapping(a, 1);
	connect(a, SIGNAL(triggered()), &_unitSignalMapper, SLOT(map()));
	_contextMenuActionGroup.addAction(a);
}

void ScreenCalibrationWidget::repositionSpinBox()
{
	Q_ASSERT(_sbDPI);

	_sbDPI->ensurePolished();
	switch (layoutDirection()) {
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
		// FIXME: Qt::LayoutDirectionAuto: go up the widget hierarchy until we
		// find a valid layout direction, or use QApplication's layout direction
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
	Q_ASSERT(_sbDPI);
	float x;

	double dpi = _sbDPI->value(); // dots per inch
	double dpu = dpi * _units[_curUnit].unitsPerInch; // dots per unit
	int majorTick, minorTick;

	int y = _rulerRect.top();

	QPainter painter(this);

	// Draw ruler
	painter.fillRect(_rulerRect, Qt::white);
	painter.drawRect(_rulerRect);
	painter.setClipRect(_rulerRect);

	// Print unit label
	x = _rulerRect.left() + 2;
	painter.drawText(x, y + _paperTickHeight, _units[_curUnit].label);

	// Draw tick marks
	for (majorTick = 0; majorTick * dpu < _rulerRect.width(); ++majorTick) {
		x = majorTick * dpu + _rulerRect.left();
		painter.drawLine(x, y, x, y + _majorTickHeight);

		painter.drawText(x + 2, y + _majorTickHeight, QString::number(majorTick));

		for (minorTick = 1; minorTick < 10 && x + minorTick * dpu / 10. < _rulerRect.right(); ++minorTick) {
			painter.drawLine(x + minorTick * dpu / 10., y, x + minorTick * dpu / 10., y + (minorTick == 5 ? _mediumTickHeight : _minorTickHeight));
		}
	}

	// Draw extra ticks for common paper sizes
	foreach(paperSize ps, _paperSizes) {
		if (!ps.visible)
			continue;
		x = ps.size.width() * dpi + _rulerRect.left();
		painter.setPen(ps.col);
		painter.drawLine(x, y, x, y + _paperTickHeight);
		if (ps.alignment.testFlag(Qt::AlignRight))
			x -= 2 + painter.fontMetrics().width(ps.name);
		else
			x += 2;
		painter.drawText(x, y + _paperTickHeight, ps.name);
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
		_mouseDownInches = (event->x() - _rulerRect.left()) / dpi;
	}
}

void ScreenCalibrationWidget::mouseMoveEvent(QMouseEvent * event)
{
	Q_ASSERT(_sbDPI);
	QWidget::mouseMoveEvent(event);

	if (!_isDragging && event->buttons() == Qt::LeftButton && (event->pos() - _mouseDownPos).manhattanLength() >= QApplication::startDragDistance())
		_isDragging = true;
	if (_isDragging) {
		int px = event->x() - _rulerRect.left();
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
