// win_sanitizer.h
#pragma once

#ifndef NOMINMAX
#  define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif


#ifdef min
#  undef min
#endif
#ifdef max
#  undef max
#endif
#ifdef near
#  undef near
#endif
#ifdef far
#  undef far
#endif


#ifdef _InterlockedIncrement
#  undef _InterlockedIncrement
#endif
#ifdef _InterlockedDecrement
#  undef _InterlockedDecrement
#endif
#ifdef _InterlockedExchange
#  undef _InterlockedExchange
#endif
#ifdef _InterlockedExchangeAdd
#  undef _InterlockedExchangeAdd
#endif
#ifdef _InterlockedCompareExchange
#  undef _InterlockedCompareExchange
#endif
