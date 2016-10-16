/* Copyright (C) 2000-2012 by George Williams */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

 * The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _PFAEDIT_H_
#define _PFAEDIT_H_

#include <fontforge-config.h>
#include "configure-fontforge.h"
#include <basics.h>
#include "libffstamp.h"
#include <stdio.h>
#include <string.h>
#include <intl.h>
#include "splinefont.h"
#include "uiinterface.h"

static const unsigned unicode4_size = 17*65536;
    /* Unicode goes up to 0x10ffff */

#ifdef _WIN32
extern FILE* tmpfile2 ();
#else
#define tmpfile2 tmpfile
#endif

extern int32 unicode_from_adobestd[256];

/* unicode_nameannot - Deprecated, but kept for older programs to access. */
#if _NO_LIBUNINAMESLIST
#endif

extern int default_fv_font_size;
extern int default_fv_antialias;
extern int default_fv_bbsized;
extern Encoding *default_encoding, custom;
extern int adjustwidth;
extern int adjustlbearing;
extern int autohint_before_generate;
extern int seperate_hint_controls;
extern int no_windowing_ui;
extern uint32 default_background;
extern int use_utf8_in_script;

extern int new_em_size;
extern int new_fonts_are_order2;
extern int loaded_fonts_same_as_new;

extern char *TTFFoundry;

extern Encoding *enclist;

extern int quiet;

extern MacFeat *default_mac_feature_map;

#include <stdint.h>
#include <basics.h>

typedef uint32_t Color;
#define COLOR_DEFAULT ((Color) 0xfffffffe)

typedef struct pressedOn {
    int x,y;			/* screen location of the press */
    float cx, cy;		/* Translated into character space */
    float t;			/* location on the spline where we pressed */
} PressedOn;

typedef struct findsel {
    float fudge;		/* One pixel fudge factor */
    float xl,xh, yl, yh;	/* One pixel fudge factor */
    PressedOn *p;
} FindSel;

#endif
