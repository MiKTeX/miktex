//========================================================================
//
// Error.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005, 2007 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2005, 2018, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2007 Krzysztof Kowalczyk <kkowalczyk@gmail.com>
// Copyright (C) 2012 Marek Kasik <mkasik@redhat.com>
// Copyright (C) 2013, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2020 Adam Reichold <adam.reichold@t-online.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>
#include <poppler-config.h>

#include <cstdio>
#include <cstddef>
#include <cstdarg>
#include "GooString.h"
#include "GlobalParams.h"
#include "Error.h"

static const char *errorCategoryNames[] = { "Syntax Warning", "Syntax Error", "Config Error", "Command Line Error", "I/O Error", "Permission Error", "Unimplemented Feature", "Internal Error" };

static ErrorCallback errorCbk = nullptr;

void setErrorCallback(ErrorCallback cbk)
{
    errorCbk = cbk;
}

void CDECL error(ErrorCategory category, Goffset pos, const char *msg, ...)
{
    va_list args;

    // NB: this can be called before the globalParams object is created
    if (!errorCbk && globalParams && globalParams->getErrQuiet()) {
        return;
    }
    va_start(args, msg);
    const std::unique_ptr<GooString> s = GooString::formatv(msg, args);
    va_end(args);

    GooString sanitized;
    for (int i = 0; i < s->getLength(); ++i) {
        const char c = s->getChar(i);
        if (c < (char)0x20 || c >= (char)0x7f) {
            sanitized.appendf("<{0:02x}>", c & 0xff);
        } else {
            sanitized.append(c);
        }
    }

    if (errorCbk) {
        (*errorCbk)(category, pos, sanitized.c_str());
    } else {
        if (pos >= 0) {
            fprintf(stderr, "%s (%lld): %s\n", errorCategoryNames[category], (long long)pos, sanitized.c_str());
        } else {
            fprintf(stderr, "%s: %s\n", errorCategoryNames[category], sanitized.c_str());
        }
        fflush(stderr);
    }
}
