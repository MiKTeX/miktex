#pragma once

#cmakedefine SIZEOF_LONG ${SIZEOF_LONG}
#define PACKAGE pixman-1
#define TLS __declspec(thread)

#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__x86_64__)
  #define USE_SSE2 1
  #define USE_SSSE3 1
#endif