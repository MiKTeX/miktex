#pragma once

#if defined(HAVE_LIBTIRPC)
#if defined(_WIN32)
#include "win32xdr.h"
typedef Win32XDR XDR;
#define xdrstdio_create w32_xdrstdio_create
#define xdr_destroy w32_xdr_destroy
#define xdrmem_create w32_xdrmem_create

#define xdr_u_int w32_xdr_u_int
#define xdr_int w32_xdr_int
#define xdr_float w32_xdr_float
#define xdr_double w32_xdr_double
#define xdr_long w32_xdr_long
#define xdr_u_long w32_xdr_u_long

#define xdr_longlong_t w32_xdr_longlong_t
#define xdr_u_longlong_t w32_xdr_u_longlong_t

#define xdr_short w32_xdr_short
#define xdr_u_short w32_xdr_u_short

#define xdr_char w32_xdr_char
#define xdr_u_char w32_xdr_u_char

#define XDR_DECODE W32_XDR_DECODE
#define XDR_ENCODE W32_XDR_ENCODE

#else
#ifndef _ALL_SOURCE
#define _ALL_SOURCE 1
#endif

#include <cstdio>
#include <vector>
#include <algorithm>

#include <sys/types.h>
#include <rpc/types.h>

#define quad_t long long
#define u_quad_t unsigned long long

#if defined(__CYGWIN__) || defined(__FreeBSD__)
#include <sys/select.h>
#define u_char unsigned char
#define u_int unsigned int
#define u_short unsigned short
#define u_long unsigned long
extern "C" int fseeko(FILE*, off_t, int);
extern "C" off_t ftello(FILE*);
extern "C" FILE * open_memstream(char**, size_t*);
#endif

#ifdef __APPLE__
#include <rpc/xdr.h>

inline bool_t xdr_long(XDR* __xdrs, long* __lp) {
  return xdr_longlong_t(__xdrs, (long long*)__lp);
}

inline bool_t xdr_u_long(XDR* __xdrs, unsigned long* __lp) {
  return xdr_u_longlong_t(__xdrs, (unsigned long long*) __lp);
}

#endif

#ifdef __OpenBSD__
#define xdr_longlong_t xdr_int64_t
#define xdr_u_longlong_t xdr_u_int64_t
#endif

#ifdef _POSIX_SOURCE
#undef _POSIX_SOURCE
#include <rpc/rpc.h>
#define _POSIX_SOURCE
#else
#include <rpc/rpc.h>
#endif
#endif
#endif
