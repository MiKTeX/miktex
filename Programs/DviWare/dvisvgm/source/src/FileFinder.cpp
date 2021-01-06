/*************************************************************************
** FileFinder.cpp                                                       **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2020 Martin Gieseking <martin.gieseking@uos.de>   **
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
#ifdef MIKTEX_COM
	#include "MiKTeXCom.hpp"
	#include "utility.hpp"
#else
	#ifdef KPSE_CXX_UNSAFE
	extern "C" {
	#endif
		#include <kpathsea/kpathsea.h>
	#ifdef KPSE_CXX_UNSAFE
	}
	#endif
#endif
#if defined(MIKTEX)
#include <miktex/Core/Utils>
#include <miktex/Core/c/api.h>
#endif

#include <cstdlib>
#include <fstream>
#include <map>
#include <set>
#include "FileFinder.hpp"
#include "FilePath.hpp"
#include "FileSystem.hpp"
#include "FontMap.hpp"
#include "Message.hpp"
#include "MessageException.hpp"
#if !defined(MIKTEX)
#include "Process.hpp"
#endif

std::string FileFinder::_argv0;
std::string FileFinder::_progname;
std::string FileFinder::_pathbuf;
bool FileFinder::_enableMktex = false;


/** Constructs a new file finder.
 *  @param[in] argv0 argv[0] of main() function
 *  @param[in] progname name of application using the FileFinder
 *  @param[in] enable_mktexmf if true, tfm and mf file generation is activated */
FileFinder::FileFinder () {
	addLookupDir(".");  // always lookup files in the current working directory
#ifdef MIKTEX_COM
	_miktex = util::make_unique<MiKTeXCom>();
#else
	kpse_set_program_name(_argv0.c_str(), _progname.c_str());
	// enable tfm and mf generation (actually invoked by calls of kpse_make_tex)
	kpse_set_program_enabled(kpse_tfm_format, 1, kpse_src_env);
	kpse_set_program_enabled(kpse_mf_format, 1, kpse_src_env);
	kpse_make_tex_discard_errors = true;  // suppress messages from mktexFOO tools
#endif
}


void FileFinder::init (const std::string &argv0, const std::string &progname, bool enable_mktexmf) {
	_argv0 = argv0;
	_progname = progname;
	_enableMktex = enable_mktexmf;
}


FileFinder& FileFinder::instance () {
	static FileFinder instance;
	return instance;
}


/** Returns the version string of the underlying file searching library (kpathsea, MiKTeX) */
std::string FileFinder::version () const {
#ifdef MIKTEX_COM
	return _miktex->getVersion();
#else
#if defined(MIKTEX)
	return MiKTeX::Core::Utils::GetMiKTeXBannerString();
#else
	if (const char *v = strrchr(KPSEVERSION, ' '))
		return (std::string(KPSEVERSION).substr(0, 9) == "kpathsea ") ? v+1 : KPSEVERSION;
	if (strlen(KPSEVERSION) > 0)
		return KPSEVERSION;
#endif
#endif
	return "unknown";
}


void FileFinder::addLookupDir (const std::string &path) {
	FilePath filepath(path);
	_additionalDirs.insert(filepath.absolute());
}


/** Determines filetype by the filename extension and calls kpse_find_file
 *  to actually look up the file.
 *  @param[in] fname name of file to look up
 *  @param[in] ftype expected file format of file fname; if 0, it's derived from the filename suffix
 *  @return file path on success, 0 otherwise */
const char* FileFinder::findFile (const std::string &fname, const char *ftype) const {
	if (fname.empty())
		return nullptr;

	// try to lookup the file in the additionally specified directories
	for (const std::string &dir : _additionalDirs) {
		_pathbuf = dir+"/"+fname;
		if (FileSystem::exists(_pathbuf))
			return _pathbuf.c_str();
	}
	std::string ext;
	if (ftype)
		ext = ftype;
	else {
		size_t pos = fname.rfind('.');
		if (pos == std::string::npos)
			return nullptr;  // no extension and no file type => no search
		ext = fname.substr(pos+1);
	}

#ifdef _WIN32
	if (ext == "dll" || ext == "exe")
		return lookupExecutable(fname);
#ifdef MIKTEX_COM
	else if (ext == "cmap") {
		// The MiKTeX SDK doesn't support the lookup of files without suffix (yet), thus
		// it's not possible to find cmap files which usually don't have a suffix. In order
		// to work around this, we try to lookup the files by calling kpsewhich.
		Process process("kpsewhich", "-format=cmap "+fname);
		process.run(&_pathbuf);
		_pathbuf = util::trim(_pathbuf);
		return _pathbuf.empty() ? nullptr : _pathbuf.c_str();
	}
	try {
		return _miktex->findFile(fname.c_str());
	}
	catch (const MessageException &e) {
		return nullptr;
	}
#endif  // MIKTEX
#endif  // _WIN32
#ifndef MIKTEX_COM
	static std::map<std::string, kpse_file_format_type> types = {
		{"tfm",  kpse_tfm_format},
		{"pfb",  kpse_type1_format},
		{"vf",   kpse_vf_format},
		{"mf",   kpse_mf_format},
		{"ttc",  kpse_truetype_format},
		{"ttf",  kpse_truetype_format},
		{"otf",  kpse_opentype_format},
		{"map",  kpse_fontmap_format},
		{"cmap", kpse_cmap_format},
		{"sty",  kpse_tex_format},
		{"enc",  kpse_enc_format},
		{"pro",  kpse_tex_ps_header_format},
		{"sfd",  kpse_sfd_format},
		{"eps",  kpse_pict_format},
		{"png",  kpse_pict_format},
		{"jpg",  kpse_pict_format},
		{"jpeg", kpse_pict_format},
		{"svg",  kpse_pict_format},
		{"pdf",  kpse_tex_format},
	};
	auto it = types.find(ext);
	if (it == types.end())
		return nullptr;

	if (char *path = kpse_find_file(fname.c_str(), it->second, 0)) {
		// In the current version of libkpathsea, each call of kpse_find_file produces
		// a memory leak since the path buffer is not freed. I don't think we can do
		// anything against it here...
		_pathbuf = path;
		std::free(path);
		return _pathbuf.c_str();
	}
	return nullptr;
#endif  // !MIKTEX
}


/** Checks whether the given file is mapped to a different name and if the
 *  file can be found under this name.
 *  @param[in] fname name of file to look up
 *  @return file path on success, 0 otherwise */
const char* FileFinder::findMappedFile (std::string fname) const {
	size_t pos = fname.rfind('.');
	if (pos == std::string::npos)
		return nullptr;
	const std::string ext  = fname.substr(pos+1);  // file extension
	const std::string base = fname.substr(0, pos);
	if (const FontMap::Entry *entry = FontMap::instance().lookup(base)) {
		const char *path=nullptr;
		if (entry->fontname.find('.') != std::string::npos)  // does the mapped filename has an extension?
			path = findFile(entry->fontname, nullptr);        // look for that file
		else {                             // otherwise, use extension of unmapped file
			fname = entry->fontname + "." + ext;
			(path = findFile(fname, nullptr)) || (path = mktex(fname));
		}
		return path;
	}
	return nullptr;
}


/** Runs external mktexFOO tool to create missing tfm or mf file.
 *  @param[in] fname name of file to build
 *  @return file path on success, 0 otherwise */
const char* FileFinder::mktex (const std::string &fname) const {
	size_t pos = fname.rfind('.');
	if (!_enableMktex || pos == std::string::npos)
		return nullptr;

	std::string ext  = fname.substr(pos+1);  // file extension
	if (ext != "tfm" && ext != "mf")
		return nullptr;

	const char *path = nullptr;
#ifdef MIKTEX
	// maketfm and makemf are located in miktex/bin which is in the search PATH
	std::string toolname = (ext == "tfm" ? "miktex-maketfm" : "miktex-makemf");
#if defined(MIKTEX)
#if defined(MIKTEX_WINDOWS)
	toolname += ".exe";
#endif
	miktex_system((toolname + fname).c_str());
#else
	system((toolname+".exe "+fname).c_str());
#endif
	path = findFile(fname, nullptr);
#else
	kpse_file_format_type type = (ext == "tfm" ? kpse_tfm_format : kpse_mf_format);
	path = kpse_make_tex(type, fname.c_str());
#endif
	return path;
}


/** Searches a file in the TeX directory tree.
 *  If the file doesn't exist, maximal two further steps are applied
 *  (if "extended" is true):
 *  - checks whether the filename is mapped to a different name and returns
 *    the path to that name
 *  - in case of tfm or mf files: invokes the external mktextfm/mktexmf tool
 *    to create the missing file
 *  @param[in] fname name of file to look up
 *  @param[in] ftype type/format of file to look up
 *  @param[in] extended if true, use fontmap lookup and mktexFOO calls
 *  @return path to file on success, 0 otherwise */
const char* FileFinder::lookup (const std::string &fname, const char *ftype, bool extended) const {
	const char *path;
	if ((path = findFile(fname, ftype)) || (extended  && ((path = findMappedFile(fname)) || (path = mktex(fname)))))
		return path;
	return nullptr;
}


/** Looks up the location of an executable file.
 *  @param[in] fname name of file to look up
 *  @param[in] addSuffix if true, ".exe" is appended to the given filename (Windows only)
 *  @return absolute path of file or nullptr if not found */
const char* FileFinder::lookupExecutable (const std::string &fname, bool addSuffix) const {
#ifdef MIKTEX_COM
	_pathbuf = _miktex->getBinDir() + "/" + fname;
	if (addSuffix)
		_pathbuf += ".exe";
	if (FileSystem::exists(_pathbuf))
		return _pathbuf.c_str();
	try {
		return _miktex->findFile(fname.c_str());
	}
	catch (...) {
	}
#else
	// lookup executables in directory where dvisvgm is located
	if (const char *path = kpse_var_value("SELFAUTOLOC")) {
		_pathbuf = std::string(path) + "/" + fname;
#if defined(MIKTEX)
                free((void*)path); path = nullptr;
#endif
#ifdef _WIN32
		if (addSuffix)
			_pathbuf += ".exe";
#endif
		if (FileSystem::exists(_pathbuf))
			return _pathbuf.c_str();
	}
#endif  // !MIKTEX
	return nullptr;
}
