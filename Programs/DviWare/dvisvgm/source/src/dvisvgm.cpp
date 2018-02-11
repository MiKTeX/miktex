/*************************************************************************
** dvisvgm.cpp                                                          **
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
#include <clipper.hpp>
#include <fstream>
#include <iostream>
#include <list>
#include <potracelib.h>
#include <sstream>
#include <xxhash.h>
#include <zlib.h>
#include "CommandLine.hpp"
#include "DVIToSVG.hpp"
#include "DVIToSVGActions.hpp"
#include "EPSToSVG.hpp"
#include "FileFinder.hpp"
#include "FileSystem.hpp"
#include "Font.hpp"
#include "FontEngine.hpp"
#include "Ghostscript.hpp"
#include "HyperlinkManager.hpp"
#include "Message.hpp"
#include "PageSize.hpp"
#include "PSInterpreter.hpp"
#include "PsSpecialHandler.hpp"
#include "SignalHandler.hpp"
#include "SVGOutput.hpp"
#include "System.hpp"
#include "utility.hpp"
#include "version.hpp"

#ifndef DISABLE_WOFF
#include <brotli/encode.h>
#include <woff2/version.h>
#include "ffwrapper.h"
#include "TTFAutohint.hpp"
#endif
#if defined(MIKTEX)
#  include <miktex/Definitions>
#  if defined(MIKTEX_WINDOWS)
#   include <miktex/Util/CharBuffer>
#   define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#  endif
#endif

using namespace std;

////////////////////////////////////////////////////////////////////////////////

static string remove_path (string fname) {
	fname = FileSystem::adaptPathSeperators(fname);
	size_t slashpos = fname.rfind('/');
	if (slashpos == string::npos)
		return fname;
	return fname.substr(slashpos+1);
}


static string ensure_suffix (string fname, bool eps) {
	size_t dotpos = remove_path(fname).rfind('.');
	if (dotpos == string::npos)
		fname += (eps ? ".eps" : ".dvi");
	return fname;
}


static string get_transformation_string (const CommandLine &args) {
	ostringstream oss;
	if (args.rotateOpt.given())
		oss << 'R' << args.rotateOpt.value() << ",w/2,h/2";
	if (args.translateOpt.given())
		oss << 'T' << args.translateOpt.value();
	if (args.scaleOpt.given())
		oss << 'S' << args.scaleOpt.value();
	if (args.transformOpt.given())
		oss << args.transformOpt.value();
	return oss.str();
}


static void set_libgs (CommandLine &args) {
#if !defined(DISABLE_GS) && !defined(HAVE_LIBGS)
	if (args.libgsOpt.given())
		Ghostscript::LIBGS_NAME = args.libgsOpt.value();
	else if (getenv("LIBGS"))
		Ghostscript::LIBGS_NAME = getenv("LIBGS");
#endif
}


static bool set_cache_dir (const CommandLine &args) {
	if (args.cacheOpt.given() && !args.cacheOpt.value().empty()) {
		if (args.cacheOpt.value() == "none")
			PhysicalFont::CACHE_PATH = 0;
		else if (FileSystem::exists(args.cacheOpt.value()))
			PhysicalFont::CACHE_PATH = args.cacheOpt.value().c_str();
		else
			Message::wstream(true) << "cache directory '" << args.cacheOpt.value() << "' does not exist (caching disabled)\n";
	}
	else if (const char *userdir = FileSystem::userdir()) {
		static string cachepath = userdir + string("/.dvisvgm/cache");
		if (!FileSystem::exists(cachepath))
			FileSystem::mkdir(cachepath);
		PhysicalFont::CACHE_PATH = cachepath.c_str();
	}
	if (args.cacheOpt.given() && args.cacheOpt.value().empty()) {
		cout << "cache directory: " << (PhysicalFont::CACHE_PATH ? PhysicalFont::CACHE_PATH : "(none)") << '\n';
		try {
			if (PhysicalFont::CACHE_PATH)
				FontCache::fontinfo(PhysicalFont::CACHE_PATH, cout, true);
		}
		catch (StreamReaderException &e) {
			Message::wstream(true) << "failed reading cache data\n";
		}
		return false;
	}
	return true;
}


static bool set_temp_dir (const CommandLine &args) {
	if (args.tmpdirOpt.given()) {
		if (!args.tmpdirOpt.value().empty())
			FileSystem::TMPDIR = args.tmpdirOpt.value().c_str();
		else {
			cout << "temporary folder: " << FileSystem::tmpdir() << '\n';
			return false;
		}
	}
	return true;
}


static bool check_bbox (const string &bboxstr) {
	for (const char *fmt : {"none", "min", "preview", "papersize", "dvi"})
		if (bboxstr == fmt)
			return true;
	if (isalpha(bboxstr[0])) {
		try {
			PageSize size(bboxstr);
			return true;
		}
		catch (const PageSizeException &e) {
			Message::estream(true) << "invalid bounding box format '" << bboxstr << "'\n";
			return false;
		}
	}
	try {
		// check if given bbox argument is valid, i.e. doesn't throw an exception
		BoundingBox bbox;
		bbox.set(bboxstr);
		return true;
	}
	catch (const MessageException &e) {
		Message::estream(true) << e.what() << '\n';
		return false;
	}
}


// Helper class to generate a list of version information of the used libraries.
class VersionInfo {
	public:
		void add (const string &name, const string &version, bool ignoreEmpty=false) {
			if (!version.empty() || !ignoreEmpty)
				append(name, util::trim(version));
		}

		void add (const string &name, const char *version, bool ignoreEmpty=false) {
			if (version && *version)
				append(name, util::trim(version));
			else if (!ignoreEmpty)
				append(name, "");
		}

		void add (const string &name, const vector<int> &versionComponents) {
			string version;
			for (auto it=versionComponents.begin(); it != versionComponents.end(); ++it) {
				if (it != versionComponents.begin())
					version += '.';
				version += to_string(*it);
			}
			append(name, version);
		}

		/** Adds a version number given as a single unsigned integer, and optionally
		 *  extracts its components, e.g. 0x00010203 => "1.2.3" (3 components separated
		 *  by multiples of 256).
		 *  @param[in] name library name
		 *  @param[in] version version number
		 *  @param[in] compcount number of components the version consists of
		 *  @param[in] factor factor used to separate the components */
		void add (const string &name, uint32_t version, int compcount=1, uint32_t factor=0xffffffff) {
			string str;
			while (compcount-- > 0) {
				if (!str.empty())
					str.insert(0, ".");
				str.insert(0, to_string(version % factor));
				version /= factor;
			}
			append(name, str);
		}

		/** Writes the version information to the given output stream. */
		void write (ostream &os) {
			using Entry = pair<string,string>;
			_versionPairs.sort([](const Entry &e1, const Entry &e2) {
				return util::tolower(e1.first) < util::tolower(e2.first);
			});
			size_t maxNameLength=0;
			for (const Entry &versionPair : _versionPairs)
				maxNameLength = max(maxNameLength, versionPair.first.length());
			for (const Entry &versionPair : _versionPairs) {
				string name = versionPair.first+":";
				os << left << setw(maxNameLength+2) << name;
				os << (versionPair.second.empty() ? "unknown" : versionPair.second) << '\n';
			}
		}

	protected:
		void append (const string &name, const string &version) {
			_versionPairs.emplace_back(pair<string,string>(name, version));
		}

	private:
		list<pair<string,string>> _versionPairs;
};


static void print_version (bool extended) {
	string versionstr = string(PROGRAM_NAME)+" "+PROGRAM_VERSION;
#ifdef TARGET_SYSTEM
	if (extended && strlen(TARGET_SYSTEM) > 0)
		versionstr += " (" TARGET_SYSTEM ")";
#endif
	cout << versionstr << '\n';
	if (extended) {
		cout << string(versionstr.length(), '-') << '\n';
		VersionInfo versionInfo;
		versionInfo.add("clipper", CLIPPER_VERSION);
		versionInfo.add("freetype", FontEngine::version());
		versionInfo.add("potrace", strchr(potrace_version(), ' '));
		versionInfo.add("xxhash", XXH_versionNumber(), 3, 100);
		versionInfo.add("zlib", zlibVersion());
		versionInfo.add("Ghostscript", Ghostscript().revision(true), true);
#ifndef DISABLE_WOFF
		versionInfo.add("brotli", BrotliEncoderVersion(), 3, 0x1000);
		versionInfo.add("woff2", woff2::version, 3, 0x100);
		versionInfo.add("fontforge", ff_version());
		versionInfo.add("ttfautohint", TTFAutohint().version(), true);
#endif
#ifdef MIKTEX_COM
		versionInfo.add("MiKTeX", FileFinder::instance().version());
#else
		versionInfo.add("kpathsea", FileFinder::instance().version());
#endif
		versionInfo.write(cout);
	}
}


static void init_fontmap (const CommandLine &cmdline) {
	string mapseq;
	if (cmdline.fontmapOpt.given())
		mapseq = cmdline.fontmapOpt.value();
	bool additional = !mapseq.empty() && strchr("+-=", mapseq[0]);
	if (mapseq.empty() || additional) {
		bool found = false;
		for (string mapfile : {"ps2pk", "pdftex", "dvipdfm", "psfonts"}) {
			if ((found = FontMap::instance().read(mapfile+".map")))
				break;
		}
		if (!found)
			Message::wstream(true) << "none of the default map files could be found\n";
	}
	if (!mapseq.empty())
		FontMap::instance().read(mapseq);
}


static void set_variables (const CommandLine &cmdline) {
	Message::COLORIZE = cmdline.colorOpt.given();
	if (cmdline.progressOpt.given()) {
		DVIToSVG::COMPUTE_PROGRESS = true;
		SpecialActions::PROGRESSBAR_DELAY = cmdline.progressOpt.value();
	}
	Color::SUPPRESS_COLOR_NAMES = !cmdline.colornamesOpt.given();
	SVGTree::CREATE_CSS = !cmdline.noStylesOpt.given();
	SVGTree::USE_FONTS = !cmdline.noFontsOpt.given();
	if (!SVGTree::setFontFormat(cmdline.fontFormatOpt.value())) {
		string msg = "unknown font format '"+cmdline.fontFormatOpt.value()+"' (supported formats: ";
		ostringstream oss;
		for (const string &format : FontWriter::supportedFormats())
			oss << ", " << format;
		msg += oss.str().substr(2) + ')';
		throw CL::CommandLineException(msg);
	}
	SVGTree::CREATE_USE_ELEMENTS = cmdline.noFontsOpt.value() < 1;
	SVGTree::ZOOM_FACTOR = cmdline.zoomOpt.value();
	SVGTree::RELATIVE_PATH_CMDS = cmdline.relativeOpt.given();
	SVGTree::MERGE_CHARS = !cmdline.noMergeOpt.given();
	SVGTree::ADD_COMMENTS = cmdline.commentsOpt.given();
	DVIToSVG::TRACE_MODE = cmdline.traceAllOpt.given() ? (cmdline.traceAllOpt.value() ? 'a' : 'm') : 0;
	Message::LEVEL = cmdline.verbosityOpt.value();
	PhysicalFont::EXACT_BBOX = cmdline.exactOpt.given();
	PhysicalFont::KEEP_TEMP_FILES = cmdline.keepOpt.given();
	PhysicalFont::METAFONT_MAG = max(1.0, cmdline.magOpt.value());
	XMLString::DECIMAL_PLACES = max(0, min(6, cmdline.precisionOpt.value()));
	PsSpecialHandler::COMPUTE_CLIPPATHS_INTERSECTIONS = cmdline.clipjoinOpt.given();
	PsSpecialHandler::SHADING_SEGMENT_OVERLAP = cmdline.gradOverlapOpt.given();
	PsSpecialHandler::SHADING_SEGMENT_SIZE = max(1, cmdline.gradSegmentsOpt.value());
	PsSpecialHandler::SHADING_SIMPLIFY_DELTA = cmdline.gradSimplifyOpt.value();
}


#if defined(MIKTEX)
int MIKTEXCEECALL Main(int argc, char **argv) {
#else
int main (int argc, char *argv[]) {
#endif
	CommandLine cmdline;
	try {
		cmdline.parse(argc, argv);
		if (argc == 1 || cmdline.helpOpt.given()) {
			cmdline.help(cout, cmdline.helpOpt.value());
			return 0;
		}
		FileFinder::init(argv[0], "dvisvgm", !cmdline.noMktexmfOpt.given());
		set_libgs(cmdline);
		if (cmdline.versionOpt.given()) {
			print_version(cmdline.versionOpt.value());
			return 0;
		}
		if (cmdline.listSpecialsOpt.given()) {
			DVIToSVG::setProcessSpecials();
			SpecialManager::instance().writeHandlerInfo(cout);
			return 0;
		}
		if (!set_cache_dir(cmdline) || !set_temp_dir(cmdline))
			return 0;
		if (cmdline.stdoutOpt.given() && cmdline.zipOpt.given()) {
			Message::estream(true) << "writing SVGZ files to stdout is not supported\n";
			return 1;
		}
		if (!check_bbox(cmdline.bboxOpt.value()))
			return 1;
		if (!HyperlinkManager::setLinkMarker(cmdline.linkmarkOpt.value()))
			Message::wstream(true) << "invalid argument '"+cmdline.linkmarkOpt.value()+"' supplied for option --linkmark\n";
		if (argc > 1 && cmdline.filenames().size() < 1) {
			Message::estream(true) << "no input file given\n";
			return 1;
		}
	}
	catch (MessageException &e) {
		Message::estream() << e.what() << '\n';
		return 1;
	}

	bool eps_given = cmdline.epsOpt.given();
	string inputfile = ensure_suffix(cmdline.filenames()[0], eps_given);
#if defined(MIKTEX_WINDOWS)
        ifstream ifs(UW_(inputfile.c_str()), ios::binary | ios::in);
#else
	ifstream ifs(inputfile.c_str(), ios::binary|ios::in);
#endif
	if (!ifs) {
		Message::estream(true) << "can't open file '" << inputfile << "' for reading\n";
		return 0;
	}
	try {
		double start_time = System::time();
		set_variables(cmdline);
		SVGOutput out(cmdline.stdoutOpt.given() ? nullptr : inputfile.c_str(),
			cmdline.outputOpt.value(),
			cmdline.zipOpt.given() ? cmdline.zipOpt.value() : 0);
		SignalHandler::instance().start();
		if (cmdline.epsOpt.given()) {
			EPSToSVG eps2svg(inputfile, out);
			eps2svg.convert();
			Message::mstream().indent(0);
			Message::mstream(false, Message::MC_PAGE_NUMBER) << "file converted in " << (System::time()-start_time) << " seconds\n";
		}
		else {
			init_fontmap(cmdline);
			DVIToSVG dvi2svg(ifs, out);
			const char *ignore_specials=nullptr;
			if (cmdline.noSpecialsOpt.given())
				ignore_specials = cmdline.noSpecialsOpt.value().empty() ? "*" : cmdline.noSpecialsOpt.value().c_str();
			dvi2svg.setProcessSpecials(ignore_specials, true);
			dvi2svg.setPageTransformation(get_transformation_string(cmdline));
			dvi2svg.setPageSize(cmdline.bboxOpt.value());

			pair<int,int> pageinfo;
			dvi2svg.convert(cmdline.pageOpt.value(), &pageinfo);
			Message::mstream().indent(0);
			Message::mstream(false, Message::MC_PAGE_NUMBER) << "\n" << pageinfo.first << " of " << pageinfo.second << " page";
			if (pageinfo.second > 1)
				Message::mstream(false, Message::MC_PAGE_NUMBER) << 's';
			Message::mstream(false, Message::MC_PAGE_NUMBER) << " converted in " << (System::time()-start_time) << " seconds\n";
		}
	}
	catch (DVIException &e) {
		Message::estream() << "\nDVI error: " << e.what() << '\n';
	}
	catch (PSException &e) {
		Message::estream() << "\nPostScript error: " << e.what() << '\n';
	}
	catch (SignalException &e) {
		Message::wstream().clearline();
		Message::wstream(true) << "execution interrupted by user\n";
	}
	catch (exception &e) {
		Message::estream(true) << e.what() << '\n';
	}
	return 0;
}
