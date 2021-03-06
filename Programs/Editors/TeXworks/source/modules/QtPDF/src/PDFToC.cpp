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

#include "PDFToC.h"

namespace QtPDF {

namespace Backend {

PDFToCItem::PDFToCItem(const PDFToCItem & o)
  : _label(o._label)
  , _isOpen(o._isOpen)
  , _color(o._color)
  , _children(o._children)
  , _flags(o._flags)
{
  _action = (o._action ? o._action->clone() : nullptr);
}

PDFToCItem::~PDFToCItem()
{
  delete _action;
}

PDFToCItem & PDFToCItem::operator=(const PDFToCItem & o)
{
  if (this == &o) {
    return *this;
  }
  _label = o._label;
  _isOpen = o._isOpen;
  _color = o._color;
  _children = o._children;
  _flags = o._flags;
  setAction(o._action != nullptr ? o._action->clone() : nullptr);
  return *this;
}

void PDFToCItem::setAction(PDFAction * action)
{
  if (_action != action) {
    delete _action;
  }
  _action = action;
}

bool PDFToCItem::operator==(const PDFToCItem & o) const {
  if (_label != o._label || _isOpen != o._isOpen || _color != o._color || _flags != o._flags) {
    return false;
  }
  if (_action != nullptr && o._action != nullptr) {
    if (!(*_action == *o._action)) {
      return false;
    }
  }
  else {
    // At least one action is a nullptr
    if (_action != o._action) {
      return false;
    }
  }
  return _children == o._children;
}

} // namespace Backend

} // namespace QtPDF
