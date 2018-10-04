/*************************************************************************
** SourceInput.cpp                                                      **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2018 Martin Gieseking <martin.gieseking@uos.de>   **
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

#include <fstream>
#include <iostream>
#include "FileSystem.hpp"
#include "Message.hpp"
#include "MessageException.hpp"
#include "SourceInput.hpp"
#include "utility.hpp"
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#define WU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#endif

#ifdef _WIN32
#	include <fcntl.h>
#	include <sys/stat.h>
#	include <windows.h>
#else
#	include <config.h>
#	ifdef HAVE_UMASK
#		include <sys/stat.h>
#	endif
#endif

#ifdef _MSC_VER
#	include <io.h>
#else
#	include <cstdlib>
#	include <unistd.h>
#endif

using namespace std;

#ifdef _WIN32
static int fdwrite (int fd, const char *buf, size_t len) {return _write(fd, buf, len);}
static int fdclose (int fd) {return _close(fd);}
#else
static int fdwrite (int fd, const char *buf, size_t len) {return write(fd, buf, len);}
static int fdclose (int fd) {return close(fd);}
#endif


/** Creates a new temporary file in the configured tmp folder.
 *  If the object already holds an opened temporary file, it's closed
 *  and removed before creating the new one.
 *  @return true on success */
bool TemporaryFile::create () {
	if (opened())
		close();
	_path = FileSystem::tmpdir();
#ifndef _WIN32
	_path += "stdinXXXXXX";
#ifdef HAVE_UMASK
	mode_t mode_mask = umask(S_IXUSR | S_IRWXG | S_IRWXO);  // set file permissions to 0600
#endif
	_fd = mkstemp(&_path[0]);
#ifdef HAVE_UMASK
	umask(mode_mask);
#endif
#else  // !_WIN32
#if defined(MIKTEX_WINDOWS)
        wchar_t fname[MAX_PATH];
#else
	char fname[MAX_PATH];
#endif
	std::replace(_path.begin(), _path.end(), '/', '\\');
#if defined(MIKTEX_WINDOWS)
        if (GetTempFileNameW(UW_(_path), L"stdin", 0, fname))
        {
          _fd = _wopen(fname, _O_CREAT | _O_WRONLY | _O_BINARY, _S_IWRITE);
          _path = WU_(fname);
        }
#else
	if (GetTempFileName(_path.c_str(), "stdin", 0, fname)) {
		_fd = _open(fname, _O_CREAT | _O_WRONLY | _O_BINARY, _S_IWRITE);
		_path = fname;
	}
#endif
#endif
	return opened();
}


/** Writes a sequence of characters to the file.
 *  @param[in] buf buffer containing the characters to write
 *  @param[in] len number of characters to write
 *  @return true on success */
bool TemporaryFile::write (const char *buf, size_t len) {
	return opened() && fdwrite(_fd, buf, len) >= 0;
}


/** Closes and removes the temporary file.
 *  @return true on success */
bool TemporaryFile::close () {
	bool ok = true;
	if (opened()) {
		ok = (fdclose(_fd) >= 0);
		FileSystem::remove(_path);
		_fd = -1;
		_path.clear();
	}
	return ok;
}

////////////////////////////////////////////////////////////////

istream& SourceInput::getInputStream (bool showMessages) {
	if (!_ifs.is_open()) {
		if (!_fname.empty())
			_ifs.open(_fname, ios::binary);
		else {
#ifdef _WIN32
			if (_setmode(_fileno(stdin), _O_BINARY) == -1)
				throw MessageException("can't open stdin in binary mode");
#endif
			if (!_tmpfile.create())
				throw MessageException("can't create temporary file for writing");
			if (showMessages)
				Message::mstream() << "reading from " << getMessageFileName() << '\n';
			char buf[1024];
			while (cin) {
				cin.read(buf, 1024);
				size_t count = cin.gcount();
				if (!_tmpfile.write(buf, count))
					throw MessageException("failed to write data to temporary file");
			}
			_ifs.open(_tmpfile.path(), ios::binary);
		}
	}
	return _ifs;
}


string SourceInput::getFileName () const {
	return _fname.empty() ? "stdin" : _fname;
}


string SourceInput::getMessageFileName () const {
	return _fname.empty() ? "<stdin>" : _fname;
}


string SourceInput::getFilePath () const {
	return _tmpfile.path().empty() ? _fname : _tmpfile.path();
}
