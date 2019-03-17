/**
 * Copyright (C) 2013  Stefan Löffler
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
#include <PDFActions.h>
#include <PDFBackend.h>

namespace QtPDF {

QRectF PDFDestination::viewport(const Backend::Document * doc, const QRectF oldViewport, const float oldZoom) const
{
  QRectF retVal = oldViewport;

  if (!isValid())
    return retVal;
  if (!isExplicit()) {
    if (!doc)
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
      if (!doc)
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
      if (!doc)
        break;
      QSharedPointer<Backend::Page> p(doc->page(_page).toStrongRef());
      if (!p)
        break;
      float aspectRatio = oldViewport.width() / oldViewport.height();
      retVal = QRectF(0, 0, p->pageSizeF().width(), qMin(p->pageSizeF().width() / aspectRatio, p->pageSizeF().height()));
      break;
    }
    case Destination_FitV:
    case Destination_FitBV:
    {
      if (!doc)
        break;
      QSharedPointer<Backend::Page> p(doc->page(_page).toStrongRef());
      if (!p)
        break;
      float aspectRatio = oldViewport.width() / oldViewport.height();
      retVal = QRectF(0, 0, qMin(p->pageSizeF().height() * aspectRatio, p->pageSizeF().width()), p->pageSizeF().height());
      break;
    }
    case Destination_FitR:
      retVal = _rect;
      break;
  }
  return retVal;
}

#ifdef DEBUG
  QDebug operator<<(QDebug dbg, const PDFDestination & dest)
  {
    dbg.nospace() << "PDFDestination(";
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
    return dbg.space();
  }
#endif // defined(DEBUG)

} // namespace QtPDF

// vim: set sw=2 ts=2 et

