/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2008-2011  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef ResourcesDialog_H
#define ResourcesDialog_H

#include <QDialog>
#include <QUrl>
#include <QDesktopServices>

#include "ui_ResourcesDialog.h"


class ResourcesDialog : public QDialog, private Ui::ResourcesDialog
{
	Q_OBJECT

public:
	ResourcesDialog(QWidget *parent);
	static DialogCode doResourcesDialog(QWidget *parent);

public slots:
//	void toggleDetails();

private slots:
	void openURL(const QString& url) const { QDesktopServices::openUrl(QUrl(url)); }

private:
	void init();
	static QString pathToLink(const QString & path);
};



#endif // !defined(ResourcesDialog_H)

