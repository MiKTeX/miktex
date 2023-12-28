//========================================================================
//
// gbasename.h
//
// Wrapper for libgen's basename() call which returns a std::string.
// This is a convenience method working around questionable behavior
// in the copy of basename() provided by libgen.h.
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2018 Greg Knight <lyngvi@gmail.com>
// Copyright (C) 2019 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#ifndef GBASENAME_H
#define GBASENAME_H

#include <string>
#include "poppler_private_export.h"

std::string POPPLER_PRIVATE_EXPORT gbasename(const char *filename);

#endif // ndef GBASENAME_H
