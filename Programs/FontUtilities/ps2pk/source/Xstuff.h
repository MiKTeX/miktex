#ifndef NULL
#define NULL 0
#endif

#ifndef XMD_H
#define XMD_H 1

#include "types.h"

/*
 * Bitfield suffixes for the protocol structure elements, if you
 * need them.  Note that bitfields are not guarranteed to be signed
 * (or even unsigned) according to ANSI C.
 */
#ifdef WORD64
#define B32 :32
#define B16 :16
#else
#define B32
#define B16
#endif

typedef struct {
    SHORT leftSideBearing B16,
	  rightSideBearing B16,
	  characterWidth B16,
	  ascent B16,
	  descent B16;
    USHORT attributes;
} xCharInfo;
#endif /*  XMD_H */

#ifndef _FS_PROTO_H_
#define _FS_PROTO_H_

typedef struct {
    USHORT	x_resolution B16;
    USHORT	y_resolution B16;
    USHORT	point_size B16;
}	    fsResolution;

#endif /* _FS_PROTO_H_ */
