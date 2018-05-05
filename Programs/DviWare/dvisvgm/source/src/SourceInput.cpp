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
#include <miktex/Core/TemporaryFile>
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

#ifdef _WIN32
#include <fcntl.h>
#include <sys/stat.h>
#include <windows.h>
#endif

#ifdef _MSC_VER
#include <io.h>
#else
#include <cstdlib>
#include <unistd.h>
#endif

using namespace std;

#ifdef _WIN32
static int fdwrite (int fd, const char *buf, size_t len) {return _write(fd, buf, len);}
static int fdclose (int fd) {return _close(fd);}
#else
static int fdwrite (int fd, const char *buf, size_t len) {return write(fd, buf, len);}
static int fdclose (int fd) {return close(fd);}
#endif


SourceInput::~SourceInput () {
	// remove temporary file created for reading from stdin
	if (!_tmpfilepath.empty())
		FileSystem::remove(_tmpfilepath);
}


/** Creates a temporary file in the configured tmp folder.
 *  @param[out] path path of the created file
 *  @return file descriptor (>= 0 on success) */
static int create_tmp_file (string &path) {
#if !defined(MIKTEX_WINDOWS)
	path = FileSystem::tmpdir();
#endif
#ifndef _WIN32
	path += "stdinXXXXXX";
	int fd = mkstemp(&path[0]);
#else
#if defined(MIKTEX_WINDOWS)
        auto tmpfile = MiKTeX::Core::TemporaryFile::Create();
        int fd = _wopen(tmpfile->GetPathName().ToWideCharString().c_str(), _O_CREAT | _O_WRONLY | _O_BINARY, _S_IWRITE);
        if (fd >= 0)
        {
          tmpfile->Keep();
        }
#else
        int fd = -1;
	char fname[MAX_PATH];
	std::replace(path.begin(), path.end(), '/', '\\');
	if (GetTempFileName(path.c_str(), "stdin", 0, fname)) {
		fd = _open(fname, _O_CREAT | _O_WRONLY | _O_BINARY, _S_IWRITE);
		path = fname;
	}
#endif
#endif
	return fd;
}


istream& SourceInput::getInputStream (bool showMessages) {
	if (!_ifs.is_open()) {
		if (!_fname.empty())
			_ifs.open(_fname, ios::binary);
		else {
			int fd = create_tmp_file(_tmpfilepath);
			if (fd < 0)
				throw MessageException("can't create temporary file for writing");
#ifdef _WIN32
			if (_setmode(_fileno(stdin), _O_BINARY) == -1)
				throw MessageException("can't open stdin in binary mode");
#endif
			if (showMessages)
				Message::mstream() << "reading from " << getMessageFileName() << '\n';
			char buf[1024];
			while (cin) {
				cin.read(buf, 1024);
				size_t count = cin.gcount();
				if (fdwrite(fd, buf, count) < 0)
					throw MessageException("failed to write data to temporary file");
			}
			if (fdclose(fd) < 0)
				throw MessageException("failed to close temporary file");
			_ifs.open(_tmpfilepath, ios::binary);
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
	return _tmpfilepath.empty() ? _fname : _tmpfilepath;
}
