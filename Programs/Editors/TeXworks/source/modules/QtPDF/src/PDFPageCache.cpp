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

#include "PDFPageCache.h"

#include <QImage>

namespace QtPDF {

namespace Backend {

QSharedPointer<QImage> PDFPageCache::getImage(const PDFPageTile & tile) const
{
  QReadLocker locker(&_lock);
  CachedTileData * data = m_cache.object(tile);
  if (data) {
    return data->image;
  }
  return {};
}

PDFPageCache::TileStatus PDFPageCache::getStatus(const PDFPageTile & tile) const
{
  QReadLocker locker(&_lock);
  CachedTileData * data = m_cache.object(tile);
  if (data) {
    return data->status;
  }
  return UNKNOWN;
}

QSharedPointer<QImage> PDFPageCache::setImage(const PDFPageTile & tile, QSharedPointer<QImage> image, const TileStatus status, const bool overwrite /* = true */)
{
  QWriteLocker locker(&_lock);

  auto insert = [this](const PDFPageTile & tile, QSharedPointer<QImage> image, const TileStatus status) {
    CachedTileData * data = new CachedTileData{image, status};
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    m_cache.insert(tile, data, (image ? image->byteCount() : 0));
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // No image (1024x124x4 bytes by default) should ever come even close to the
    // 2 GB mark corresponding to INT_MAX; note that Document::Document() sets
    // the cache's max-size to 1 GB total
    m_cache.insert(tile, data, (image ? static_cast<int>(image->sizeInBytes()) : 0));
#else
    m_cache.insert(tile, data, (image ? image->sizeInBytes() : 0));
#endif
  };

  CachedTileData * data = m_cache.object(tile);
  if (!data) {
    insert(tile, image, status);
    return image;
  }
  if (data->image == image) {
    // Trying to overwrite an image with itself - just update the status
    data->status = status;
    return data->image;
  }
  if (overwrite) {
    insert(tile, image, status);
    return image;
  }
  return data->image;
}

void PDFPageCache::removeDocumentTiles(const Document *doc)
{
  QWriteLocker l(&_lock);

  const auto keys = m_cache.keys();
  for (const PDFPageTile & tile : keys) {
    if (tile.doc == doc) {
      m_cache.remove(tile);
    }
  }
}

void PDFPageCache::markOutdated(const Document * doc)
{
  QWriteLocker l(&_lock);

  const auto keys = m_cache.keys();
  for (const PDFPageTile & tile : keys) {
    if (tile.doc == doc) {
      m_cache[tile]->status = OUTDATED;
    }
  }
}

} // namespace Backend

} // namespace QtPDF
