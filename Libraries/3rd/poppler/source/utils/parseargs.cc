/*
 * parseargs.cc
 *
 * Command line argument parser.
 *
 * Copyright 1996-2003 Glyph & Cog, LLC
 */

/*========================================================================

 Modified under the Poppler project - http://poppler.freedesktop.org

 Poppler project changes to this file are under the GPLv2 or later license

 All changes made under the Poppler project to this file are licensed
 under GPL version 2 or later

 Copyright (C) 2008, 2009, 2018 Albert Astals Cid <aacid@kde.org>
 Copyright (C) 2011, 2012 Adrian Johnson <ajohnson@redneon.com>

 To see a description of the changes please see the Changelog file that
 came with your tarball or type make ChangeLog if you are building from git

========================================================================*/

#include <cstdio>
#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include "parseargs.h"

#include "goo/gstrtod.h"
#include "goo/GooString.h"

static const ArgDesc *findArg(const ArgDesc *args, char *arg);
static bool grabArg(const ArgDesc *arg, int i, int *argc, char *argv[]);

bool parseArgs(const ArgDesc *args, int *argc, char *argv[])
{
    const ArgDesc *arg;
    int i, j;
    bool ok;

    ok = true;
    i = 1;
    while (i < *argc) {
        if (!strcmp(argv[i], "--")) {
            --*argc;
            for (j = i; j < *argc; ++j) {
                argv[j] = argv[j + 1];
            }
            break;
        } else if ((arg = findArg(args, argv[i]))) {
            if (!grabArg(arg, i, argc, argv)) {
                ok = false;
            }
        } else {
            ++i;
        }
    }
    return ok;
}

void printUsage(const char *program, const char *otherArgs, const ArgDesc *args)
{
    const ArgDesc *arg;
    const char *typ;
    int w, w1;

    w = 0;
    for (arg = args; arg->arg; ++arg) {
        if ((w1 = strlen(arg->arg)) > w) {
            w = w1;
        }
    }

    fprintf(stderr, "Usage: %s [options]", program);
    if (otherArgs) {
        fprintf(stderr, " %s", otherArgs);
    }
    fprintf(stderr, "\n");

    for (arg = args; arg->arg; ++arg) {
        fprintf(stderr, "  %s", arg->arg);
        w1 = 9 + w - strlen(arg->arg);
        switch (arg->kind) {
        case argInt:
        case argIntDummy:
            typ = " <int>";
            break;
        case argFP:
        case argFPDummy:
            typ = " <fp>";
            break;
        case argString:
        case argStringDummy:
        case argGooString:
            typ = " <string>";
            break;
        case argFlag:
        case argFlagDummy:
        default:
            typ = "";
            break;
        }
        fprintf(stderr, "%-*s", w1, typ);
        if (arg->usage) {
            fprintf(stderr, ": %s", arg->usage);
        }
        fprintf(stderr, "\n");
    }
}

static const ArgDesc *findArg(const ArgDesc *args, char *arg)
{
    const ArgDesc *p;

    for (p = args; p->arg; ++p) {
        if (p->kind < argFlagDummy && !strcmp(p->arg, arg)) {
            return p;
        }
    }
    return nullptr;
}

static bool grabArg(const ArgDesc *arg, int i, int *argc, char *argv[])
{
    int n;
    int j;
    bool ok;

    ok = true;
    n = 0;
    switch (arg->kind) {
    case argFlag:
        *(bool *)arg->val = true;
        n = 1;
        break;
    case argInt:
        if (i + 1 < *argc && isInt(argv[i + 1])) {
            *(int *)arg->val = atoi(argv[i + 1]);
            n = 2;
        } else {
            ok = false;
            n = 1;
        }
        break;
    case argFP:
        if (i + 1 < *argc && isFP(argv[i + 1])) {
            *(double *)arg->val = gatof(argv[i + 1]);
            n = 2;
        } else {
            ok = false;
            n = 1;
        }
        break;
    case argString:
        if (i + 1 < *argc) {
            strncpy((char *)arg->val, argv[i + 1], arg->size - 1);
            ((char *)arg->val)[arg->size - 1] = '\0';
            n = 2;
        } else {
            ok = false;
            n = 1;
        }
        break;
    case argGooString:
        if (i + 1 < *argc) {
            ((GooString *)arg->val)->Set(argv[i + 1]);
            n = 2;
        } else {
            ok = false;
            n = 1;
        }
        break;
    default:
        fprintf(stderr, "Internal error in arg table\n");
        n = 1;
        break;
    }
    if (n > 0) {
        *argc -= n;
        for (j = i; j < *argc; ++j) {
            argv[j] = argv[j + n];
        }
    }
    return ok;
}

bool isInt(const char *s)
{
    if (*s == '-' || *s == '+') {
        ++s;
    }
    while (isdigit(*s)) {
        ++s;
    }
    if (*s) {
        return false;
    }
    return true;
}

bool isFP(const char *s)
{
    int n;

    if (*s == '-' || *s == '+') {
        ++s;
    }
    n = 0;
    while (isdigit(*s)) {
        ++s;
        ++n;
    }
    if (*s == '.') {
        ++s;
    }
    while (isdigit(*s)) {
        ++s;
        ++n;
    }
    if (n > 0 && (*s == 'e' || *s == 'E')) {
        ++s;
        if (*s == '-' || *s == '+') {
            ++s;
        }
        n = 0;
        if (!isdigit(*s)) {
            return false;
        }
        do {
            ++s;
        } while (isdigit(*s));
    }
    if (*s) {
        return false;
    }
    return true;
}
