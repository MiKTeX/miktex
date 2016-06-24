// see: http://www.gotw.ca/gotw/029.htm

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(E034F645499649F392B9201728F88A26)
#define E034F645499649F392B9201728F88A26

#include <miktex/Core/config.h>

#include <cctype>
#include <functional>
#include <string>

#include <miktex/Util/inliners.h>

#include "Debug.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

struct ci_char_traits : public std::char_traits<char>
{
  static bool eq(char ch1, char ch2)
  {
    MIKTEX_ASSERT(static_cast<unsigned>(ch1) < 128 && static_cast<unsigned>(ch2) < 128);
    return tolower(ch1) == tolower(ch2);
  }

  static bool lt(char ch1, char ch2)
  {
    MIKTEX_ASSERT(static_cast<unsigned>(ch1) < 128 && static_cast<unsigned>(ch2) < 128);
    return tolower(ch1) < tolower(ch2);
  }

  static int compare(const char * s1, const char * s2, size_t n)
  {
    return MiKTeX::Util::StringCompare(s1, s2, n, true);
  }

  static const char * find(const char * s, int n, const char & ch)
  {
    while (n-- > 0 && tolower(*s) != tolower(ch))
    {
      ++s;
    }
    return s;
  }
};

typedef std::basic_string<char, ci_char_traits> ci_string;

MIKTEX_CORE_END_NAMESPACE;

namespace std
{
  template <> struct hash<MiKTeX::Core::ci_string>
  {
    size_t operator ()(const MiKTeX::Core::ci_string & str) const
    {
      // see http://www.isthe.com/chongo/tech/comp/fnv/index.html
#if defined(_M_AMD64) || defined(_M_X64) || defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
      MIKTEX_ASSERT(sizeof(size_t) == 8);
      const size_t FNV_prime = 1099511628211ULL;
      const size_t offset_basis = 14695981039346656037ULL;
#else
      MIKTEX_ASSERT(sizeof(size_t) == 4);
      const size_t FNV_prime = 16777619;
      const size_t offset_basis = 2166136261;
#endif
      size_t hash = offset_basis;
      for (MiKTeX::Core::ci_string::const_iterator it = str.begin(); it != str.end(); ++it)
      {
	char ch = *it;
	MIKTEX_ASSERT(static_cast<unsigned>(ch) < 128);
	ch = tolower(ch);
	hash ^= (size_t)ch;
	hash *= FNV_prime;
      }
      return hash;
    }
  };
}

#endif
