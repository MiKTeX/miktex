//========================================================================
//
// ImageEmbeddingUtils.h
//
// Copyright (C) 2021 Georgiy Sgibnev <georgiy@sgibnev.com>. Work sponsored by lab50.net.
//
// This file is licensed under the GPLv2 or later
//
//========================================================================

#ifndef IMAGE_EMBEDDING_UTILS_H
#define IMAGE_EMBEDDING_UTILS_H

#include <string>

#include "poppler_private_export.h"

class GooFile;
struct Ref;
class XRef;

namespace ImageEmbeddingUtils {

// Creates a new base image (an object of type XObject referred to in a resource dictionary).
// Supported formats: PNG, JPEG.
// Args:
//     xref: Document's xref.
//     imageFile: An image file to embed.
// Returns ref to a new object or Ref::INVALID.
Ref POPPLER_PRIVATE_EXPORT embed(XRef *xref, const GooFile &imageFile);

// Same as above, but imagePath is a path to an image file.
Ref POPPLER_PRIVATE_EXPORT embed(XRef *xref, const std::string &imagePath);

}
#endif
