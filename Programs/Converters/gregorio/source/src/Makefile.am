# Copyright (C) 2006-2016 The Gregorio Project (see CONTRIBUTORS.md)
#
# This file is part of Gregorio.
#
# Gregorio is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Gregorio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Gregorio.  If not, see <http://www.gnu.org/licenses/>.

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/src/gabc  -I$(top_srcdir)/src/dump  -I$(top_srcdir)/src/gregoriotex
AM_CFLAGS = $(KPSE_CFLAGS)
LDADD = $(KPSE_LIBS)

bin_PROGRAMS = gregorio-$(FILENAME_VERSION)
gregorio___FILENAME_VERSION__SOURCES = \
	gregorio-utils.c characters.c characters.h messages.c messages.h struct.c \
	struct.h struct_iter.h enum_generator.h unicode.c unicode.h sha1.c sha1.h \
	support.c support.h config.h bool.h plugins.h utf8strings.h dump/dump.c \
	gregoriotex/gregoriotex-write.c gregoriotex/gregoriotex-position.c \
	gregoriotex/gregoriotex.h

@MK@ifneq ($(wildcard ../.git),)
@MK@  _tag_ = $(shell git describe --exact-match HEAD 2>/dev/null)
@MK@  ifeq ($(_tag_),)
@MK@    DEFS += -DBRANCH_VERSION='"$(shell git rev-parse --abbrev-ref HEAD)-$(shell git rev-parse --short HEAD)-$(shell git rev-list HEAD --count)"'
@MK@  endif
@MK@else
@MK@  ifneq ($(wildcard ../.hg/git),)
@MK@    _gitnode_ = $(shell hg log --template='{gitnode}' -r .)
@MK@    _tag_ = $(shell git --git-dir=../.hg/git describe --exact-match $(_gitnode_) 2>/dev/null)
@MK@    ifeq ($(_tag_),)
@MK@      DEFS += -DBRANCH_VERSION='"$(shell cat ../.hg/bookmarks.current)-$(shell git --git-dir=../.hg/git rev-parse --short '$(_gitnode_)')-$(shell git --git-dir=../.hg/git rev-list '$(_gitnode_)' --count)"'
@MK@    endif
@MK@  endif
@MK@endif

# gabc files
gregorio___FILENAME_VERSION__SOURCES += \
	gabc/gabc-elements-determination.c gabc/gabc-write.c \
	gabc/gabc-glyphs-determination.c gabc/gabc.h \
	gabc/gabc-score-determination.h gabc/gabc-score-determination.c \
	gabc/gabc-score-determination-y.h gabc/gabc-score-determination-y.c \
	gabc/gabc-score-determination-l.h gabc/gabc-score-determination-l.c \
	gabc/gabc-notes-determination-l.c vowel/vowel.h vowel/vowel.c \
	vowel/vowel-rules.h  vowel/vowel-rules-l.h vowel/vowel-rules-l.c \
	vowel/vowel-rules-y.h vowel/vowel-rules-y.c

EXTRA_DIST = encode_utf8strings.c utf8strings.h.in utf8strings.h \
			 gabc/gabc-notes-determination.l gabc/gabc-notes-determination-l.c \
			 gabc/gabc-score-determination.h gabc/gabc-score-determination.y \
			 gabc/gabc-score-determination-y.h \
			 gabc/gabc-score-determination-y.c gabc/gabc-score-determination.l \
			 gabc/gabc-score-determination-l.h \
			 gabc/gabc-score-determination-l.c vowel/vowel-rules.l \
			 vowel/vowel-rules-l.h vowel/vowel-rules-l.c vowel/vowel-rules.y \
			 vowel/vowel-rules-y.h vowel/vowel-rules-y.c

if HAVE_RC
# Windows resources (see windows/README.md)
gregorio-resources.o: ../windows/gregorio-resources.rc ../windows/gregorio.ico
	$(RC) $(RCFLAGS) $< -o $@

gregorio___FILENAME_VERSION__SOURCES += ../windows/gregorio-resources.rc ../windows/gregorio.ico
LDADD += gregorio-resources.o
endif

gabc/gabc-score-determination-y.c: gabc/gabc-score-determination.y
	$(YACC) -d -p "gabc_score_determination_" -o $@ $<
gabc/gabc-score-determination-y.h: gabc/gabc-score-determination-y.c
	## Recover from the removal of gabc/gabc-score-determination-y.h, see
	## https://www.gnu.org/software/automake/manual/html_node/Multiple-Outputs.html
	@if test -f $@; then :; else \
		rm -f gabc/gabc-score-determination-y.c; \
		$(MAKE) $(AM_MAKEFLAGS) gabc/gabc-score-determination-y.c; \
	fi

gabc/gabc-score-determination-l.c: gabc/gabc-score-determination.l
	$(LEX) -o $@ --header-file=gabc/gabc-score-determination-l.h $<
gabc/gabc-score-determination-l.h: gabc/gabc-score-determination-l.c
	## Recover from the removal of gabc/gabc-score-determination-l.h, see
	## https://www.gnu.org/software/automake/manual/html_node/Multiple-Outputs.html
	@if test -f $@; then :; else \
		rm -f gabc/gabc-score-determination-l.c; \
		$(MAKE) $(AM_MAKEFLAGS) gabc/gabc-score-determination-l.c; \
	fi

gabc/gabc-notes-determination-l.c: gabc/gabc-notes-determination.l
	$(LEX) -o $@ $<

vowel/vowel-rules-y.c: vowel/vowel-rules.y
	$(YACC) -d -o $@ $<
vowel/vowel-rules-y.h: vowel/vowel-rules-y.c
	## Recover from the removal of vowel/vowel-rules-y.h, see
	## https://www.gnu.org/software/automake/manual/html_node/Multiple-Outputs.html
	@if test -f $@; then :; else \
		rm -f vowel/vowel-rules-y.c; \
		$(MAKE) $(AM_MAKEFLAGS) vowel/vowel-rules-y.c; \
	fi

vowel/vowel-rules-l.c: vowel/vowel-rules.l
	$(LEX) -o $@ --header-file=vowel/vowel-rules-l.h $<
vowel/vowel-rules-l.h: vowel/vowel-rules-l.c
	## Recover from the removal of vowel/vowel-rules-l.h, see
	## https://www.gnu.org/software/automake/manual/html_node/Multiple-Outputs.html
	@if test -f $@; then :; else \
		rm -f vowel/vowel-rules-l.c; \
		$(MAKE) $(AM_MAKEFLAGS) vowel/vowel-rules-l.c; \
	fi

utf8strings.h: utf8strings.h.in
	$(MAKE) $(AM_MAKEFLAGS) encode_utf8strings${EXEEXT}
	./encode_utf8strings${EXEEXT} $< $@

encode_utf8strings${EXEEXT}: encode_utf8strings.c
	$(CC) -o $@ $<

clean-local:
	find . -name '*.gcno' -print | xargs rm -f --
	find . -name '*.gcda' -print | xargs rm -f --

BUILT_SOURCES = utf8strings.h gabc/gabc-notes-determination-l.c \
				gabc/gabc-score-determination-l.c \
				gabc/gabc-score-determination-l.h \
				gabc/gabc-score-determination-y.c \
				gabc/gabc-score-determination-y.h vowel/vowel-rules-l.c \
				vowel/vowel-rules-l.h vowel/vowel-rules-y.c \
				vowel/vowel-rules-y.h

CLEANFILES = encode_utf8strings${EXEEXT}
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
