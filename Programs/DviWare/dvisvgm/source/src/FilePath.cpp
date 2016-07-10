/*************************************************************************
** FilePath.cpp                                                         **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2016 Martin Gieseking <martin.gieseking@uos.de>   **
**                                                                      **
** This program is free software; you can redistribute it and/or        **
** modify it under the terms of the GNU General Public License as       **
** published by the Free Software Foundation; either version 3 of       **
** the License, or (at your option) any later version.                  **
**                                                                      **
** This program is distributed in the hope that it will be useful, but  **
** WITHOUT ANY WARRANTY; without even the implied warranty of           **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         **
** GNU General Public License for more details.                         **
**                                                                      **
** You should have received a copy of the GNU General Public License    **
** along with this program; if not, see <http://www.gnu.org/licenses/>. **
*************************************************************************/

#include <config.h>
#include <cctype>
#include "FilePath.h"
#include "FileSystem.h"
#include "MessageException.h"
#include "macros.h"

#if defined(MIKTEX)
#  include <miktex/Core/PathName>
#endif

using namespace std;


/** Removes redundant slashes from a given path. */
static string& single_slashes (string &str) {
	size_t pos=0;
	while ((pos = str.find("//", pos)) != string::npos)
		str.replace(pos, 2, "/");
	return str;
}


#ifdef _WIN32
static char strip_drive_letter (string &path) {
	char letter = 0;
	if (path.length() >= 2 && path[1] == ':' && isalpha(path[0])) {
		letter = tolower(path[0]);
		path.erase(0, 2);
	}
	return letter;
}


static char adapt_current_path (string &path, char target_drive) {
	if (char current_drive = strip_drive_letter(path)) {
		if (target_drive != current_drive) {
			if (target_drive == 0)
				target_drive = current_drive;
			if (path.empty() || path[0] != '/') {
				if (FileSystem::chdir(string(1, target_drive) + ":")) {
					path.insert(0, FileSystem::getcwd()+"/");
					strip_drive_letter(path);
				}
				else
					throw MessageException("drive " + string(1, target_drive) + ": not accessible");
			}
		}
	}
	return target_drive;
}


static void tolower (string &str) {
#if defined(MIKTEX)
  MiKTeX::Core::PathName path(str);
  str = path.Convert({ MiKTeX::Core::ConvertPathNameOption::MakeLower }).ToString();
#else
	for (size_t i=0; i < str.length(); ++i)
		str[i] = tolower(str[i]);
#endif
}
#endif


/** Constructs a FilePath object from a given path. Relative paths are
 *  relative to the current working directory.
 *  @param[in] path absolute or relative path to a file or directory */
FilePath::FilePath (const string &path) {
	init(path, !FileSystem::isDirectory(path.c_str()), FileSystem::getcwd());
}


/** Constructs a FilePath object.
 *  @param[in] path absolute or relative path to a file or directory
 *  @param[in] isfile true if 'path' references a file, false if a directory is referenced
 *  @param[in] current_dir if 'path' is a relative path expression it will be related to 'current_dir' */
FilePath::FilePath (const string &path, bool isfile, string current_dir) {
	init(path, isfile, current_dir);
}


/** Initializes a FilePath object. This method should be called by the constructors only.
 *  @param[in] path absolute or relative path to a file or directory
 *  @param[in] isfile true if 'path' references a file, false if a directory is referenced
 *  @param[in] current_dir if 'path' is a relative path expression it will be related to 'current_dir' */
void FilePath::init (string path, bool isfile, string current_dir) {
	single_slashes(path);
	single_slashes(current_dir);
#ifdef _WIN32
	tolower(path);
	path = FileSystem::adaptPathSeperators(path);
	_drive = strip_drive_letter(path);
#endif
	if (isfile) {
		size_t pos = path.rfind('/');
		_fname = path.substr((pos == string::npos) ? 0 : pos+1);
		if (pos != string::npos)
			path.erase(pos);
		else
			path.clear();
	}
	if (current_dir.empty())
		current_dir = FileSystem::getcwd();
#ifdef _WIN32
	tolower(current_dir);
	_drive = adapt_current_path(current_dir, _drive);
#endif
	if (!path.empty()) {
		if (path[0] == '/')
			current_dir.clear();
		else if (current_dir[0] != '/')
			current_dir = "/";
		else {
			FilePath curr(current_dir, false, "/");
			current_dir = curr.absolute();
#ifdef _WIN32
			adapt_current_path(current_dir, _drive);
#endif
		}
	}
	path.insert(0, current_dir + "/");
	string elem;
	FORALL (path, string::const_iterator, it) {
		if (*it == '/') {
			add(elem);
			elem.clear();
		}
		else
			elem += *it;
	}
	add(elem);
}


/** Adds a location step to the current path. */
void FilePath::add (const string &dir) {
	if (dir == ".." && !_dirs.empty())
		_dirs.pop_back();
	else if (dir.length() > 0 && dir != ".")
		_dirs.push_back(dir);
}


/** Returns the suffix of the filename. If FilePath represents the
 *  location of a directory (and not of a file) an empty string
 *  is returned. */
string FilePath::suffix () const {
	size_t start = 0;
	// ignore leading dots
	while (start < _fname.length() && _fname[start] == '.')
		start++;
	string sub = _fname.substr(start);
	size_t pos = sub.rfind('.');
	if (pos != string::npos && pos < sub.length()-1)
		return sub.substr(pos+1);
	return "";
}


/** Changes the suffix of the filename. If FilePath represents the
 *  location of a directory (and not of a file) nothing happens.
 *  @param[in] s new suffix */
void FilePath::suffix (const string &s) {
	if (!_fname.empty()) {
		string current_suffix = suffix();
		if (!current_suffix.empty())
			_fname.erase(_fname.length()-current_suffix.length()-1);
		_fname += "."+s;
	}
}


/** Returns the filename without suffix.
 *  Example: FilePath("/a/b/c.def", true) == "c" */
string FilePath::basename () const {
	if (!_fname.empty()) {
		size_t len = suffix().length();
		if (len > 0)
			len++; // strip dot too
		return _fname.substr(0, _fname.length()-len);
	}
	return "";
}


/** Returns the absolute path string of this FilePath.
 *  @param[in] with_filename if false, the filename is omitted
 *  @return the absolute path string */
string FilePath::absolute (bool with_filename) const {
	string path;
	FORALL (_dirs, ConstIterator, it) {
		path += "/" + *it;
	}
	if (path.empty())
		path = "/";
	if (with_filename && !_fname.empty())
		path += "/"+_fname;
#ifdef _WIN32
	if (_drive)
		path.insert(0, string(1, _drive) + ":");
#endif
	return single_slashes(path);
}


/** Returns a path string of this FilePath relative to reldir. If we wanted to
 *  navigate from /a/b/c/d to /a/b/e/f using the shell command "cd", we could do that
 *  with the relative path argument: "cd ../../e/f". This function returns such a relative
 *  path. Example: FilePath("/a/b/e/f").relative("/a/b/c/d") => "../../e/f".
 *  @param[in] reldir absolute path to a directory
 *  @param[in] with_filename if false, the filename is omitted
 *  @return the relative path string */
string FilePath::relative (string reldir, bool with_filename) const {
	if (reldir.empty())
		reldir = FileSystem::getcwd();
#ifdef _WIN32
	adapt_current_path(reldir, _drive);
#endif
	if (reldir[0] != '/')
		return absolute();
	FilePath rel(reldir, false);
	string path;
#ifdef _WIN32
	if (rel._drive && _drive && rel._drive != _drive)
		path += string(1, _drive) + ":";
#endif
	ConstIterator i = _dirs.begin();
	ConstIterator j = rel._dirs.begin();
	while (i != _dirs.end() && j != rel._dirs.end() && *i == *j)
		++i, ++j;
	for (; j != rel._dirs.end(); ++j)
		path += "../";
	for (; i != _dirs.end(); ++i)
		path += *i + "/";
	if (!path.empty())
		path.erase(path.length()-1, 1);  // remove trailing slash
	if (with_filename && !_fname.empty()) {
		if (!path.empty() && path != "/")
			path += "/";
		path += _fname;
	}
	if (path.empty())
		path = ".";
	return single_slashes(path);
}

