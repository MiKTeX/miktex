s:undef *HAVE_MEMORY_H *$:define HAVE_MEMORY_H 1:
s:undef *HAVE_STDLIB_H *$:define HAVE_STDLIB_H 1:
s:undef *HAVE_STRING_H *$:define HAVE_STRING_H 1:
s:undef *HAVE_SYS_STAT_H *$:define HAVE_SYS_STAT_H 1:
s:undef *HAVE_SYS_TYPES_H *$:define HAVE_SYS_TYPES_H 1:
s:undef *HAVE_DIRECT_H *$:define HAVE_DIRECT_H 1:
s:undef *HAVE_IO_H *$:define HAVE_IO_H 1:
s:undef *HAVE_WINDOWS_H *$:define HAVE_WINDOWS_H 1  /* yes, this is windows */:
s:undef *HAVE_WINBASE_H *$:define HAVE_WINBASE_H 1  /* hmm, is that win32 ? */:
s:undef *HAVE_WINNT_H *$:define HAVE_WINNT_H 1      /* is that always true? */:
s:undef *HAVE_ZLIB_H *$:define HAVE_ZLIB_H 1      /* you do have it, right? */:
s:undef *PACKAGE *$:define PACKAGE "zziplib-msvc"     /* yes, make it known */:
s:undef *SIZEOF_INT *$:define SIZEOF_INT 4:
s:undef *SIZEOF_LONG *$:define SIZEOF_LONG 4:
s:undef *SIZEOF_SHORT *$:define SIZEOF_SHORT 2:
s:undef *STDC_HEADERS *$:define STDC_HEADERS 1:
s:undef *inline *$:define inline __inline:
s:undef *off_t *$:define off_t long:
s:undef *off64_t *$:define off64_t __int64:
s:undef *ssize_t *$:define ssize_t int:
s:undef restrict *$:define restrict:
s:undef VERSION:define VERSION "0.13.x":
s:^ *# *undef.*$:/* & */:
/Generated automatically .* by autoheader/a\
/* config values have been automatically set by zzip/_msvc.sed */

