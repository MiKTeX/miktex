// -*- related-file-name: "../include/lcdf/filename.hh" -*-

/* filename.{cc,hh} -- filenames
 *
 * Copyright (c) 1999-2019 Eddie Kohler
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
#include <lcdf/filename.hh>
#include <lcdf/landmark.hh>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef PATHNAME_SEPARATOR
# define PATHNAME_SEPARATOR '/'
#endif
#ifndef CURRENT_DIRECTORY
# define CURRENT_DIRECTORY "./"
#endif


Filename::Filename(const String &fn)
    : _path(fn), _actual(0)
{
    if (!fn)
	return;

    int last_slash = fn.find_right(PATHNAME_SEPARATOR);
    if (last_slash >= 0) {
	_dir = _path.substring(0, last_slash + 1);
	_name = _path.substring(last_slash + 1);
    } else {
	_dir = CURRENT_DIRECTORY;
	_name = fn;
    }

    assert(_dir.back() == PATHNAME_SEPARATOR);
}

Filename::Filename(const String &dir, const String &name)
    : _name(name), _actual(0)
{
    if (!name)
	return;

    if (_name[0] == PATHNAME_SEPARATOR)
	_dir = "";
    else if (dir) {
	_dir = dir;
	if (dir.back() != PATHNAME_SEPARATOR)
	    _dir += PATHNAME_SEPARATOR;
    } else
	_dir = CURRENT_DIRECTORY;

    int slash = name.find_right(PATHNAME_SEPARATOR);
    if (slash >= 0) {
	_dir += name.substring(0, slash + 1);
	_name = name.substring(slash + 1);
    }

    _path = _dir + _name;
}

Filename::Filename(FILE *actual, const String &name)
    : _name(name), _path(name), _actual(actual)
{
}


String
Filename::extension() const
{
    int dot = _name.find_right('.');
    while (dot > 0 && _name[dot - 1] == '.')
	dot--;
    if (dot > 0)
	return _name.substring(dot + 1);
    else
	return String();
}

String
Filename::base() const
{
    if (String ex = extension())
	return _name.substring(0, _name.length() - ex.length() - 1);
    else
	return _name;
}


FILE *
Filename::open_read(bool binary) const
{
    if (_actual || !_path)
	return _actual;
    else
	return fopen(_path.c_str(), binary ? "rb" : "r");
}

bool
Filename::readable() const
{
    struct stat s;
    if (!_path)
	return false;
    return _actual || (_path && (stat(_path.c_str(), &s) >= 0));
}


FILE *
Filename::open_write(bool binary) const
{
    if (_actual || !_path)
	return _actual;
    else
	return fopen(_path.c_str(), binary ? "wb" : "w");
}
