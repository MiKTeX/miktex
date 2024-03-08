/**
 * Copyright (C) 2020-2023  Charlie Sharpsteen, Stefan Löffler
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

#include <QHash>
#include <QRect>
#include <QVector>

#ifdef DEBUG
#include <QString>
#endif

namespace QtPDF {

namespace Backend {

class Document;
class Page;

class PDFPageTile
{
  using size_type = QVector<Page*>::size_type;
public:
  PDFPageTile(double xres, double yres, QRect render_box, const Document * doc, size_type page_num):
    xres(xres), yres(yres),
    render_box(render_box),
    doc(doc),
    page_num(page_num)
  {}

  double xres, yres;
  QRect render_box;
  const Document * doc;
  size_type page_num;

  bool operator==(const PDFPageTile &other) const
  {
    return (xres == other.xres && yres == other.yres && render_box == other.render_box && doc == other.doc && page_num == other.page_num);
  }

  bool operator <(const PDFPageTile &other) const;

#ifdef DEBUG
  operator QString() const;
#endif
};

// ### Cache for Rendered Images
decltype(::qHash(0)) qHash(const PDFPageTile &tile) noexcept;

} // namespace Backend

} // namespace QtPDF

#endif // !defined(PDFPageTile_H)
