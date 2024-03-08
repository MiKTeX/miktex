/**
 * Copyright (C) 2022-2023  Stefan Löffler
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

GuidelineEditDialog::size_type GuidelineEditDialog::guidelinePage() const
{
  return cPage->value();
}

void GuidelineEditDialog::setGuidelinePage(const size_type p)
{
  using value_type = decltype(cPage->value());
  cPage->setValue(static_cast<value_type>(p));
}

Physical::Length GuidelineEditDialog::guidelinePos() const
{
  return Physical::Length(cPosition->value(), m_unit);
}

void GuidelineEditDialog::setGuidelinePos(const Physical::Length & pos)
{
  cPosition->setValue(pos.val(m_unit));
}

GuidelineEditDialog::size_type GuidelineEditDialog::numPages() const
{
  return cPage->maximum();
}

void GuidelineEditDialog::setNumPages(const size_type n)
{
  using value_type = decltype(cPage->value());
  cPage->setMaximum(static_cast<value_type>(n));
}

void GuidelineEditDialog::setUnit(const Physical::Length::Unit unit)
{
  if (unit == m_unit)
    return;
  cPosition->setValue(Physical::Length::convert(cPosition->value(), m_unit, unit));

  m_unit = unit;
  switch(unit) {
    case Physical::Length::Centimeters: cPositionUnit->setCurrentIndex(0); break;
    case Physical::Length::Inches: cPositionUnit->setCurrentIndex(1); break;
    case Physical::Length::Bigpoints: cPositionUnit->setCurrentIndex(2); break;
  }
}

void GuidelineEditDialog::convertPositionToNewUnit()
{
  switch (cPositionUnit->currentIndex()) {
    case 0: setUnit(Physical::Length::Centimeters); break;
    case 1: setUnit(Physical::Length::Inches); break;
    case 2: setUnit(Physical::Length::Bigpoints); break;
  }
}

} // namespace QtPDF
