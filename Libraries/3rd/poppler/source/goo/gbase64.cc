//========================================================================
//
// gbase64.cc
//
// Implementation of a base64 encoder, because another one did not immediately
// avail itself.
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2018 Greg Knight <lyngvi@gmail.com>
//
//========================================================================

#include "gbase64.h"
#include <sstream>

static void b64encodeTriplet(char output[4], unsigned char a, unsigned char b, unsigned char c)
{
    static const char *base64table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    output[0] = base64table[((a >> 2) & 0x3f)]; // upper 6 of first byte
    output[1] = base64table[((a << 4) & 0x30) | ((b >> 4) & 0x0f)]; // lower 2 of first byte, upper 4 of second byte
    output[2] = base64table[((b << 2) & 0x3c) | ((c >> 6) & 0x03)]; // lower 4 of second byte, upper 2 of third byte
    output[3] = base64table[((c)&0x3f)]; // lower 6 of third byte
}

std::string gbase64Encode(const void *input, size_t len)
{
    char quad[4];
    size_t pos = 0;
    std::stringstream buf;
    auto bytes = static_cast<const unsigned char *>(input);
    for (; pos + 3 <= len; pos += 3) {
        b64encodeTriplet(quad, bytes[0], bytes[1], bytes[2]);
        buf.write(&quad[0], 4);
        bytes += 3;
    }
    switch (len - pos) {
    case 1:
        b64encodeTriplet(quad, bytes[0], 0, 0);
        quad[2] = quad[3] = '=';
        buf.write(&quad[0], 4);
        break;
    case 2:
        b64encodeTriplet(quad, bytes[0], bytes[1], 0);
        quad[3] = '=';
        buf.write(&quad[0], 4);
        break;
    }
    return buf.str();
}
