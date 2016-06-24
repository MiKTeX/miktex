/*************************************************************************
** MetafontWrapper.cpp                                                  **
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
#include <cstdlib>
#include <cctype>
#include <fstream>
#include <sstream>
#include "FileSystem.h"
#include "FileFinder.h"
#include "Message.h"
#include "MetafontWrapper.h"
#include "Process.h"
#include "XMLString.h"

#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

using namespace std;


MetafontWrapper::MetafontWrapper (const string &fname) : _fontname(fname)
{
}


/** Calls Metafont and evaluates the logfile. If a gf file was successfully
 *  generated the dpi value is stripped from the filename
 *  (e.g. cmr10.600gf => cmr10.gf). This makes life easier...
 *  @param[in] mode Metafont mode, e.g. "ljfour"
 *  @param[in] mag magnification factor
 *  @return true on success */
bool MetafontWrapper::call (const string &mode, double mag) {
	if (!FileFinder::lookup(_fontname+".mf"))
		return false;     // mf file not available => no need to call the "slow" Metafont
	FileSystem::remove(_fontname+".gf");

#ifdef __WIN32__
#ifdef TEXLIVEWIN32
	const char *mfname = "mf-nowin.exe";
#else
	const char *mfname = "mf.exe";
#endif
	const char *cmd = FileFinder::lookup(mfname, false);
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
	mf_process.run(&mf_messages);

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
	string gfname = _fontname + ".";
	if (resolution > 0)
		gfname += XMLString(resolution);
	gfname += "gf";
	FileSystem::rename(gfname, _fontname+".gf");  // remove resolution from filename
	return FileSystem::exists(_fontname+".gf");
}


/** Calls Metafont if output files (tfm and gf) don't already exist.
 *  @param[in] mode Metafont mode to be used (e.g. 'ljfour')
 *  @param[in] mag magnification factor
 *  @return true on success */
bool MetafontWrapper::make (const string &mode, double mag) {
#if defined(MIKTEX_WINDOWS)
  ifstream tfm(UW_((_fontname + ".tfm").c_str()));
  ifstream gf(UW_((_fontname + ".gf").c_str()));
#else
	ifstream tfm((_fontname+".tfm").c_str());
	ifstream gf((_fontname+".gf").c_str());
#endif
	if (gf && tfm) // @@ distinguish between gf and tfm
		return true;
	return call(mode, mag);
}


bool MetafontWrapper::success () const {
#if defined(MIKTEX_WINDOWS)
  ifstream tfm(UW_((_fontname + ".tfm").c_str()));
  ifstream gf(UW_((_fontname + ".gf").c_str()));
#else
	ifstream tfm((_fontname+".tfm").c_str());
	ifstream gf((_fontname+".gf").c_str());
#endif
	return tfm && gf;
}


/** Remove all files created by a Metafont call (tfm, gf, log).
 *  @param[in] keepGF if true, GF files won't be removed */
void MetafontWrapper::removeOutputFiles (bool keepGF) {
	removeOutputFiles(_fontname, keepGF);
}


/** Remove all files created by a Metafont call for a given font (tfm, gf, log).
 *  @param[in] fontname name of font whose temporary files should be removed
 *  @param[in] keepGF if true, GF files will be kept */
void MetafontWrapper::removeOutputFiles (const string &fontname, bool keepGF) {
	const char *ext[] = {"gf", "tfm", "log", 0};
	for (const char **p = keepGF ? ext+2 : ext; *p; ++p)
		FileSystem::remove(fontname + "." + *p);
}
