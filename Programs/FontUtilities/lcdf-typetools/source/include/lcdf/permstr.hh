// -*- related-file-name: "../../liblcdf/permstr.cc" -*-
#ifndef LCDF_PERMSTR_HH
#define LCDF_PERMSTR_HH
#include <assert.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <lcdf/inttypes.h>
#include <lcdf/hashcode.hh>
class PermString;
inline bool operator==(PermString a, PermString b);
inline bool operator!=(PermString a, PermString b);

class PermString { struct Doodad; public:

    typedef Doodad *Capsule;
    // Declare a PermString::Initializer in any file in which you declare
    // static global PermStrings.
    struct Initializer { Initializer(); };

    PermString()
	: _rep(zero_char_doodad.data) {
    }
    explicit PermString(char c)
	: _rep(one_char_doodad[(unsigned char) c].data) {
    }
    inline PermString(const char *s);
    inline PermString(const char *s, int len);
    inline PermString(const char *begin, const char *end);

    typedef int (PermString::*unspecified_bool_type)() const;
    inline operator unspecified_bool_type() const;
    inline bool operator!() const;

    inline int length() const;
    char operator[](int i) const;

    inline const char* data() const {
        return _rep;
    }
    inline const char *c_str() const {
	return _rep;
    }
    inline char operator*() const {
	return *_rep;
    }

    inline const char *begin() const;
    inline const char *end() const;

    inline bool equals(const char *s, int len) const;
    friend inline bool operator==(PermString a, PermString b);
    friend inline bool operator!=(PermString a, PermString b);

    inline Capsule capsule() const;
    inline static PermString decapsule(Capsule c);

    friend PermString permprintf(const char*, ...);
    friend PermString vpermprintf(const char*, va_list);
    friend PermString permcat(PermString, PermString);
    friend PermString permcat(PermString, PermString, PermString);

  private:

    struct Doodad {
	Doodad *next;
	int length;
	char data[2];
    };

    const char *_rep;

    PermString(Doodad* d)		: _rep(d->data) { }
    void initialize(const char*, int);
    Doodad* doodad() const { return (Doodad*)(_rep - offsetof(Doodad, data)); }

    friend struct PermString::Initializer;
    static void static_initialize();

    enum { NHASH = 1024 };	// must be power of 2
    static Doodad zero_char_doodad, one_char_doodad[256], *buckets[NHASH];

};


inline PermString::PermString(const char* s)
{
    initialize(s, -1);
}

inline PermString::PermString(const char* s, int len)
{
    initialize(s, len);
}

inline PermString::PermString(const char* begin, const char* end)
{
    assert(end);
    initialize(begin, end > begin ? end - begin : 0);
}

inline PermString::operator unspecified_bool_type() const
{
    return _rep != zero_char_doodad.data ? &PermString::length : 0;
}

inline bool PermString::operator!() const
{
    return _rep == zero_char_doodad.data;
}

inline int PermString::length() const
{
    return doodad()->length;
}

inline const char *PermString::begin() const
{
    return _rep;
}

inline const char *PermString::end() const
{
    return _rep + doodad()->length;
}

inline char PermString::operator[](int i) const
{
    assert((unsigned) i < (unsigned) length());
    return c_str()[i];
}

inline bool operator==(PermString a, PermString b)
{
    return a._rep == b._rep;
}

inline bool operator==(PermString a, const char *b)
{
    return (!a || !b ? !a && !b : strcmp(a.c_str(), b) == 0);
}

inline bool operator==(const char *a, PermString b)
{
    return b == a;
}

inline bool operator!=(PermString a, PermString b)
{
    return a._rep != b._rep;
}

inline bool operator!=(PermString a, const char *b)
{
    return !(a == b);
}

inline bool operator!=(const char *a, PermString b)
{
    return !(b == a);
}

inline bool operator<(PermString a, PermString b)
{
    // NOT lexicographic ordering!
    return a.begin() < b.begin();
}

inline bool PermString::equals(const char *s, int len) const
{
    if (len > 0)
	return len == length() && memcmp(s, _rep, len) == 0;
    else
	return strcmp(s, _rep) == 0;
}

inline PermString::Capsule PermString::capsule() const
{
    return doodad();
}

inline PermString PermString::decapsule(Capsule c)
{
    return PermString(c);
}

inline hashcode_t hashcode(PermString s)
{
    return (uintptr_t) s.c_str();
}

PermString permprintf(const char *format, ...);
PermString vpermprintf(const char *format, va_list val);

PermString permcat(PermString a, PermString b);

#endif
