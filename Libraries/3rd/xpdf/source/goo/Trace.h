//========================================================================
//
// Trace.h
//
// Nested tracing.
//
// Copyright 2020 Glyph & Cog, LLC
//
//========================================================================

#ifndef TRACING_H
#define TRACING_H

#include <aconf.h>

#if ENABLE_TRACING

// Enter a nesting level.
extern void traceBegin(void *nestHandle, const char *fmt, ...);

// Exit a nesting level.
extern void traceEnd(void *nestHandle, const char *fmt, ...);

// Mark a resource as allocated.
extern void traceAlloc(void *resourceHandle, const char *fmt, ...);

// Mark a resource as freed.
extern void traceFree(void *resourceHandle, const char *fmt, ...);

// Misc message.
extern void traceMessage(const char *fmt, ...);

#else // ENABLE_TRACING

static inline void traceBegin(void *nestHandle, const char *fmt, ...) {}
static inline void traceEnd(void *nestHandle, const char *fmt, ...) {}
static inline void traceAlloc(void *resourceHandle, const char *fmt, ...) {}
static inline void traceFree(void *nestHandle, const char *fmt, ...) {}
static inline void traceMessage(const char *fmt, ...) {}

#endif // ENABLE_TRACING

#endif // TRACING_H
