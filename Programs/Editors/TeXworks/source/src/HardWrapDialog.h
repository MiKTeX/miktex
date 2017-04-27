/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2008-2013  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef HardWrapDialog_H
#define HardWrapDialog_H

#include <QDialog>
#include "ui_HardWrapDialog.h"

const int kDefault_HardWrapWidth = 64;
const int kHardWrapMode_Window = 0;
const int kHardWrapMode_Fixed = 1;
const int kHardWrapMode_Unwrap = 2;

class HardWrapDialog : public QDialog, private Ui::HardWrapDialog
{
	Q_OBJECT

public:
	HardWrapDialog(QWidget *parent);

	virtual ~HardWrapDialog() { }

	unsigned int lineWidth() const { // returns 0 for current window size, or char count
	    return radio_fixedLineLength->isChecked() ? spinbox_charCount->value() : 0;
	}

	bool rewrap() const { // whether to re-wrap paragraphs (fill lines)
	    return checkbox_rewrap->isChecked();
	}
	
	int mode() const; // returns one of the kHardWrapMode_* values
	
	void saveSettings();

protected:
	void init();

protected slots:
	void unwrapModeToggled(const bool selected);
};

#endif // !defined(HardWrapDialog_H)
