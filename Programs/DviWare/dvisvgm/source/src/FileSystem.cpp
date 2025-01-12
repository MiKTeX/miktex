/*************************************************************************
** FileSystem.cpp                                                       **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2025 Martin Gieseking <martin.gieseking@uos.de>   **
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
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include "FileSystem.hpp"
#include "utility.hpp"
#include "version.hpp"
#include "XXHashFunction.hpp"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined(MIKTEX)
#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>
#include <miktex/Core/File>
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/PathNameUtil>
#define EXPATH_(x) MiKTeX::Util::PathNameUtil::ToLengthExtendedPathName(x)
#endif
#endif

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


using namespace std;

string FileSystem::TMPDIR;
FileSystem::TemporaryDirectory FileSystem::_tmpdir;


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
	ifstream ifs(EXPATH_(src), ios::in | ios::binary);
	ofstream ofs(EXPATH_(dest), ios::out | ios::binary);
#else
	ifstream ifs(src, ios::in|ios::binary);
	ofstream ofs(dest, ios::out|ios::binary);
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
        GetFileAttributesExW(EXPATH_(fname).c_str(), GetFileExInfoStandard, &attr);
#else
	GetFileAttributesExA(fname.c_str(), GetFileExInfoStandard, &attr);
#endif
	return (static_cast<uint64_t>(attr.nFileSizeHigh) << (8*sizeof(attr.nFileSizeLow))) | attr.nFileSizeLow;
#else
	struct stat attr;
	return (stat(fname.c_str(), &attr) == 0) ? attr.st_size : 0;
#endif
}


string FileSystem::ensureForwardSlashes (string path) {
#ifdef _WIN32
	std::replace(path.begin(), path.end(), PATHSEP, '/');
#endif
	return path;
}


string FileSystem::ensureSystemSlashes (string path) {
#ifdef _WIN32
	std::replace(path.begin(), path.end(), '/', PATHSEP);
#endif
	return path;
}


/** Returns the absolute path of the current working directory. */
string FileSystem::getcwd () {
	char buf[1024];
#ifdef _WIN32
#if defined(MIKTEX_UTF8_WRAP__GETCWD)
        return ensureForwardSlashes(miktex_utf8__getcwd(buf, 1024));
#else
	GetCurrentDirectoryA(1024, buf);
	return ensureForwardSlashes(buf);
#endif
#else
	return ::getcwd(buf, 1024);
#endif
}


#ifdef _WIN32
/** Returns the absolute path of the current directory of a given drive.
 *  Windows keeps a current directory for every drive, i.e. when accessing a drive
 *  without specifying a path (e.g. with "cd z:"), the current directory of that
 *  drive is used.
 *  @param[in] drive letter of drive to get the current directory from
 *  @return absolute path of the directory */
string FileSystem::getcwd (char drive) {
	string cwd = getcwd();
	if (cwd.length() > 1 && cwd[1] == ':' && tolower(cwd[0]) != tolower(drive)) {
		chdir(string(1, drive)+":");
		string cwd2 = cwd;
		cwd = getcwd();
		chdir(string(1, cwd2[0])+":");
	}
	return cwd;
}
#endif


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
	return nullptr;
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


/** Returns the path of the temporary folder.
 *  @param[in] inpace if true, don't create a uniquely named subfolder */
string FileSystem::tmpdir (bool inplace) {
	if (_tmpdir.path().empty()) {
		string basedir;
		if (!TMPDIR.empty())
			basedir = ensureForwardSlashes(TMPDIR);
		else {
#ifdef _WIN32
#if defined(MIKTEX)
          MiKTeX::Util::PathName tmpdir;
          tmpdir.SetToTempDirectory();
	  basedir = tmpdir.ToString();
#else
			char buf[MAX_PATH];
			if (GetTempPath(MAX_PATH, buf))
				basedir = ensureForwardSlashes(buf);
			else
				basedir = ".";
#endif
#else
			if (const char *path = getenv("TMPDIR"))
				basedir = path;
			else
				basedir = "/tmp";
#endif
		}
		if (basedir.length() > 2 && string(basedir.end()-2, basedir.end()) == "//") {
			inplace = true;
			basedir.pop_back();
		}
		if (basedir.front() != '/' && basedir.back() == '/')
			basedir.pop_back();
		_tmpdir = TemporaryDirectory(basedir, PROGRAM_NAME, inplace);
	}
	return _tmpdir.path();
}


/** Creates a new folder relative to the current work directory. If necessary,
 *  the parent folders are also created.
 *  @param[in] dir single folder name or path to folder
 *  @return true if folder(s) could be created */
bool FileSystem::mkdir (const string &dirname) {
	bool success = false;
	if (const char *cdirname = dirname.c_str()) {
		success = true;
		const string dirstr = ensureForwardSlashes(util::trim(cdirname));
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
                MiKTeX::Core::Directory::Delete(MiKTeX::Util::PathName(dirname), true);
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
                return MiKTeX::Core::File::Exists(MiKTeX::Util::PathName(fname)) || MiKTeX::Core::Directory::Exists(MiKTeX::Util::PathName(fname));
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
                return MiKTeX::Core::Directory::Exists(MiKTeX::Util::PathName(fname));
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
		return MiKTeX::Core::File::Exists(MiKTeX::Util::PathName(fname));
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
        unique_ptr<MiKTeX::Core::DirectoryLister> lister = MiKTeX::Core::DirectoryLister::Open(MiKTeX::Util::PathName(dirname));
        MiKTeX::Core::DirectoryEntry entry;
        while (lister->GetNext(entry))
        {
          MiKTeX::Util::PathName path(dirname);
          path /= entry.name;
          string typechar = isFile(path.ToString()) ? "f" : isDirectory(path.ToString()) ? "d" : "?";
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


/** Creates a temporary directory in a given folder or treats the given folder as temporary directory.
 *  @param[in] folder folder path in which the directory is to be created
 *  @param[in] prefix initial string of the directory name
 *  @param[in] inplace if true, 'folder' is treated as temporary directory and no subfolder is created */
FileSystem::TemporaryDirectory::TemporaryDirectory (const std::string &folder, string prefix, bool inplace) {
	if (inplace) {
		_path = folder;
		if (!_path.empty() && _path.back() != '/')
			_path.push_back('/');
	}
	else {
		using namespace std::chrono;
		auto now = system_clock::now().time_since_epoch();
		auto now_ms = duration_cast<milliseconds>(now).count();
		auto hash = XXH64HashFunction(to_string(now_ms)).digestValue();
		if (!prefix.empty() && prefix.back() != '-')
			prefix.push_back('-');
		for (int i = 0; i < 10 && _path.empty(); i++) {
			hash++;
			stringstream oss;
			oss << folder << '/' << prefix << hex << hash;
			if (exists(oss.str()))
				continue;
			if (s_mkdir(oss.str()))
				_path = oss.str() + "/";
			else
				break;
		}
	}
}


FileSystem::TemporaryDirectory::~TemporaryDirectory () {
	if (!_path.empty())
		s_rmdir(_path);
}
