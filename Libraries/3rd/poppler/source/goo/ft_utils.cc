//========================================================================
//
// ft_util.cc
//
// FreeType helper functions.
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2022 Adrian Johnson <ajohnson@redneon.com>
//
//========================================================================

#include <cstdio>

#include "ft_utils.h"
#include "gfile.h"

#ifdef _WIN32
static unsigned long ft_stream_read(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count)
{
    FILE *file = (FILE *)stream->descriptor.pointer;
    fseek(file, offset, SEEK_SET);
    return fread(buffer, 1, count, file);
}

static void ft_stream_close(FT_Stream stream)
{
    FILE *file = (FILE *)stream->descriptor.pointer;
    fclose(file);
    delete stream;
}
#endif

// Same as FT_New_Face() but handles UTF-8 filenames on Windows
FT_Error ft_new_face_from_file(FT_Library library, const char *filename_utf8, FT_Long face_index, FT_Face *aface)
{
#ifdef _WIN32
    FILE *file;
    long size;

    if (!filename_utf8)
        return FT_Err_Invalid_Argument;

    file = openFile(filename_utf8, "rb");
    if (!file)
        return FT_Err_Cannot_Open_Resource;

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    if (size <= 0)
        return FT_Err_Cannot_Open_Stream;

    FT_StreamRec *stream = new FT_StreamRec;
    *stream = {};
    stream->size = size;
    stream->read = ft_stream_read;
    stream->close = ft_stream_close;
    stream->descriptor.pointer = file;

    FT_Open_Args args = {};
    args.flags = FT_OPEN_STREAM;
    args.stream = stream;

    return FT_Open_Face(library, &args, face_index, aface);
#else
    // On POSIX, FT_New_Face mmaps font files. If not Windows, prefer FT_New_Face over our stdio.h based FT_Open_Face.
    return FT_New_Face(library, filename_utf8, face_index, aface);
#endif
}
