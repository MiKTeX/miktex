#pragma once

#if defined(UNIVERSAL_ENDIAN_BUILD)
#if defined(__BIG_ENDIAN__)
#define WORDS_BIGENDIAN 1
#endif // defined(__BIG_ENDIAN__)
#else // defined(UNIVERSAL_ENDIAN_BUILD)
#ifndef WORDS_BIGENDIAN
#undef WORDS_BIGENDIAN
#endif // !defined(WORDS_BIGENDIAN)
#endif // defined(UNIVERSAL_ENDIAN_BUILD)
