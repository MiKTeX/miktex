/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2009-2019  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "HardWrapDialog.h"
#include "Settings.h"

HardWrapDialog::HardWrapDialog(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	init();
}

void
HardWrapDialog::init()
{
	Tw::Settings settings;
	int	wrapWidth = settings.value(QString::fromLatin1("hardWrapWidth"), kDefault_HardWrapWidth).toInt();
	spinbox_charCount->setMaximum(INT_MAX);
	spinbox_charCount->setValue(wrapWidth);
	spinbox_charCount->selectAll();
	
	connect(radio_Unwrap, SIGNAL(toggled(bool)), this, SLOT(unwrapModeToggled(bool)));

	int wrapMode = settings.value(QString::fromLatin1("hardWrapMode"), kHardWrapMode_Fixed).toInt();
	radio_currentWidth->setChecked(wrapMode == kHardWrapMode_Window);
	radio_fixedLineLength->setChecked(wrapMode == kHardWrapMode_Fixed);
	radio_Unwrap->setChecked(wrapMode == kHardWrapMode_Unwrap);
	
	bool rewrapParagraphs = settings.value(QString::fromLatin1("hardWrapRewrap"), false).toBool();
	checkbox_rewrap->setChecked(rewrapParagraphs);

#if defined(Q_OS_DARWIN)
	setWindowFlags(Qt::Sheet);
#endif
}

void
HardWrapDialog::saveSettings()
{
	Tw::Settings settings;
	settings.setValue(QString::fromLatin1("hardWrapWidth"), spinbox_charCount->value());
	settings.setValue(QString::fromLatin1("hardWrapMode"), mode());
	settings.setValue(QString::fromLatin1("hardWrapRewrap"), checkbox_rewrap->isChecked());
}

int
HardWrapDialog::mode() const
{
	if (radio_currentWidth->isChecked())
		return kHardWrapMode_Window;
	if (radio_fixedLineLength->isChecked())
		return kHardWrapMode_Fixed;
	if (radio_Unwrap->isChecked())
		return kHardWrapMode_Unwrap;
	
	return kHardWrapMode_Fixed;
}

void
HardWrapDialog::unwrapModeToggled(const bool selected)
{
	checkbox_rewrap->setEnabled(!selected);
}
