/* $XConsortium: fsmasks.h,v 1.2 91/05/13 16:46:16 gildea Exp $ */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this protoype software
 * and its documentation to Members and Affiliates of the MIT X Consortium
 * any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * MIT not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * @(#)fsmasks.h	4.1	91/05/02
 *
 */

/*
 * masks & values used by the font lib and the font server
 */

#ifndef _FSMASKS_H_
#define _FSMASKS_H_

#include "types.h"

/* font format macros */
#define BitmapFormatByteOrderMask       (1L << 0)
#define BitmapFormatBitOrderMask        (1L << 1)
#define BitmapFormatImageRectMask       (3L << 2)
#define BitmapFormatScanlinePadMask     (3L << 8)
#define BitmapFormatScanlineUnitMask    (3L << 12)

#define BitmapFormatByteOrderLSB        (0)
#define BitmapFormatByteOrderMSB        (1L << 0)
#define BitmapFormatBitOrderLSB         (0)
#define BitmapFormatBitOrderMSB         (1L << 1)

#define BitmapFormatImageRectMin        (0L << 2)
#define BitmapFormatImageRectMaxWidth   (1L << 2)
#define BitmapFormatImageRectMax        (2L << 2)

#define BitmapFormatScanlinePad8        (0L << 8)
#define BitmapFormatScanlinePad16       (1L << 8)
#define BitmapFormatScanlinePad32       (2L << 8)
#define BitmapFormatScanlinePad64       (3L << 8)

#define BitmapFormatScanlineUnit8       (0L << 12)
#define BitmapFormatScanlineUnit16      (1L << 12)
#define BitmapFormatScanlineUnit32      (2L << 12)
#define BitmapFormatScanlineUnit64      (3L << 12)

#define BitmapFormatMaskByte            (1L << 0)
#define BitmapFormatMaskBit             (1L << 1)
#define BitmapFormatMaskImageRectangle  (1L << 2)
#define BitmapFormatMaskScanLinePad     (1L << 3)
#define BitmapFormatMaskScanLineUnit    (1L << 4)

typedef uint32_t  fsBitmapFormat;
typedef uint32_t  fsBitmapFormatMask;

#endif	/* _FSMASKS_H_ */
