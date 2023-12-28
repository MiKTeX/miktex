//========================================================================
//
// Function.cc
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006, 2008-2010, 2013-2015, 2017-2020, 2022, 2023 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2006 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2010 Christian Feuersänger <cfeuersaenger@googlemail.com>
// Copyright (C) 2011 Andrea Canciani <ranma42@gmail.com>
// Copyright (C) 2012 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2012 Adam Reichold <adamreichold@myopera.com>
// Copyright (C) 2013 Fabio D'Urso <fabiodurso@hotmail.it>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cmath>
#include "goo/gmem.h"
#include "goo/gstrtod.h"
#include "Object.h"
#include "Dict.h"
#include "Stream.h"
#include "Error.h"
#include "Function.h"

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

//------------------------------------------------------------------------
// Function
//------------------------------------------------------------------------

Function::Function() : domain {} { }

Function::~Function() { }

Function *Function::parse(Object *funcObj)
{
    std::set<int> usedParents;
    return parse(funcObj, &usedParents);
}

Function *Function::parse(Object *funcObj, std::set<int> *usedParents)
{
    Function *func;
    Dict *dict;
    int funcType;

    if (funcObj->isStream()) {
        dict = funcObj->streamGetDict();
    } else if (funcObj->isDict()) {
        dict = funcObj->getDict();
    } else if (funcObj->isName("Identity")) {
        return new IdentityFunction();
    } else {
        error(errSyntaxError, -1, "Expected function dictionary or stream");
        return nullptr;
    }

    Object obj1 = dict->lookup("FunctionType");
    if (!obj1.isInt()) {
        error(errSyntaxError, -1, "Function type is missing or wrong type");
        return nullptr;
    }
    funcType = obj1.getInt();

    if (funcType == 0) {
        func = new SampledFunction(funcObj, dict);
    } else if (funcType == 2) {
        func = new ExponentialFunction(funcObj, dict);
    } else if (funcType == 3) {
        func = new StitchingFunction(funcObj, dict, usedParents);
    } else if (funcType == 4) {
        func = new PostScriptFunction(funcObj, dict);
    } else {
        error(errSyntaxError, -1, "Unimplemented function type ({0:d})", funcType);
        return nullptr;
    }
    if (!func->isOk()) {
        delete func;
        return nullptr;
    }

    return func;
}

Function::Function(const Function *func)
{
    m = func->m;
    n = func->n;

    memcpy(domain, func->domain, funcMaxInputs * 2 * sizeof(double));
    memcpy(range, func->range, funcMaxOutputs * 2 * sizeof(double));

    hasRange = func->hasRange;
}

bool Function::init(Dict *dict)
{
    Object obj1;
    int i;

    //----- Domain
    obj1 = dict->lookup("Domain");
    if (!obj1.isArray()) {
        error(errSyntaxError, -1, "Function is missing domain");
        return false;
    }
    m = obj1.arrayGetLength() / 2;
    if (m > funcMaxInputs) {
        error(errSyntaxError, -1, "Functions with more than {0:d} inputs are unsupported", funcMaxInputs);
        return false;
    }
    for (i = 0; i < m; ++i) {
        Object obj2 = obj1.arrayGet(2 * i);
        if (!obj2.isNum()) {
            error(errSyntaxError, -1, "Illegal value in function domain array");
            return false;
        }
        domain[i][0] = obj2.getNum();
        obj2 = obj1.arrayGet(2 * i + 1);
        if (!obj2.isNum()) {
            error(errSyntaxError, -1, "Illegal value in function domain array");
            return false;
        }
        domain[i][1] = obj2.getNum();
    }

    //----- Range
    hasRange = false;
    n = 0;
    obj1 = dict->lookup("Range");
    if (obj1.isArray()) {
        hasRange = true;
        n = obj1.arrayGetLength() / 2;
        if (n > funcMaxOutputs) {
            error(errSyntaxError, -1, "Functions with more than {0:d} outputs are unsupported", funcMaxOutputs);
            return false;
        }
        for (i = 0; i < n; ++i) {
            Object obj2 = obj1.arrayGet(2 * i);
            if (!obj2.isNum()) {
                error(errSyntaxError, -1, "Illegal value in function range array");
                return false;
            }
            range[i][0] = obj2.getNum();
            obj2 = obj1.arrayGet(2 * i + 1);
            if (!obj2.isNum()) {
                error(errSyntaxError, -1, "Illegal value in function range array");
                return false;
            }
            range[i][1] = obj2.getNum();
        }
    }

    return true;
}

//------------------------------------------------------------------------
// IdentityFunction
//------------------------------------------------------------------------

IdentityFunction::IdentityFunction()
{
    int i;

    // fill these in with arbitrary values just in case they get used
    // somewhere
    m = funcMaxInputs;
    n = funcMaxOutputs;
    for (i = 0; i < funcMaxInputs; ++i) {
        domain[i][0] = 0;
        domain[i][1] = 1;
    }
    hasRange = false;
}

IdentityFunction::~IdentityFunction() { }

void IdentityFunction::transform(const double *in, double *out) const
{
    int i;

    for (i = 0; i < funcMaxOutputs; ++i) {
        out[i] = in[i];
    }
}

//------------------------------------------------------------------------
// SampledFunction
//------------------------------------------------------------------------

SampledFunction::SampledFunction(Object *funcObj, Dict *dict) : cacheOut {}
{
    Stream *str;
    int sampleBits;
    double sampleMul;
    Object obj1;
    unsigned int buf, bitMask;
    int bits;
    unsigned int s;
    double in[funcMaxInputs];
    int i, j, t, bit, idx;

    idxOffset = nullptr;
    samples = nullptr;
    sBuf = nullptr;
    ok = false;

    //----- initialize the generic stuff
    if (!init(dict)) {
        return;
    }
    if (!hasRange) {
        error(errSyntaxError, -1, "Type 0 function is missing range");
        return;
    }
    if (m > sampledFuncMaxInputs) {
        error(errSyntaxError, -1, "Sampled functions with more than {0:d} inputs are unsupported", sampledFuncMaxInputs);
        return;
    }

    //----- buffer
    sBuf = (double *)gmallocn(1 << m, sizeof(double));

    //----- get the stream
    if (!funcObj->isStream()) {
        error(errSyntaxError, -1, "Type 0 function isn't a stream");
        return;
    }
    str = funcObj->getStream();

    //----- Size
    obj1 = dict->lookup("Size");
    if (!obj1.isArray() || obj1.arrayGetLength() != m) {
        error(errSyntaxError, -1, "Function has missing or invalid size array");
        return;
    }
    for (i = 0; i < m; ++i) {
        Object obj2 = obj1.arrayGet(i);
        if (!obj2.isInt()) {
            error(errSyntaxError, -1, "Illegal value in function size array");
            return;
        }
        sampleSize[i] = obj2.getInt();
        if (sampleSize[i] <= 0) {
            error(errSyntaxError, -1, "Illegal non-positive value in function size array");
            return;
        }
    }
    idxOffset = (int *)gmallocn(1 << m, sizeof(int));
    for (i = 0; i < (1 << m); ++i) {
        idx = 0;
        for (j = m - 1, t = i; j >= 1; --j, t <<= 1) {
            if (sampleSize[j] == 1) {
                bit = 0;
            } else {
                bit = (t >> (m - 1)) & 1;
            }
            idx = (idx + bit) * sampleSize[j - 1];
        }
        if (m > 0 && sampleSize[0] == 1) {
            bit = 0;
        } else {
            bit = (t >> (m - 1)) & 1;
        }
        idxOffset[i] = (idx + bit) * n;
    }

    //----- BitsPerSample
    obj1 = dict->lookup("BitsPerSample");
    if (!obj1.isInt()) {
        error(errSyntaxError, -1, "Function has missing or invalid BitsPerSample");
        return;
    }
    sampleBits = obj1.getInt();
    if (unlikely(sampleBits < 1 || sampleBits > 32)) {
        error(errSyntaxError, -1, "Function invalid BitsPerSample");
        return;
    }
    sampleMul = 1.0 / (pow(2.0, (double)sampleBits) - 1);

    //----- Encode
    obj1 = dict->lookup("Encode");
    if (obj1.isArray() && obj1.arrayGetLength() == 2 * m) {
        for (i = 0; i < m; ++i) {
            Object obj2 = obj1.arrayGet(2 * i);
            if (!obj2.isNum()) {
                error(errSyntaxError, -1, "Illegal value in function encode array");
                return;
            }
            encode[i][0] = obj2.getNum();
            obj2 = obj1.arrayGet(2 * i + 1);
            if (!obj2.isNum()) {
                error(errSyntaxError, -1, "Illegal value in function encode array");
                return;
            }
            encode[i][1] = obj2.getNum();
        }
    } else {
        for (i = 0; i < m; ++i) {
            encode[i][0] = 0;
            encode[i][1] = sampleSize[i] - 1;
        }
    }
    for (i = 0; i < m; ++i) {
        if (unlikely((domain[i][1] - domain[i][0]) == 0)) {
            error(errSyntaxError, -1, "Illegal value in function domain array");
            return;
        }
        inputMul[i] = (encode[i][1] - encode[i][0]) / (domain[i][1] - domain[i][0]);
    }

    //----- Decode
    obj1 = dict->lookup("Decode");
    if (obj1.isArray() && obj1.arrayGetLength() == 2 * n) {
        for (i = 0; i < n; ++i) {
            Object obj2 = obj1.arrayGet(2 * i);
            if (!obj2.isNum()) {
                error(errSyntaxError, -1, "Illegal value in function decode array");
                return;
            }
            decode[i][0] = obj2.getNum();
            obj2 = obj1.arrayGet(2 * i + 1);
            if (!obj2.isNum()) {
                error(errSyntaxError, -1, "Illegal value in function decode array");
                return;
            }
            decode[i][1] = obj2.getNum();
        }
    } else {
        for (i = 0; i < n; ++i) {
            decode[i][0] = range[i][0];
            decode[i][1] = range[i][1];
        }
    }

    //----- samples
    nSamples = n;
    for (i = 0; i < m; ++i) {
        nSamples *= sampleSize[i];
    }
    samples = (double *)gmallocn_checkoverflow(nSamples, sizeof(double));
    if (!samples) {
        error(errSyntaxError, -1, "Function has invalid number of samples");
        return;
    }
    buf = 0;
    bits = 0;
    bitMask = (1 << sampleBits) - 1;
    str->reset();
    for (i = 0; i < nSamples; ++i) {
        if (sampleBits == 8) {
            s = str->getChar();
        } else if (sampleBits == 16) {
            s = str->getChar();
            s = (s << 8) + str->getChar();
        } else if (sampleBits == 32) {
            s = str->getChar();
            s = (s << 8) + str->getChar();
            s = (s << 8) + str->getChar();
            s = (s << 8) + str->getChar();
        } else {
            while (bits < sampleBits) {
                buf = (buf << 8) | (str->getChar() & 0xff);
                bits += 8;
            }
            s = (buf >> (bits - sampleBits)) & bitMask;
            bits -= sampleBits;
        }
        samples[i] = (double)s * sampleMul;
    }
    str->close();

    // set up the cache
    for (i = 0; i < m; ++i) {
        in[i] = domain[i][0];
        cacheIn[i] = in[i] - 1;
    }
    transform(in, cacheOut);

    ok = true;
}

SampledFunction::~SampledFunction()
{
    if (idxOffset) {
        gfree(idxOffset);
    }
    if (samples) {
        gfree(samples);
    }
    if (sBuf) {
        gfree(sBuf);
    }
}

SampledFunction::SampledFunction(const SampledFunction *func) : Function(func)
{
    memcpy(sampleSize, func->sampleSize, funcMaxInputs * sizeof(int));

    memcpy(encode, func->encode, funcMaxInputs * 2 * sizeof(double));
    memcpy(decode, func->decode, funcMaxOutputs * 2 * sizeof(double));

    memcpy(inputMul, func->inputMul, funcMaxInputs * sizeof(double));

    nSamples = func->nSamples;

    idxOffset = (int *)gmallocn(1 << m, sizeof(int));
    memcpy(idxOffset, func->idxOffset, (1 << m) * (int)sizeof(int));

    samples = (double *)gmallocn(nSamples, sizeof(double));
    memcpy(samples, func->samples, nSamples * sizeof(double));

    sBuf = (double *)gmallocn(1 << m, sizeof(double));

    memcpy(cacheIn, func->cacheIn, funcMaxInputs * sizeof(double));
    memcpy(cacheOut, func->cacheOut, funcMaxOutputs * sizeof(double));

    ok = func->ok;
}

void SampledFunction::transform(const double *in, double *out) const
{
    double x;
    int e[funcMaxInputs];
    double efrac0[funcMaxInputs];
    double efrac1[funcMaxInputs];

    // check the cache
    bool inCache = true;
    for (int i = 0; i < m; ++i) {
        if (in[i] != cacheIn[i]) {
            inCache = false;
            break;
        }
    }
    if (inCache) {
        for (int i = 0; i < n; ++i) {
            out[i] = cacheOut[i];
        }
        return;
    }

    // map input values into sample array
    for (int i = 0; i < m; ++i) {
        x = (in[i] - domain[i][0]) * inputMul[i] + encode[i][0];
        if (x < 0 || std::isnan(x)) {
            x = 0;
        } else if (x > sampleSize[i] - 1) {
            x = sampleSize[i] - 1;
        }
        e[i] = (int)x;
        if (e[i] == sampleSize[i] - 1 && sampleSize[i] > 1) {
            // this happens if in[i] = domain[i][1]
            e[i] = sampleSize[i] - 2;
        }
        efrac1[i] = x - e[i];
        efrac0[i] = 1 - efrac1[i];
    }

    // compute index for the first sample to be used
    int idx0 = 0;
    for (int k = m - 1; k >= 1; --k) {
        idx0 = (idx0 + e[k]) * sampleSize[k - 1];
    }
    idx0 = (idx0 + e[0]) * n;

    // for each output, do m-linear interpolation
    for (int i = 0; i < n; ++i) {

        // pull 2^m values out of the sample array
        for (int j = 0; j < (1 << m); ++j) {
            int idx = idx0 + idxOffset[j] + i;
            if (likely(idx >= 0 && idx < nSamples)) {
                sBuf[j] = samples[idx];
            } else {
                sBuf[j] = 0; // TODO Investigate if this is what Adobe does
            }
        }

        // do m sets of interpolations
        for (int j = 0, t = (1 << m); j < m; ++j, t >>= 1) {
            for (int k = 0; k < t; k += 2) {
                sBuf[k >> 1] = efrac0[j] * sBuf[k] + efrac1[j] * sBuf[k + 1];
            }
        }

        // map output value to range
        out[i] = sBuf[0] * (decode[i][1] - decode[i][0]) + decode[i][0];
        if (out[i] < range[i][0]) {
            out[i] = range[i][0];
        } else if (out[i] > range[i][1]) {
            out[i] = range[i][1];
        }
    }

    // save current result in the cache
    for (int i = 0; i < m; ++i) {
        cacheIn[i] = in[i];
    }
    for (int i = 0; i < n; ++i) {
        cacheOut[i] = out[i];
    }
}

bool SampledFunction::hasDifferentResultSet(const Function *func) const
{
    if (func->getType() == 0) {
        SampledFunction *compTo = (SampledFunction *)func;
        if (compTo->getSampleNumber() != nSamples) {
            return true;
        }
        const double *compSamples = compTo->getSamples();
        for (int i = 0; i < nSamples; i++) {
            if (samples[i] != compSamples[i]) {
                return true;
            }
        }
    }
    return false;
}

//------------------------------------------------------------------------
// ExponentialFunction
//------------------------------------------------------------------------

ExponentialFunction::ExponentialFunction(Object *funcObj, Dict *dict)
{
    Object obj1;

    ok = false;

    //----- initialize the generic stuff
    if (!init(dict)) {
        return;
    }
    if (m != 1) {
        error(errSyntaxError, -1, "Exponential function with more than one input");
        return;
    }

    //----- C0
    obj1 = dict->lookup("C0");
    if (obj1.isArray()) {
        if (hasRange && obj1.arrayGetLength() != n) {
            error(errSyntaxError, -1, "Function's C0 array is wrong length");
            return;
        }
        n = obj1.arrayGetLength();
        if (unlikely(n > funcMaxOutputs)) {
            error(errSyntaxError, -1, "Function's C0 array is wrong length");
            n = funcMaxOutputs;
        }
        for (int i = 0; i < n; ++i) {
            Object obj2 = obj1.arrayGet(i);
            if (!obj2.isNum()) {
                error(errSyntaxError, -1, "Illegal value in function C0 array");
                return;
            }
            c0[i] = obj2.getNum();
        }
    } else {
        if (hasRange && n != 1) {
            error(errSyntaxError, -1, "Function's C0 array is wrong length");
            return;
        }
        n = 1;
        c0[0] = 0;
    }

    //----- C1
    obj1 = dict->lookup("C1");
    if (obj1.isArray()) {
        if (obj1.arrayGetLength() != n) {
            error(errSyntaxError, -1, "Function's C1 array is wrong length");
            return;
        }
        for (int i = 0; i < n; ++i) {
            Object obj2 = obj1.arrayGet(i);
            if (!obj2.isNum()) {
                error(errSyntaxError, -1, "Illegal value in function C1 array");
                return;
            }
            c1[i] = obj2.getNum();
        }
    } else {
        if (n != 1) {
            error(errSyntaxError, -1, "Function's C1 array is wrong length");
            return;
        }
        c1[0] = 1;
    }

    //----- N (exponent)
    obj1 = dict->lookup("N");
    if (!obj1.isNum()) {
        error(errSyntaxError, -1, "Function has missing or invalid N");
        return;
    }
    e = obj1.getNum();

    isLinear = fabs(e - 1.) < 1e-10;
    ok = true;
}

ExponentialFunction::~ExponentialFunction() { }

ExponentialFunction::ExponentialFunction(const ExponentialFunction *func) : Function(func)
{
    memcpy(c0, func->c0, funcMaxOutputs * sizeof(double));
    memcpy(c1, func->c1, funcMaxOutputs * sizeof(double));

    e = func->e;
    isLinear = func->isLinear;
    ok = func->ok;
}

void ExponentialFunction::transform(const double *in, double *out) const
{
    double x;
    int i;

    if (in[0] < domain[0][0]) {
        x = domain[0][0];
    } else if (in[0] > domain[0][1]) {
        x = domain[0][1];
    } else {
        x = in[0];
    }
    for (i = 0; i < n; ++i) {
        out[i] = c0[i] + (isLinear ? x : pow(x, e)) * (c1[i] - c0[i]);
        if (hasRange) {
            if (out[i] < range[i][0]) {
                out[i] = range[i][0];
            } else if (out[i] > range[i][1]) {
                out[i] = range[i][1];
            }
        }
    }
    return;
}

//------------------------------------------------------------------------
// StitchingFunction
//------------------------------------------------------------------------

StitchingFunction::StitchingFunction(Object *funcObj, Dict *dict, std::set<int> *usedParents)
{
    Object obj1;
    int i;

    ok = false;
    funcs = nullptr;
    bounds = nullptr;
    encode = nullptr;
    scale = nullptr;

    //----- initialize the generic stuff
    if (!init(dict)) {
        return;
    }
    if (m != 1) {
        error(errSyntaxError, -1, "Stitching function with more than one input");
        return;
    }

    //----- Functions
    obj1 = dict->lookup("Functions");
    if (!obj1.isArray()) {
        error(errSyntaxError, -1, "Missing 'Functions' entry in stitching function");
        return;
    }
    k = obj1.arrayGetLength();
    funcs = (Function **)gmallocn(k, sizeof(Function *));
    bounds = (double *)gmallocn(k + 1, sizeof(double));
    encode = (double *)gmallocn(2 * k, sizeof(double));
    scale = (double *)gmallocn(k, sizeof(double));
    for (i = 0; i < k; ++i) {
        funcs[i] = nullptr;
    }
    for (i = 0; i < k; ++i) {
        std::set<int> usedParentsAux = *usedParents;
        Ref ref;
        Object obj2 = obj1.getArray()->get(i, &ref);
        if (ref != Ref::INVALID()) {
            if (usedParentsAux.find(ref.num) == usedParentsAux.end()) {
                usedParentsAux.insert(ref.num);
            } else {
                return;
            }
        }
        if (!(funcs[i] = Function::parse(&obj2, &usedParentsAux))) {
            return;
        }
        if (funcs[i]->getInputSize() != 1 || (i > 0 && funcs[i]->getOutputSize() != funcs[0]->getOutputSize())) {
            error(errSyntaxError, -1, "Incompatible subfunctions in stitching function");
            return;
        }
    }

    //----- Bounds
    obj1 = dict->lookup("Bounds");
    if (!obj1.isArray() || obj1.arrayGetLength() != k - 1) {
        error(errSyntaxError, -1, "Missing or invalid 'Bounds' entry in stitching function");
        return;
    }
    bounds[0] = domain[0][0];
    for (i = 1; i < k; ++i) {
        Object obj2 = obj1.arrayGet(i - 1);
        if (!obj2.isNum()) {
            error(errSyntaxError, -1, "Invalid type in 'Bounds' array in stitching function");
            return;
        }
        bounds[i] = obj2.getNum();
    }
    bounds[k] = domain[0][1];

    //----- Encode
    obj1 = dict->lookup("Encode");
    if (!obj1.isArray() || obj1.arrayGetLength() != 2 * k) {
        error(errSyntaxError, -1, "Missing or invalid 'Encode' entry in stitching function");
        return;
    }
    for (i = 0; i < 2 * k; ++i) {
        Object obj2 = obj1.arrayGet(i);
        if (!obj2.isNum()) {
            error(errSyntaxError, -1, "Invalid type in 'Encode' array in stitching function");
            return;
        }
        encode[i] = obj2.getNum();
    }

    //----- pre-compute the scale factors
    for (i = 0; i < k; ++i) {
        if (bounds[i] == bounds[i + 1]) {
            // avoid a divide-by-zero -- in this situation, function i will
            // never be used anyway
            scale[i] = 0;
        } else {
            scale[i] = (encode[2 * i + 1] - encode[2 * i]) / (bounds[i + 1] - bounds[i]);
        }
    }

    n = funcs[0]->getOutputSize();
    ok = true;
    return;
}

StitchingFunction::StitchingFunction(const StitchingFunction *func) : Function(func)
{
    k = func->k;

    funcs = (Function **)gmallocn(k, sizeof(Function *));
    for (int i = 0; i < k; ++i) {
        funcs[i] = func->funcs[i]->copy();
    }

    bounds = (double *)gmallocn(k + 1, sizeof(double));
    memcpy(bounds, func->bounds, (k + 1) * sizeof(double));

    encode = (double *)gmallocn(2 * k, sizeof(double));
    memcpy(encode, func->encode, 2 * k * sizeof(double));

    scale = (double *)gmallocn(k, sizeof(double));
    memcpy(scale, func->scale, k * sizeof(double));

    ok = func->ok;
}

StitchingFunction::~StitchingFunction()
{
    int i;

    if (funcs) {
        for (i = 0; i < k; ++i) {
            if (funcs[i]) {
                delete funcs[i];
            }
        }
    }
    gfree(funcs);
    gfree(bounds);
    gfree(encode);
    gfree(scale);
}

void StitchingFunction::transform(const double *in, double *out) const
{
    double x;
    int i;

    if (in[0] < domain[0][0]) {
        x = domain[0][0];
    } else if (in[0] > domain[0][1]) {
        x = domain[0][1];
    } else {
        x = in[0];
    }
    for (i = 0; i < k - 1; ++i) {
        if (x < bounds[i + 1]) {
            break;
        }
    }
    x = encode[2 * i] + (x - bounds[i]) * scale[i];
    funcs[i]->transform(&x, out);
}

//------------------------------------------------------------------------
// PostScriptFunction
//------------------------------------------------------------------------

enum PSOp
{
    psOpAbs,
    psOpAdd,
    psOpAnd,
    psOpAtan,
    psOpBitshift,
    psOpCeiling,
    psOpCopy,
    psOpCos,
    psOpCvi,
    psOpCvr,
    psOpDiv,
    psOpDup,
    psOpEq,
    psOpExch,
    psOpExp,
    psOpFalse,
    psOpFloor,
    psOpGe,
    psOpGt,
    psOpIdiv,
    psOpIndex,
    psOpLe,
    psOpLn,
    psOpLog,
    psOpLt,
    psOpMod,
    psOpMul,
    psOpNe,
    psOpNeg,
    psOpNot,
    psOpOr,
    psOpPop,
    psOpRoll,
    psOpRound,
    psOpSin,
    psOpSqrt,
    psOpSub,
    psOpTrue,
    psOpTruncate,
    psOpXor,
    psOpIf,
    psOpIfelse,
    psOpReturn
};

// Note: 'if' and 'ifelse' are parsed separately.
// The rest are listed here in alphabetical order.
// The index in this table is equivalent to the entry in PSOp.
static const char *psOpNames[] = { "abs",   "add", "and", "atan", "bitshift", "ceiling", "copy", "cos", "cvi", "cvr", "div", "dup", "eq",   "exch",  "exp", "false", "floor", "ge",   "gt",       "idiv",
                                   "index", "le",  "ln",  "log",  "lt",       "mod",     "mul",  "ne",  "neg", "not", "or",  "pop", "roll", "round", "sin", "sqrt",  "sub",   "true", "truncate", "xor" };

#define nPSOps (sizeof(psOpNames) / sizeof(char *))

enum PSObjectType
{
    psBool,
    psInt,
    psReal,
    psOperator,
    psBlock
};

// In the code array, 'if'/'ifelse' operators take up three slots
// plus space for the code in the subclause(s).
//
//         +---------------------------------+
//         | psOperator: psOpIf / psOpIfelse |
//         +---------------------------------+
//         | psBlock: ptr=<A>                |
//         +---------------------------------+
//         | psBlock: ptr=<B>                |
//         +---------------------------------+
//         | if clause                       |
//         | ...                             |
//         | psOperator: psOpReturn          |
//         +---------------------------------+
//     <A> | else clause                     |
//         | ...                             |
//         | psOperator: psOpReturn          |
//         +---------------------------------+
//     <B> | ...                             |
//
// For 'if', pointer <A> is present in the code stream but unused.

struct PSObject
{
    PSObjectType type;
    union {
        bool booln; // boolean (stack only)
        int intg; // integer (stack and code)
        double real; // real (stack and code)
        PSOp op; // operator (code only)
        int blk; // if/ifelse block pointer (code only)
    };
};

#define psStackSize 100

class PSStack
{
public:
    PSStack() { sp = psStackSize; }
    void clear() { sp = psStackSize; }
    void pushBool(bool booln)
    {
        if (checkOverflow()) {
            stack[--sp].type = psBool;
            stack[sp].booln = booln;
        }
    }
    void pushInt(int intg)
    {
        if (checkOverflow()) {
            stack[--sp].type = psInt;
            stack[sp].intg = intg;
        }
    }
    void pushReal(double real)
    {
        if (checkOverflow()) {
            stack[--sp].type = psReal;
            stack[sp].real = real;
        }
    }
    bool popBool()
    {
        if (checkUnderflow() && checkType(psBool, psBool)) {
            return stack[sp++].booln;
        }
        return false;
    }
    int popInt()
    {
        if (checkUnderflow() && checkType(psInt, psInt)) {
            return stack[sp++].intg;
        }
        return 0;
    }
    double popNum()
    {
        double ret;

        if (checkUnderflow() && checkType(psInt, psReal)) {
            ret = (stack[sp].type == psInt) ? (double)stack[sp].intg : stack[sp].real;
            ++sp;
            return ret;
        }
        return 0;
    }
    bool empty() { return sp == psStackSize; }
    bool topIsInt() { return sp < psStackSize && stack[sp].type == psInt; }
    bool topTwoAreInts() { return sp < psStackSize - 1 && stack[sp].type == psInt && stack[sp + 1].type == psInt; }
    bool topIsReal() { return sp < psStackSize && stack[sp].type == psReal; }
    bool topTwoAreNums() { return sp < psStackSize - 1 && (stack[sp].type == psInt || stack[sp].type == psReal) && (stack[sp + 1].type == psInt || stack[sp + 1].type == psReal); }
    void copy(int n);
    void roll(int n, int j);
    void index(int i)
    {
        if (!checkOverflow()) {
            return;
        }
        --sp;
        if (unlikely(sp + i + 1 >= psStackSize)) {
            error(errSyntaxError, -1, "Stack underflow in PostScript function");
            return;
        }
        if (unlikely(sp + i + 1 < 0)) {
            error(errSyntaxError, -1, "Stack overflow in PostScript function");
            return;
        }
        stack[sp] = stack[sp + 1 + i];
    }
    void pop()
    {
        if (!checkUnderflow()) {
            return;
        }
        ++sp;
    }

private:
    bool checkOverflow(int n = 1)
    {
        if (sp - n < 0) {
            error(errSyntaxError, -1, "Stack overflow in PostScript function");
            return false;
        }
        return true;
    }
    bool checkUnderflow()
    {
        if (sp == psStackSize) {
            error(errSyntaxError, -1, "Stack underflow in PostScript function");
            return false;
        }
        return true;
    }
    bool checkType(PSObjectType t1, PSObjectType t2)
    {
        if (stack[sp].type != t1 && stack[sp].type != t2) {
            error(errSyntaxError, -1, "Type mismatch in PostScript function");
            return false;
        }
        return true;
    }
    PSObject stack[psStackSize];
    int sp;
};

void PSStack::copy(int n)
{
    int i;

    int aux;
    if (unlikely(checkedAdd(sp, n, &aux) || aux > psStackSize)) {
        error(errSyntaxError, -1, "Stack underflow in PostScript function");
        return;
    }
    if (unlikely(checkedSubtraction(sp, n, &aux) || aux > psStackSize)) {
        error(errSyntaxError, -1, "Stack underflow in PostScript function");
        return;
    }
    if (!checkOverflow(n)) {
        return;
    }
    for (i = sp + n - 1; i >= sp; --i) {
        stack[i - n] = stack[i];
    }
    sp -= n;
}

void PSStack::roll(int n, int j)
{
    PSObject obj;
    int i, k;

    if (unlikely(n == 0)) {
        return;
    }
    if (j >= 0) {
        j %= n;
    } else {
        j = -j % n;
        if (j != 0) {
            j = n - j;
        }
    }
    if (n <= 0 || j == 0 || n > psStackSize || sp + n > psStackSize) {
        return;
    }
    if (j <= n / 2) {
        for (i = 0; i < j; ++i) {
            obj = stack[sp];
            for (k = sp; k < sp + n - 1; ++k) {
                stack[k] = stack[k + 1];
            }
            stack[sp + n - 1] = obj;
        }
    } else {
        j = n - j;
        for (i = 0; i < j; ++i) {
            obj = stack[sp + n - 1];
            for (k = sp + n - 1; k > sp; --k) {
                stack[k] = stack[k - 1];
            }
            stack[sp] = obj;
        }
    }
}

PostScriptFunction::PostScriptFunction(Object *funcObj, Dict *dict)
{
    Stream *str;
    int codePtr;
    double in[funcMaxInputs];
    int i;

    code = nullptr;
    codeString = nullptr;
    codeSize = 0;
    ok = false;

    //----- initialize the generic stuff
    if (!init(dict)) {
        goto err1;
    }
    if (!hasRange) {
        error(errSyntaxError, -1, "Type 4 function is missing range");
        goto err1;
    }

    //----- get the stream
    if (!funcObj->isStream()) {
        error(errSyntaxError, -1, "Type 4 function isn't a stream");
        goto err1;
    }
    str = funcObj->getStream();

    //----- parse the function
    codeString = new GooString();
    str->reset();
    if (getToken(str)->cmp("{") != 0) {
        error(errSyntaxError, -1, "Expected '{{' at start of PostScript function");
        goto err1;
    }
    codePtr = 0;
    if (!parseCode(str, &codePtr)) {
        goto err2;
    }
    str->close();

    //----- set up the cache
    for (i = 0; i < m; ++i) {
        in[i] = domain[i][0];
        cacheIn[i] = in[i] - 1;
    }
    transform(in, cacheOut);

    ok = true;

err2:
    str->close();
err1:
    return;
}

PostScriptFunction::PostScriptFunction(const PostScriptFunction *func) : Function(func)
{
    codeSize = func->codeSize;

    code = (PSObject *)gmallocn(codeSize, sizeof(PSObject));
    memcpy(code, func->code, codeSize * sizeof(PSObject));

    codeString = func->codeString->copy();

    memcpy(cacheIn, func->cacheIn, funcMaxInputs * sizeof(double));
    memcpy(cacheOut, func->cacheOut, funcMaxOutputs * sizeof(double));

    ok = func->ok;
}

PostScriptFunction::~PostScriptFunction()
{
    gfree(code);
    delete codeString;
}

void PostScriptFunction::transform(const double *in, double *out) const
{
    PSStack stack;
    int i;

    // check the cache
    for (i = 0; i < m; ++i) {
        if (in[i] != cacheIn[i]) {
            break;
        }
    }
    if (i == m) {
        for (i = 0; i < n; ++i) {
            out[i] = cacheOut[i];
        }
        return;
    }

    for (i = 0; i < m; ++i) {
        //~ may need to check for integers here
        stack.pushReal(in[i]);
    }
    exec(&stack, 0);
    for (i = n - 1; i >= 0; --i) {
        out[i] = stack.popNum();
        if (out[i] < range[i][0]) {
            out[i] = range[i][0];
        } else if (out[i] > range[i][1]) {
            out[i] = range[i][1];
        }
    }
    stack.clear();

    // if (!stack->empty()) {
    //   error(errSyntaxWarning, -1,
    //         "Extra values on stack at end of PostScript function");
    // }

    // save current result in the cache
    for (i = 0; i < m; ++i) {
        cacheIn[i] = in[i];
    }
    for (i = 0; i < n; ++i) {
        cacheOut[i] = out[i];
    }
}

bool PostScriptFunction::parseCode(Stream *str, int *codePtr)
{
    bool isReal;
    int opPtr, elsePtr;
    int a, b, mid, cmp;

    while (true) {
        // This needs to be on the heap to help make parseCode
        // able to call itself more times recursively
        std::unique_ptr<GooString> tok = getToken(str);
        const char *p = tok->c_str();
        if (isdigit(*p) || *p == '.' || *p == '-') {
            isReal = false;
            for (; *p; ++p) {
                if (*p == '.') {
                    isReal = true;
                    break;
                }
            }
            resizeCode(*codePtr);
            if (isReal) {
                code[*codePtr].type = psReal;
                code[*codePtr].real = gatof(tok->c_str());
            } else {
                code[*codePtr].type = psInt;
                code[*codePtr].intg = atoi(tok->c_str());
            }
            ++*codePtr;
        } else if (!tok->cmp("{")) {
            opPtr = *codePtr;
            *codePtr += 3;
            resizeCode(opPtr + 2);
            if (!parseCode(str, codePtr)) {
                return false;
            }
            tok = getToken(str);
            if (!tok->cmp("{")) {
                elsePtr = *codePtr;
                if (!parseCode(str, codePtr)) {
                    return false;
                }
                tok = getToken(str);
            } else {
                elsePtr = -1;
            }
            if (!tok->cmp("if")) {
                if (elsePtr >= 0) {
                    error(errSyntaxError, -1, "Got 'if' operator with two blocks in PostScript function");
                    return false;
                }
                code[opPtr].type = psOperator;
                code[opPtr].op = psOpIf;
                code[opPtr + 2].type = psBlock;
                code[opPtr + 2].blk = *codePtr;
            } else if (!tok->cmp("ifelse")) {
                if (elsePtr < 0) {
                    error(errSyntaxError, -1, "Got 'ifelse' operator with one block in PostScript function");
                    return false;
                }
                code[opPtr].type = psOperator;
                code[opPtr].op = psOpIfelse;
                code[opPtr + 1].type = psBlock;
                code[opPtr + 1].blk = elsePtr;
                code[opPtr + 2].type = psBlock;
                code[opPtr + 2].blk = *codePtr;
            } else {
                error(errSyntaxError, -1, "Expected if/ifelse operator in PostScript function");
                return false;
            }
        } else if (!tok->cmp("}")) {
            resizeCode(*codePtr);
            code[*codePtr].type = psOperator;
            code[*codePtr].op = psOpReturn;
            ++*codePtr;
            break;
        } else {
            a = -1;
            b = nPSOps;
            cmp = 0; // make gcc happy
            // invariant: psOpNames[a] < tok < psOpNames[b]
            while (b - a > 1) {
                mid = (a + b) / 2;
                cmp = tok->cmp(psOpNames[mid]);
                if (cmp > 0) {
                    a = mid;
                } else if (cmp < 0) {
                    b = mid;
                } else {
                    a = b = mid;
                }
            }
            if (cmp != 0) {
                error(errSyntaxError, -1, "Unknown operator '{0:t}' in PostScript function", tok.get());
                return false;
            }
            resizeCode(*codePtr);
            code[*codePtr].type = psOperator;
            code[*codePtr].op = (PSOp)a;
            ++*codePtr;
        }
    }
    return true;
}

std::unique_ptr<GooString> PostScriptFunction::getToken(Stream *str)
{
    int c;
    bool comment;

    std::string s;
    comment = false;
    while (true) {
        if ((c = str->getChar()) == EOF) {
            break;
        }
        codeString->append(c);
        if (comment) {
            if (c == '\x0a' || c == '\x0d') {
                comment = false;
            }
        } else if (c == '%') {
            comment = true;
        } else if (!isspace(c)) {
            break;
        }
    }
    if (c == '{' || c == '}') {
        s.push_back((char)c);
    } else if (isdigit(c) || c == '.' || c == '-') {
        while (true) {
            s.push_back((char)c);
            c = str->lookChar();
            if (c == EOF || !(isdigit(c) || c == '.' || c == '-')) {
                break;
            }
            str->getChar();
            codeString->append(c);
        }
    } else {
        while (true) {
            s.push_back((char)c);
            c = str->lookChar();
            if (c == EOF || !isalnum(c)) {
                break;
            }
            str->getChar();
            codeString->append(c);
        }
    }
    return std::make_unique<GooString>(s);
}

void PostScriptFunction::resizeCode(int newSize)
{
    if (newSize >= codeSize) {
        codeSize += 64;
        code = (PSObject *)greallocn(code, codeSize, sizeof(PSObject));
    }
}

void PostScriptFunction::exec(PSStack *stack, int codePtr) const
{
    int i1, i2;
    double r1, r2, result;
    bool b1, b2;

    while (true) {
        switch (code[codePtr].type) {
        case psInt:
            stack->pushInt(code[codePtr++].intg);
            break;
        case psReal:
            stack->pushReal(code[codePtr++].real);
            break;
        case psOperator:
            switch (code[codePtr++].op) {
            case psOpAbs:
                if (stack->topIsInt()) {
                    stack->pushInt(abs(stack->popInt()));
                } else {
                    stack->pushReal(fabs(stack->popNum()));
                }
                break;
            case psOpAdd:
                if (stack->topTwoAreInts()) {
                    i2 = stack->popInt();
                    i1 = stack->popInt();
                    stack->pushInt(i1 + i2);
                } else {
                    r2 = stack->popNum();
                    r1 = stack->popNum();
                    stack->pushReal(r1 + r2);
                }
                break;
            case psOpAnd:
                if (stack->topTwoAreInts()) {
                    i2 = stack->popInt();
                    i1 = stack->popInt();
                    stack->pushInt(i1 & i2);
                } else {
                    b2 = stack->popBool();
                    b1 = stack->popBool();
                    stack->pushBool(b1 && b2);
                }
                break;
            case psOpAtan:
                r2 = stack->popNum();
                r1 = stack->popNum();
                result = atan2(r1, r2) * 180.0 / M_PI;
                if (result < 0) {
                    result += 360.0;
                }
                stack->pushReal(result);
                break;
            case psOpBitshift:
                i2 = stack->popInt();
                i1 = stack->popInt();
                if (i2 > 0) {
                    stack->pushInt(i1 << i2);
                } else if (i2 < 0) {
                    stack->pushInt((int)((unsigned int)i1 >> -i2));
                } else {
                    stack->pushInt(i1);
                }
                break;
            case psOpCeiling:
                if (!stack->topIsInt()) {
                    stack->pushReal(ceil(stack->popNum()));
                }
                break;
            case psOpCopy:
                stack->copy(stack->popInt());
                break;
            case psOpCos:
                stack->pushReal(cos(stack->popNum() * M_PI / 180.0));
                break;
            case psOpCvi:
                if (!stack->topIsInt()) {
                    stack->pushInt((int)stack->popNum());
                }
                break;
            case psOpCvr:
                if (!stack->topIsReal()) {
                    stack->pushReal(stack->popNum());
                }
                break;
            case psOpDiv:
                r2 = stack->popNum();
                r1 = stack->popNum();
                stack->pushReal(r1 / r2);
                break;
            case psOpDup:
                stack->copy(1);
                break;
            case psOpEq:
                if (stack->topTwoAreInts()) {
                    i2 = stack->popInt();
                    i1 = stack->popInt();
                    stack->pushBool(i1 == i2);
                } else if (stack->topTwoAreNums()) {
                    r2 = stack->popNum();
                    r1 = stack->popNum();
                    stack->pushBool(r1 == r2);
                } else {
                    b2 = stack->popBool();
                    b1 = stack->popBool();
                    stack->pushBool(b1 == b2);
                }
                break;
            case psOpExch:
                stack->roll(2, 1);
                break;
            case psOpExp:
                r2 = stack->popNum();
                r1 = stack->popNum();
                stack->pushReal(pow(r1, r2));
                break;
            case psOpFalse:
                stack->pushBool(false);
                break;
            case psOpFloor:
                if (!stack->topIsInt()) {
                    stack->pushReal(floor(stack->popNum()));
                }
                break;
            case psOpGe:
                if (stack->topTwoAreInts()) {
                    i2 = stack->popInt();
                    i1 = stack->popInt();
                    stack->pushBool(i1 >= i2);
                } else {
                    r2 = stack->popNum();
                    r1 = stack->popNum();
                    stack->pushBool(r1 >= r2);
                }
                break;
            case psOpGt:
                if (stack->topTwoAreInts()) {
                    i2 = stack->popInt();
                    i1 = stack->popInt();
                    stack->pushBool(i1 > i2);
                } else {
                    r2 = stack->popNum();
                    r1 = stack->popNum();
                    stack->pushBool(r1 > r2);
                }
                break;
            case psOpIdiv:
                i2 = stack->popInt();
                i1 = stack->popInt();
                if (likely((i2 != 0) && !(i2 == -1 && i1 == INT_MIN))) {
                    stack->pushInt(i1 / i2);
                }
                break;
            case psOpIndex:
                stack->index(stack->popInt());
                break;
            case psOpLe:
                if (stack->topTwoAreInts()) {
                    i2 = stack->popInt();
                    i1 = stack->popInt();
                    stack->pushBool(i1 <= i2);
                } else {
                    r2 = stack->popNum();
                    r1 = stack->popNum();
                    stack->pushBool(r1 <= r2);
                }
                break;
            case psOpLn:
                stack->pushReal(log(stack->popNum()));
                break;
            case psOpLog:
                stack->pushReal(log10(stack->popNum()));
                break;
            case psOpLt:
                if (stack->topTwoAreInts()) {
                    i2 = stack->popInt();
                    i1 = stack->popInt();
                    stack->pushBool(i1 < i2);
                } else {
                    r2 = stack->popNum();
                    r1 = stack->popNum();
                    stack->pushBool(r1 < r2);
                }
                break;
            case psOpMod:
                i2 = stack->popInt();
                i1 = stack->popInt();
                if (likely(i2 != 0)) {
                    stack->pushInt(i1 % i2);
                }
                break;
            case psOpMul:
                if (stack->topTwoAreInts()) {
                    i2 = stack->popInt();
                    i1 = stack->popInt();
                    //~ should check for out-of-range, and push a real instead
                    stack->pushInt(i1 * i2);
                } else {
                    r2 = stack->popNum();
                    r1 = stack->popNum();
                    stack->pushReal(r1 * r2);
                }
                break;
            case psOpNe:
                if (stack->topTwoAreInts()) {
                    i2 = stack->popInt();
                    i1 = stack->popInt();
                    stack->pushBool(i1 != i2);
                } else if (stack->topTwoAreNums()) {
                    r2 = stack->popNum();
                    r1 = stack->popNum();
                    stack->pushBool(r1 != r2);
                } else {
                    b2 = stack->popBool();
                    b1 = stack->popBool();
                    stack->pushBool(b1 != b2);
                }
                break;
            case psOpNeg:
                if (stack->topIsInt()) {
                    stack->pushInt(-stack->popInt());
                } else {
                    stack->pushReal(-stack->popNum());
                }
                break;
            case psOpNot:
                if (stack->topIsInt()) {
                    stack->pushInt(~stack->popInt());
                } else {
                    stack->pushBool(!stack->popBool());
                }
                break;
            case psOpOr:
                if (stack->topTwoAreInts()) {
                    i2 = stack->popInt();
                    i1 = stack->popInt();
                    stack->pushInt(i1 | i2);
                } else {
                    b2 = stack->popBool();
                    b1 = stack->popBool();
                    stack->pushBool(b1 || b2);
                }
                break;
            case psOpPop:
                stack->pop();
                break;
            case psOpRoll:
                i2 = stack->popInt();
                i1 = stack->popInt();
                stack->roll(i1, i2);
                break;
            case psOpRound:
                if (!stack->topIsInt()) {
                    r1 = stack->popNum();
                    stack->pushReal((r1 >= 0) ? floor(r1 + 0.5) : ceil(r1 - 0.5));
                }
                break;
            case psOpSin:
                stack->pushReal(sin(stack->popNum() * M_PI / 180.0));
                break;
            case psOpSqrt:
                stack->pushReal(sqrt(stack->popNum()));
                break;
            case psOpSub:
                if (stack->topTwoAreInts()) {
                    i2 = stack->popInt();
                    i1 = stack->popInt();
                    stack->pushInt(i1 - i2);
                } else {
                    r2 = stack->popNum();
                    r1 = stack->popNum();
                    stack->pushReal(r1 - r2);
                }
                break;
            case psOpTrue:
                stack->pushBool(true);
                break;
            case psOpTruncate:
                if (!stack->topIsInt()) {
                    r1 = stack->popNum();
                    stack->pushReal((r1 >= 0) ? floor(r1) : ceil(r1));
                }
                break;
            case psOpXor:
                if (stack->topTwoAreInts()) {
                    i2 = stack->popInt();
                    i1 = stack->popInt();
                    stack->pushInt(i1 ^ i2);
                } else {
                    b2 = stack->popBool();
                    b1 = stack->popBool();
                    stack->pushBool(b1 ^ b2);
                }
                break;
            case psOpIf:
                b1 = stack->popBool();
                if (b1) {
                    exec(stack, codePtr + 2);
                }
                codePtr = code[codePtr + 1].blk;
                break;
            case psOpIfelse:
                b1 = stack->popBool();
                if (b1) {
                    exec(stack, codePtr + 2);
                } else {
                    exec(stack, code[codePtr].blk);
                }
                codePtr = code[codePtr + 1].blk;
                break;
            case psOpReturn:
                return;
            }
            break;
        default:
            error(errSyntaxError, -1, "Internal: bad object in PostScript function code");
            break;
        }
    }
}
