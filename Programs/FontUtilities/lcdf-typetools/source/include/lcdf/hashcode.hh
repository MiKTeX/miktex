#ifndef LCDF_HASHCODE_HH
#define LCDF_HASHCODE_HH
#include <stddef.h>

typedef size_t hashcode_t;	///< Typical type for a hashcode() value.

inline hashcode_t
hashcode(int x)
{
    return static_cast<hashcode_t>(x);
}

inline hashcode_t
hashcode(unsigned x)
{
    return static_cast<hashcode_t>(x);
}

inline hashcode_t
hashcode(long x)
{
    return static_cast<hashcode_t>(x);
}

inline hashcode_t
hashcode(unsigned long x)
{
    return static_cast<hashcode_t>(x);
}

#endif
