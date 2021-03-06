/**
 * Copyright (C) 2013-2020  Stefan Löffler
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

#include "PDFAnnotations.h"
#include "PDFBackend.h"

namespace QtPDF {

namespace Annotation {

// Annotations
// =================

bool AbstractAnnotation::operator==(const AbstractAnnotation & o) const
{
  return (type() == o.type() &&
          rect() == o.rect() &&
          contents() == o.contents() &&
          page() == o.page() &&
          name() == o.name() &&
          lastModified() == o.lastModified() &&
          flags() == o.flags() &&
          color() == o.color());
}

Markup::~Markup()
{
  delete _popup;
}

Markup::Markup(const Markup & o)
  : AbstractAnnotation(o)
  , _title(o._title)
  , _richContents(o._richContents)
  , _creationDate(o._creationDate)
  , _subject(o._subject)
{
  if (o.popup() != nullptr) {
    setPopup(new Popup(*(o.popup())));
  }
}

Markup & Markup::operator=(const Markup & o)
{
  if (this == &o) {
    return *this;
  }
  AbstractAnnotation::operator=(o);
  _title = o._title;
  _richContents = o._richContents;
  _creationDate = o._creationDate;
  _subject = o._subject;
  if (o.popup() != nullptr) {
    setPopup(new Popup(*(o.popup())));
  }
  else {
    setPopup(nullptr);
  }
  return *this;
}

void Markup::setPopup(Popup * popup)
{
  delete _popup;
  _popup = popup;
  if (_popup != nullptr) {
    _popup->setParent(this);
  }
}

bool Markup::operator==(const AbstractAnnotation & o) const
{
  if (!AbstractAnnotation::operator==(o)) {
    return false;
  }
  const Markup & m(dynamic_cast<const Markup &>(o));

  if (title() != m.title() || author() != m.author() ||
      richContents() != m.richContents() || creationDate() != m.creationDate() ||
      subject() != m.subject()) {
    return false;
  }
  if (popup() != nullptr && m.popup() != nullptr) {
    return (*(popup()) == *(m.popup()));
  }
  return (popup() == m.popup());
}

Link::~Link()
{
  delete _actionOnActivation;
}

QPolygonF Link::quadPoints() const
{
  if (_quadPoints.isEmpty())
    return QPolygonF(rect());
  // The PDF specs (1.7) state that: "QuadPoints should be ignored if any
  // coordinate in the array lies outside the region specified by Rect."
  foreach (QPointF p, _quadPoints) {
    if (!rect().contains(p))
      return QPolygonF(rect());
  }
  return _quadPoints;
}

void Link::setActionOnActivation(PDFAction * const action)
{
  delete _actionOnActivation;
  _actionOnActivation = action;
}

bool Link::operator==(const AbstractAnnotation & o) const
{
  if (!(AbstractAnnotation::operator==(o))) {
    return false;
  }
  const Link & l(dynamic_cast<const Link&>(o));
  if (highlightingMode() != l.highlightingMode() || quadPoints() != l.quadPoints()) {
    return false;
  }
  if (actionOnActivation() != nullptr && l.actionOnActivation() != nullptr) {
    return (*actionOnActivation() == *(l.actionOnActivation()));
  }
  return (actionOnActivation() == l.actionOnActivation());
}

bool Popup::operator==(const AbstractAnnotation & o) const
{
  if (!(AbstractAnnotation::operator==(o))) {
    return false;
  }
  const Popup & p(dynamic_cast<const Popup&>(o));
  return (isOpen() == p.isOpen() && title() == p.title());
  // Don't compare _parent values. _parent just modifies where some data (e.g.
  // _contents) is taken from (the _parent or the popup itself) but does not
  // have any other visible effects.
  // Also, this function is called as part of Markup::operator==, which would
  // be called again when comparing *_parent values, resulting in an infinite
  // loop
}

} // namespace Annotation

} // namespace QtPDF

// vim: set sw=2 ts=2 et
