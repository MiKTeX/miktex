/*************************************************************************
** FilePath.cpp                                                         **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2022 Martin Gieseking <martin.gieseking@uos.de>   **
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

#include <cctype>
#include "FilePath.hpp"
#include "FileSystem.hpp"
#include "MessageException.hpp"
#include "utility.hpp"

using namespace std;


/** Removes redundant slashes from a given path. */
static string& single_slashes (string &str) {
	size_t pos=0;
	while ((pos = str.find("//", pos)) != string::npos)
		str.replace(pos, 2, "/");
	return str;
}


#ifdef _WIN32
/** Returns the drive letter of a given path string or 0 if there's none. */
static char drive_letter (const string &path) {
	if (path.length() >= 2 && path[1] == ':' && isalpha(path[0]))
		return tolower(path[0]);
	return '\0';
}

/** Removes the drive letter and following colon from a given path string if present.
 *  @param[in] path path to strip drive letter from
 *  @return drive letter or 0 if there was none */
static char strip_drive_letter (string &path) {
	char letter = '\0';
	if (path.length() >= 2 && path[1] == ':' && isalpha(path[0])) {
		letter = path[0];
		path.erase(0, 2);
	}
	return tolower(letter);
}
#endif


bool FilePath::Directory::operator == (const Directory &dir) const {
#ifdef _WIN32
	// letter case is not significant on Windows systems
	return util::tolower(_dirstr) == util::tolower(dir._dirstr);
#else
	return _dirstr == dir._dirstr;
#endif
}


/** Constructs a FilePath object.
 *  @param[in] path absolute or relative path to a file or directory
 *  @param[in] isfile true if 'path' references a file, false if a directory is referenced
 *  @param[in] current_dir if 'path' is a relative path expression it will be related to 'current_dir' */
FilePath::FilePath (const string &path, bool isfile, const string &current_dir) {
	init(path, isfile, current_dir);
}


/** Assigns a new path of a file or directory that already exists.
 *  Relative paths are relative to the current working directory.
 *  @param[in] path absolute or relative path to a file or directory */
void FilePath::set (const string &path) {
	set(path, !FileSystem::isDirectory(path));
}


/** Assigns a new path. Relative paths are relative to the current working directory.
 *  @param[in] path absolute or relative path to a file or directory
 *  @param[in] isfile true if 'path' references a file, false if a directory is referenced */
void FilePath::set (const string &path, bool isfile) {
	init(path, isfile, "");
}


/** Assigns a new path. Relative paths are relative to the current working directory.
 *  @param[in] path absolute or relative path to a file or directory
 *  @param[in] isfile true if 'path' references a file, false if a directory is referenced
 *  @param[in] current_dir if 'path' is a relative path expression it will be related to 'current_dir' */
void FilePath::set (const string &path, bool isfile, const string &current_dir) {
	init(path, isfile, current_dir);
}


/** Initializes a FilePath object. This method should be called by the constructors only.
 *  @param[in] path absolute or relative path to a file or directory
 *  @param[in] isfile true if 'path' references a file, false if a directory is referenced
 *  @param[in] current_dir if 'path' is a relative path expression it will be related to 'current_dir' */
void FilePath::init (string path, bool isfile, string current_dir) {
	_dirs.clear();
	_fname.clear();
	single_slashes(path);
	single_slashes(current_dir);
#ifndef _WIN32
	if (current_dir.empty())
		current_dir = FileSystem::getcwd();
#else
	_drive = strip_drive_letter(path);
	if (current_dir.empty() || drive_letter(current_dir) != _drive)
		current_dir = FileSystem::getcwd(_drive);
	if (!_drive)
		_drive = drive_letter(current_dir);
	strip_drive_letter(current_dir);
	path = FileSystem::ensureForwardSlashes(path);
#endif
	if (isfile) {
		size_t pos = path.rfind('/');
		_fname = path.substr((pos == string::npos) ? 0 : pos+1);
		// remove filename from path
		if (pos == 0 && _fname.length() > 1)  // file in root directory?
			path.erase(1);
		else if (pos != string::npos)
			path.erase(pos);
		else
			path.clear();
	}
	if ((path.empty() || path[0] != '/') && !current_dir.empty())
		path.insert(0, current_dir + "/");
	string elem;
	for (char c : path) {
		if (c != '/')
			elem += c;
		else if (!elem.empty()){
			add(elem);
			elem.clear();
		}
	}
	if (!elem.empty())
		add(elem);
}


/** Adds a location step to the current path. */
void FilePath::add (const string &dir) {
	if (dir == ".." && !_dirs.empty())
		_dirs.pop_back();
	else if (!dir.empty() && dir != ".")
		_dirs.emplace_back(dir);
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
 *  location of a directory (and not of a file) nothing happens. An
 *  empty new suffix leads to the removal of the current one.
 *  @param[in] newSuffix the new suffix */
void FilePath::suffix (const string &newSuffix) {
	if (!_fname.empty()) {
		string current_suffix = suffix();
		if (!current_suffix.empty())
			_fname.erase(_fname.length()-current_suffix.length()-1);
		if (!newSuffix.empty())
			_fname += "."+newSuffix;
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
#ifdef _WIN32
	if (_drive)
		path = string(1, _drive) + ":";
#endif
	for (const Directory &dir : _dirs)
		path += "/" + string(dir);
	if (path.empty())
		path = "/";
	if (with_filename && !_fname.empty())
		path += "/"+_fname;
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
#ifdef _WIN32
	char reldrive = drive_letter(reldir);
	if (reldir.empty()) {
		reldir = FileSystem::getcwd(_drive);
		reldrive = drive_letter(FileSystem::getcwd());
	}
	bool isAbsolute = (reldir[0] == '/')
		|| (reldir.length() >= 3 && isalpha(reldir[0]) && reldir[1] == ':' && reldir[2] == '/');
#else
	if (reldir.empty())
		reldir = FileSystem::getcwd();
	bool isAbsolute = (reldir[0] == '/');
#endif
	if (!isAbsolute)
		return absolute();
	FilePath relpath(reldir, false);
	string path;
	auto it1 = _dirs.begin();
	auto it2 = relpath._dirs.begin();
	while (it1 != _dirs.end() && it2 != relpath._dirs.end() && *it1 == *it2)
		++it1, ++it2;
	for (; it2 != relpath._dirs.end(); ++it2)
		path += "../";
	for (; it1 != _dirs.end(); ++it1)
		path += string(*it1) + "/";
	if (!path.empty() && path.back() == '/')
		path.erase(path.length()-1, 1);  // remove trailing slash
	if (with_filename && !_fname.empty()) {
		if (!path.empty() && path != "/")
			path += "/";
		path += _fname;
	}
	if (path.empty())
		path = ".";
#ifdef _WIN32
	if (relpath._drive && _drive && _drive != reldrive)
		path.insert(0, string(1, _drive) + ":");
#endif
	return single_slashes(path);
}


string FilePath::relative (const FilePath &filepath, bool with_filename) const {
	return relative(filepath.absolute(false), with_filename);
}


/** Return the absolute or relative path whichever is shorter.
*  @param[in] reldir absolute path to a directory
*  @param[in] with_filename if false, the filename is omitted */
string FilePath::shorterAbsoluteOrRelative (string reldir, bool with_filename) const {
	string abs = absolute(with_filename);
	string rel = relative(reldir, with_filename);
	return abs.length() < rel.length() ? abs : rel;
}


bool FilePath::exists () const {
	return empty() ? false : FileSystem::exists(absolute());
}


/** Checks if a given path is absolute or relative.
 *  @param[in] path path string to check
 *  @return true if path is absolute */
bool FilePath::isAbsolute (string path) {
	path = util::trim(path);
#ifdef _WIN32
	path = FileSystem::ensureForwardSlashes(path);
	if (path.length() >= 2 && path[1] == ':' && isalpha(path[0]))
		path.erase(0, 2);  // remove drive letter and colon
#endif
	return !path.empty() && path[0] == '/';
}
