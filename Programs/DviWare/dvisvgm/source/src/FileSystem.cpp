/*************************************************************************
** FileSystem.cpp                                                       **
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

#include <config.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include "FileSystem.hpp"
#include "utility.hpp"
#include "version.hpp"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined(MIKTEX)
#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>
#include <miktex/Core/File>
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif
#endif

using namespace std;

#ifdef _WIN32
	#include <direct.h>
	#include "windows.hpp"
	const char *FileSystem::DEVNULL = "nul";
	const char FileSystem::PATHSEP = '\\';
#if !defined(MIKTEX_WINDOWS)
	#define unlink _unlink
#endif
#else
	#include <dirent.h>
	#include <pwd.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	const char *FileSystem::DEVNULL = "/dev/null";
	const char FileSystem::PATHSEP = '/';
#endif


FileSystem FileSystem::_fs;
string FileSystem::TMPDIR;
const char *FileSystem::TMPSUBDIR = nullptr;


/** Private wrapper function for mkdir: creates a single folder.
 *  @param[in] dir folder name
 *  @return true on success */
static bool s_mkdir (const string &dirname) {
	bool success = true;
	if (!FileSystem::exists(dirname)) {
#ifdef _WIN32
#if defined(MIKTEX_UTF8_WRAP__MKDIR)
                success = miktex_utf8__mkdir(dirname.c_str()) == 0;
#else
		success = (_mkdir(dirname.c_str()) == 0);
#endif
#else
		success = (::mkdir(dirname.c_str(), 0775) == 0);
#endif
	}
	return success;
}


static bool inline s_rmdir (const string &dirname) {
#ifdef _WIN32
#if defined(MIKTEX_UTF8_WRAP__RMDIR)
        return miktex_utf8__rmdir(dirname.c_str());
#else
	return (_rmdir(dirname.c_str()) == 0);
#endif
#else
	return (::rmdir(dirname.c_str()) == 0);
#endif
}


FileSystem::~FileSystem () {
	// remove the subdirectory from the system's temp folder (if empty)
	if (TMPSUBDIR)
		s_rmdir(tmpdir());
}


bool FileSystem::remove (const string &fname) {
	return unlink(fname.c_str()) == 0;
}


/** Copies a file.
 *  @param[in] src path of file to copy
 *  @param[in] dest path of target file
 *  @param[in] remove_src remove file 'src' if true
 *  @return true on success */
bool FileSystem::copy (const string &src, const string &dest, bool remove_src) {
#if defined(MIKTEX_WINDOWS)
        ifstream ifs(UW_(src.c_str()), ios::in | ios::binary);
        ofstream ofs(UW_(dest.c_str()), ios::out | ios::binary);
#else
	ifstream ifs(src.c_str(), ios::in|ios::binary);
	ofstream ofs(dest.c_str(), ios::out|ios::binary);
#endif
	if (ifs && ofs) {
		ofs << ifs.rdbuf();
		if (!ifs.fail() && !ofs.fail() && remove_src) {
			ofs.close();
			ifs.close();
			return remove(src);
		}
		else
			return !remove_src;
	}
	return false;
}


bool FileSystem::rename (const string &oldname, const string &newname) {
#if defined(MIKTEX_UTF8_WRAP_RENAME)
        return miktex_utf8_rename(oldname.c_str(), newname.c_str()) == 0;
#else
	return ::rename(oldname.c_str(), newname.c_str()) == 0;
#endif
}


uint64_t FileSystem::filesize (const string &fname) {
#ifdef _WIN32
	// unfortunately, stat doesn't work properly under Windows
	// so we have to use this freaky code
	WIN32_FILE_ATTRIBUTE_DATA attr;
#if defined(MIKTEX)
        GetFileAttributesExW(UW_(fname.c_str()), GetFileExInfoStandard, &attr);
#else
	GetFileAttributesExA(fname.c_str(), GetFileExInfoStandard, &attr);
#endif
	return (static_cast<uint64_t>(attr.nFileSizeHigh) << (8*sizeof(attr.nFileSizeLow))) | attr.nFileSizeLow;
#else
	struct stat attr;
	return (stat(fname.c_str(), &attr) == 0) ? attr.st_size : 0;
#endif
}


string FileSystem::adaptPathSeperators (string path) {
	std::replace(path.begin(), path.end(), PATHSEP, '/');
	return path;
}


string FileSystem::getcwd () {
	char buf[1024];
#ifdef _WIN32
#if defined(MIKTEX_UTF8_WRAP__GETCWD)
        return adaptPathSeperators(miktex_utf8__getcwd(buf, 1024));
#else
	return adaptPathSeperators(_getcwd(buf, 1024));
#endif
#else
	return ::getcwd(buf, 1024);
#endif
}


/** Changes the work directory.
 *  @param[in] dir path to new work directory
 *  @return true on success */
bool FileSystem::chdir (const std::string &dirname) {
	bool success = false;
	if (const char *cdirname = dirname.c_str()) {
#ifdef _WIN32
#if defined(MIKTEX_UTF8_WRAP__CHDIR)
                success = miktex_utf8__chdir(cdirname) == 0;
#else
		success = (_chdir(cdirname) == 0);
#endif
#else
		success = (::chdir(cdirname) == 0);
#endif
	}
	return success;
}


/** Returns the user's home directory. */
const char* FileSystem::userdir () {
#ifdef _WIN32
	const char *drive=getenv("HOMEDRIVE");
	const char *path=getenv("HOMEPATH");
	if (drive && path) {
		static string ret = string(drive)+path;
		if (!ret.empty())
			return ret.c_str();
	}
	return 0;
#else
	const char *dir=getenv("HOME");
	if (!dir) {
		if (const char *user=getenv("USER")) {
			if (struct passwd *pw=getpwnam(user))
				dir = pw->pw_dir;
		}
	}
	return dir;
#endif
}


/** Returns the path of the temporary folder. */
string FileSystem::tmpdir () {
	string ret;
	if (!TMPDIR.empty())
		ret = TMPDIR;
	else {
#if defined(MIKTEX)
          MiKTeX::Core::PathName tmpdir;
          tmpdir.SetToTempDirectory();
          ret = tmpdir.ToString();
#else
#ifdef _WIN32
		char buf[MAX_PATH];
		if (GetTempPath(MAX_PATH, buf))
			ret = adaptPathSeperators(buf);
		else
			ret = ".";
#else
		if (const char *path = getenv("TMPDIR"))
			ret = path;
		else
			ret = "/tmp";
#endif
		if (ret.back() == '/')
			ret.pop_back();
		static bool initialized=false;
		if (!initialized && ret != ".") {
			TMPSUBDIR = PROGRAM_NAME;
			s_mkdir(ret + "/" + TMPSUBDIR);
			initialized = true;
		}
		if (TMPSUBDIR)
			ret += string("/") + TMPSUBDIR;
#endif
	}
	if (!ret.empty() && ret.back() != '/')
		ret += '/';
	return ret;
}


/** Creates a new folder relative to the current work directory. If necessary,
 *  the parent folders are also created.
 *  @param[in] dir single folder name or path to folder
 *  @return true if folder(s) could be created */
bool FileSystem::mkdir (const string &dirname) {
	bool success = false;
	if (const char *cdirname = dirname.c_str()) {
		success = true;
		const string dirstr = adaptPathSeperators(util::trim(cdirname));
		for (size_t pos=1; success && (pos = dirstr.find('/', pos)) != string::npos; pos++)
			success &= s_mkdir(dirstr.substr(0, pos));
		success &= s_mkdir(dirstr);
	}
	return success;
}


/** Removes a directory and its contents.
 *  @param[in] dirname path to directory
 *  @return true on success */
bool FileSystem::rmdir (const string &dirname) {
	bool ok = false;
	if (isDirectory(dirname)) {
		ok = true;
#if defined(MIKTEX)
                MiKTeX::Core::Directory::Delete(dirname, true);
#else
#ifdef _WIN32
		string pattern = dirname + "/*";
		WIN32_FIND_DATA data;
		HANDLE h = FindFirstFile(pattern.c_str(), &data);
		bool ready = (h == INVALID_HANDLE_VALUE);
		while (!ready && ok) {
			const char *fname = data.cFileName;
			string path = dirname + "/" + fname;
			if (isDirectory(path)) {
				if (strcmp(fname, ".") != 0 && strcmp(fname, "..") != 0)
					ok = rmdir(path) && s_rmdir(path);
			}
			else if (isFile(path))
				ok = remove(path);
			else
				ok = false;
			ready = !FindNextFile(h, &data);
		}
		FindClose(h);
#else
		if (DIR *dir = opendir(dirname.c_str())) {
			struct dirent *ent;
			while ((ent = readdir(dir)) && ok) {
				const char *fname = ent->d_name;
				string path = dirname + "/" + fname;
				if (isDirectory(path)) {
					if (strcmp(fname, ".") != 0 && strcmp(fname, "..") != 0)
						ok = rmdir(path) && s_rmdir(path);
				}
				else if (isFile(path))
					ok = remove(path);
				else
					ok = false;
			}
			closedir(dir);
		}
#endif
		ok = s_rmdir(dirname);
#endif
	}
	return ok;
}


/** Checks if a file or directory exits. */
bool FileSystem::exists (const string &fname) {
	if (const char *cfname = fname.c_str()) {

#if defined(MIKTEX)
                return MiKTeX::Core::File::Exists(fname) || MiKTeX::Core::Directory::Exists(fname);
#else
#ifdef _WIN32
		return GetFileAttributes(cfname) != INVALID_FILE_ATTRIBUTES;
#else
		struct stat attr;
		return stat(cfname, &attr) == 0;
#endif
#endif
	}
	return false;
}


/** Returns true if 'fname' references a directory. */
bool FileSystem::isDirectory (const string &fname) {
	if (const char *cfname = fname.c_str()) {
#if defined(MIKTEX)
                return MiKTeX::Core::Directory::Exists(fname);
#else
#ifdef _WIN32
		auto attr = GetFileAttributes(cfname);
		return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
#else
		struct stat attr;
		return stat(cfname, &attr) == 0 && S_ISDIR(attr.st_mode);
#endif
#endif
	}
	return false;
}


/** Returns true if 'fname' references a file. */
bool FileSystem::isFile (const string &fname) {
	if (const char *cfname = fname.c_str()) {
#if defined(MIKTEX)
                return MiKTeX::Core::File::Exists(fname);
#else
#ifdef _WIN32
		ifstream ifs(cfname);
		return (bool)ifs;
#else
		struct stat attr;
		return stat(cfname, &attr) == 0 && S_ISREG(attr.st_mode);
#endif
#endif
	}
	return false;
}


int FileSystem::collect (const std::string &dirname, vector<string> &entries) {
	entries.clear();
#if defined(MIKTEX)
        unique_ptr<MiKTeX::Core::DirectoryLister> lister = MiKTeX::Core::DirectoryLister::Open(dirname);
        MiKTeX::Core::DirectoryEntry entry;
        while (lister->GetNext(entry))
        {
          MiKTeX::Core::PathName path(dirname);
          path /= entry.name;
          string typechar = isFile(path.GetData()) ? "f" : isDirectory(path.GetData()) ? "d" : "?";
          if (entry.name != "." && entry.name != "..")
          {
            entries.push_back(typechar + entry.name);
          }
        }
        lister->Close();
#else
#ifdef _WIN32
	string pattern = dirname + "/*";
	WIN32_FIND_DATA data;
	HANDLE h = FindFirstFile(pattern.c_str(), &data);
	bool ready = (h == INVALID_HANDLE_VALUE);
	while (!ready) {
		string fname = data.cFileName;
		string path = dirname+"/"+fname;
		string typechar = isFile(path) ? "f" : isDirectory(path) ? "d" : "?";
		if (fname != "." && fname != "..")
			entries.emplace_back(typechar+fname);
		ready = !FindNextFile(h, &data);
	}
	FindClose(h);
#else
	if (DIR *dir = opendir(dirname.c_str())) {
		struct dirent *ent;
		while ((ent = readdir(dir))) {
			string fname = ent->d_name;
			string path = dirname+"/"+fname;
			string typechar = isFile(path) ? "f" : isDirectory(path) ? "d" : "?";
			if (fname != "." && fname != "..")
				entries.emplace_back(typechar+fname);
		}
		closedir(dir);
	}
#endif
#endif
	return entries.size();
}
