//========================================================================
//
// InMemoryFile.cc
//
// Represents a file in-memory with GNU's stdio wrappers.
// NOTE as of this writing, open() depends on the glibc 'fopencookie'
// extension and is not supported on other platforms. The
// HAVE_IN_MEMORY_FILE macro is intended to reflect whether this class is
// usable.
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2018, 2019 Greg Knight <lyngvi@gmail.com>
// Copyright (C) 2020 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#include "InMemoryFile.h"

#include <cstring>
#include <sstream>

InMemoryFile::InMemoryFile() = default;

#ifdef HAVE_IN_MEMORY_FILE_FOPENCOOKIE
ssize_t InMemoryFile::_read(char *buf, size_t sz)
{
    auto toRead = std::min<size_t>(data.size() - iohead, sz);
    memcpy(&buf[0], &data[iohead], toRead);
    iohead += toRead;
    return toRead;
}

ssize_t InMemoryFile::_write(const char *buf, size_t sz)
{
    if (iohead + sz > data.size()) {
        data.resize(iohead + sz);
    }
    memcpy(&data[iohead], buf, sz);
    iohead += sz;
    return sz;
}

int InMemoryFile::_seek(off64_t *offset, int whence)
{
    switch (whence) {
    case SEEK_SET:
        iohead = (*offset);
        break;
    case SEEK_CUR:
        iohead += (*offset);
        break;
    case SEEK_END:
        iohead -= (*offset);
        break;
    }
    (*offset) = std::min<off64_t>(std::max<off64_t>(iohead, 0l), data.size());
    iohead = static_cast<size_t>(*offset);
    return 0;
}
#endif // def HAVE_IN_MEMORY_FILE_FOPENCOOKIE

FILE *InMemoryFile::open(const char *mode)
{
#ifdef HAVE_IN_MEMORY_FILE_FOPENCOOKIE
    if (fptr != nullptr) {
        fprintf(stderr, "InMemoryFile: BUG: Why is this opened more than once?");
        return nullptr; // maybe there's some legit reason for it, whoever comes up with one can remove this line
    }
    static const cookie_io_functions_t methods = {
        /* .read = */ [](void *self, char *buf, size_t sz) { return ((InMemoryFile *)self)->_read(buf, sz); },
        /* .write = */ [](void *self, const char *buf, size_t sz) { return ((InMemoryFile *)self)->_write(buf, sz); },
        /* .seek = */ [](void *self, off64_t *offset, int whence) { return ((InMemoryFile *)self)->_seek(offset, whence); },
        /* .close = */
        [](void *self) {
            ((InMemoryFile *)self)->fptr = nullptr;
            return 0;
        },
    };
    return fptr = fopencookie(this, mode, methods);
#else
    fprintf(stderr, "If you can read this, your platform does not support the features necessary to achieve your goals.");
    return nullptr;
#endif
}
