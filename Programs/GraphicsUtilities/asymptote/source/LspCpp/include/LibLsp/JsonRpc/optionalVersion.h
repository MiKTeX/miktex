#pragma once
#ifndef boost
#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)

#include <optional>
using std::optional;

#else
#include <tl/optional.hpp>
using tl::optional;
#endif
#endif
