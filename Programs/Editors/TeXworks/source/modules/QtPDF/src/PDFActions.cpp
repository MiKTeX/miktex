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

#include "PDFActions.h"
#include "PDFBackend.h"

namespace QtPDF {

QRectF PDFDestination::viewport(Backend::Document * doc, const QRectF oldViewport, const qreal oldZoom) const
{
  QRectF retVal = oldViewport;

  if (!isValid())
    return retVal;
  if (!isExplicit()) {
    if (!doc || !doc->isValid())
      return retVal;
    return doc->resolveDestination(*this).viewport(doc, oldViewport, oldZoom);
  }

  // TODO: Fit & FitB (and related) are not strictly the same; Fit refers to
  // "the entire page", whereas FitB refers to "the bounding box of the content"
  // TODO: We use values < 0 to reflect pdf (null) values (i.e., invalid values
  // for top, left, zoom that are to be ignored). Check if negative values are
  // indeed impossible/not allowed.
  // FIXME: Consistency between retVal < oldViewport and retVal >= oldViewport
  switch (_type) {
    case Destination_XYZ:
    {
      if (_rect.left() >= 0)
        retVal.moveLeft(_rect.left());
      if (_rect.top() >= 0)
        retVal.moveTop(_rect.top());
      if (_zoom > 0)
        retVal = QTransform::fromScale(_zoom / oldZoom, _zoom / oldZoom).mapRect(retVal);
      break;
    }
    case Destination_Fit:
    case Destination_FitB:
    {
      if (!doc || !doc->isValid())
        break;
      QSharedPointer<Backend::Page> p(doc->page(_page).toStrongRef());
      if (!p)
        break;
      retVal = QRectF(QPointF(0, 0), p->pageSizeF());
      break;
    }
    case Destination_FitH:
    case Destination_FitBH:
    {
      if (!doc || !doc->isValid())
        break;
      QSharedPointer<Backend::Page> p(doc->page(_page).toStrongRef());
      if (!p)
        break;
      qreal aspectRatio = oldViewport.width() / oldViewport.height();
      retVal = QRectF(0, 0, p->pageSizeF().width(), qMin(p->pageSizeF().width() / aspectRatio, p->pageSizeF().height()));
      break;
    }
    case Destination_FitV:
    case Destination_FitBV:
    {
      if (!doc || !doc->isValid())
        break;
      QSharedPointer<Backend::Page> p(doc->page(_page).toStrongRef());
      if (!p)
        break;
      qreal aspectRatio = oldViewport.width() / oldViewport.height();
      retVal = QRectF(0, 0, qMin(p->pageSizeF().height() * aspectRatio, p->pageSizeF().width()), p->pageSizeF().height());
      break;
    }
    case Destination_FitR:
      retVal = _rect;
      break;
  }
  return retVal;
}

bool PDFDestination::operator==(const PDFDestination & o) const
{
  if (isValid() != o.isValid()) {
    return false;
  }
  if (!isValid()) {
    // Two invalid destinations are considered equal
    return true;
  }
  if (!isExplicit() || !o.isExplicit()) {
    // NB: a named destination does not compare equal to its resolved (explicit)
    // variant as we have no Document handle to resolve the named destination
    return _destinationName == o._destinationName;
  }

  return (_page == o._page && _type == o._type && _rect == o._rect && qFuzzyCompare(_zoom, o._zoom));
}

#ifdef DEBUG
  QDebug operator<<(QDebug dbg, const PDFDestination & dest)
  {
    bool oldSpace = dbg.autoInsertSpaces();
    dbg.setAutoInsertSpaces(false);
    dbg << "PDFDestination(";
    if (dest.isValid()) {
      if (dest.isExplicit()) {
        dbg << dest.page() << " ";
        switch (dest.type()) {
          case PDFDestination::Destination_XYZ:
            dbg << "/XYZ " << dest.left() << " " << dest.top() << " " << dest.zoom();
            break;
          case PDFDestination::Destination_Fit:
            dbg << "/Fit";
            break;
          case PDFDestination::Destination_FitH:
            dbg << "/FitH " << dest.top();
            break;
          case PDFDestination::Destination_FitV:
            dbg << "/FitV " << dest.left();
            break;
          case PDFDestination::Destination_FitR:
            dbg << "/FitR " << dest.left() << " " << dest.rect().bottom() << " " << dest.rect().right() << " " << dest.top();
            break;
          case PDFDestination::Destination_FitB:
            dbg << "/FitB";
            break;
          case PDFDestination::Destination_FitBH:
            dbg << "/FitBH " << dest.top();
            break;
          case PDFDestination::Destination_FitBV:
            dbg << "/FitBV " << dest.left();
            break;
        }
      }
      else
        dbg << "name=" << dest.destinationName();
    }
    dbg << ")";
    dbg.setAutoInsertSpaces(oldSpace);
    return dbg;
  }
#endif // defined(DEBUG)

bool PDFURIAction::operator==(const PDFAction & o) const
{
  if (o.type() != ActionTypeURI) {
    return false;
  }
  return (*this == dynamic_cast<const PDFURIAction&>(o));
}

bool PDFURIAction::operator==(const PDFURIAction & o) const
{
  return (o._url == _url && o._isMap == _isMap);
}

bool PDFGotoAction::operator==(const PDFAction & o) const
{
  if (o.type() != ActionTypeGoTo) {
    return false;
  }
  return (*this == dynamic_cast<const PDFGotoAction&>(o));
}

bool PDFGotoAction::operator==(const PDFGotoAction & o) const
{
  return (_destination == o._destination && _filename == o._filename && _isRemote == o._isRemote && _openInNewWindow == o._openInNewWindow);
}

bool PDFLaunchAction::operator==(const PDFAction & o) const
{
  if (o.type() != ActionTypeLaunch) {
    return false;
  }
  return (*this == dynamic_cast<const PDFLaunchAction&>(o));
}

bool PDFLaunchAction::operator==(const PDFLaunchAction & o) const
{
  return (_command == o._command);
}

#ifdef DEBUG
QDebug operator<<(QDebug dbg, const PDFAction & action)
{
  bool oldSpace = dbg.autoInsertSpaces();
  dbg.setAutoInsertSpaces(false);
  switch (action.type()) {
  case PDFAction::ActionTypeURI:
    dbg << "PDFURIAction(" << dynamic_cast<const PDFURIAction&>(action).url() << ")";
    break;
  case PDFAction::ActionTypeGoTo:
  {
    auto a = dynamic_cast<const PDFGotoAction&>(action);
    dbg << "PDFGotoAction(" << a.destination() << ", remote=" << a.isRemote() << ", filename=" << a.filename() << ", newWin=" << a.openInNewWindow() << ")";
    break;
  }
  case PDFAction::ActionTypeLaunch:
    dbg << "PDFLaunchAction(" << dynamic_cast<const PDFLaunchAction&>(action).command() << ")";
    break;
  default:
    dbg << "PDFAction(type=" << action.type() << ")";
  }
  dbg.setAutoInsertSpaces(oldSpace);
  return dbg;
}
#endif

} // namespace QtPDF

// vim: set sw=2 ts=2 et

