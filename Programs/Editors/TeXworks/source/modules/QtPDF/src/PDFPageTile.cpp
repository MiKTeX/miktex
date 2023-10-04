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

#include "PDFPageTile.h"

#include <QPair>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

#if QT_VERSION < QT_VERSION_CHECK(5, 3, 0)

// Taken from Qt 4.7.2 sources (<Qt>/src/corelib/tools/qhash.cpp)
static uint hash(const uchar *p, int n)
{
  uint h = 0;

  while (n--) {
    h = (h << 4) + *p++;
    h ^= (h & 0xf0000000) >> 23;
    h &= 0x0fffffff;
  }
  return h;
}

inline uint qHash(const double &d)
{
  // We interpret the double as an array of bytes and use the hash() function on
  // it.
  // NOTE: Due to rounding errors, this is not 100% reliable - two doubles that
  // _look_ the same may actually differ in their bit representations (e.g., if
  // the same value was calculated in two different ways). So this function may
  // report different hashes for doubles that look the same (which should not be
  // a problem in our case, however).
  // Note also that the QDataStream approach used previously also works on the
  // binary representation of doubles internally and so the same problem would
  // occur there as well.
  return hash(reinterpret_cast<const uchar*>(&d), sizeof(d));
}

#endif // Qt < 5.3.0

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

inline uint qHash(const QRect &key) {
  return qHash(
        QPair< QPair< int, int >, QPair< int, int > >(
          QPair< int, int >(key.x(), key.y()),
          QPair< int, int >(key.width(), key.height())
        )
        );
}

#endif

namespace QtPDF {

namespace Backend {

bool PDFPageTile::operator <(const PDFPageTile &other) const
{
  return qHash(*this) < qHash(other);
}

#ifdef DEBUG
PDFPageTile::operator QString() const
{
  return QString::fromUtf8("p%1,%2x%3,r%4|%5x%6|%7").arg(page_num).arg(xres).arg(yres).arg(render_box.x()).arg(render_box.y()).arg(render_box.width()).arg(render_box.height());
}
#endif

// Overlad qHash so PDFPageTile can be used, e.g., in a QMap or QCache
decltype(::qHash(0)) qHash(const PDFPageTile &tile) noexcept
{
  QByteArray ba;
  QDataStream strm{&ba, QIODevice::WriteOnly};
  strm << tile.xres << tile.yres << tile.render_box << reinterpret_cast<quint64>(tile.doc) << tile.page_num;
  return ::qHash(ba);
}

} // namespace Backend

} // namespace QtPDF
