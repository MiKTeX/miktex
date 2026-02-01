/*************************************************************************
** PSInterpreter.cpp                                                    **
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

#if defined(MIKTEX)
#include <config.h>
#endif
#include <cstring>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include "algorithm.hpp"
#include "FileFinder.hpp"
#include "FileSystem.hpp"
#include "InputReader.hpp"
#include "Message.hpp"
#include "PSInterpreter.hpp"
#include "SignalHandler.hpp"
#include "utility.hpp"

using namespace std;


/** Constructs a new PSInterpreter object.
 *  @param[in] actions template methods to be executed after recognizing the corresponding PS operator. */
PSInterpreter::PSInterpreter (PSActions *actions)
	: _mode(PS_NONE), _actions(actions)
{
}


PSInterpreter::~PSInterpreter () {
	// don't process any further PS code that might be received when exiting GS
	_gs.finalize();
	_actions = nullptr;
}


void PSInterpreter::init () {
	if (!_initialized) {
		vector<const char*> gsargs {
			"gs",                // dummy name
			"-q",                // be quiet, suppress gs banner
			"-dNODISPLAY",       // we don't need a display device
			"-dNOPAUSE",         // keep going
			"-dWRITESYSTEMDICT", // leave systemdict writable as some operators must be replaced
			"-dNOPROMPT"
		};
		if (int gsrev = _gs.revision()) {
			gsargs.emplace_back(gsrev == 922 ? "-dREALLYDELAYBIND" : "-dDELAYBIND");
			// As of GS 9.50, -dSAFER is active by default which leads to warnings
			// in conjunction with -dDELAYBIND and -dWRITESYSTEMDICT.
			// Thus, -dDELAYSAFER (or -dNOSAFER) must be added.
			// https://www.ghostscript.com/doc/9.50/Use.htm#Safer
			if (gsrev >= 950) {
				gsargs.emplace_back("-dDELAYSAFER");
				gsargs.emplace_back("-dALLOWPSTRANSPARENCY");
			}
			// GS 9.55.0 introduced a new, C-based PDF interpreter which is enabled by default
			// as of GS 9.56.0. Since dvisvgm relies on the old PS-based interpreter for its
			// PDF support, we try to disable the new one.
			// https://www.ghostscript.com/doc/9.56.0/Use.htm#PDF_switches
			// As of GS 10.02.0, option NEWPDF has been deprecated and has no effect any longer.
			if (gsrev >= 9560 && gsrev < 10020)
				gsargs.emplace_back("-dNEWPDF=false");
		}
		_gs.init(gsargs.size(), gsargs.data(), this);
		_gs.set_stdio(input, output, error);
		_initialized = true;
		// Before executing any random PS code redefine some operators and run
		// initializing PS code. This cannot be done in the constructor because we
		// need the completely initialized PSInterpreter object here.
		execute(PSDEFS);
	}
}


PSActions* PSInterpreter::setActions (PSActions *actions) {
	PSActions *old_actions = _actions;
	_actions = actions;
	return old_actions;
}


/** Checks if the given status value returned by Ghostscript indicates an error.
 *  @param[in] status status value returned by Ghostscript after the execution of a PS fragment
 *  @throw PSException if the status value indicates a PostScript error */
void PSInterpreter::checkStatus (int status) {
	if (status < 0) {
		_mode = PS_QUIT;
		if (status < -100)
			throw PSException("fatal error");
		if (_errorMessage.empty())
			throw PSException(_gs.error_name(status));
		size_t pos = _errorMessage.rfind("Operand stack:");
		if (pos != string::npos && pos > 0 && !isspace(_errorMessage[pos-1]))
			_errorMessage.insert(pos, "\n");
		throw PSException(_errorMessage);
	}
}


/** Executes a chunk of PostScript code.
 *  @param[in] str buffer containing the code
 *  @param[in] len number of characters in buffer
 *  @param[in] flush If true, a final 'flush' is sent which forces the output buffer to be written immediately.
 *  @return true if the assigned number of bytes have been read */
bool PSInterpreter::execute (const char *str, size_t len, bool flush) {
	init();
	if (_mode == PS_QUIT)
		return false;
	if (_mode == PS_EXCEPTION) {
		if (_errorMessage.empty())
			return false;
		throw PSException(_errorMessage);
	}

	int status=0;
	if (_mode == PS_NONE) {
		_gs.run_string_begin(0, &status);
		_mode = PS_RUNNING;
	}
	checkStatus(status);

	bool complete=false;
	if (_bytesToRead > 0 && len >= _bytesToRead) {
		len = _bytesToRead;
		complete = true;
	}

	// feed Ghostscript with code chunks that are not larger than 64KB
	// => see documentation of gsapi_run_string_foo()
	const char *p=str;
	while (_mode == PS_RUNNING && len > 0) {
		SignalHandler::instance().check();
		size_t chunksize = min(len, (size_t)0xffff);
		_gs.run_string_continue(p, chunksize, 0, &status);
		p += chunksize;
		len -= chunksize;
		if (_bytesToRead > 0)
			_bytesToRead -= chunksize;
		if (status == -101)  // e_Quit
			_mode = PS_QUIT;
		else
			checkStatus(status);
	}
	if (flush) {
		// force writing contents of output buffer
		_gs.run_string_continue("\nflush ", 7, 0, &status);
	}
	return complete;
}


/** Executes a chunk of PostScript code read from a stream. The method returns on EOF.
 *  @param[in] is the input stream
 *  @param[in] flush If true, a final 'flush' is sent which forces the output buffer to be written immediately.
 *  @return true if the assigned number of bytes have been read */
bool PSInterpreter::execute (istream &is, bool flush) {
	bool finished = false;
	while (is && !is.eof() && !finished) {
		char buf[4096];
		is.read(buf, 4096);
		finished = execute(buf, is.gcount(), false);
	}
	execute("\n", 1, flush);
	return finished;
}


bool PSInterpreter::executeRaw (const string &str, int n) {
	_rawData.clear();
	ostringstream oss;
	oss << str << ' ' << n << " (raw) prcmd\n";
	execute(oss.str());
	return !_rawData.empty();
}


/** This callback function handles input from stdin to Ghostscript. Currently not needed.
 *  @param[in] inst pointer to calling instance of PSInterpreter
 *  @param[in] buf takes the read characters
 *  @param[in] len size of buffer buf
 *  @return number of characters read */
int GSDLLCALL PSInterpreter::input (void *inst, char *buf, int len) noexcept {
	return 0;
}


/** This callback function handles output from Ghostscript to stdout. It looks for
 *  emitted commands staring with "dvi." and executes them by calling method callActions.
 *  Ghostscript sends the text in chunks by several calls of this function.
 *  Unfortunately, the PostScript specification wants error messages also to be sent to stdout
 *  instead of stderr. Thus, we must collect and concatenate the chunks until an evaluable text
 *  fragment is completely received. Furthermore, error messages have to be recognized and to be
 *  filtered out.
 *  @param[in] inst pointer to calling instance of PSInterpreter
 *  @param[in] buf contains the characters to be output
 *  @param[in] len number of characters in buf
 *  @return number of processed characters (equals 'len') */
int GSDLLCALL PSInterpreter::output (void *inst, const char *buf, int len) noexcept {
	auto self = static_cast<PSInterpreter*>(inst);
	if (!self || !self->_actions || self->_mode == PS_EXCEPTION)
		return len;

	try {
		SplittedCharInputBuffer ib(self->_unprocessedChars.data(), self->_unprocessedChars.length(), buf, len);
		BufferInputReader ir(ib);
		while (!ir.eof()) {
			if (self->_inError) {
				self->_errorMessage += ib.toString();
				ib.invalidate();
			}
			else if (ir.check("Unrecoverable error: ")) {
				self->_errorMessage = ib.toString();
				ib.invalidate();
				self->_inError = true;
			}
			else {
				ir.skipSpace();
				int spacepos = ib.find(' ');
				if (spacepos <= 0)  // no separating space found?
					break;           // => wait for more data
				string entry = ir.getString(spacepos);
				if (self->_command.empty()) {   // not yet collecting command parameters?
					if (entry.length() < 4 || entry.substr(0, 4) != "dvi.")
						ir.skipUntil("\n");
					else
						self->_command.emplace_back(entry.substr(4));
				}
				else if (entry[0] != '|') {   // not yet at end of command?
					self->_command.emplace_back(std::move(entry));
				}
				else {
					self->processCommand();
					self->_command.clear();
					ir.skipSpace();
				}
			}
		}
		if (ir.eof())
			self->_unprocessedChars.clear();
		else
			self->_unprocessedChars = ib.toString();
	}
	catch (exception &e) {
		self->_mode = PS_EXCEPTION;
		self->_errorMessage = e.what();
	}
	return len;
}


/** Evaluates a command emitted by Ghostscript and invokes the corresponding
 *  method of interface class PSActions. */
void PSInterpreter::processCommand () {
	if (!_actions || _command.empty())
		return;
	struct Operator {
		int pcount;       // number of parameters (< 0 : variable number of parameters)
		void (PSActions::*handler)(vector<double> &p);  // operation handler
	};
	static const unordered_map<string, Operator> operators {
		{"applyscalevals",         { 3, &PSActions::applyscalevals}},
		{"clip",                   { 0, &PSActions::clip}},
		{"clippath",               { 0, &PSActions::clippath}},
		{"closepath",              { 0, &PSActions::closepath}},
		{"curveto",                { 6, &PSActions::curveto}},
		{"eoclip",                 { 0, &PSActions::eoclip}},
		{"eofill",                 { 0, &PSActions::eofill}},
		{"fill",                   { 0, &PSActions::fill}},
		{"grestore",               { 0, &PSActions::grestore}},
		{"grestoreall",            { 0, &PSActions::grestoreall}},
		{"gsave",                  { 0, &PSActions::gsave}},
		{"image",                  { 3, &PSActions::image}},
		{"initclip",               { 0, &PSActions::initclip}},
		{"lineto",                 { 2, &PSActions::lineto}},
		{"makepattern",            {-1, &PSActions::makepattern}},
		{"moveto",                 { 2, &PSActions::moveto}},
		{"newpath",                { 1, &PSActions::newpath}},
		{"querypos",               { 2, &PSActions::querypos}},
		{"restore",                { 1, &PSActions::restore}},
		{"rotate",                 { 1, &PSActions::rotate}},
		{"save",                   { 1, &PSActions::save}},
		{"scale",                  { 2, &PSActions::scale}},
		{"setalphaisshape",        { 1, &PSActions::setalphaisshape}},
		{"setblendmode",           { 1, &PSActions::setblendmode}},
		{"setcolorspace",          { 1, &PSActions::setcolorspace}},
		{"setcmykcolor",           { 4, &PSActions::setcmykcolor}},
		{"setdash",                {-1, &PSActions::setdash}},
		{"setfillconstantalpha",   { 1, &PSActions::setfillconstantalpha}},
		{"setgray",                { 1, &PSActions::setgray}},
		{"sethsbcolor",            { 3, &PSActions::sethsbcolor}},
		{"setlinecap",             { 1, &PSActions::setlinecap}},
		{"setlinejoin",            { 1, &PSActions::setlinejoin}},
		{"setlinewidth",           { 1, &PSActions::setlinewidth}},
		{"setmatrix",              { 6, &PSActions::setmatrix}},
		{"setmiterlimit",          { 1, &PSActions::setmiterlimit}},
		{"setnulldevice",          { 1, &PSActions::setnulldevice}},
		{"setpagedevice",          { 0, &PSActions::setpagedevice}},
		{"setpattern",             {-1, &PSActions::setpattern}},
		{"setrgbcolor",            { 3, &PSActions::setrgbcolor}},
		{"setstrokeconstantalpha", { 1, &PSActions::setstrokeconstantalpha}},
		{"shfill",                 {-1, &PSActions::shfill}},
		{"stroke",                 { 0, &PSActions::stroke}},
		{"translate",              { 2, &PSActions::translate}},
	};

	if (_command[0] == "raw") {
		_rawData.resize(_command.size()-1);
		std::copy(_command.begin()+1, _command.end(), _rawData.begin());
	}
	else {
		vector<double> numbers(_command.size()-1);
		transform(_command.begin()+1, _command.end(), numbers.begin(), [](const string &str) {
			return stod(str);
		});
		auto it = operators.find(_command[0]);
		if (it != operators.end()) {
			(_actions->*it->second.handler)(numbers);
			_actions->executed();
		}
	}
}


/** This callback function handles output from Ghostscript to stderr.
 *  @param[in] inst pointer to calling instance of PSInterpreter
 *  @param[in] buf contains the characters to be output
 *  @param[in] len number of chars in buf
 *  @return number of processed characters */
int GSDLLCALL PSInterpreter::error (void *inst, const char *buf, int len) noexcept {
	return len;
}


/** Returns the total number of pages of a PDF file.
 *  @param[in] fname name/path of the PDF file */
int PSInterpreter::pdfPageCount (const string &fname) {
	executeRaw("\n("+FileSystem::ensureForwardSlashes(fname)+")@pdfpagecount ", 1);
	if (!_rawData.empty()) {
		size_t index;
		int ret = stoi(_rawData[0], &index, 10);
		if (index > 0)
			return ret;
	}
	return 0;
}


/** Returns the bounding box of a PDF page. If the selected page doesn't exist,
 *  the "invalid" flag of the returned bounding box is set.
 *  @param[in] fname name/path of the PDF file
 *  @param[in] pageno page number
 *  @return the bounding box of the given page */
BoundingBox PSInterpreter::pdfPageBox (const string &fname, int pageno) {
	BoundingBox pagebox;
	executeRaw("\n"+to_string(pageno)+"("+FileSystem::ensureForwardSlashes(fname)+")@pdfpagebox ", 4);
	if (_rawData.size() < 4)
		pagebox.invalidate();
	else
		pagebox = BoundingBox(stod(_rawData[0]), stod(_rawData[1]), stod(_rawData[2]), stod(_rawData[3]));
	return pagebox;
}


vector<PSDeviceInfo> PSInterpreter::getImageDeviceInfos () {
	vector<PSDeviceInfo> infos {
		{"none", "no processing of bitmap images"},
		{"jpeg", "color JPEG format"},
		{"jpeggray", "grayscale JPEG format"},
		{"png", "grayscale or 24-bit color PNG format"},
		{"pnggray", "grayscale PNG format"},
		{"pngmono", "black-and-white PNG format"},
		{"pngmonod", "dithered black-and-white PNG format"},
		{"png16", "4-bit color PNG format"},
		{"png256", "8-bit color PNG format"},
		{"png16m", "24-bit color PNG format"},
	};
	return infos;
}


void PSInterpreter::listImageDeviceInfos (ostream &os) {
	for (const PSDeviceInfo &info : getImageDeviceInfos())
		os << setw(8) << left << info.name << " | " << info.description << '\n';
}


/** Returns true if a given PS device name is known. The function deosn't
 *  check whether the device is actually available.
 *  @param[in] deviceStr device specifier of the form <device name>[:<param>] */
bool PSInterpreter::imageDeviceKnown (string deviceStr) {
	if (deviceStr.empty() || !isalpha(deviceStr[0]))
		return false;
	size_t colonpos = deviceStr.find(':');
	if (colonpos != string::npos)
		deviceStr.resize(colonpos);  // strip optional argument
	auto infos = getImageDeviceInfos();
	auto it = algo::find_if(infos, [&](const PSDeviceInfo &info) {
		return info.name == deviceStr;
	});
	return it != infos.end();
}


/** Sets the output device used to create bitmap images.
 *  @param[in] deviceStr device specifier of the form <device name>[:<param>]
 *  @return true on success, false if device is not supported */
bool PSInterpreter::setImageDevice (const string &deviceStr) {
	auto params = util::split(deviceStr, ":");
	string name = util::tolower(params[0]);
	if (!imageDeviceKnown(name))
		return false;
	if (name != "jpeg" && name != "png" && name != "none") {
		// check if image device is supported by Ghostscript
		executeRaw("devicedict/"+name+" known{1}{0}ifelse\n", 1);
		if (_rawData.empty() || _rawData[0] != "1")
			throw PSException("output device '"+name+"' is not available");
	}
	string ps = "/@imgdevice("+name+")store ";
	try {
		if (params.size() > 1) {
			// set JPEG quality level if given
			if (name.substr(0, 4) == "jpeg") {
				int quality = max(0, min(stoi(params[1]), 100));
				ps += "/JPEGQ "+to_string(quality)+" def ";
			}
			else if (name == "pngmonod") {
				int minFeatureSize = max(0, min(stoi(params[1]), 4));
				ps += "/MinFeatureSize "+to_string(minFeatureSize)+" def ";
			}
		}
	}
	catch (...) {
		throw PSException("invalid device option '"+params[1]+"' (integer expected)");
	}
	execute(ps);
	return true;
}
