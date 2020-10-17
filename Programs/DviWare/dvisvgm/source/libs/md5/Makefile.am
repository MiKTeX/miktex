if USE_BUNDLED_MD5
noinst_LIBRARIES = libmd5.a

libmd5_a_SOURCES = md5.c md5.h
AM_CXXFLAGS = -Wall

md5.c: md5.h

@CODE_COVERAGE_RULES@
endif

CLEANFILES = *.gcda *.gcno

