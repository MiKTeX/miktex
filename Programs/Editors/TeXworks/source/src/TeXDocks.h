/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2008-2015  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef TEXDOCKS_H
#define TEXDOCKS_H

#include <QDockWidget>
#include <QTreeWidget>
#include <QListWidget>
#include <QScrollArea>

class TeXDocument;
class QListWidget;
class QTableWidget;
class QTreeWidgetItem;

class TeXDock : public QDockWidget
{
	Q_OBJECT

public:
	TeXDock(const QString& title, TeXDocument *doc = 0);
	virtual ~TeXDock();

protected:
	virtual void fillInfo() = 0;

	TeXDocument *document;

	bool filled;

private slots:
	void myVisibilityChanged(bool visible);
};


class TagsDock : public TeXDock
{
	Q_OBJECT

public:
	TagsDock(TeXDocument *doc = 0);
	virtual ~TagsDock();

public slots:
	virtual void listChanged();

protected:
	virtual void fillInfo();

private slots:
	void followTagSelection();

private:
	QTreeWidget *tree;
	int saveScrollValue;
};

class TeXDockTreeWidget : public QTreeWidget
{
	Q_OBJECT

public:
	TeXDockTreeWidget(QWidget* parent);
	virtual ~TeXDockTreeWidget();

	virtual QSize sizeHint() const;
};

#endif
