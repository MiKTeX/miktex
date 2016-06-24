/* File:    types.h
 * Purpose: define basic types so that program is portable to non 32 bits
 *          platforms
 * Author:  Piet Tutelaers
 * (see README for license)
 */

#ifndef __PS2PK_TYPES__
#define __PS2PK_TYPES__

#ifdef HAVE_CONFIG_H
#include "c-auto.h"
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#ifdef KPATHSEA
#include <kpathsea/kpathsea.h>
#endif /* KPATHSEA */

typedef unsigned short USHORT; /* 16 bits */
typedef short SHORT;           /* 16 bits */
typedef double DOUBLE;         /* 64 bits */

typedef char *encoding[256];
extern void getenc(char **, char **, encoding, int [256]);

#endif /* __PS2PK_TYPES__ */
