// -*- related-file-name: "../include/lcdf/error.hh" -*-
/*
 * error.{cc,hh} -- flexible classes for error reporting
 * Eddie Kohler
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 * Copyright (c) 2001-2019 Eddie Kohler
 * Copyright (c) 2008 Meraki, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <lcdf/error.hh>
#include <lcdf/straccum.hh>
#include <lcdf/hashmap.hh>
#include <errno.h>
#include <ctype.h>
#include <algorithm>
#ifndef __KERNEL__
# include <stdlib.h>
# if HAVE_UNISTD_H
#  include <unistd.h>
# endif
#endif

/** @file error.hh
 * @brief Flexible error handling classes.
 */

struct ErrorHandler::Conversion {
    String name;
    ConversionFunction hook;
    Conversion *next;
};
static ErrorHandler::Conversion *error_items;

const char ErrorHandler::e_abort[] = "<-999>";
const char ErrorHandler::e_fatal[] = "<-1>";
const char ErrorHandler::e_emergency[] = "<0>";
const char ErrorHandler::e_alert[] = "<1>";
const char ErrorHandler::e_critical[] = "<2>";
const char ErrorHandler::e_error[] = "<3>";
const char ErrorHandler::e_warning[] = "<4>";
const char ErrorHandler::e_warning_annotated[] = "<4>warning: ";
const char ErrorHandler::e_notice[] = "<5>";
const char ErrorHandler::e_info[] = "<6>";
const char ErrorHandler::e_debug[] = "<7>";

const int ErrorHandler::ok_result = 0;
const int ErrorHandler::error_result = -EINVAL;

ErrorHandler *ErrorHandler::the_default_handler = 0;
ErrorHandler *ErrorHandler::the_silent_handler = 0;


// ANNOTATION MANAGEMENT

static const char *
parse_level(const char *begin, const char *end, int *result)
{
    int x = 0;
    const char *s = begin;

    bool negative = false;
    if (s != end && *s == '-') {
	negative = true;
	++s;
    } else if (s != end && *s == '+')
	++s;

    const char *digits = s;
    for (; s != end && *s >= '0' && *s <= '9'; ++s)
	x = x * 10 + *s - '0';

    if (s != end && *s == '.')
	for (++s; s != end && *s >= '0' && *s <= '9'; ++s)
	    /* nada */;

    if (s == digits || (s == digits + 1 && s[-1] == '.'))
	return begin;
    if (result)
	*result = (negative ? -x : x);
    return s;
}

String
ErrorHandler::make_anno(const char *name, const String &value)
{
    StringAccum sa;
    sa.reserve(value.length() + 10);

    // level annotation requires special handling
    if (name[0] == '<' && name[1] == '>' && name[2] == 0) {
	if (parse_level(value.begin(), value.end(), 0) == value.end()) {
	    sa << '<' << value << '>';
	    return sa.take_string();
	} else
	    return String();
    }

    sa << '{' << name << ':';
    const char *last = value.begin(), *end = value.end();
    for (const char *s = value.begin(); s != end; ++s)
	if (*s == '\\' || *s == '}') {
	    sa.append(last, s);
	    sa << '\\' << *s;
	    last = s + 1;
	} else if (*s == '\n') {
	    sa.append(last, s);
	    sa << '\\' << 'n';
	    last = s + 1;
	}
    sa.append(last, end);
    sa << '}';
    return sa.take_string();
}

const char *
ErrorHandler::skip_anno(const String &str, const char *begin, const char *end,
			String *name_result, String *value_result, bool raw)
{
    String name, value;
    const char *s = begin;

    if (s + 3 <= end && *s == '<') {
	const char *x = parse_level(s + 1, end, 0);
	if (x != s + 1 && x != end && *x == '>') {
	    name = String::make_stable("<>", 2);
	    if (str)
		value = str.substring(begin + 1, x);
	    begin = x + 1;
	}

    } else if (s + 2 <= end && *s == '{' && s[1] == '}')
	begin = s + 2;

    else if (s + 3 <= end && *s == '{' && str) {
	for (++s; s != end && isalnum((unsigned char) *s); ++s)
	    /* nada */;
	if (s == end || s == begin + 1 || (*s != '}' && *s != ':'))
	    /* not an annotation */;
	else if (*s == '}' && str) {
	    name = str.substring(begin + 1, s);
	    begin = s + 1;
	} else if (*s == '}') {
	    name = String::make_stable("{}", 2);
	    begin = s + 1;
	} else if (str) {
	    const char *x, *last = s + 1;
	    StringAccum sa;
	    for (x = s + 1; x != end && *x != '\n' && *x != '}'; ++x)
		if (*x == '\\' && x + 1 != end && x[1] != '\n') {
		    if (!raw) {
			sa.append(last, x);
			sa << (x[1] == 'n' ? '\n' : x[1]);
			last = x + 2;
		    }
		    ++x;
		}
	    if (x != end && *x == '}') {
		name = str.substring(begin + 1, s);
		if (sa) {
		    sa.append(last, x);
		    value = sa.take_string();
		} else
		    value = str.substring(s + 1, x);
		begin = x + 1;
	    }
	} else {
	    const char *x;
	    for (x = s + 1; x != end && *x != '\n' && *x != '}'; ++x)
		if (*x == '\\' && x + 1 != end && x[1] != '\n')
		    ++x;
	    if (x != end && *x == '}') {
		name = String::make_stable("{}", 2);
		begin = x + 1;
	    }
	}
    }

    if (name_result)
	*name_result = name;
    if (value_result)
	*value_result = value;
    return begin;
}

const char *
ErrorHandler::parse_anno(const String &str, const char *begin, const char *end,
			 ...)
{
    const char *names[8];
    void *values[8];
    int nanno = 0;

    va_list val;
    va_start(val, end);
    while (const char *n = va_arg(val, const char *)) {
	assert(nanno < 8);
	names[nanno] = n;
	if (n[0] == '#')
	    values[nanno] = va_arg(val, int *);
	else
	    values[nanno] = va_arg(val, String *);
	++nanno;
    }

    String name, value;
    while (1) {
	begin = skip_anno(str, begin, end, &name, &value, false);
	if (!name)
	    break;
	for (int i = 0; i < nanno; ++i)
	    if (names[i][0] == '#') {
		if (name == (names[i] + 1))
		    parse_level(value.begin(), value.end(), (int *) values[i]);
	    } else {
		if (name == names[i])
		    *(String *) values[i] = value;
	    }
    }

    return begin;
}

String
ErrorHandler::combine_anno(const String &text, const String &anno)
{
    if (!anno)
	return text;

    String names[8], values[8];
    int nanno = 0;
    const char *abegin = anno.begin();
    while (abegin != anno.end()) {
	assert(nanno < 8);
	abegin = skip_anno(anno, abegin, anno.end(), &names[nanno], &values[nanno], true);
	if (names[nanno])
	    ++nanno;
	else
	    break;
    }

    const char *last = text.begin(), *s = last;
    String name;
    StringAccum sa;
    while (s != text.end()) {
	const char *line = s;
	uint32_t mask = (1U << nanno) - 1;
	while (1) {
	    s = skip_anno(text, s, text.end(), &name, 0, false);
	    if (!name)
		break;
	    for (int i = 0; i < nanno; ++i)
		if (name == names[i])
		    mask &= ~(1U << i);
	}

	if (mask) {
	    sa.append(last, line);
	    for (int i = 0; i < nanno; ++i)
		if (mask & (1U << i)) {
		    if (names[i].equals("<>", 2))
			sa << '<' << values[i] << '>';
		    else
			sa << '{' << names[i] << ':' << values[i] << '}';
		}
	    last = line;
	}
	if (abegin != anno.end()) {
	    sa.append(last, s);
	    sa.append(abegin, anno.end());
	    last = s;
	}

	while (s != text.end() && *s != '\n')
	    ++s;
	if (s != text.end())
	    ++s;
    }

    if (sa) {
	sa.append(last, text.end());
	return sa.take_string();
    } else
	return text;
}

String
ErrorHandler::clean_landmark(const String &landmark, bool with_colon)
{
    const char *end = landmark.end();
    while (end != landmark.begin() && isspace((unsigned char) end[-1]))
	--end;
    if (end != landmark.begin() && end[-1] == ':')
	--end;
    if (end == landmark.begin())
	return String();
    else if (with_colon)
	return landmark.substring(landmark.begin(), end) + ": ";
    else
	return landmark.substring(landmark.begin(), end);
}


// FORMATTING

#define NUMBUF_SIZE	128
#define ErrH		ErrorHandler

#if SIZEOF_UNSIGNED_LONG >= SIZEOF_VOID_P
typedef unsigned long do_number_t;
#else
typedef uintptr_t do_number_t;
#endif

static char*
do_number(do_number_t num, char *after_last, int base, int flags)
{
    const char *digits =
	((flags & ErrH::cf_uppercase) ? "0123456789ABCDEF" : "0123456789abcdef");
    char *pos = after_last;
    while (num) {
	*--pos = digits[num % base];
	num /= base;
    }
    if (pos == after_last)
	*--pos = '0';
    return pos;
}

static char *
do_number_flags(char *pos, char *after_last, int base, int flags,
		int precision, int field_width)
{
    // remove cf_alternate_form for zero results in base 16
    if ((flags & ErrH::cf_alternate_form) && base == 16 && *pos == '0')
	flags &= ~ErrH::cf_alternate_form;

    // account for zero padding
    if (precision >= 0)
	while (after_last - pos < precision)
	    *--pos = '0';
    else if (flags & ErrH::cf_zero_pad) {
	if ((flags & ErrH::cf_alternate_form) && base == 16)
	    field_width -= 2;
	if ((flags & ErrH::cf_negative)
	    || (flags & (ErrH::cf_plus_positive | ErrH::cf_space_positive)))
	    field_width--;
	while (after_last - pos < field_width)
	    *--pos = '0';
    }

    // alternate forms
    if ((flags & ErrH::cf_alternate_form) && base == 8 && pos[1] != '0')
	*--pos = '0';
    else if ((flags & ErrH::cf_alternate_form) && base == 16) {
	*--pos = ((flags & ErrH::cf_uppercase) ? 'X' : 'x');
	*--pos = '0';
    }

    // sign
    if (flags & ErrH::cf_negative)
	*--pos = '-';
    else if (flags & ErrH::cf_plus_positive)
	*--pos = '+';
    else if (flags & ErrH::cf_space_positive)
	*--pos = ' ';

    return pos;
}

String
ErrorHandler::vxformat(int default_flags, const char *s, va_list val)
{
    StringAccum msg;

    char numbuf[NUMBUF_SIZE];	// for numerics
    numbuf[NUMBUF_SIZE-1] = 0;

    String strstore;		// to ensure temporaries aren't destroyed

    // declare and initialize these here to make gcc shut up about possible
    // use before initialization
    int flags = 0;
    int field_width = -1;
    int precision = -1;
    int width_flag = 0;
    int base = 10;
    while (1) {

	const char *pct = strchr(s, '%');
	if (!pct) {
	    if (*s)
		msg << s;
	    break;
	}
	if (pct != s) {
	    msg.append(s, pct - s);
	    s = pct;
	}

	// parse flags
	flags = default_flags;
    flags:
	switch (*++s) {
	case '#': flags |= cf_alternate_form; goto flags;
	case '0': flags |= cf_zero_pad; goto flags;
	case '-': flags |= cf_left_just; goto flags;
	case ' ': flags |= cf_space_positive; goto flags;
	case '+': flags |= cf_plus_positive; goto flags;
	case '\'': flags |= cf_singlequote; goto flags;
	case '_': flags &= ~cf_utf8; goto flags;
	}

	// parse field width
	field_width = -1;
	if (*s == '*') {
	    field_width = va_arg(val, int);
	    if (field_width < 0) {
		field_width = -field_width;
		flags |= cf_left_just;
	    }
	    s++;
	} else if (*s >= '0' && *s <= '9')
	    for (field_width = 0; *s >= '0' && *s <= '9'; s++)
		field_width = 10*field_width + *s - '0';

	// parse precision
	precision = -1;
	if (*s == '.') {
	    s++;
	    precision = 0;
	    if (*s == '*') {
		precision = va_arg(val, int);
		s++;
	    } else if (*s >= '0' && *s <= '9')
		for (; *s >= '0' && *s <= '9'; s++)
		    precision = 10*precision + *s - '0';
	}

	// parse width flags
	width_flag = 0;
    width_flags:
	switch (*s) {
	case 'h': case 'l':
	    if (width_flag == *s)
		width_flag = *s + 'A' - 'a';
	    else if (width_flag)
		break;
	    else
		width_flag = *s;
	    s++;
	    goto width_flags;
	case 'z':
	case 't':
	    if (width_flag)
		break;
	    width_flag = *s++;
	    break;
	case '^':
	    if (!isdigit((unsigned char) s[1]) || width_flag)
		break;
	    for (s++; isdigit((unsigned char) *s); s++)
		width_flag = width_flag * 10 + *s - '0';
	    width_flag = -width_flag;
	    break;
	}

	// conversion character
	// after switch, data lies between `s1' and `s2'
	const char *s1 = 0, *s2 = 0;
	base = 10;
	switch (*s++) {

	case 's': {
	    s1 = va_arg(val, const char *);
	    if (!s1)
		s1 = "(null)";

	    // fetch length
	    int len;
	    if (precision < 0)
		len = strlen(s1);
	    else
		len = strnlen(s1, precision);

	    // transform string if alternate form
	    if (flags & cf_alternate_form) {
		strstore = String(s1, len).printable();
		if (precision < 0 || strstore.length() < precision)
		    len = strstore.length();
	    }

	    // quote characters that look like annotations, readjusting length
	    if (flags & (cf_singlequote | cf_alternate_form)) {
		if (!(flags & cf_alternate_form))
		    strstore = String(s1, len);

		// check first line, considering trailing part of 'msg'
		const char *mbegin = msg.end();
		while (mbegin != msg.begin() && mbegin[-1] != '\n')
		    --mbegin;
		if (skip_anno(strstore.begin(), strstore.end()) != strstore.begin()
		    && skip_anno(mbegin, msg.end()) == msg.end()) {
		    strstore = String::make_stable("{}", 2) + strstore;
		    len += 2;
		}

		// check subsequent lines
		const char *s = std::find(strstore.begin(), strstore.end(), '\n');
		while (s != strstore.end() && s + 1 != strstore.end()) {
		    size_t nextpos = (s + 1) - strstore.begin();
		    if (skip_anno(s + 1, strstore.end()) != s + 1) {
			strstore = strstore.substring(strstore.begin(), s + 1)
			    + String::make_stable("{}", 2)
			    + strstore.substring(s + 1, strstore.end());
			len += 2;
		    }
		    s = std::find(strstore.begin() + nextpos, strstore.end(), '\n');
		}
	    }

	    // obtain begin and end pointers
	    if (flags & (cf_singlequote | cf_alternate_form))
		s1 = strstore.begin();
	    s2 = s1 + len;
	    break;
	}

	case 'c': {
	    int c = va_arg(val, int);
	    // check for extension of 'signed char' to 'int'
	    if (c < 0)
		c += 256;
	    // assume ASCII
	    if (c == '\n')
		strcpy(numbuf, "\\n");
	    else if (c == '\t')
		strcpy(numbuf, "\\t");
	    else if (c == '\r')
		strcpy(numbuf, "\\r");
	    else if (c == '\0')
		strcpy(numbuf, "\\0");
	    else if (c < 0 || c >= 256)
		strcpy(numbuf, "(bad char)");
	    else if (c < 32 || c >= 0177)
		sprintf(numbuf, "\\%03o", c);
	    else
		sprintf(numbuf, "%c", c);
	    s1 = numbuf;
	    s2 = strchr(numbuf, 0);
	    break;
	}

	case '%': {
	    numbuf[0] = '%';
	    s1 = numbuf;
	    s2 = s1 + 1;
	    break;
	}

	case '<':
	    s1 = (flags & cf_utf8 ? "\342\200\230" : "\'");
	    s2 = s1 + strlen(s1);
	    break;

	case '>':
	case ',':
	    s1 = (flags & cf_utf8 ? "\342\200\231" : "\'");
	    s2 = s1 + strlen(s1);
	    break;

	case 'd':
	case 'i':
	    flags |= cf_signed;
            /* fallthru */
	case 'u':
	number: {
	    // protect numbuf from overflow
	    if (field_width > NUMBUF_SIZE)
		field_width = NUMBUF_SIZE;
	    if (precision > NUMBUF_SIZE - 4)
		precision = NUMBUF_SIZE - 4;

	    s2 = numbuf + NUMBUF_SIZE;

	    do_number_t num;
	    switch (width_flag) {
	    case 'H':
	    case -8:
		num = (unsigned char) va_arg(val, int);
		if ((flags & cf_signed) && (signed char) num < 0)
		    num = -(signed char) num, flags |= cf_negative;
		break;
	    case 'h':
	    case -16:
		num = (unsigned short) va_arg(val, int);
		if ((flags & cf_signed) && (short) num < 0)
		    num = -(short) num, flags |= cf_negative;
		break;
	    case 0:
	    case -32:
#if SIZEOF_LONG == 4
	    case 'l':
#endif
#if SIZEOF_SIZE_T == 4
	    case 'z':
#endif
#if SIZEOF_PTRDIFF_T == 4
	    case 't':
#endif
		num = va_arg(val, unsigned);
		if ((flags & cf_signed) && (int) num < 0)
		    num = -(int) num, flags |= cf_negative;
		break;
#if HAVE_INT64_TYPES
# if SIZEOF_LONG == 8
	    case 'l':
# endif
# if SIZEOF_LONG_LONG == 8
	    case 'L':
# endif
# if SIZEOF_SIZE_T == 8
	    case 'z':
# endif
# if SIZEOF_PTRDIFF_T == 8
	    case 't':
# endif
	    case -64: {
		uint64_t qnum = va_arg(val, uint64_t);
		if ((flags & cf_signed) && (int64_t)qnum < 0)
		    qnum = -(int64_t) qnum, flags |= cf_negative;
		StringAccum sa;
		sa.append_numeric(static_cast<String::uintmax_t>(qnum), base, (flags & cf_uppercase));
		s1 = s2 - sa.length();
		memcpy(const_cast<char*>(s1), sa.data(), s2 - s1);
		goto got_number;
	    }
#endif
	    default:
		goto error;
	    }
	    s1 = do_number(num, (char*) s2, base, flags);

#if HAVE_INT64_TYPES
	got_number:
#endif
	    s1 = do_number_flags((char*)s1, (char*) s2, base, flags, precision, field_width);
	    break;
	}

	case 'o':
	    base = 8;
	    goto number;

	case 'X':
	    flags |= cf_uppercase;
            /* fallthru */
	case 'x':
	    base = 16;
	    goto number;

	case 'p': {
	    if (*s == '{') {
		s1 = s2 = s + 1;
		while (*s2 && *s2 != '}' && !isspace((unsigned char) *s2))
		    ++s2;
		if (*s2 == '}')
		    goto braces;
	    }
	    void* v = va_arg(val, void*);
	    s2 = numbuf + NUMBUF_SIZE;
	    s1 = do_number((do_number_t) v, (char*) s2, 16, flags);
	    s1 = do_number_flags((char*) s1, (char*) s2, 16, flags | cf_alternate_form, precision, field_width);
	    break;
	}

#ifndef __KERNEL__
	case 'e': case 'f': case 'g':
	case 'E': case 'F': case 'G': {
	    char format[80], *f = format, new_numbuf[NUMBUF_SIZE];
	    *f++ = '%';
	    if (flags & cf_alternate_form)
		*f++ = '#';
	    if (precision >= 0)
		f += sprintf(f, ".%d", precision);
	    *f++ = s[-1];
	    *f++ = 0;

	    int len = sprintf(new_numbuf, format, va_arg(val, double));

	    s2 = numbuf + NUMBUF_SIZE;
	    s1 = s2 - len;
	    memcpy((char *)s1, new_numbuf, len); // note: no terminating \0
	    s1 = do_number_flags((char *)s1, (char *)s2, 10, flags & ~cf_alternate_form, -1, field_width);
	    break;
	}
#endif

	case '{':
	    s1 = s2 = s;
	    while (*s2 && *s2 != '}' && !isspace((unsigned char) *s2))
		++s2;
	    if (*s2 != '}')
		goto error;
	    goto braces;

	braces:
	    s = s2 + 1;
	    for (Conversion *item = error_items; item; item = item->next)
		if (item->name.equals(s1, s2 - s1)) {
		    strstore = item->hook(flags, VA_LIST_REF(val));
		    s1 = strstore.begin();
		    s2 = strstore.end();
		    goto got_result;
		}
	    goto error;

	error:
	default:
	    assert(0 /* Bad % in error */);
	    break;

	}

	// add result of conversion
    got_result:
	int slen = s2 - s1;
	if (slen > field_width)
	    field_width = slen;
	char *dest = msg.extend(field_width);
	if (flags & cf_left_just) {
	    memcpy(dest, s1, slen);
	    memset(dest + slen, ' ', field_width - slen);
	} else {
	    memcpy(dest + field_width - slen, s1, slen);
	    memset(dest, (flags & cf_zero_pad ? '0' : ' '), field_width - slen);
	}
    }

    return msg.take_string();
}

String
ErrorHandler::xformat(int default_flags, const char *fmt, ...)
{
    va_list val;
    va_start(val, fmt);
    String s = vxformat(default_flags, fmt, val);
    va_end(val);
    return s;
}

String
ErrorHandler::xformat(const char *fmt, ...)
{
    va_list val;
    va_start(val, fmt);
    String s = vxformat(0, fmt, val);
    va_end(val);
    return s;
}

String
ErrorHandler::format(const char *fmt, ...)
{
    va_list val;
    va_start(val, fmt);
    String s = vformat(fmt, val);
    va_end(val);
    return s;
}


// ERROR MESSAGE SHORTHAND

void
ErrorHandler::debug(const char *fmt, ...)
{
    va_list val;
    va_start(val, fmt);
    xmessage(String::make_stable(e_debug, 3), fmt, val);
    va_end(val);
}

void
ErrorHandler::message(const char *fmt, ...)
{
    va_list val;
    va_start(val, fmt);
    xmessage(String::make_stable(e_info, 3), fmt, val);
    va_end(val);
}

int
ErrorHandler::warning(const char *fmt, ...)
{
    va_list val;
    va_start(val, fmt);
    int r = xmessage(String::make_stable(e_warning_annotated, 12), fmt, val);
    va_end(val);
    return r;
}

int
ErrorHandler::error(const char *fmt, ...)
{
    va_list val;
    va_start(val, fmt);
    int r = xmessage(String::make_stable(e_error, 3), fmt, val);
    va_end(val);
    return r;
}

void
ErrorHandler::fatal(const char *fmt, ...)
{
    va_list val;
    va_start(val, fmt);
    (void) xmessage(String::make_stable(e_fatal, 4), fmt, val);
    va_end(val);
    abort();
}

void
ErrorHandler::ldebug(const String &landmark, const char *fmt, ...)
{
    va_list val;
    va_start(val, fmt);
    String l = make_landmark_anno(landmark);
    xmessage(String::make_stable(e_debug, 3) + l, fmt, val);
    va_end(val);
}

void
ErrorHandler::lmessage(const String &landmark, const char *fmt, ...)
{
    va_list val;
    va_start(val, fmt);
    String l = make_landmark_anno(landmark);
    xmessage(String::make_stable(e_info, 3) + l, fmt, val);
    va_end(val);
}

int
ErrorHandler::lwarning(const String &landmark, const char *fmt, ...)
{
    va_list val;
    va_start(val, fmt);
    String l = make_landmark_anno(landmark);
    int r = xmessage(l + String::make_stable(e_warning_annotated, 12), fmt, val);
    va_end(val);
    return r;
}

int
ErrorHandler::lerror(const String &landmark, const char *fmt, ...)
{
    va_list val;
    va_start(val, fmt);
    String l = make_landmark_anno(landmark);
    int r = xmessage(String::make_stable(e_error, 3) + l, fmt, val);
    va_end(val);
    return r;
}

void
ErrorHandler::lfatal(const String &landmark, const char *fmt, ...)
{
    va_list val;
    va_start(val, fmt);
    String l = make_landmark_anno(landmark);
    (void) xmessage(String::make_stable(e_fatal, 4) + l, fmt, val);
    va_end(val);
    abort();
}

int
ErrorHandler::xmessage(const String &str)
{
    String xstr = decorate(str);

    int min_level = 1000, xlevel = 1000;
    const char *s = xstr.begin(), *end = xstr.end();
    void *user_data = 0;
    while (s != end) {
	const char *l = parse_anno(xstr, s, end, "#<>", &xlevel,
				   (const char *) 0);
	const char *nl = std::find(l, end, '\n');
	String line = xstr.substring(s, nl);
	s = nl + (nl != end);
	user_data = emit(line, user_data, s != end);
	min_level = (xlevel < min_level ? xlevel : min_level);
    }

    account(min_level);

    return (min_level <= el_warning ? error_result : ok_result);
}

String
ErrorHandler::vformat(const char *fmt, va_list val)
{
    return vxformat(0, fmt, val);
}

String
ErrorHandler::decorate(const String &str)
{
    return str;
}

void *
ErrorHandler::emit(const String &, void *user_data, bool)
{
    return user_data;
}

void
ErrorHandler::account(int level)
{
    if (level <= el_error)
        ++_nerrors;
#ifndef __KERNEL__
    if (level <= el_abort)
      abort();
    else if (level <= el_fatal)
#if defined(MIKTEX)
      throw(-level);
#else
        exit(-level);
#endif
#endif
}


#ifndef __KERNEL__
//
// FILE ERROR HANDLER
//

FileErrorHandler::FileErrorHandler(FILE *f, const String &context)
    : _f(f), _context(context), _default_flags(0)
{
# if HAVE_UNISTD_H
    if (isatty(fileno(_f))) {
# endif
	char *s = getenv("LANG");
	if (s && (strstr(s, "UTF-8") != 0 || strstr(s, "UTF8") != 0
		  || strstr(s, "utf8") != 0))
	    _default_flags |= cf_utf8;
# if HAVE_UNISTD_H
    }
# endif
}

String
FileErrorHandler::vformat(const char *fmt, va_list val)
{
    return vxformat(_default_flags, fmt, val);
}

void *
FileErrorHandler::emit(const String &str, void *, bool)
{
    String landmark;
    const char *s = parse_anno(str, str.begin(), str.end(),
			       "l", &landmark, (const char *) 0);
    StringAccum sa;
    sa << _context << clean_landmark(landmark, true)
       << str.substring(s, str.end()) << '\n';
    ssize_t result = fwrite(sa.begin(), 1, sa.length(), _f);
    (void) result;
    return 0;
}

#endif


//
// STATIC ERROR HANDLERS
//

ErrorHandler::Conversion *
ErrorHandler::add_conversion(const String &name, ConversionFunction function)
{
    if (Conversion *c = new Conversion) {
	c->name = name;
	c->hook = function;
	c->next = error_items;
	error_items = c;
	return c;
    } else
	return 0;
}

int
ErrorHandler::remove_conversion(ErrorHandler::Conversion *conv)
{
    Conversion **pprev = &error_items;
    for (Conversion *c = error_items; c; pprev = &c->next, c = *pprev)
	if (c == conv) {
	    *pprev = c->next;
	    delete c;
	    return 0;
	}
    return -1;
}

ErrorHandler *
ErrorHandler::static_initialize(ErrorHandler *default_handler)
{
    if (!the_silent_handler) {
	the_default_handler = default_handler;
	the_silent_handler = new SilentErrorHandler;
    }
    return default_handler;
}

void
ErrorHandler::static_cleanup()
{
    delete the_default_handler;
    delete the_silent_handler;
    the_default_handler = the_silent_handler = 0;
    while (error_items) {
	Conversion *next = error_items->next;
	delete error_items;
	error_items = next;
    }
}

void
ErrorHandler::set_default_handler(ErrorHandler *errh)
{
    the_default_handler = errh;
}


//
// ERROR VENEER
//

String
ErrorVeneer::vformat(const char *fmt, va_list val)
{
    if (_errh)
	return _errh->vformat(fmt, val);
    else
	return ErrorHandler::vformat(fmt, val);
}

String
ErrorVeneer::decorate(const String &str)
{
    if (_errh)
	return _errh->decorate(str);
    else
	return ErrorHandler::decorate(str);
}

void *
ErrorVeneer::emit(const String &str, void *user_data, bool more)
{
    if (_errh)
	return _errh->emit(str, user_data, more);
    else
	return ErrorHandler::emit(str, user_data, more);
}

void
ErrorVeneer::account(int level)
{
    ErrorHandler::account(level);
    if (_errh)
	_errh->account(level);
}


//
// CONTEXT ERROR HANDLER
//

ContextErrorHandler::ContextErrorHandler(ErrorHandler *errh, const char *fmt,
					 ...)
    : ErrorVeneer(errh), _indent(String::make_stable("  ", 2)),
      _context_landmark("{l:}"), _context_printed(false)
{
    va_list val;
    va_start(val, fmt);
    _context = ErrorVeneer::vformat(fmt, val);
    va_end(val);
    if (_context)
	_context = combine_anno(_context, String::make_stable("{context:context}", 17));
}

String
ContextErrorHandler::decorate(const String &str)
{
    String context_anno;
    const char *str_endanno = parse_anno(str, str.begin(), str.end(),
					 "context", &context_anno,
					 (const char *) 0);
    if (context_anno.equals("no", 2))
	return ErrorVeneer::decorate(str);

    String istr;
    if (context_anno.equals("noindent", 8))
	istr = combine_anno(str, _context_landmark);
    else
	istr = combine_anno(str, _context_landmark + _indent);

    if (!_context_printed && !context_anno.equals("nocontext", 9)) {
	String astr = combine_anno(combine_anno(_context, _context_landmark),
				   str.substring(str.begin(), str_endanno));
	if (astr && astr.back() != '\n')
	    astr += '\n';
	_context_printed = true;
	return ErrorVeneer::decorate(astr + istr);
    } else
	return ErrorVeneer::decorate(istr);
}


//
// PREFIX ERROR HANDLER
//

PrefixErrorHandler::PrefixErrorHandler(ErrorHandler *errh,
				       const String &prefix)
    : ErrorVeneer(errh), _prefix(prefix)
{
}

String
PrefixErrorHandler::decorate(const String &str)
{
    return ErrorVeneer::decorate(combine_anno(str, _prefix));
}


//
// LANDMARK ERROR HANDLER
//

LandmarkErrorHandler::LandmarkErrorHandler(ErrorHandler *errh, const String &landmark)
    : ErrorVeneer(errh), _landmark(make_landmark_anno(landmark))
{
}

String
LandmarkErrorHandler::decorate(const String &str)
{
    return ErrorVeneer::decorate(combine_anno(str, _landmark));
}


//
// BAIL ERROR HANDLER
//

#ifndef __KERNEL__

BailErrorHandler::BailErrorHandler(ErrorHandler *errh, int l)
    : ErrorVeneer(errh), _level(l)
{
}

void
BailErrorHandler::account(int level)
{
    ErrorVeneer::account(level);
    if (level <= _level)
#if defined(MIKTEX)
      throw(1);
#else
	exit(1);
#endif
}

#endif
