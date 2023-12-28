//========================================================================
//
// gbase64.h
//
// Implementation of a base64 encoder, because another one did not immediately
// avail itself.
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2018 Greg Knight <lyngvi@gmail.com>
// Copyright (C) 2019 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#ifndef GOO_GBASE64_H
#define GOO_GBASE64_H

#include "poppler_private_export.h"

#include <string>
#include <vector>

std::string POPPLER_PRIVATE_EXPORT gbase64Encode(const void *input, size_t len);

inline std::string gbase64Encode(const std::vector<char> &input)
{
    return input.empty() ? std::string() : gbase64Encode(&input[0], input.size());
}

inline std::string gbase64Encode(const std::vector<unsigned char> &input)
{
    return input.empty() ? std::string() : gbase64Encode(&input[0], input.size());
}

#endif // ndef GOO_GBASE64_H
