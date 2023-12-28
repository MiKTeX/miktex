//========================================================================
//
// FILECacheLoader.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
// Copyright 2010, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright 2010 Jonathan Liu <net147@gmail.com>
// Copyright 2021 Peter Williams <peter@newton.cx>
// Copyright 2021 Christian Persch <chpe@src.gnome.org>
//
//========================================================================

#include <config.h>

#include "FILECacheLoader.h"

#if defined(_WIN32) || defined(__CYGWIN__)
#    include <fcntl.h> // for O_BINARY
#    include <io.h> // for _setmode
#endif

FILECacheLoader::~FILECacheLoader()
{
    if (file != stdin) {
        fclose(file);
    }
}

size_t FILECacheLoader::init(CachedFile *cachedFile)
{
    size_t read, size = 0;
    char buf[CachedFileChunkSize];

#if defined(_WIN32) || defined(__CYGWIN__)
    _setmode(fileno(file), O_BINARY);
#endif

    CachedFileWriter writer = CachedFileWriter(cachedFile, nullptr);
    do {
        read = fread(buf, 1, CachedFileChunkSize, file);
        (writer.write)(buf, CachedFileChunkSize);
        size += read;
    } while (read == CachedFileChunkSize);

    return size;
}

int FILECacheLoader::load(const std::vector<ByteRange> &ranges, CachedFileWriter *writer)
{
    return 0;
}
