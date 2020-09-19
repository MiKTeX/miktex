/* util.{cc,hh} -- various bits
 *
 * Copyright (c) 2003-2019 Eddie Kohler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "util.hh"
#include <lcdf/error.hh>
#include <lcdf/straccum.hh>
#include <lcdf/vector.hh>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#if defined(_MSDOS) || defined(_WIN32)
# include <fcntl.h>
# include <io.h>
#endif

String
read_file(String filename, ErrorHandler *errh, bool warning)
{
    FILE *f;
    if (!filename || filename == "-") {
	filename = "<stdin>";
	f = stdin;
#if defined(_MSDOS) || defined(_WIN32)
	// Set the file mode to binary
	_setmode(_fileno(f), _O_BINARY);
#endif
    } else if (!(f = fopen(filename.c_str(), "rb"))) {
	errh->xmessage((warning ? errh->e_warning : errh->e_error) + ErrorHandler::make_landmark_anno(filename), strerror(errno));
	return String();
    }

    StringAccum sa;
    int amt;
    do {
	if (char *x = sa.reserve(8192)) {
	    amt = fread(x, 1, 8192, f);
	    sa.adjust_length(amt);
	} else
	    amt = 0;
    } while (amt != 0);
    if (!feof(f) || ferror(f))
	errh->xmessage((warning ? errh->e_warning : errh->e_error) + ErrorHandler::make_landmark_anno(filename), strerror(errno));
    if (f != stdin)
	fclose(f);
    return sa.take_string();
}

String
printable_filename(const String &s)
{
    if (!s || s == "-")
	return String::make_stable("<stdin>");
    else
	return s;
}

String
pathname_filename(const String &path)
{
    int slash = path.find_right('/');
    if (slash >= 0 && slash != path.length() - 1)
	return path.substring(slash + 1);
    else
	return path;
}
