/* This file is part of ff-woff, a reduced version of the FontForge library.
* It provides the function tmpfile2() with the functionality of tmpfile() but
* works on POSIX and Windows systems.
* License: Revised BSD license used by FontForge
* https://github.com/fontforge/fontforge/blob/master/LICENSE
* (C) 2017-2022 Martin Gieseking <martin.gieseking@uos.de> */

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

#if _WIN32
#include <fcntl.h>
#include <sys/stat.h>
#include <windows.h>
#ifdef _MSC_VER
#include <io.h>
#endif
#endif
#if defined(MIKTEX)
#include <miktex/Core/File>
#include <miktex/Core/TemporaryFile>
#endif

using namespace std;

class TmpFileException : public exception {
	public:
		TmpFileException (const char *msg) : _message(msg) {}
		const char* what () const noexcept override {return _message.c_str();}

	private:
		string _message;
};


static inline void tmpfile_error (const char *msg) {
#if defined(_MSC_VER) || defined(__MINGW32__)
	// Visual C++ and MinGW support exception handling
	// between extern "C" and C++ functions.
	throw TmpFileException(msg);
#else
	cerr << msg << endl;
	exit(EXIT_FAILURE);
#endif
}


/* Creates a temporary binary file in the system's temp folder and returns
* its file pointer. The file is automatically removed when closing it.
* If the temp file can't be created for some reason, the function calls
* exit() or throws a TmpFileException (depending on OS and compiler). */
extern "C" FILE* tmpfile2 () {
#if defined(MIKTEX)
  std::unique_ptr<MiKTeX::Core::TemporaryFile> tmpfile = MiKTeX::Core::TemporaryFile::Create();
  FILE* result = MiKTeX::Core::File::Open(tmpfile->GetPathName(), MiKTeX::Core::FileMode::Create, MiKTeX::Core::FileAccess::ReadWrite, false, { MiKTeX::Core::FileOpenOption::DeleteOnClose });
  tmpfile->Keep();
  return result;
#else
#ifndef _WIN32
	if (FILE *fp = std::tmpfile())
		return fp;
#else
	char tmpdir[MAX_PATH+1];
	DWORD len = GetTempPath(MAX_PATH+1, tmpdir);
	if (len > 0) {
		if (len >= MAX_PATH-14)
			tmpfile_error("path to temp folder too long");
		char fname[MAX_PATH];
		if (GetTempFileName(tmpdir, "tmp", 0, fname)) {
			int fd = _open(fname, _O_CREAT | _O_TEMPORARY | _O_RDWR | _O_BINARY, _S_IREAD | _S_IWRITE);
			if (fd >= 0) {
				if (FILE *fp = _fdopen(fd, "w+b"))
					return fp;
				int saved_errno = errno;
				_close(fd);
				errno = saved_errno;
			}
		}
	}
#endif
	tmpfile_error("failed to create temporary file");
#endif
}
