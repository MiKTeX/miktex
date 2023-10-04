/**
 * Copyright (C) 2023  Stefan Löffler, Charlie Sharpsteen
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

#ifndef PDFPageCache_H
#define PDFPageCache_H

#include "PDFPageTile.h"

#include <QCache>
#include <QMap>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <QWriteLocker>

class QImage;

namespace QtPDF {

namespace Backend {

// This class is thread-safe
class PDFPageCache
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  using size_type = int;
#else
  using size_type = qsizetype;
#endif
public:
  enum TileStatus { UNKNOWN, PLACEHOLDER, CURRENT, OUTDATED };

  size_type maxCost() const { QReadLocker locker(&_lock); return m_cache.maxCost(); }
  void setMaxCost(const size_type cost) { QWriteLocker locker(&_lock); m_cache.setMaxCost(cost); }

  // Returns the image under the key `tile` or nullptr if it doesn't exist
  QSharedPointer<QImage> getImage(const PDFPageTile & tile) const;
  TileStatus getStatus(const PDFPageTile & tile) const;
  // Returns the pointer to the image in the cache under the key `tile` after
  // the insertion. If overwrite == true, this will always be image, otherwise
  // it can be different
  QSharedPointer<QImage> setImage(const PDFPageTile & tile, QSharedPointer<QImage> image, const TileStatus status, const bool overwrite = true);

  void clear() { QWriteLocker l(&_lock); m_cache.clear(); }
  void removeDocumentTiles(const Document *doc);
  // Mark all tiles outdated
  void markOutdated(const Document *doc);

  QList<PDFPageTile> tiles() const { QReadLocker locker(&_lock); return m_cache.keys(); }
protected:
  struct CachedTileData {
    QSharedPointer<QImage> image;
    TileStatus status;
  };

  mutable QReadWriteLock _lock;

  // Set cache for rendered pages to be 1GB. This is enough for 256 RGBA tiles
  // (1024 x 1024 pixels x 4 bytes per pixel).
  QCache<PDFPageTile, CachedTileData> m_cache{1024 * 1024 * 1024};
};

} // namespace Backend

} // namespace QtPDF

#endif // !defined(PDFPageCache_H)
