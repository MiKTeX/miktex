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

#ifndef PDFPageTile_H
#define PDFPageTile_H

#include <QRect>

#ifdef DEBUG
#include <QString>
#endif

namespace QtPDF {

namespace Backend {

class PDFPageTile
{
public:
  // TODO:
  // We may want an application-wide cache instead of a document-specific cache
  // to keep memory usage down. This may require an additional piece of
  // information---the document that the page belongs to.
  PDFPageTile(double xres, double yres, QRect render_box, int page_num):
    xres(xres), yres(yres),
    render_box(render_box),
    page_num(page_num)
  {}

  double xres, yres;
  QRect render_box;
  int page_num;

  bool operator==(const PDFPageTile &other) const
  {
    return (xres == other.xres && yres == other.yres && render_box == other.render_box && page_num == other.page_num);
  }

  bool operator <(const PDFPageTile &other) const;

#ifdef DEBUG
  operator QString() const;
#endif
};

// ### Cache for Rendered Images
uint qHash(const PDFPageTile &tile) noexcept;

} // namespace Backend

} // namespace QtPDF

#endif // !defined(PDFPageTile_H)
