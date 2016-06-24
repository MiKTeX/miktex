/*************************************************************************
** FileFinder.cpp                                                       **
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

#ifdef MIKTEX_COM
	#include "MessageException.h"
	#include "MiKTeXCom.h"
	static MiKTeXCom *miktex=0;
#else
	#ifdef KPSE_CXX_UNSAFE
	extern "C" {
	#endif
	#include <kpathsea/kpathsea.h>
	#ifdef KPSE_CXX_UNSAFE
	}
	#endif
#endif

#include <cstdlib>
#include <fstream>
#include <map>
#include <set>
#include "FileFinder.h"
#include "FileSystem.h"
#include "FontMap.h"
#include "Message.h"
#include "Process.h"

// ---------------------------------------------------

static bool _initialized = false;
static bool _mktex_enabled = false;
static std::set<std::string> _additional_dirs;

// ---------------------------------------------------

static const char* find_file (const std::string &fname, const char *ftype);
static const char* find_mapped_file (std::string fname);
static const char* mktex (const std::string &fname);


/** Initializes the file finder. This function must be called before any other
 *  FileFinder function.
 *  @param[in] argv0 argv[0] of main() function
 *  @param[in] progname name of application using the FileFinder
 *  @param[in] enable_mktexmf if true, tfm and mf file generation is activated */
void FileFinder::init (const char *argv0, const char *progname, bool enable_mktexmf) {
	if (_initialized)
		return;

	_mktex_enabled = enable_mktexmf;
	addLookupDir(".");  // always lookup files in the current working directory
#ifdef MIKTEX_COM
	miktex = new MiKTeXCom;
#else
	kpse_set_program_name(argv0, progname);
	// enable tfm and mf generation (actually invoked by calls of kpse_make_tex)
	kpse_set_program_enabled(kpse_tfm_format, 1, kpse_src_env);
	kpse_set_program_enabled(kpse_mf_format, 1, kpse_src_env);
	kpse_make_tex_discard_errors = true;  // suppress messages from mktexFOO tools
#ifdef TEXLIVEWIN32
	texlive_gs_init();
#endif
#endif
	_initialized = true;
}


/** Cleans up the FileFinder. This function must be called before leaving the
 *  application's main() function. */
void FileFinder::finish () {
#ifdef MIKTEX_COM
	if (miktex) {
		delete miktex;
		miktex = 0;
	}
#endif
	_initialized = false;
}


/** Returns the version string of the underlying file searching library (kpathsea, MiKTeX) */
std::string FileFinder::version () {
#ifdef MIKTEX_COM
	bool autoinit=false;
	try {
		if (!_initialized) {
			init("", "", false);
			autoinit = true;
		}
		std::string ret = miktex->getVersion();
		if (autoinit)
			finish();
		return ret;
	}
	catch (MessageException &e) {
		if (autoinit)
			finish();
	}
#else
	if (const char *v = strrchr(KPSEVERSION, ' '))
		return (std::string(KPSEVERSION).substr(0, 9) == "kpathsea ") ? v+1 : KPSEVERSION;
	if (strlen(KPSEVERSION) > 0)
		return KPSEVERSION;
#endif
	return "unknown";
}


void FileFinder::addLookupDir (const std::string &path) {
	_additional_dirs.insert(path);
}


/** Determines filetype by the filename extension and calls kpse_find_file
 *  to actually look up the file.
 *  @param[in] fname name of file to look up
 *  @param[in] ftype expected file format of file fname; if 0, it's derived from the filename suffix
 *  @return file path on success, 0 otherwise */
static const char* find_file (const std::string &fname, const char *ftype) {
	if (!_initialized || fname.empty())
		return 0;

	static std::string buf;
	// try to lookup the file in the additionally specified directories
	for (std::set<std::string>::iterator it=_additional_dirs.begin(); it != _additional_dirs.end(); ++it) {
		if (it->at(0) == '/')
			buf.clear();
		else
			buf = FileSystem::getcwd()+"/";
		buf += (*it) + "/" + fname;
		if (FileSystem::exists(buf))
			return buf.c_str();
	}

	std::string ext;
	if (ftype)
		ext = ftype;
	else {
		size_t pos = fname.rfind('.');
		if (pos == std::string::npos)
			return 0;  // no extension and no file type => no search
		ext = fname.substr(pos+1);
	}

#ifdef MIKTEX_COM
	if (ext == "dll" || ext == "exe") {
		// lookup dll and exe files in the MiKTeX bin directory first
		buf = miktex->getBinDir() + "/" + fname;
		if (FileSystem::exists(buf))
			return buf.c_str();
	}
	else if (ext == "cmap") {
		// The MiKTeX SDK doesn't support the lookup of files without suffix (yet), thus
		// it's not possible to find cmap files which usually don't have a suffix. In order
		// to work around this, we try to lookup the files by calling kpsewhich.
		Process process("kpsewhich", std::string("-format=cmap ")+fname);
		process.run(&buf);
		return buf.empty() ? 0 : buf.c_str();
	}
	try {
		return miktex->findFile(fname.c_str());
	}
	catch (const MessageException &e) {
		return 0;
	}
#else
#if defined(MIKTEX)
        if (ext == "dll" || ext == "exe")
        {
          const char * path = kpse_var_value("SELFAUTOLOC");
          if (path != nullptr)
          {
            buf = std::string(path) + "/" + fname;
            return FileSystem::exists(buf.c_str()) ? buf.c_str() : nullptr;
          }
          return nullptr;
        }
#endif
#ifdef TEXLIVEWIN32
	if (ext == "exe") {
		// lookup exe files in directory where dvisvgm is located
		if (const char *path = kpse_var_value("SELFAUTOLOC")) {
			buf = std::string(path) + "/" + fname;
			return FileSystem::exists(buf) ? buf.c_str() : 0;
		}
		return 0;
	}
#endif
	static std::map<std::string, kpse_file_format_type> types;
	if (types.empty()) {
		types["tfm"]  = kpse_tfm_format;
		types["pfb"]  = kpse_type1_format;
		types["vf"]   = kpse_vf_format;
		types["mf"]   = kpse_mf_format;
		types["ttc"]  = kpse_truetype_format;
		types["ttf"]  = kpse_truetype_format;
		types["otf"]  = kpse_opentype_format;
		types["map"]  = kpse_fontmap_format;
		types["cmap"] = kpse_cmap_format;
		types["sty"]  = kpse_tex_format;
		types["enc"]  = kpse_enc_format;
		types["pro"]  = kpse_tex_ps_header_format;
		types["sfd"]  = kpse_sfd_format;
	}
	std::map<std::string, kpse_file_format_type>::iterator it = types.find(ext.c_str());
	if (it == types.end())
		return 0;

	if (char *path = kpse_find_file(fname.c_str(), it->second, 0)) {
		// In the current version of libkpathsea, each call of kpse_find_file produces
		// a memory leak since the path buffer is not freed. I don't think we can do
		// anything against it here...
		buf = path;
		std::free(path);
		return buf.c_str();
	}
	return 0;
#endif
}


/** Checks whether the given file is mapped to a different name and if the
 *  file can be found under this name.
 *  @param[in] fname name of file to look up
 *  @return file path on success, 0 otherwise */
static const char* find_mapped_file (std::string fname) {
	size_t pos = fname.rfind('.');
	if (pos == std::string::npos)
		return 0;
	const std::string ext  = fname.substr(pos+1);  // file extension
	const std::string base = fname.substr(0, pos);
	if (const FontMap::Entry *entry = FontMap::instance().lookup(base)) {
		const char *path=0;
		if (entry->fontname.find('.') != std::string::npos)  // does the mapped filename has an extension?
			path = find_file(entry->fontname, 0);             // look for that file
		else {                             // otherwise, use extension of unmapped file
			fname = entry->fontname + "." + ext;
			(path = find_file(fname, 0)) || (path = mktex(fname));
		}
		return path;
	}
	return 0;
}


/** Runs external mktexFOO tool to create missing tfm or mf file.
 *  @param[in] fname name of file to build
 *  @return file path on success, 0 otherwise */
static const char* mktex (const std::string &fname) {
	if (!_initialized)
		return 0;

	size_t pos = fname.rfind('.');
	if (!_mktex_enabled || pos == std::string::npos)
		return 0;

	std::string ext  = fname.substr(pos+1);  // file extension
	if (ext != "tfm" && ext != "mf")
		return 0;

	const char *path = 0;
#ifdef MIKTEX
	// maketfm and makemf are located in miktex/bin which is in the search PATH
	std::string toolname = (ext == "tfm" ? "miktex-maketfm" : "miktex-makemf");
	system((toolname+".exe "+fname).c_str());
	path = find_file(fname, 0);
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
const char* FileFinder::lookup (const std::string &fname, const char *ftype, bool extended) {
	const char *path;
	if ((path = find_file(fname, ftype)) || (extended  && ((path = find_mapped_file(fname)) || (path = mktex(fname)))))
		return path;
	return 0;
}
