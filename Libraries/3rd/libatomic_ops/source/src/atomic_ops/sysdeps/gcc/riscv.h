/*
 * THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
 * OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 *
 * Permission is hereby granted to use or copy this program
 * for any purpose,  provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 */

/* As of gcc-7.2.0, some __GCC_HAVE_SYNC_COMPARE_AND_SWAP_n are missing. */
/* The operations are lock-free (even for the types smaller than word).  */
#define AO_GCC_FORCE_HAVE_CAS

/* While double-word atomic operations are provided by the compiler     */
/* (which requires -latomic currently), they are not lock-free as       */
/* riscv itself does not have the double-word atomic operations.        */

#include "generic.h"

#undef AO_GCC_FORCE_HAVE_CAS
