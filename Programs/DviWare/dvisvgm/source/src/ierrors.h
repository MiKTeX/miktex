/* Copyright (C) 2001-2022 Artifex Software, Inc.
   All Rights Reserved.

   This software is provided AS-IS with no warranty, either express or
   implied.

   This software is distributed under license and may not be copied,
   modified or distributed except as expressly authorized under the terms
   of the license contained in the file LICENSE in this distribution.

   Refer to licensing information at http://www.artifex.com or contact
   Artifex Software, Inc.,  1305 Grant Avenue - Suite 200, Novato,
   CA 94945, U.S.A., +1(415)492-9861, for further information.
*/


/* Definition of error codes */

#ifndef ierrors_INCLUDED
#  define ierrors_INCLUDED

//#include "gserrors.h"

/*
 * DO NOT USE THIS FILE IN THE GRAPHICS LIBRARY.
 * THIS FILE IS PART OF THE POSTSCRIPT INTERPRETER.
 * USE gserrors.h IN THE LIBRARY.
 */

/*
 * A procedure that may return an error always returns
 * a non-negative value (zero, unless otherwise noted) for success,
 * or negative for failure.
 * We use ints rather than an enum to avoid a lot of casting.
 */

/* Define the error name table */
extern const char *const gs_error_names[];

                /* ------ PostScript Level 1 errors ------ */

#define LEVEL1_ERROR_NAMES\
 "unknownerror", "dictfull", "dictstackoverflow", "dictstackunderflow",\
 "execstackoverflow", "interrupt", "invalidaccess", "invalidexit",\
 "invalidfileaccess", "invalidfont", "invalidrestore", "ioerror",\
 "limitcheck", "nocurrentpoint", "rangecheck", "stackoverflow",\
 "stackunderflow", "syntaxerror", "timeout", "typecheck", "undefined",\
 "undefinedfilename", "undefinedresult", "unmatchedmark", "VMerror"

        /* ------ Additional Level 2 errors (also in DPS) ------ */

#define LEVEL2_ERROR_NAMES\
 "configurationerror", "undefinedresource", "unregistered"

        /* ------ Additional DPS errors ------ */

#define DPS_ERROR_NAMES\
 "invalidcontext", "invalidid"

#define PDF_ERROR_NAMES\
 "pdf_stackoverflow", "pdf_circular_reference"

#define ERROR_NAMES\
 LEVEL1_ERROR_NAMES, LEVEL2_ERROR_NAMES, DPS_ERROR_NAMES, PDF_ERROR_NAMES

/*
 * Define which error codes require re-executing the current object.
 */
#define GS_ERROR_IS_INTERRUPT(ecode)\
  ((ecode) == gs_error_interrupt || (ecode) == gs_error_timeout)

#endif /* ierrors_INCLUDED */
