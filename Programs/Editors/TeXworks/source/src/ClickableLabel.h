/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2011-2019  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef ClickableLabel_H
#define ClickableLabel_H

#include <QLabel>
#include <QMouseEvent>
#include <QApplication>

class ClickableLabel : public QLabel
{
	Q_OBJECT
public:
	explicit ClickableLabel(QWidget * parent = nullptr, Qt::WindowFlags f = nullptr);
	explicit ClickableLabel(const QString & text, QWidget * parent = nullptr, Qt::WindowFlags f = Q_NULLPTR);
	~ClickableLabel() override = default;
	
signals:
	void mouseDoubleClick(QMouseEvent * event);
	void mouseLeftClick(QMouseEvent * event);
	void mouseMiddleClick(QMouseEvent * event);
	void mouseRightClick(QMouseEvent * event);

protected:
	void mouseDoubleClickEvent(QMouseEvent * event) override;
	void mousePressEvent(QMouseEvent * event) override;
	void mouseReleaseEvent(QMouseEvent * event) override;

	QPoint mouseStartPoint;
};

#endif // !defined(ClickableLabel_H)
