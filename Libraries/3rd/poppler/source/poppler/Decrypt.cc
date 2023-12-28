//========================================================================
//
// Decrypt.cc
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
// Copyright (C) 2008 Julien Rebetez <julien@fhtagn.net>
// Copyright (C) 2008, 2010, 2016-2021 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Matthias Franz <matthias@ktug.or.kr>
// Copyright (C) 2009 David Benjamin <davidben@mit.edu>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2013, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2016 Alok Anand <alok4nand@gmail.com>
// Copyright (C) 2016 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cstdint>
#include <cstring>
#include "goo/gmem.h"
#include "goo/grandom.h"
#include "Decrypt.h"
#include "Error.h"

static void rc4InitKey(const unsigned char *key, int keyLen, unsigned char *state);
static unsigned char rc4DecryptByte(unsigned char *state, unsigned char *x, unsigned char *y, unsigned char c);

static bool aesReadBlock(Stream *str, unsigned char *in, bool addPadding);

static void aesKeyExpansion(DecryptAESState *s, const unsigned char *objKey, int objKeyLen, bool decrypt);
static void aesEncryptBlock(DecryptAESState *s, const unsigned char *in);
static void aesDecryptBlock(DecryptAESState *s, const unsigned char *in, bool last);

static void aes256KeyExpansion(DecryptAES256State *s, const unsigned char *objKey, int objKeyLen, bool decrypt);
static void aes256EncryptBlock(DecryptAES256State *s, const unsigned char *in);
static void aes256DecryptBlock(DecryptAES256State *s, const unsigned char *in, bool last);

static void sha256(unsigned char *msg, int msgLen, unsigned char *hash);
static void sha384(unsigned char *msg, int msgLen, unsigned char *hash);
static void sha512(unsigned char *msg, int msgLen, unsigned char *hash);

static void revision6Hash(const GooString *inputPassword, unsigned char *K, const char *userKey);

static const unsigned char passwordPad[32] = { 0x28, 0xbf, 0x4e, 0x5e, 0x4e, 0x75, 0x8a, 0x41, 0x64, 0x00, 0x4e, 0x56, 0xff, 0xfa, 0x01, 0x08, 0x2e, 0x2e, 0x00, 0xb6, 0xd0, 0x68, 0x3e, 0x80, 0x2f, 0x0c, 0xa9, 0xfe, 0x64, 0x53, 0x69, 0x7a };

//------------------------------------------------------------------------
// Decrypt
//------------------------------------------------------------------------

bool Decrypt::makeFileKey(int encVersion, int encRevision, int keyLength, const GooString *ownerKey, const GooString *userKey, const GooString *ownerEnc, const GooString *userEnc, int permissions, const GooString *fileID,
                          const GooString *ownerPassword, const GooString *userPassword, unsigned char *fileKey, bool encryptMetadata, bool *ownerPasswordOk)
{
    DecryptAES256State state;
    unsigned char test[127 + 56], test2[32];
    GooString *userPassword2;
    unsigned char fState[256];
    unsigned char tmpKey[16];
    unsigned char fx, fy;
    int len, i, j;

    *ownerPasswordOk = false;

    if (encRevision == 5 || encRevision == 6) {

        // check the owner password
        if (ownerPassword) {
            //~ this is supposed to convert the password to UTF-8 using "SASLprep"
            len = ownerPassword->getLength();
            if (len > 127) {
                len = 127;
            }
            memcpy(test, ownerPassword->c_str(), len);
            memcpy(test + len, ownerKey->c_str() + 32, 8);
            memcpy(test + len + 8, userKey->c_str(), 48);
            sha256(test, len + 56, test);
            if (encRevision == 6) {
                // test contains the initial SHA-256 hash as input K.
                revision6Hash(ownerPassword, test, userKey->c_str());
            }
            if (!memcmp(test, ownerKey->c_str(), 32)) {

                // compute the file key from the owner password
                memcpy(test, ownerPassword->c_str(), len);
                memcpy(test + len, ownerKey->c_str() + 40, 8);
                memcpy(test + len + 8, userKey->c_str(), 48);
                sha256(test, len + 56, test);
                if (encRevision == 6) {
                    // test contains the initial SHA-256 hash input K.
                    revision6Hash(ownerPassword, test, userKey->c_str());
                }
                aes256KeyExpansion(&state, test, 32, true);
                for (i = 0; i < 16; ++i) {
                    state.cbc[i] = 0;
                }
                aes256DecryptBlock(&state, (unsigned char *)ownerEnc->c_str(), false);
                memcpy(fileKey, state.buf, 16);
                aes256DecryptBlock(&state, (unsigned char *)ownerEnc->c_str() + 16, false);
                memcpy(fileKey + 16, state.buf, 16);

                *ownerPasswordOk = true;
                return true;
            }
        }

        // check the user password
        if (userPassword) {
            //~ this is supposed to convert the password to UTF-8 using "SASLprep"
            len = userPassword->getLength();
            if (len > 127) {
                len = 127;
            }
            memcpy(test, userPassword->c_str(), len);
            memcpy(test + len, userKey->c_str() + 32, 8);
            sha256(test, len + 8, test);
            if (encRevision == 6) {
                // test contains the initial SHA-256 hash input K.
                // user key is not used in checking user password.
                revision6Hash(userPassword, test, nullptr);
            }
            if (!memcmp(test, userKey->c_str(), 32)) {

                // compute the file key from the user password
                memcpy(test, userPassword->c_str(), len);
                memcpy(test + len, userKey->c_str() + 40, 8);
                sha256(test, len + 8, test);
                if (encRevision == 6) {
                    // test contains the initial SHA-256 hash input K.
                    // user key is not used in computing intermediate user key.
                    revision6Hash(userPassword, test, nullptr);
                }
                aes256KeyExpansion(&state, test, 32, true);
                for (i = 0; i < 16; ++i) {
                    state.cbc[i] = 0;
                }
                aes256DecryptBlock(&state, (unsigned char *)userEnc->c_str(), false);
                memcpy(fileKey, state.buf, 16);
                aes256DecryptBlock(&state, (unsigned char *)userEnc->c_str() + 16, false);
                memcpy(fileKey + 16, state.buf, 16);

                return true;
            }
        }

        return false;
    } else {

        // try using the supplied owner password to generate the user password
        if (ownerPassword) {
            len = ownerPassword->getLength();
            if (len < 32) {
                memcpy(test, ownerPassword->c_str(), len);
                memcpy(test + len, passwordPad, 32 - len);
            } else {
                memcpy(test, ownerPassword->c_str(), 32);
            }
            md5(test, 32, test);
            if (encRevision == 3) {
                for (i = 0; i < 50; ++i) {
                    md5(test, keyLength, test);
                }
            }
            if (encRevision == 2) {
                rc4InitKey(test, keyLength, fState);
                fx = fy = 0;
                for (i = 0; i < 32; ++i) {
                    test2[i] = rc4DecryptByte(fState, &fx, &fy, ownerKey->getChar(i));
                }
            } else {
                memcpy(test2, ownerKey->c_str(), 32);
                for (i = 19; i >= 0; --i) {
                    for (j = 0; j < keyLength; ++j) {
                        tmpKey[j] = test[j] ^ i;
                    }
                    rc4InitKey(tmpKey, keyLength, fState);
                    fx = fy = 0;
                    for (j = 0; j < 32; ++j) {
                        test2[j] = rc4DecryptByte(fState, &fx, &fy, test2[j]);
                    }
                }
            }
            userPassword2 = new GooString((char *)test2, 32);
            if (makeFileKey2(encVersion, encRevision, keyLength, ownerKey, userKey, permissions, fileID, userPassword2, fileKey, encryptMetadata)) {
                *ownerPasswordOk = true;
                delete userPassword2;
                return true;
            }
            delete userPassword2;
        }

        // try using the supplied user password
        return makeFileKey2(encVersion, encRevision, keyLength, ownerKey, userKey, permissions, fileID, userPassword, fileKey, encryptMetadata);
    }
}

bool Decrypt::makeFileKey2(int encVersion, int encRevision, int keyLength, const GooString *ownerKey, const GooString *userKey, int permissions, const GooString *fileID, const GooString *userPassword, unsigned char *fileKey,
                           bool encryptMetadata)
{
    unsigned char *buf;
    unsigned char test[32];
    unsigned char fState[256];
    unsigned char tmpKey[16];
    unsigned char fx, fy;
    int len, i, j;
    bool ok;

    // generate file key
    buf = (unsigned char *)gmalloc(72 + fileID->getLength());
    if (userPassword) {
        len = userPassword->getLength();
        if (len < 32) {
            memcpy(buf, userPassword->c_str(), len);
            memcpy(buf + len, passwordPad, 32 - len);
        } else {
            memcpy(buf, userPassword->c_str(), 32);
        }
    } else {
        memcpy(buf, passwordPad, 32);
    }
    memcpy(buf + 32, ownerKey->c_str(), 32);
    buf[64] = permissions & 0xff;
    buf[65] = (permissions >> 8) & 0xff;
    buf[66] = (permissions >> 16) & 0xff;
    buf[67] = (permissions >> 24) & 0xff;
    memcpy(buf + 68, fileID->c_str(), fileID->getLength());
    len = 68 + fileID->getLength();
    if (!encryptMetadata) {
        buf[len++] = 0xff;
        buf[len++] = 0xff;
        buf[len++] = 0xff;
        buf[len++] = 0xff;
    }
    md5(buf, len, fileKey);
    if (encRevision == 3) {
        for (i = 0; i < 50; ++i) {
            md5(fileKey, keyLength, fileKey);
        }
    }

    // test user password
    if (encRevision == 2) {
        rc4InitKey(fileKey, keyLength, fState);
        fx = fy = 0;
        for (i = 0; i < 32; ++i) {
            test[i] = rc4DecryptByte(fState, &fx, &fy, userKey->getChar(i));
        }
        ok = memcmp(test, passwordPad, 32) == 0;
    } else if (encRevision == 3) {
        memcpy(test, userKey->c_str(), 32);
        for (i = 19; i >= 0; --i) {
            for (j = 0; j < keyLength; ++j) {
                tmpKey[j] = fileKey[j] ^ i;
            }
            rc4InitKey(tmpKey, keyLength, fState);
            fx = fy = 0;
            for (j = 0; j < 32; ++j) {
                test[j] = rc4DecryptByte(fState, &fx, &fy, test[j]);
            }
        }
        memcpy(buf, passwordPad, 32);
        memcpy(buf + 32, fileID->c_str(), fileID->getLength());
        md5(buf, 32 + fileID->getLength(), buf);
        ok = memcmp(test, buf, 16) == 0;
    } else {
        ok = false;
    }

    gfree(buf);
    return ok;
}

//------------------------------------------------------------------------
// BaseCryptStream
//------------------------------------------------------------------------

BaseCryptStream::BaseCryptStream(Stream *strA, const unsigned char *fileKey, CryptAlgorithm algoA, int keyLength, Ref refA) : FilterStream(strA)
{
    algo = algoA;

    // construct object key
    for (int i = 0; i < keyLength; ++i) {
        objKey[i] = fileKey[i];
    }
    for (std::size_t i = keyLength; i < sizeof(objKey); ++i) {
        objKey[i] = 0;
    }

    switch (algo) {
    case cryptRC4:
        if (likely(keyLength < static_cast<int>(sizeof(objKey) - 4))) {
            objKey[keyLength] = refA.num & 0xff;
            objKey[keyLength + 1] = (refA.num >> 8) & 0xff;
            objKey[keyLength + 2] = (refA.num >> 16) & 0xff;
            objKey[keyLength + 3] = refA.gen & 0xff;
            objKey[keyLength + 4] = (refA.gen >> 8) & 0xff;
            md5(objKey, keyLength + 5, objKey);
        }
        if ((objKeyLength = keyLength + 5) > 16) {
            objKeyLength = 16;
        }
        break;
    case cryptAES:
        objKey[keyLength] = refA.num & 0xff;
        objKey[keyLength + 1] = (refA.num >> 8) & 0xff;
        objKey[keyLength + 2] = (refA.num >> 16) & 0xff;
        objKey[keyLength + 3] = refA.gen & 0xff;
        objKey[keyLength + 4] = (refA.gen >> 8) & 0xff;
        objKey[keyLength + 5] = 0x73; // 's'
        objKey[keyLength + 6] = 0x41; // 'A'
        objKey[keyLength + 7] = 0x6c; // 'l'
        objKey[keyLength + 8] = 0x54; // 'T'
        md5(objKey, keyLength + 9, objKey);
        if ((objKeyLength = keyLength + 5) > 16) {
            objKeyLength = 16;
        }
        break;
    case cryptAES256:
        objKeyLength = keyLength;
        break;
    case cryptNone:
        break;
    }

    charactersRead = 0;
    nextCharBuff = EOF;
    autoDelete = true;
}

BaseCryptStream::~BaseCryptStream()
{
    if (autoDelete) {
        delete str;
    }
}

void BaseCryptStream::reset()
{
    charactersRead = 0;
    nextCharBuff = EOF;
    str->reset();
}

Goffset BaseCryptStream::getPos()
{
    return charactersRead;
}

int BaseCryptStream::getChar()
{
    // Read next character and empty the buffer, so that a new character will be read next time
    int c = lookChar();
    nextCharBuff = EOF;

    if (c != EOF) {
        charactersRead++;
    }
    return c;
}

bool BaseCryptStream::isBinary(bool last) const
{
    return str->isBinary(last);
}

void BaseCryptStream::setAutoDelete(bool val)
{
    autoDelete = val;
}

//------------------------------------------------------------------------
// EncryptStream
//------------------------------------------------------------------------

EncryptStream::EncryptStream(Stream *strA, const unsigned char *fileKey, CryptAlgorithm algoA, int keyLength, Ref refA) : BaseCryptStream(strA, fileKey, algoA, keyLength, refA)
{
    // Fill the CBC initialization vector for AES and AES-256
    switch (algo) {
    case cryptAES:
        grandom_fill(state.aes.cbc, 16);
        break;
    case cryptAES256:
        grandom_fill(state.aes256.cbc, 16);
        break;
    default:
        break;
    }
}

EncryptStream::~EncryptStream() { }

void EncryptStream::reset()
{
    BaseCryptStream::reset();

    switch (algo) {
    case cryptRC4:
        state.rc4.x = state.rc4.y = 0;
        rc4InitKey(objKey, objKeyLength, state.rc4.state);
        break;
    case cryptAES:
        aesKeyExpansion(&state.aes, objKey, objKeyLength, false);
        memcpy(state.aes.buf, state.aes.cbc, 16); // Copy CBC IV to buf
        state.aes.bufIdx = 0;
        state.aes.paddingReached = false;
        break;
    case cryptAES256:
        aes256KeyExpansion(&state.aes256, objKey, objKeyLength, false);
        memcpy(state.aes256.buf, state.aes256.cbc, 16); // Copy CBC IV to buf
        state.aes256.bufIdx = 0;
        state.aes256.paddingReached = false;
        break;
    case cryptNone:
        break;
    }
}

int EncryptStream::lookChar()
{
    unsigned char in[16];
    int c;

    if (nextCharBuff != EOF) {
        return nextCharBuff;
    }

    c = EOF; // make gcc happy
    switch (algo) {
    case cryptRC4:
        if ((c = str->getChar()) != EOF) {
            // RC4 is XOR-based: the decryption algorithm works for encryption too
            c = rc4DecryptByte(state.rc4.state, &state.rc4.x, &state.rc4.y, (unsigned char)c);
        }
        break;
    case cryptAES:
        if (state.aes.bufIdx == 16 && !state.aes.paddingReached) {
            state.aes.paddingReached = !aesReadBlock(str, in, true);
            aesEncryptBlock(&state.aes, in);
        }
        if (state.aes.bufIdx == 16) {
            c = EOF;
        } else {
            c = state.aes.buf[state.aes.bufIdx++];
        }
        break;
    case cryptAES256:
        if (state.aes256.bufIdx == 16 && !state.aes256.paddingReached) {
            state.aes256.paddingReached = !aesReadBlock(str, in, true);
            aes256EncryptBlock(&state.aes256, in);
        }
        if (state.aes256.bufIdx == 16) {
            c = EOF;
        } else {
            c = state.aes256.buf[state.aes256.bufIdx++];
        }
        break;
    case cryptNone:
        break;
    }
    return (nextCharBuff = c);
}

//------------------------------------------------------------------------
// DecryptStream
//------------------------------------------------------------------------

DecryptStream::DecryptStream(Stream *strA, const unsigned char *fileKey, CryptAlgorithm algoA, int keyLength, Ref refA) : BaseCryptStream(strA, fileKey, algoA, keyLength, refA) { }

DecryptStream::~DecryptStream() { }

void DecryptStream::reset()
{
    int i;
    BaseCryptStream::reset();

    switch (algo) {
    case cryptRC4:
        state.rc4.x = state.rc4.y = 0;
        rc4InitKey(objKey, objKeyLength, state.rc4.state);
        break;
    case cryptAES:
        aesKeyExpansion(&state.aes, objKey, objKeyLength, true);
        for (i = 0; i < 16; ++i) {
            state.aes.cbc[i] = str->getChar();
        }
        state.aes.bufIdx = 16;
        break;
    case cryptAES256:
        aes256KeyExpansion(&state.aes256, objKey, objKeyLength, true);
        for (i = 0; i < 16; ++i) {
            state.aes256.cbc[i] = str->getChar();
        }
        state.aes256.bufIdx = 16;
        break;
    case cryptNone:
        break;
    }
}

int DecryptStream::lookChar()
{
    unsigned char in[16];
    int c;

    if (nextCharBuff != EOF) {
        return nextCharBuff;
    }

    c = EOF; // make gcc happy
    switch (algo) {
    case cryptRC4:
        if ((c = str->getChar()) != EOF) {
            c = rc4DecryptByte(state.rc4.state, &state.rc4.x, &state.rc4.y, (unsigned char)c);
        }
        break;
    case cryptAES:
        if (state.aes.bufIdx == 16) {
            if (aesReadBlock(str, in, false)) {
                aesDecryptBlock(&state.aes, in, str->lookChar() == EOF);
            }
        }
        if (state.aes.bufIdx == 16) {
            c = EOF;
        } else {
            c = state.aes.buf[state.aes.bufIdx++];
        }
        break;
    case cryptAES256:
        if (state.aes256.bufIdx == 16) {
            if (aesReadBlock(str, in, false)) {
                aes256DecryptBlock(&state.aes256, in, str->lookChar() == EOF);
            }
        }
        if (state.aes256.bufIdx == 16) {
            c = EOF;
        } else {
            c = state.aes256.buf[state.aes256.bufIdx++];
        }
        break;
    case cryptNone:
        break;
    }
    return (nextCharBuff = c);
}

//------------------------------------------------------------------------
// RC4-compatible decryption
//------------------------------------------------------------------------

static void rc4InitKey(const unsigned char *key, int keyLen, unsigned char *state)
{
    unsigned char index1, index2;
    unsigned char t;
    int i;

    for (i = 0; i < 256; ++i) {
        state[i] = i;
    }

    if (unlikely(keyLen == 0)) {
        return;
    }

    index1 = index2 = 0;
    for (i = 0; i < 256; ++i) {
        index2 = (key[index1] + state[i] + index2) % 256;
        t = state[i];
        state[i] = state[index2];
        state[index2] = t;
        index1 = (index1 + 1) % keyLen;
    }
}

static unsigned char rc4DecryptByte(unsigned char *state, unsigned char *x, unsigned char *y, unsigned char c)
{
    unsigned char x1, y1, tx, ty;

    x1 = *x = (*x + 1) % 256;
    y1 = *y = (state[*x] + *y) % 256;
    tx = state[x1];
    ty = state[y1];
    state[x1] = ty;
    state[y1] = tx;
    return c ^ state[(tx + ty) % 256];
}

//------------------------------------------------------------------------
// AES decryption
//------------------------------------------------------------------------

// Returns false if EOF was reached, true otherwise
static bool aesReadBlock(Stream *str, unsigned char *in, bool addPadding)
{
    int c, i;

    for (i = 0; i < 16; ++i) {
        if ((c = str->getChar()) != EOF) {
            in[i] = (unsigned char)c;
        } else {
            break;
        }
    }

    if (i == 16) {
        return true;
    } else {
        if (addPadding) {
            c = 16 - i;
            while (i < 16) {
                in[i++] = (unsigned char)c;
            }
        }
        return false;
    }
}

static const unsigned char sbox[256] = { 0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, 0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
                                         0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, 0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
                                         0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, 0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
                                         0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, 0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
                                         0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, 0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
                                         0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, 0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
                                         0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, 0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
                                         0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, 0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };

static const unsigned char invSbox[256] = { 0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb, 0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
                                            0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e, 0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
                                            0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92, 0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
                                            0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06, 0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
                                            0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73, 0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
                                            0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b, 0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
                                            0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f, 0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
                                            0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61, 0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d };

static const unsigned int rcon[11] = { 0x00000000, // unused
                                       0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000, 0x1b000000, 0x36000000 };

static inline unsigned int subWord(unsigned int x)
{
    return (sbox[x >> 24] << 24) | (sbox[(x >> 16) & 0xff] << 16) | (sbox[(x >> 8) & 0xff] << 8) | sbox[x & 0xff];
}

static inline unsigned int rotWord(unsigned int x)
{
    return ((x << 8) & 0xffffffff) | (x >> 24);
}

static inline void subBytes(unsigned char *state)
{
    int i;

    for (i = 0; i < 16; ++i) {
        state[i] = sbox[state[i]];
    }
}

static inline void invSubBytes(unsigned char *state)
{
    int i;

    for (i = 0; i < 16; ++i) {
        state[i] = invSbox[state[i]];
    }
}

static inline void shiftRows(unsigned char *state)
{
    unsigned char t;

    t = state[4];
    state[4] = state[5];
    state[5] = state[6];
    state[6] = state[7];
    state[7] = t;

    t = state[8];
    state[8] = state[10];
    state[10] = t;
    t = state[9];
    state[9] = state[11];
    state[11] = t;

    t = state[15];
    state[15] = state[14];
    state[14] = state[13];
    state[13] = state[12];
    state[12] = t;
}

static inline void invShiftRows(unsigned char *state)
{
    unsigned char t;

    t = state[7];
    state[7] = state[6];
    state[6] = state[5];
    state[5] = state[4];
    state[4] = t;

    t = state[8];
    state[8] = state[10];
    state[10] = t;
    t = state[9];
    state[9] = state[11];
    state[11] = t;

    t = state[12];
    state[12] = state[13];
    state[13] = state[14];
    state[14] = state[15];
    state[15] = t;
}

// {02} \cdot s
struct Mul02Table
{
    constexpr Mul02Table() : values()
    {
        for (int s = 0; s < 256; s++) {
            values[s] = (s & 0x80) ? ((s << 1) ^ 0x1b) : (s << 1);
        }
    }

    constexpr unsigned char operator()(uint8_t i) const { return values[i]; }

    unsigned char values[256];
};

static constexpr Mul02Table mul02;

// {03} \cdot s
struct Mul03Table
{
    constexpr Mul03Table() : values()
    {
        for (int s = 0; s < 256; s++) {
            const unsigned char s2 = (s & 0x80) ? ((s << 1) ^ 0x1b) : (s << 1);
            values[s] = s ^ s2;
        }
    }

    constexpr unsigned char operator()(uint8_t i) const { return values[i]; }

    unsigned char values[256];
};

static constexpr Mul03Table mul03;

// {09} \cdot s
struct Mul09Table
{
    constexpr Mul09Table() : values()
    {
        for (int s = 0; s < 256; s++) {
            const unsigned char s2 = (s & 0x80) ? ((s << 1) ^ 0x1b) : (s << 1);
            const unsigned char s4 = (s2 & 0x80) ? ((s2 << 1) ^ 0x1b) : (s2 << 1);
            const unsigned char s8 = (s4 & 0x80) ? ((s4 << 1) ^ 0x1b) : (s4 << 1);
            values[s] = s ^ s8;
        }
    }

    constexpr unsigned char operator()(uint8_t i) const { return values[i]; }

    unsigned char values[256];
};

static constexpr Mul09Table mul09;

// {0b} \cdot s
struct Mul0bTable
{
    constexpr Mul0bTable() : values()
    {
        for (int s = 0; s < 256; s++) {
            const unsigned char s2 = (s & 0x80) ? ((s << 1) ^ 0x1b) : (s << 1);
            const unsigned char s4 = (s2 & 0x80) ? ((s2 << 1) ^ 0x1b) : (s2 << 1);
            const unsigned char s8 = (s4 & 0x80) ? ((s4 << 1) ^ 0x1b) : (s4 << 1);
            values[s] = s ^ s2 ^ s8;
        }
    }

    constexpr unsigned char operator()(uint8_t i) const { return values[i]; }

    unsigned char values[256];
};

static constexpr Mul0bTable mul0b;

// {0d} \cdot s
struct Mul0dTable
{
    constexpr Mul0dTable() : values()
    {
        for (int s = 0; s < 256; s++) {
            const unsigned char s2 = (s & 0x80) ? ((s << 1) ^ 0x1b) : (s << 1);
            const unsigned char s4 = (s2 & 0x80) ? ((s2 << 1) ^ 0x1b) : (s2 << 1);
            const unsigned char s8 = (s4 & 0x80) ? ((s4 << 1) ^ 0x1b) : (s4 << 1);
            values[s] = s ^ s4 ^ s8;
        }
    }

    constexpr unsigned char operator()(uint8_t i) const { return values[i]; }

    unsigned char values[256];
};

static constexpr Mul0dTable mul0d;

// {0e} \cdot s
struct Mul0eTable
{
    constexpr Mul0eTable() : values()
    {
        for (int s = 0; s < 256; s++) {
            const unsigned char s2 = (s & 0x80) ? ((s << 1) ^ 0x1b) : (s << 1);
            const unsigned char s4 = (s2 & 0x80) ? ((s2 << 1) ^ 0x1b) : (s2 << 1);
            const unsigned char s8 = (s4 & 0x80) ? ((s4 << 1) ^ 0x1b) : (s4 << 1);
            values[s] = s2 ^ s4 ^ s8;
        }
    }

    constexpr unsigned char operator()(uint8_t i) const { return values[i]; }

    unsigned char values[256];
};

static constexpr Mul0eTable mul0e;

static inline void mixColumns(unsigned char *state)
{
    int c;
    unsigned char s0, s1, s2, s3;

    for (c = 0; c < 4; ++c) {
        s0 = state[c];
        s1 = state[4 + c];
        s2 = state[8 + c];
        s3 = state[12 + c];
        state[c] = mul02(s0) ^ mul03(s1) ^ s2 ^ s3;
        state[4 + c] = s0 ^ mul02(s1) ^ mul03(s2) ^ s3;
        state[8 + c] = s0 ^ s1 ^ mul02(s2) ^ mul03(s3);
        state[12 + c] = mul03(s0) ^ s1 ^ s2 ^ mul02(s3);
    }
}

static inline void invMixColumns(unsigned char *state)
{
    int c;
    unsigned char s0, s1, s2, s3;

    for (c = 0; c < 4; ++c) {
        s0 = state[c];
        s1 = state[4 + c];
        s2 = state[8 + c];
        s3 = state[12 + c];
        state[c] = mul0e(s0) ^ mul0b(s1) ^ mul0d(s2) ^ mul09(s3);
        state[4 + c] = mul09(s0) ^ mul0e(s1) ^ mul0b(s2) ^ mul0d(s3);
        state[8 + c] = mul0d(s0) ^ mul09(s1) ^ mul0e(s2) ^ mul0b(s3);
        state[12 + c] = mul0b(s0) ^ mul0d(s1) ^ mul09(s2) ^ mul0e(s3);
    }
}

static inline void invMixColumnsW(unsigned int *w)
{
    int c;
    unsigned char s0, s1, s2, s3;

    for (c = 0; c < 4; ++c) {
        s0 = w[c] >> 24;
        s1 = w[c] >> 16;
        s2 = w[c] >> 8;
        s3 = w[c];
        w[c] = ((mul0e(s0) ^ mul0b(s1) ^ mul0d(s2) ^ mul09(s3)) << 24) | ((mul09(s0) ^ mul0e(s1) ^ mul0b(s2) ^ mul0d(s3)) << 16) | ((mul0d(s0) ^ mul09(s1) ^ mul0e(s2) ^ mul0b(s3)) << 8) | (mul0b(s0) ^ mul0d(s1) ^ mul09(s2) ^ mul0e(s3));
    }
}

static inline void addRoundKey(unsigned char *state, const unsigned int *w)
{
    int c;

    for (c = 0; c < 4; ++c) {
        state[c] ^= w[c] >> 24;
        state[4 + c] ^= w[c] >> 16;
        state[8 + c] ^= w[c] >> 8;
        state[12 + c] ^= w[c];
    }
}

static void aesKeyExpansion(DecryptAESState *s, const unsigned char *objKey, int /*objKeyLen*/, bool decrypt)
{
    unsigned int temp;
    int i, round;

    //~ this assumes objKeyLen == 16

    for (i = 0; i < 4; ++i) {
        s->w[i] = (objKey[4 * i] << 24) + (objKey[4 * i + 1] << 16) + (objKey[4 * i + 2] << 8) + objKey[4 * i + 3];
    }
    for (i = 4; i < 44; ++i) {
        temp = s->w[i - 1];
        if (!(i & 3)) {
            temp = subWord(rotWord(temp)) ^ rcon[i / 4];
        }
        s->w[i] = s->w[i - 4] ^ temp;
    }

    /* In case of decryption, adjust the key schedule for the equivalent inverse cipher */
    if (decrypt) {
        for (round = 1; round <= 9; ++round) {
            invMixColumnsW(&s->w[round * 4]);
        }
    }
}

static void aesEncryptBlock(DecryptAESState *s, const unsigned char *in)
{
    int c, round;

    // initial state (input is xor'd with previous output because of CBC)
    for (c = 0; c < 4; ++c) {
        s->state[c] = in[4 * c] ^ s->buf[4 * c];
        s->state[4 + c] = in[4 * c + 1] ^ s->buf[4 * c + 1];
        s->state[8 + c] = in[4 * c + 2] ^ s->buf[4 * c + 2];
        s->state[12 + c] = in[4 * c + 3] ^ s->buf[4 * c + 3];
    }

    // round 0
    addRoundKey(s->state, &s->w[0]);

    // rounds 1-9
    for (round = 1; round <= 9; ++round) {
        subBytes(s->state);
        shiftRows(s->state);
        mixColumns(s->state);
        addRoundKey(s->state, &s->w[round * 4]);
    }

    // round 10
    subBytes(s->state);
    shiftRows(s->state);
    addRoundKey(s->state, &s->w[10 * 4]);

    for (c = 0; c < 4; ++c) {
        s->buf[4 * c] = s->state[c];
        s->buf[4 * c + 1] = s->state[4 + c];
        s->buf[4 * c + 2] = s->state[8 + c];
        s->buf[4 * c + 3] = s->state[12 + c];
    }

    s->bufIdx = 0;
}

static void aesDecryptBlock(DecryptAESState *s, const unsigned char *in, bool last)
{
    int c, round, n, i;

    // initial state
    for (c = 0; c < 4; ++c) {
        s->state[c] = in[4 * c];
        s->state[4 + c] = in[4 * c + 1];
        s->state[8 + c] = in[4 * c + 2];
        s->state[12 + c] = in[4 * c + 3];
    }

    // round 0
    addRoundKey(s->state, &s->w[10 * 4]);

    // rounds 1-9
    for (round = 9; round >= 1; --round) {
        invSubBytes(s->state);
        invShiftRows(s->state);
        invMixColumns(s->state);
        addRoundKey(s->state, &s->w[round * 4]);
    }

    // round 10
    invSubBytes(s->state);
    invShiftRows(s->state);
    addRoundKey(s->state, &s->w[0]);

    // CBC
    for (c = 0; c < 4; ++c) {
        s->buf[4 * c] = s->state[c] ^ s->cbc[4 * c];
        s->buf[4 * c + 1] = s->state[4 + c] ^ s->cbc[4 * c + 1];
        s->buf[4 * c + 2] = s->state[8 + c] ^ s->cbc[4 * c + 2];
        s->buf[4 * c + 3] = s->state[12 + c] ^ s->cbc[4 * c + 3];
    }

    // save the input block for the next CBC
    for (i = 0; i < 16; ++i) {
        s->cbc[i] = in[i];
    }

    // remove padding
    s->bufIdx = 0;
    if (last) {
        n = s->buf[15];
        if (n < 1 || n > 16) { // this should never happen
            n = 16;
        }
        for (i = 15; i >= n; --i) {
            s->buf[i] = s->buf[i - n];
        }
        s->bufIdx = n;
    }
}

//------------------------------------------------------------------------
// AES-256 decryption
//------------------------------------------------------------------------

static void aes256KeyExpansion(DecryptAES256State *s, const unsigned char *objKey, int objKeyLen, bool decrypt)
{
    unsigned int temp;
    int i, round;

    //~ this assumes objKeyLen == 32

    for (i = 0; i < 8; ++i) {
        s->w[i] = (objKey[4 * i] << 24) + (objKey[4 * i + 1] << 16) + (objKey[4 * i + 2] << 8) + objKey[4 * i + 3];
    }
    for (i = 8; i < 60; ++i) {
        temp = s->w[i - 1];
        if ((i & 7) == 0) {
            temp = subWord(rotWord(temp)) ^ rcon[i / 8];
        } else if ((i & 7) == 4) {
            temp = subWord(temp);
        }
        s->w[i] = s->w[i - 8] ^ temp;
    }

    /* In case of decryption, adjust the key schedule for the equivalent inverse cipher */
    if (decrypt) {
        for (round = 1; round <= 13; ++round) {
            invMixColumnsW(&s->w[round * 4]);
        }
    }
}

static void aes256EncryptBlock(DecryptAES256State *s, const unsigned char *in)
{
    int c, round;

    // initial state (input is xor'd with previous output because of CBC)
    for (c = 0; c < 4; ++c) {
        s->state[c] = in[4 * c] ^ s->buf[4 * c];
        s->state[4 + c] = in[4 * c + 1] ^ s->buf[4 * c + 1];
        s->state[8 + c] = in[4 * c + 2] ^ s->buf[4 * c + 2];
        s->state[12 + c] = in[4 * c + 3] ^ s->buf[4 * c + 3];
    }

    // round 0
    addRoundKey(s->state, &s->w[0]);

    // rounds 1-13
    for (round = 1; round <= 13; ++round) {
        subBytes(s->state);
        shiftRows(s->state);
        mixColumns(s->state);
        addRoundKey(s->state, &s->w[round * 4]);
    }

    // round 14
    subBytes(s->state);
    shiftRows(s->state);
    addRoundKey(s->state, &s->w[14 * 4]);

    for (c = 0; c < 4; ++c) {
        s->buf[4 * c] = s->state[c];
        s->buf[4 * c + 1] = s->state[4 + c];
        s->buf[4 * c + 2] = s->state[8 + c];
        s->buf[4 * c + 3] = s->state[12 + c];
    }

    s->bufIdx = 0;
}

static void aes256DecryptBlock(DecryptAES256State *s, const unsigned char *in, bool last)
{
    int c, round, n, i;

    // initial state
    for (c = 0; c < 4; ++c) {
        s->state[c] = in[4 * c];
        s->state[4 + c] = in[4 * c + 1];
        s->state[8 + c] = in[4 * c + 2];
        s->state[12 + c] = in[4 * c + 3];
    }

    // round 0
    addRoundKey(s->state, &s->w[14 * 4]);

    // rounds 13-1
    for (round = 13; round >= 1; --round) {
        invSubBytes(s->state);
        invShiftRows(s->state);
        invMixColumns(s->state);
        addRoundKey(s->state, &s->w[round * 4]);
    }

    // round 14
    invSubBytes(s->state);
    invShiftRows(s->state);
    addRoundKey(s->state, &s->w[0]);

    // CBC
    for (c = 0; c < 4; ++c) {
        s->buf[4 * c] = s->state[c] ^ s->cbc[4 * c];
        s->buf[4 * c + 1] = s->state[4 + c] ^ s->cbc[4 * c + 1];
        s->buf[4 * c + 2] = s->state[8 + c] ^ s->cbc[4 * c + 2];
        s->buf[4 * c + 3] = s->state[12 + c] ^ s->cbc[4 * c + 3];
    }

    // save the input block for the next CBC
    for (i = 0; i < 16; ++i) {
        s->cbc[i] = in[i];
    }

    // remove padding
    s->bufIdx = 0;
    if (last) {
        n = s->buf[15];
        if (n < 1 || n > 16) { // this should never happen
            n = 16;
        }
        for (i = 15; i >= n; --i) {
            s->buf[i] = s->buf[i - n];
        }
        s->bufIdx = n;
        if (n > 16) {
            error(errSyntaxError, -1, "Reducing bufIdx from {0:d} to 16 to not crash", n);
            s->bufIdx = 16;
        }
    }
}

//------------------------------------------------------------------------
// MD5 message digest
//------------------------------------------------------------------------

// this works around a bug in older Sun compilers
static inline unsigned long rotateLeft(unsigned long x, int r)
{
    x &= 0xffffffff;
    return ((x << r) | (x >> (32 - r))) & 0xffffffff;
}

static inline unsigned long md5Round1(unsigned long a, unsigned long b, unsigned long c, unsigned long d, unsigned long Xk, unsigned long s, unsigned long Ti)
{
    return b + rotateLeft((a + ((b & c) | (~b & d)) + Xk + Ti), s);
}

static inline unsigned long md5Round2(unsigned long a, unsigned long b, unsigned long c, unsigned long d, unsigned long Xk, unsigned long s, unsigned long Ti)
{
    return b + rotateLeft((a + ((b & d) | (c & ~d)) + Xk + Ti), s);
}

static inline unsigned long md5Round3(unsigned long a, unsigned long b, unsigned long c, unsigned long d, unsigned long Xk, unsigned long s, unsigned long Ti)
{
    return b + rotateLeft((a + (b ^ c ^ d) + Xk + Ti), s);
}

static inline unsigned long md5Round4(unsigned long a, unsigned long b, unsigned long c, unsigned long d, unsigned long Xk, unsigned long s, unsigned long Ti)
{
    return b + rotateLeft((a + (c ^ (b | ~d)) + Xk + Ti), s);
}

struct MD5State
{
    unsigned long a, b, c, d;
    unsigned char buf[64];
    int bufLen;
    int msgLen;
    unsigned char digest[16];
};

static void md5Start(MD5State *state)
{
    state->a = 0x67452301;
    state->b = 0xefcdab89;
    state->c = 0x98badcfe;
    state->d = 0x10325476;
    state->bufLen = 0;
    state->msgLen = 0;
}

static void md5ProcessBlock(MD5State *state)
{
    unsigned long x[16];

    for (int i = 0; i < 16; ++i) {
        x[i] = state->buf[4 * i] | (state->buf[4 * i + 1] << 8) | (state->buf[4 * i + 2] << 16) | (state->buf[4 * i + 3] << 24);
    }

    unsigned long a = state->a;
    unsigned long b = state->b;
    unsigned long c = state->c;
    unsigned long d = state->d;

    // round 1
    a = md5Round1(a, b, c, d, x[0], 7, 0xd76aa478);
    d = md5Round1(d, a, b, c, x[1], 12, 0xe8c7b756);
    c = md5Round1(c, d, a, b, x[2], 17, 0x242070db);
    b = md5Round1(b, c, d, a, x[3], 22, 0xc1bdceee);
    a = md5Round1(a, b, c, d, x[4], 7, 0xf57c0faf);
    d = md5Round1(d, a, b, c, x[5], 12, 0x4787c62a);
    c = md5Round1(c, d, a, b, x[6], 17, 0xa8304613);
    b = md5Round1(b, c, d, a, x[7], 22, 0xfd469501);
    a = md5Round1(a, b, c, d, x[8], 7, 0x698098d8);
    d = md5Round1(d, a, b, c, x[9], 12, 0x8b44f7af);
    c = md5Round1(c, d, a, b, x[10], 17, 0xffff5bb1);
    b = md5Round1(b, c, d, a, x[11], 22, 0x895cd7be);
    a = md5Round1(a, b, c, d, x[12], 7, 0x6b901122);
    d = md5Round1(d, a, b, c, x[13], 12, 0xfd987193);
    c = md5Round1(c, d, a, b, x[14], 17, 0xa679438e);
    b = md5Round1(b, c, d, a, x[15], 22, 0x49b40821);

    // round 2
    a = md5Round2(a, b, c, d, x[1], 5, 0xf61e2562);
    d = md5Round2(d, a, b, c, x[6], 9, 0xc040b340);
    c = md5Round2(c, d, a, b, x[11], 14, 0x265e5a51);
    b = md5Round2(b, c, d, a, x[0], 20, 0xe9b6c7aa);
    a = md5Round2(a, b, c, d, x[5], 5, 0xd62f105d);
    d = md5Round2(d, a, b, c, x[10], 9, 0x02441453);
    c = md5Round2(c, d, a, b, x[15], 14, 0xd8a1e681);
    b = md5Round2(b, c, d, a, x[4], 20, 0xe7d3fbc8);
    a = md5Round2(a, b, c, d, x[9], 5, 0x21e1cde6);
    d = md5Round2(d, a, b, c, x[14], 9, 0xc33707d6);
    c = md5Round2(c, d, a, b, x[3], 14, 0xf4d50d87);
    b = md5Round2(b, c, d, a, x[8], 20, 0x455a14ed);
    a = md5Round2(a, b, c, d, x[13], 5, 0xa9e3e905);
    d = md5Round2(d, a, b, c, x[2], 9, 0xfcefa3f8);
    c = md5Round2(c, d, a, b, x[7], 14, 0x676f02d9);
    b = md5Round2(b, c, d, a, x[12], 20, 0x8d2a4c8a);

    // round 3
    a = md5Round3(a, b, c, d, x[5], 4, 0xfffa3942);
    d = md5Round3(d, a, b, c, x[8], 11, 0x8771f681);
    c = md5Round3(c, d, a, b, x[11], 16, 0x6d9d6122);
    b = md5Round3(b, c, d, a, x[14], 23, 0xfde5380c);
    a = md5Round3(a, b, c, d, x[1], 4, 0xa4beea44);
    d = md5Round3(d, a, b, c, x[4], 11, 0x4bdecfa9);
    c = md5Round3(c, d, a, b, x[7], 16, 0xf6bb4b60);
    b = md5Round3(b, c, d, a, x[10], 23, 0xbebfbc70);
    a = md5Round3(a, b, c, d, x[13], 4, 0x289b7ec6);
    d = md5Round3(d, a, b, c, x[0], 11, 0xeaa127fa);
    c = md5Round3(c, d, a, b, x[3], 16, 0xd4ef3085);
    b = md5Round3(b, c, d, a, x[6], 23, 0x04881d05);
    a = md5Round3(a, b, c, d, x[9], 4, 0xd9d4d039);
    d = md5Round3(d, a, b, c, x[12], 11, 0xe6db99e5);
    c = md5Round3(c, d, a, b, x[15], 16, 0x1fa27cf8);
    b = md5Round3(b, c, d, a, x[2], 23, 0xc4ac5665);

    // round 4
    a = md5Round4(a, b, c, d, x[0], 6, 0xf4292244);
    d = md5Round4(d, a, b, c, x[7], 10, 0x432aff97);
    c = md5Round4(c, d, a, b, x[14], 15, 0xab9423a7);
    b = md5Round4(b, c, d, a, x[5], 21, 0xfc93a039);
    a = md5Round4(a, b, c, d, x[12], 6, 0x655b59c3);
    d = md5Round4(d, a, b, c, x[3], 10, 0x8f0ccc92);
    c = md5Round4(c, d, a, b, x[10], 15, 0xffeff47d);
    b = md5Round4(b, c, d, a, x[1], 21, 0x85845dd1);
    a = md5Round4(a, b, c, d, x[8], 6, 0x6fa87e4f);
    d = md5Round4(d, a, b, c, x[15], 10, 0xfe2ce6e0);
    c = md5Round4(c, d, a, b, x[6], 15, 0xa3014314);
    b = md5Round4(b, c, d, a, x[13], 21, 0x4e0811a1);
    a = md5Round4(a, b, c, d, x[4], 6, 0xf7537e82);
    d = md5Round4(d, a, b, c, x[11], 10, 0xbd3af235);
    c = md5Round4(c, d, a, b, x[2], 15, 0x2ad7d2bb);
    b = md5Round4(b, c, d, a, x[9], 21, 0xeb86d391);

    // increment a, b, c, d
    state->a += a;
    state->b += b;
    state->c += c;
    state->d += d;

    state->bufLen = 0;
}

static void md5Append(MD5State *state, const unsigned char *data, int dataLen)
{
    const unsigned char *p = data;
    int remain = dataLen;
    while (state->bufLen + remain >= 64) {
        const int k = 64 - state->bufLen;
        memcpy(state->buf + state->bufLen, p, k);
        state->bufLen = 64;
        md5ProcessBlock(state);
        p += k;
        remain -= k;
    }
    if (remain > 0) {
        memcpy(state->buf + state->bufLen, p, remain);
        state->bufLen += remain;
    }
    state->msgLen += dataLen;
}

static void md5Finish(MD5State *state)
{
    // padding and length
    state->buf[state->bufLen++] = 0x80;
    if (state->bufLen > 56) {
        while (state->bufLen < 64) {
            state->buf[state->bufLen++] = 0x00;
        }
        md5ProcessBlock(state);
    }
    while (state->bufLen < 56) {
        state->buf[state->bufLen++] = 0x00;
    }
    state->buf[56] = (unsigned char)(state->msgLen << 3);
    state->buf[57] = (unsigned char)(state->msgLen >> 5);
    state->buf[58] = (unsigned char)(state->msgLen >> 13);
    state->buf[59] = (unsigned char)(state->msgLen >> 21);
    state->buf[60] = (unsigned char)(state->msgLen >> 29);
    state->buf[61] = (unsigned char)0;
    state->buf[62] = (unsigned char)0;
    state->buf[63] = (unsigned char)0;
    state->bufLen = 64;
    md5ProcessBlock(state);

    // break digest into bytes
    state->digest[0] = (unsigned char)state->a;
    state->digest[1] = (unsigned char)(state->a >> 8);
    state->digest[2] = (unsigned char)(state->a >> 16);
    state->digest[3] = (unsigned char)(state->a >> 24);
    state->digest[4] = (unsigned char)state->b;
    state->digest[5] = (unsigned char)(state->b >> 8);
    state->digest[6] = (unsigned char)(state->b >> 16);
    state->digest[7] = (unsigned char)(state->b >> 24);
    state->digest[8] = (unsigned char)state->c;
    state->digest[9] = (unsigned char)(state->c >> 8);
    state->digest[10] = (unsigned char)(state->c >> 16);
    state->digest[11] = (unsigned char)(state->c >> 24);
    state->digest[12] = (unsigned char)state->d;
    state->digest[13] = (unsigned char)(state->d >> 8);
    state->digest[14] = (unsigned char)(state->d >> 16);
    state->digest[15] = (unsigned char)(state->d >> 24);
}

void md5(const unsigned char *msg, int msgLen, unsigned char *digest)
{
    if (msgLen < 0) {
        return;
    }
    MD5State state;
    md5Start(&state);
    md5Append(&state, msg, msgLen);
    md5Finish(&state);
    for (int i = 0; i < 16; ++i) {
        digest[i] = state.digest[i];
    }
}

//------------------------------------------------------------------------
// SHA-256 hash
//------------------------------------------------------------------------

static const unsigned int sha256K[64] = { 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
                                          0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
                                          0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
                                          0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };

static inline unsigned int rotr(unsigned int x, unsigned int n)
{
    return (x >> n) | (x << (32 - n));
}

static inline unsigned int sha256Ch(unsigned int x, unsigned int y, unsigned int z)
{
    return (x & y) ^ (~x & z);
}

static inline unsigned int sha256Maj(unsigned int x, unsigned int y, unsigned int z)
{
    return (x & y) ^ (x & z) ^ (y & z);
}

static inline unsigned int sha256Sigma0(unsigned int x)
{
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

static inline unsigned int sha256Sigma1(unsigned int x)
{
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

static inline unsigned int sha256sigma0(unsigned int x)
{
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

static inline unsigned int sha256sigma1(unsigned int x)
{
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

static void sha256HashBlock(const unsigned char *blk, unsigned int *H)
{
    unsigned int W[64];
    unsigned int a, b, c, d, e, f, g, h;
    unsigned int T1, T2;
    unsigned int t;

    // 1. prepare the message schedule
    for (t = 0; t < 16; ++t) {
        W[t] = (blk[t * 4] << 24) | (blk[t * 4 + 1] << 16) | (blk[t * 4 + 2] << 8) | blk[t * 4 + 3];
    }
    for (t = 16; t < 64; ++t) {
        W[t] = sha256sigma1(W[t - 2]) + W[t - 7] + sha256sigma0(W[t - 15]) + W[t - 16];
    }

    // 2. initialize the eight working variables
    a = H[0];
    b = H[1];
    c = H[2];
    d = H[3];
    e = H[4];
    f = H[5];
    g = H[6];
    h = H[7];

    // 3.
    for (t = 0; t < 64; ++t) {
        T1 = h + sha256Sigma1(e) + sha256Ch(e, f, g) + sha256K[t] + W[t];
        T2 = sha256Sigma0(a) + sha256Maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
    }

    // 4. compute the intermediate hash value
    H[0] += a;
    H[1] += b;
    H[2] += c;
    H[3] += d;
    H[4] += e;
    H[5] += f;
    H[6] += g;
    H[7] += h;
}

static void sha256(unsigned char *msg, int msgLen, unsigned char *hash)
{
    unsigned char blk[64];
    unsigned int H[8];
    int blkLen, i;

    H[0] = 0x6a09e667;
    H[1] = 0xbb67ae85;
    H[2] = 0x3c6ef372;
    H[3] = 0xa54ff53a;
    H[4] = 0x510e527f;
    H[5] = 0x9b05688c;
    H[6] = 0x1f83d9ab;
    H[7] = 0x5be0cd19;

    blkLen = 0;
    for (i = 0; i + 64 <= msgLen; i += 64) {
        sha256HashBlock(msg + i, H);
    }
    blkLen = msgLen - i;
    if (blkLen > 0) {
        memcpy(blk, msg + i, blkLen);
    }

    // pad the message
    blk[blkLen++] = 0x80;
    if (blkLen > 56) {
        while (blkLen < 64) {
            blk[blkLen++] = 0;
        }
        sha256HashBlock(blk, H);
        blkLen = 0;
    }
    while (blkLen < 56) {
        blk[blkLen++] = 0;
    }
    blk[56] = 0;
    blk[57] = 0;
    blk[58] = 0;
    blk[59] = 0;
    blk[60] = (unsigned char)(msgLen >> 21);
    blk[61] = (unsigned char)(msgLen >> 13);
    blk[62] = (unsigned char)(msgLen >> 5);
    blk[63] = (unsigned char)(msgLen << 3);
    sha256HashBlock(blk, H);

    // copy the output into the buffer (convert words to bytes)
    for (i = 0; i < 8; ++i) {
        hash[i * 4] = (unsigned char)(H[i] >> 24);
        hash[i * 4 + 1] = (unsigned char)(H[i] >> 16);
        hash[i * 4 + 2] = (unsigned char)(H[i] >> 8);
        hash[i * 4 + 3] = (unsigned char)H[i];
    }
}
//------------------------------------------------------------------------
// SHA-512 hash (see FIPS 180-4)
//------------------------------------------------------------------------
// SHA 384 and SHA 512 use the same sequence of eighty constant 64 bit words.
static const uint64_t shaK[80] = { 0x428a2f98d728ae22ull, 0x7137449123ef65cdull, 0xb5c0fbcfec4d3b2full, 0xe9b5dba58189dbbcull, 0x3956c25bf348b538ull, 0x59f111f1b605d019ull, 0x923f82a4af194f9bull, 0xab1c5ed5da6d8118ull,
                                   0xd807aa98a3030242ull, 0x12835b0145706fbeull, 0x243185be4ee4b28cull, 0x550c7dc3d5ffb4e2ull, 0x72be5d74f27b896full, 0x80deb1fe3b1696b1ull, 0x9bdc06a725c71235ull, 0xc19bf174cf692694ull,
                                   0xe49b69c19ef14ad2ull, 0xefbe4786384f25e3ull, 0x0fc19dc68b8cd5b5ull, 0x240ca1cc77ac9c65ull, 0x2de92c6f592b0275ull, 0x4a7484aa6ea6e483ull, 0x5cb0a9dcbd41fbd4ull, 0x76f988da831153b5ull,
                                   0x983e5152ee66dfabull, 0xa831c66d2db43210ull, 0xb00327c898fb213full, 0xbf597fc7beef0ee4ull, 0xc6e00bf33da88fc2ull, 0xd5a79147930aa725ull, 0x06ca6351e003826full, 0x142929670a0e6e70ull,
                                   0x27b70a8546d22ffcull, 0x2e1b21385c26c926ull, 0x4d2c6dfc5ac42aedull, 0x53380d139d95b3dfull, 0x650a73548baf63deull, 0x766a0abb3c77b2a8ull, 0x81c2c92e47edaee6ull, 0x92722c851482353bull,
                                   0xa2bfe8a14cf10364ull, 0xa81a664bbc423001ull, 0xc24b8b70d0f89791ull, 0xc76c51a30654be30ull, 0xd192e819d6ef5218ull, 0xd69906245565a910ull, 0xf40e35855771202aull, 0x106aa07032bbd1b8ull,
                                   0x19a4c116b8d2d0c8ull, 0x1e376c085141ab53ull, 0x2748774cdf8eeb99ull, 0x34b0bcb5e19b48a8ull, 0x391c0cb3c5c95a63ull, 0x4ed8aa4ae3418acbull, 0x5b9cca4f7763e373ull, 0x682e6ff3d6b2b8a3ull,
                                   0x748f82ee5defb2fcull, 0x78a5636f43172f60ull, 0x84c87814a1f0ab72ull, 0x8cc702081a6439ecull, 0x90befffa23631e28ull, 0xa4506cebde82bde9ull, 0xbef9a3f7b2c67915ull, 0xc67178f2e372532bull,
                                   0xca273eceea26619cull, 0xd186b8c721c0c207ull, 0xeada7dd6cde0eb1eull, 0xf57d4f7fee6ed178ull, 0x06f067aa72176fbaull, 0x0a637dc5a2c898a6ull, 0x113f9804bef90daeull, 0x1b710b35131c471bull,
                                   0x28db77f523047d84ull, 0x32caab7b40c72493ull, 0x3c9ebe0a15c9bebcull, 0x431d67c49c100d4cull, 0x4cc5d4becb3e42b6ull, 0x597f299cfc657e2aull, 0x5fcb6fab3ad6faecull, 0x6c44198c4a475817ull };

static inline uint64_t rotr(uint64_t x, uint64_t n)
{
    return (x >> n) | (x << (64 - n));
}
static inline uint64_t sha512Ch(uint64_t x, uint64_t y, uint64_t z)
{
    return (x & y) ^ (~x & z);
}
static inline uint64_t sha512Maj(uint64_t x, uint64_t y, uint64_t z)
{
    return (x & y) ^ (x & z) ^ (y & z);
}
static inline uint64_t sha512Sigma0(uint64_t x)
{
    return rotr(x, 28) ^ rotr(x, 34) ^ rotr(x, 39);
}
static inline uint64_t sha512Sigma1(uint64_t x)
{
    return rotr(x, 14) ^ rotr(x, 18) ^ rotr(x, 41);
}
static inline uint64_t sha512sigma0(uint64_t x)
{
    return rotr(x, 1) ^ rotr(x, 8) ^ (x >> 7);
}
static inline uint64_t sha512sigma1(uint64_t x)
{
    return rotr(x, 19) ^ rotr(x, 61) ^ (x >> 6);
}

static void sha512HashBlock(const unsigned char *blk, uint64_t *H)
{
    uint64_t W[80];
    uint64_t a, b, c, d, e, f, g, h;
    uint64_t T1, T2;
    unsigned int t;

    // 1. prepare the message schedule
    for (t = 0; t < 16; ++t) {
        W[t] = (((uint64_t)blk[t * 8] << 56) | ((uint64_t)blk[t * 8 + 1] << 48) | ((uint64_t)blk[t * 8 + 2] << 40) | ((uint64_t)blk[t * 8 + 3] << 32) | ((uint64_t)blk[t * 8 + 4] << 24) | ((uint64_t)blk[t * 8 + 5] << 16)
                | ((uint64_t)blk[t * 8 + 6] << 8) | ((uint64_t)blk[t * 8 + 7]));
    }
    for (t = 16; t < 80; ++t) {
        W[t] = sha512sigma1(W[t - 2]) + W[t - 7] + sha512sigma0(W[t - 15]) + W[t - 16];
    }

    // 2. initialize the eight working variables
    a = H[0];
    b = H[1];
    c = H[2];
    d = H[3];
    e = H[4];
    f = H[5];
    g = H[6];
    h = H[7];

    // 3.
    for (t = 0; t < 80; ++t) {
        T1 = h + sha512Sigma1(e) + sha512Ch(e, f, g) + shaK[t] + W[t];
        T2 = sha512Sigma0(a) + sha512Maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
    }

    // 4. compute the intermediate hash value
    H[0] += a;
    H[1] += b;
    H[2] += c;
    H[3] += d;
    H[4] += e;
    H[5] += f;
    H[6] += g;
    H[7] += h;
}

static void sha512(unsigned char *msg, int msgLen, unsigned char *hash)
{
    unsigned char blk[128];
    uint64_t H[8];
    int blkLen = 0, i;
    // setting the initial hash value.
    H[0] = 0x6a09e667f3bcc908ull;
    H[1] = 0xbb67ae8584caa73bull;
    H[2] = 0x3c6ef372fe94f82bull;
    H[3] = 0xa54ff53a5f1d36f1ull;
    H[4] = 0x510e527fade682d1ull;
    H[5] = 0x9b05688c2b3e6c1full;
    H[6] = 0x1f83d9abfb41bd6bull;
    H[7] = 0x5be0cd19137e2179ull;

    for (i = 0; i + 128 <= msgLen; i += 128) {
        sha512HashBlock(msg + i, H);
    }
    blkLen = msgLen - i;
    if (blkLen > 0) {
        memcpy(blk, msg + i, blkLen);
    }

    // pad the message
    blk[blkLen++] = 0x80;
    if (blkLen > 112) {
        while (blkLen < 128) {
            blk[blkLen++] = 0;
        }
        sha512HashBlock(blk, H);
        blkLen = 0;
    }
    while (blkLen < 112) {
        blk[blkLen++] = 0;
    }
    blk[112] = 0;
    blk[113] = 0;
    blk[114] = 0;
    blk[115] = 0;
    blk[116] = 0;
    blk[117] = 0;
    blk[118] = 0;
    blk[119] = 0;
    blk[120] = 0;
    blk[121] = 0;
    blk[122] = 0;
    blk[123] = 0;
    blk[124] = (unsigned char)(msgLen >> 21);
    blk[125] = (unsigned char)(msgLen >> 13);
    blk[126] = (unsigned char)(msgLen >> 5);
    blk[127] = (unsigned char)(msgLen << 3);

    sha512HashBlock(blk, H);

    // copy the output into the buffer (convert words to bytes)
    for (i = 0; i < 8; ++i) {
        hash[i * 8] = (unsigned char)(H[i] >> 56);
        hash[i * 8 + 1] = (unsigned char)(H[i] >> 48);
        hash[i * 8 + 2] = (unsigned char)(H[i] >> 40);
        hash[i * 8 + 3] = (unsigned char)(H[i] >> 32);
        hash[i * 8 + 4] = (unsigned char)(H[i] >> 24);
        hash[i * 8 + 5] = (unsigned char)(H[i] >> 16);
        hash[i * 8 + 6] = (unsigned char)(H[i] >> 8);
        hash[i * 8 + 7] = (unsigned char)H[i];
    }
}

//------------------------------------------------------------------------
// SHA-384 (see FIPS 180-4)
//------------------------------------------------------------------------
// The algorithm is defined in the exact same manner as SHA 512 with 2 exceptions
// 1.Initial hash value is different.
// 2.A 384 bit message digest is obtained by truncating the final hash value.
static void sha384(unsigned char *msg, int msgLen, unsigned char *hash)
{
    unsigned char blk[128];
    uint64_t H[8];
    int blkLen, i;
    // setting initial hash values
    H[0] = 0xcbbb9d5dc1059ed8ull;
    H[1] = 0x629a292a367cd507ull;
    H[2] = 0x9159015a3070dd17ull;
    H[3] = 0x152fecd8f70e5939ull;
    H[4] = 0x67332667ffc00b31ull;
    H[5] = 0x8eb44a8768581511ull;
    H[6] = 0xdb0c2e0d64f98fa7ull;
    H[7] = 0x47b5481dbefa4fa4ull;
    // SHA 384 will use the same sha512HashBlock function.
    blkLen = 0;
    for (i = 0; i + 128 <= msgLen; i += 128) {
        sha512HashBlock(msg + i, H);
    }
    blkLen = msgLen - i;
    if (blkLen > 0) {
        memcpy(blk, msg + i, blkLen);
    }

    // pad the message
    blk[blkLen++] = 0x80;
    if (blkLen > 112) {
        while (blkLen < 128) {
            blk[blkLen++] = 0;
        }
        sha512HashBlock(blk, H);
        blkLen = 0;
    }
    while (blkLen < 112) {
        blk[blkLen++] = 0;
    }
    blk[112] = 0;
    blk[113] = 0;
    blk[114] = 0;
    blk[115] = 0;
    blk[116] = 0;
    blk[117] = 0;
    blk[118] = 0;
    blk[119] = 0;
    blk[120] = 0;
    blk[121] = 0;
    blk[122] = 0;
    blk[123] = 0;
    blk[124] = (unsigned char)(msgLen >> 21);
    blk[125] = (unsigned char)(msgLen >> 13);
    blk[126] = (unsigned char)(msgLen >> 5);
    blk[127] = (unsigned char)(msgLen << 3);

    sha512HashBlock(blk, H);

    // copy the output into the buffer (convert words to bytes)
    // hash is truncated to 384 bits.
    for (i = 0; i < 6; ++i) {
        hash[i * 8] = (unsigned char)(H[i] >> 56);
        hash[i * 8 + 1] = (unsigned char)(H[i] >> 48);
        hash[i * 8 + 2] = (unsigned char)(H[i] >> 40);
        hash[i * 8 + 3] = (unsigned char)(H[i] >> 32);
        hash[i * 8 + 4] = (unsigned char)(H[i] >> 24);
        hash[i * 8 + 5] = (unsigned char)(H[i] >> 16);
        hash[i * 8 + 6] = (unsigned char)(H[i] >> 8);
        hash[i * 8 + 7] = (unsigned char)H[i];
    }
}

//------------------------------------------------------------------------
// Section 7.6.3.3 (Encryption Key algorithm) of ISO/DIS 32000-2
// Algorithm 2.B:Computing a hash (for revision 6).
//------------------------------------------------------------------------
static void revision6Hash(const GooString *inputPassword, unsigned char *K, const char *userKey)
{
    unsigned char K1[64 * (127 + 64 + 48)];
    unsigned char E[64 * (127 + 64 + 48)];
    DecryptAESState state;
    unsigned char aesKey[16];
    unsigned char BE16byteNumber[16];

    int inputPasswordLength = inputPassword->getLength();
    int KLength = 32;
    const int userKeyLength = userKey ? 48 : 0;
    int sequenceLength;
    int totalLength;
    int rounds = 0;

    while (rounds < 64 || rounds < E[totalLength - 1] + 32) {
        sequenceLength = inputPasswordLength + KLength + userKeyLength;
        totalLength = 64 * sequenceLength;
        // a.make the string K1
        memcpy(K1, inputPassword->c_str(), inputPasswordLength);
        memcpy(K1 + inputPasswordLength, K, KLength);
        if (userKey) {
            memcpy(K1 + inputPasswordLength + KLength, userKey, userKeyLength);
        }
        for (int i = 1; i < 64; ++i) {
            memcpy(K1 + (i * sequenceLength), K1, sequenceLength);
        }
        // b.Encrypt K1
        memcpy(aesKey, K, 16);
        memcpy(state.cbc, K + 16, 16);
        memcpy(state.buf, state.cbc, 16); // Copy CBC IV to buf
        state.bufIdx = 0;
        state.paddingReached = false;
        aesKeyExpansion(&state, aesKey, 16, false);

        for (int i = 0; i < (4 * sequenceLength); i++) {
            aesEncryptBlock(&state, K1 + (16 * i));
            memcpy(E + (16 * i), state.buf, 16);
        }
        memcpy(BE16byteNumber, E, 16);
        // c.Taking the first 16 Bytes of E as unsigned big-endian integer,
        // compute the remainder,modulo 3.
        uint64_t N1 = 0, N2 = 0, N3 = 0;
        // N1 contains first 8 bytes of BE16byteNumber
        N1 = ((uint64_t)BE16byteNumber[0] << 56 | (uint64_t)BE16byteNumber[1] << 48 | (uint64_t)BE16byteNumber[2] << 40 | (uint64_t)BE16byteNumber[3] << 32 | (uint64_t)BE16byteNumber[4] << 24 | (uint64_t)BE16byteNumber[5] << 16
              | (uint64_t)BE16byteNumber[6] << 8 | (uint64_t)BE16byteNumber[7]);
        uint64_t rem = N1 % 3;
        // N2 contains 0s in higher 4 bytes and 9th to 12 th bytes of BE16byteNumber in lower 4 bytes.
        N2 = ((uint64_t)BE16byteNumber[8] << 24 | (uint64_t)BE16byteNumber[9] << 16 | (uint64_t)BE16byteNumber[10] << 8 | (uint64_t)BE16byteNumber[11]);
        rem = ((rem << 32) | N2) % 3;
        // N3 contains 0s in higher 4 bytes and 13th to 16th bytes of BE16byteNumber in lower 4 bytes.
        N3 = ((uint64_t)BE16byteNumber[12] << 24 | (uint64_t)BE16byteNumber[13] << 16 | (uint64_t)BE16byteNumber[14] << 8 | (uint64_t)BE16byteNumber[15]);
        rem = ((rem << 32) | N3) % 3;

        // d.If remainder is 0 perform SHA-256
        if (rem == 0) {
            KLength = 32;
            sha256(E, totalLength, K);
        }
        // remainder is 1 perform SHA-384
        else if (rem == 1) {
            KLength = 48;
            sha384(E, totalLength, K);
        }
        // remainder is 2 perform SHA-512
        else if (rem == 2) {
            KLength = 64;
            sha512(E, totalLength, K);
        }
        rounds++;
    }
    // the first 32 bytes of the final K are the output of the function.
}
