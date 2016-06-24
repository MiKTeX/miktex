/*************************************************************************
** Directory.cpp                                                        **
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
#include "Directory.h"

using namespace std;

#if defined(MIKTEX)
#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>
#else
#ifdef __WIN32__
	#include <windows.h>
#else
	#include <errno.h>
	#include <sys/stat.h>
#endif
#endif

Directory::Directory () {
#if defined(MIKTEX)
#else
#if __WIN32__
	handle = INVALID_HANDLE_VALUE;
	firstread = true;
	memset(&fileData, 0, sizeof(WIN32_FIND_DATA));
#else
	_dir = 0;
	_dirent = 0;
#endif
#endif
}


Directory::Directory (string dirname) {
#if defined(MIKTEX)
#else
#if __WIN32__
	handle = INVALID_HANDLE_VALUE;
	firstread = true;
	memset(&fileData, 0, sizeof(WIN32_FIND_DATA));
#else
	_dir = 0;
	_dirent = 0;
#endif
#endif
	open(dirname);
}


Directory::~Directory () {
	close();
}


bool Directory::open (string dname) {
	_dirname = dname;
#if defined(MIKTEX)
        if (!MiKTeX::Core::Directory::Exists(dname))
        {
          return false;
        }
        directoryLister = MiKTeX::Core::DirectoryLister::Open(dname);
        return true;
#else
#ifdef __WIN32__
	firstread = true;
	if (dname[dname.length()-1] == '/' || dname[dname.length()-1] == '\\')
		dname = dname.substr(0, dname.length()-1);
	dname += "\\*";
	handle = FindFirstFile(dname.c_str(), &fileData);
	return handle != INVALID_HANDLE_VALUE;
#else
	_dir = opendir(_dirname.c_str());
	return bool(_dir);
#endif
#endif
}


void Directory::close () {
#if defined(MIKTEX)
  directoryLister->Close();
#else
#ifdef __WIN32__
	FindClose(handle);
#else
	closedir(_dir);
#endif
#endif
}


/** Reads first/next directory entry.
 *  @param[in] type type of entry to return (a: file or dir, f: file, d: dir)
 *  @return name of entry */
const char* Directory::read (EntryType type) {
#if defined(MIKTEX)
  if (directoryLister == nullptr)
  {
    return nullptr;
  }
  while (directoryLister->GetNext(currentEntry))
  {
    if (currentEntry.isDirectory)
    {
      if (type == ET_FILE_OR_DIR || type == ET_DIR)
      {
        return currentEntry.name.c_str();
      }
    }
    else if (type == ET_FILE_OR_DIR || type == ET_FILE)
    {
      return currentEntry.name.c_str();
    }
  }
  directoryLister->Close();
  directoryLister = nullptr;
  return nullptr;
#else
#ifdef __WIN32__
	if (handle == INVALID_HANDLE_VALUE)
		return 0;
	while (firstread || FindNextFile(handle, &fileData)) {
		firstread = false;
		if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (type == ET_FILE_OR_DIR || type == ET_DIR)
				return fileData.cFileName;
		}
		else if (type == ET_FILE_OR_DIR || type == ET_FILE)
			return fileData.cFileName;
	}
	FindClose(handle);
	handle = INVALID_HANDLE_VALUE;
	return 0;
#else
	if (!_dir)
		return 0;
	while ((_dirent = readdir(_dir))) {
		string path = string(_dirname) + "/" + _dirent->d_name;
		struct stat stats;
		if (stat(path.c_str(), &stats) == 0) {
			if (S_ISDIR(stats.st_mode)) {
				if (type == ET_FILE_OR_DIR || type == ET_DIR)
					return _dirent->d_name;
			}
			else if (type == ET_FILE_OR_DIR || type == ET_FILE)
				return _dirent->d_name;
		}
	}
	closedir(_dir);
	_dir = 0;
	return 0;
#endif
#endif
}


