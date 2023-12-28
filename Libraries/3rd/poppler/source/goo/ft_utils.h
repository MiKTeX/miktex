//========================================================================
//
// ft_util.h
//
// FreeType helper functions.
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2022 Adrian Johnson <ajohnson@redneon.com>
//
//========================================================================

#ifndef FT_UTILS_H
#define FT_UTILS_H

#include "config.h"
#include "poppler_private_export.h"

#include <ft2build.h>
#include FT_FREETYPE_H

// Same as FT_New_Face() but handles UTF-8 filenames on Windows
POPPLER_PRIVATE_EXPORT FT_Error ft_new_face_from_file(FT_Library library, const char *filename_utf8, FT_Long face_index, FT_Face *aface);

#endif // FT_UTILS_H
