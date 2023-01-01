//========================================================================
//
// Trace.cc
//
// Nested tracing.
//
// Copyright 2020 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#if ENABLE_TRACING

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include "GString.h"
#include "Trace.h"

// NB: This module is NOT thread-safe.

static bool traceInitialized = false;
static FILE *traceOut = NULL;

static void traceInit() {
  if (traceInitialized) {
    return;
  }
  //~ this could read an env var to set up an output file
  GString *fileName = GString::format("/tmp/trace.{0:d}", (int)getpid());
  traceOut = fopen(fileName->getCString(), "w");
  delete fileName;
  traceInitialized = true;
}

static void traceHeader(char flag, void *handle) {
  timeval tv;
  gettimeofday(&tv, NULL);
  if (handle) {
    fprintf(traceOut, "%c %ld %06ld %p ", flag, tv.tv_sec, tv.tv_usec, handle);
  } else {
    fprintf(traceOut, "%c %ld %06ld 0x0 ", flag, tv.tv_sec, tv.tv_usec);
  }
}

void traceBegin(void *nestHandle, const char *fmt, ...) {
  traceInit();
  if (!traceOut) {
    return;
  }
  traceHeader('B', nestHandle);
  va_list args;
  va_start(args, fmt);
  vfprintf(traceOut, fmt, args);
  va_end(args);
  fprintf(traceOut, "\n");
}

void traceEnd(void *nestHandle, const char *fmt, ...) {
  traceInit();
  if (!traceOut) {
    return;
  }
  traceHeader('E', nestHandle);
  va_list args;
  va_start(args, fmt);
  vfprintf(traceOut, fmt, args);
  va_end(args);
  fprintf(traceOut, "\n");
}

void traceAlloc(void *resourceHandle, const char *fmt, ...) {
  traceInit();
  if (!traceOut) {
    return;
  }
  traceHeader('A', resourceHandle);
  va_list args;
  va_start(args, fmt);
  vfprintf(traceOut, fmt, args);
  va_end(args);
  fprintf(traceOut, "\n");
}

void traceFree(void *resourceHandle, const char *fmt, ...) {
  traceInit();
  if (!traceOut) {
    return;
  }
  traceHeader('F', resourceHandle);
  va_list args;
  va_start(args, fmt);
  vfprintf(traceOut, fmt, args);
  va_end(args);
  fprintf(traceOut, "\n");
}

void traceMessage(const char *fmt, ...) {
  traceInit();
  if (!traceOut) {
    return;
  }
  traceHeader('M', NULL);
  va_list args;
  va_start(args, fmt);
  vfprintf(traceOut, fmt, args);
  va_end(args);
  fprintf(traceOut, "\n");
}

#endif // ENABLE_TRACING
