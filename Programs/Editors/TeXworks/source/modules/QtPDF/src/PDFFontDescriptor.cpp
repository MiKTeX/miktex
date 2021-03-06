/**
 * Copyright (C) 2020  Charlie Sharpsteen, Stefan Löffler
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

#include "PDFFontDescriptor.h"

#include <QChar>

namespace QtPDF {

namespace Backend {

PDFFontDescriptor::PDFFontDescriptor(const QString & fontName /* = QString() */) :
  _name(fontName)
{
}

bool PDFFontDescriptor::isSubset() const
{
  // Subset fonts have a tag of 6 upper-case letters, followed by a '+',
  // prefixed to the font name
  if (_name.length() < 7 || _name[6] != QChar::fromLatin1('+'))
    return false;
  for (int i = 0; i < 6; ++i) {
    if (!_name[i].isUpper())
      return false;
  }
  return true;
}

QString PDFFontDescriptor::pureName() const
{
  if (!isSubset())
    return _name;
  return _name.mid(7);
}

bool PDFFontDescriptor::operator==(const PDFFontDescriptor & o) const {
  // TODO: more in-depth comparisons once the data is made available
  return (_name == o._name);
}

} // namespace Backend

} // namespace QtPDF

