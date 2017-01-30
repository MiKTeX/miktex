/*************************************************************************
** MetafontWrapper.cpp                                                  **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2017 Martin Gieseking <martin.gieseking@uos.de>   **
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
#include <cstdlib>
#include <cctype>
#include <fstream>
#include <sstream>
#include "FileSystem.hpp"
#include "FileFinder.hpp"
#include "Message.hpp"
#include "MetafontWrapper.hpp"
#include "Process.hpp"
#include "XMLString.hpp"

#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

using namespace std;


MetafontWrapper::MetafontWrapper (const string &fname, const string &dir) : _fontname(fname), _dir(dir)
{
	// ensure that folder paths ends with slash
	if (_dir.empty())
		_dir = "./";
	else if (_dir != "/" && dir[dir.length()-1] != '/')
		_dir += '/';
}


/** Calls Metafont and evaluates the logfile. If a gf file was successfully
 *  generated the dpi value is stripped from the filename
 *  (e.g. cmr10.600gf => cmr10.gf). This makes life easier...
 *  @param[in] mode Metafont mode, e.g. "ljfour"
 *  @param[in] mag magnification factor
 *  @return true on success */
bool MetafontWrapper::call (const string &mode, double mag) {
	if (!FileFinder::instance().lookup(_fontname+".mf"))
		return false;     // mf file not available => no need to call the "slow" Metafont
	FileSystem::remove(_fontname+".gf");

#ifdef _WIN32
#ifdef TEXLIVEWIN32
	const char *mfname = "mf-nowin.exe";
#else
	const char *mfname = "mf.exe";
#endif
	const char *cmd = FileFinder::instance().lookup(mfname, false);
	if (!cmd) {
		Message::estream(true) << "can't run Metafont (" << mfname << " not found)\n";
		return false;
	}
#else
	const char *cmd = "mf";
#endif
	ostringstream oss;
	oss << "\"\\mode=" << mode  << ";"  // set MF mode, e.g. 'proof', 'ljfour' or 'localfont'
		"mode_setup;"                    // initialize MF variables
		"mag:=" << mag << ";"            // set magnification factor
		"show pixels_per_inch*mag;"      // print character resolution to stdout
		"batchmode;"                     // don't halt on errors and don't print informational messages
		"input " << _fontname << "\"";   // load font description
	Message::mstream(false, Message::MC_STATE) << "\nrunning Metafont for " << _fontname << '\n';
	Process mf_process(cmd, oss.str().c_str());
	string mf_messages;
	mf_process.run(_dir, &mf_messages);

	// get resolution value from stdout created by above MF command
	char buf[256];
	istringstream iss(mf_messages);
	int resolution = 0;
	while (iss) {
		iss.getline(buf, sizeof(buf));
		string line = buf;
		if (line.substr(0, 3) == ">> ") {
			resolution = atoi(line.substr(3).c_str());
			break;
		}
	}
	// compose expected name of GF file (see Metafont Book, p. 324)
	string gfname = _dir + _fontname + ".";
	if (resolution > 0)
		gfname += XMLString(resolution);
	gfname += "gf";
	FileSystem::rename(gfname, _dir+_fontname+".gf");  // remove resolution value from filename
	return FileSystem::exists(_dir+_fontname+".gf");
}


/** Calls Metafont if output files (tfm and gf) don't already exist.
 *  @param[in] mode Metafont mode to be used (e.g. 'ljfour')
 *  @param[in] mag magnification factor
 *  @return true on success */
bool MetafontWrapper::make (const string &mode, double mag) {
#if defined(MIKTEX_WINDOWS)
        ifstream tfm(UW_((_dir+_fontname+".tfm").c_str()));
        ifstream gf(UW_((_dir+_fontname+".gf").c_str()));
#else
	ifstream tfm((_dir+_fontname+".tfm").c_str());
	ifstream gf((_dir+_fontname+".gf").c_str());
#endif
	if (gf && tfm) // @@ distinguish between gf and tfm
		return true;
	return call(mode, mag);
}


bool MetafontWrapper::success () const {
#if defined(MIKTEX_WINDOWS)
        ifstream tfm(UW_((_dir+_fontname+".tfm").c_str()));
        ifstream gf(UW_((_dir+_fontname+".gf").c_str()));
#else
	ifstream tfm((_dir+_fontname+".tfm").c_str());
	ifstream gf((_dir+_fontname+".gf").c_str());
#endif
	return tfm && gf;
}
