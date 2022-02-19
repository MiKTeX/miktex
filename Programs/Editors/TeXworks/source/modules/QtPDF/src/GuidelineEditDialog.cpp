/**
 * Copyright (C) 2022  Stefan Löffler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */
#include "GuidelineEditDialog.h"

#include "PDFGuideline.h"

namespace QtPDF {

GuidelineEditDialog::GuidelineEditDialog(PDFGuideline * parent)
  : QDialog(parent)
{
  setupUi(this);

  connect(cPositionUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GuidelineEditDialog::convertPositionToNewUnit);
}

int GuidelineEditDialog::guidelinePage() const
{
  return cPage->value();
}

void GuidelineEditDialog::setGuidelinePage(const int p)
{
  cPage->setValue(p);
}

Physical::Length GuidelineEditDialog::guidelinePos() const
{
  return Physical::Length(cPosition->value(), m_unit);
}

void GuidelineEditDialog::setGuidelinePos(const Physical::Length & pos)
{
  cPosition->setValue(pos.val(m_unit));
}

int GuidelineEditDialog::numPages() const
{
  return cPage->maximum();
}

void GuidelineEditDialog::setNumPages(const int n)
{
  cPage->setMaximum(n);
}

void GuidelineEditDialog::convertPositionToNewUnit()
{
  Physical::Length::Unit oldUnit = m_unit;
  switch (cPositionUnit->currentIndex()) {
    case 0: m_unit = Physical::Length::Centimeters; break;
    case 1: m_unit = Physical::Length::Inches; break;
    case 2: m_unit = Physical::Length::Bigpoints; break;
  }
  cPosition->setValue(Physical::Length::convert(cPosition->value(), oldUnit, m_unit));
}

} // namespace QtPDF
