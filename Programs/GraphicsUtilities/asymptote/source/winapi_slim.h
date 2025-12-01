// winapi_slim.h
#pragma once

#ifndef NOMINMAX
#  define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif

#pragma push_macro("min")
#pragma push_macro("max")
#pragma push_macro("near")
#pragma push_macro("far")
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

#include <Windows.h>

#pragma pop_macro("far")
#pragma pop_macro("near")
#pragma pop_macro("max")
#pragma pop_macro("min")