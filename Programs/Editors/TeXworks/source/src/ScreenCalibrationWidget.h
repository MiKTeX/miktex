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
#ifndef SCREENCALIBRATIONWIDGET_H
#define SCREENCALIBRATIONWIDGET_H

#include <QWidget>
#include <QDoubleSpinBox>
#include <QResizeEvent>
#include <QMenu>
#include <QSignalMapper>

class ScreenCalibrationWidget : public QWidget
{
	Q_OBJECT
public:
	ScreenCalibrationWidget(QWidget * parent = nullptr);
	~ScreenCalibrationWidget() override = default;

	double dpi() const;

public slots:
	void setDpi(const double dpi);
	void setUnit(const int unitIdx);

signals:
	void dpiChanged(double dpi);

protected slots:
	void recalculateSizes();
	void retranslate();
	void repositionSpinBox();

protected:
	void paintEvent(QPaintEvent * event) override;
	void resizeEvent(QResizeEvent * event) override;
	void mousePressEvent(QMouseEvent * event) override;
	void mouseMoveEvent(QMouseEvent * event) override;
	void mouseReleaseEvent(QMouseEvent * event) override;
	void changeEvent(QEvent * event) override;
	void contextMenuEvent(QContextMenuEvent * event) override;
	bool eventFilter(QObject * object, QEvent * event) override;


	QDoubleSpinBox * _sbDPI;
	QRect _rulerRect;
	int _majorTickHeight{20}, _mediumTickHeight{10}, _minorTickHeight{5}, _paperTickHeight{40};
	QMenu _contextMenu;
	QActionGroup _contextMenuActionGroup;
	QSignalMapper _unitSignalMapper;

	struct unit {
		QString label;
		float unitsPerInch;
	};
	QList<unit> _units;

	struct paperSize {
		QString name;
		QSizeF size; // specify in inch
		QColor col; // for display
		Qt::Alignment alignment;
		bool visible;
	};
	QList<paperSize> _paperSizes;
	int _curUnit;
	int _hSpace{0};

	QPoint _mouseDownPos;
	double _mouseDownInches{0};
	bool _isDragging{false};
};

#endif // SCREENCALIBRATIONWIDGET_H
