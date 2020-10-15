#ifndef __ZZIP_INTERNAL_STRING_H
#define __ZZIP_INTERNAL_STRING_H

#ifdef __linux__
#define _GNU_SOURCE _glibc_developers_are_idiots_to_call_strndup_gnu_specific_
#endif

#include <zzip/conf.h>

#if   defined ZZIP_HAVE_STRING_H
#include <string.h>
#elif defined ZZIP_HAVE_STRINGS_H
#include <strings.h>
#endif

#if defined ZZIP_HAVE_STRNLEN || defined strnlen
#define _zzip_strnlen strnlen
#else
#include <stdlib.h>

/* if your system does not have strnlen: */
static size_t
_zzip_strnlen(const char *p, size_t maxlen)
{
    const char * stop = (char *)memchr(p, '\0', maxlen);
    return stop ? (size_t)(stop - p) : maxlen;
}
#endif


#if defined ZZIP_HAVE_STRNDUP || defined strndup
#define _zzip_strndup strndup
#else
#include <stdlib.h>

/* if your system does not have strndup: */
zzip__new__ static char *
_zzip_strndup(char const *p, size_t maxlen)
{
    if (p == NULL)
    {
       return p;
    } else 
    {
        size_t len = _zzip_strnlen(p, maxlen);
        char* r = (char *)malloc(len + 1);
        if (r == NULL)
            return NULL; /* errno = ENOMEM */
        r[len] = '\0';
        return memcpy(r, p, len);
    }
}
#endif

#if defined ZZIP_HAVE_STRCASECMP || defined strcasecmp
#define _zzip_strcasecmp strcasecmp
#else

/* if your system does not have strcasecmp: */
static int
_zzip_strcasecmp(char *__zzip_restrict a, char *_zzip_restrict b)
{
    if (! a)
        return (b) ? 1 : 0;
    if (! b)
        return -1;
    while (1)
    {
        int v = tolower(*a) - tolower(*b);
        if (v)
            return v;
        if (! *a)
            return 1;
        if (! *b)
            return -1;
        a++;
        b++;
    }
}
#endif

#endif
