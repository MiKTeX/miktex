//========================================================================
//
// pdfsig.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2010 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#ifndef NUMBEROFCHARACTERS_H
#define NUMBEROFCHARACTERS_H

static int numberOfCharacters(unsigned int n)
{
    int charNum = 0;
    while (n >= 10) {
        n = n / 10;
        charNum++;
    }
    charNum++;
    return charNum;
}

#endif
