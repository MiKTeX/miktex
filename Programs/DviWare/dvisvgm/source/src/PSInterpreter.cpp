/*************************************************************************
** PSInterpreter.cpp                                                    **
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

#if defined(MIKTEX)
#  include <config.h>
#endif
#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include "FileFinder.hpp"
#include "InputReader.hpp"
#include "Message.hpp"
#include "PSFilter.hpp"
#include "PSInterpreter.hpp"
#include "SignalHandler.hpp"

using namespace std;


const char *PSInterpreter::GSARGS[] = {
	"gs",                // dummy name
	"-q",                // be quiet, suppress gs banner
//	"-dSAFER",           // disallow writing of files
	"-dNODISPLAY",       // we don't need a display device
	"-dNOPAUSE",         // keep going
	"-dWRITESYSTEMDICT", // leave systemdict writable as some operators must be replaced
	"-dNOPROMPT",
//	"-dNOBIND",
};


/** Constructs a new PSInterpreter object.
 *  @param[in] actions template methods to be executed after recognizing the corresponding PS operator. */
PSInterpreter::PSInterpreter (PSActions *actions)
	: _mode(PS_NONE), _actions(actions), _filter(0), _bytesToRead(0), _inError(false), _initialized(false)
{
}


void PSInterpreter::init () {
	if (!_initialized) {
		_gs.init(sizeof(GSARGS)/sizeof(char*), GSARGS, this);
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
 *  @param[in] status status value returned by Ghostscript after the execution of a PS snippet
 *  @throw PSException if the status value indicates a PostScript error */
void PSInterpreter::checkStatus (int status) {
	if (status < 0) {
		_mode = PS_QUIT;
		if (status < -100)
			throw PSException("fatal error");
		if (_errorMessage.empty())
			throw PSException(_gs.error_name(status));
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

	if (_filter && _filter->active()) {
		PSFilter *filter = _filter;
		_filter = 0;             // prevent recursion when filter calls execute()
		filter->execute(str, len);
		if (filter->active())    // filter still active after execution?
			_filter = filter;
		return complete;
	}

	// feed Ghostscript with code chunks that are not larger than 64KB
	// => see documentation of gsapi_run_string_foo()
	const char *p=str;
	while (PS_RUNNING && len > 0) {
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
	char buf[4096];
	bool finished = false;
	while (is && !is.eof() && !finished) {
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
int GSDLLCALL PSInterpreter::input (void *inst, char *buf, int len) {
	return 0;
}


/** This callback function handles output from Ghostscript to stdout. It looks for
 *  emitted commands staring with "dvi." and executes them by calling method callActions.
 *  Ghostscript sends the text in chunks by several calls of this function.
 *  Unfortunately, the PostScript specification wants error messages also to be sent to stdout
 *  instead of stderr. Thus, we must collect and concatenate the chunks until an evaluable text
 *  snippet is completely received. Furthermore, error messages have to be recognized and to be
 *  filtered out.
 *  @param[in] inst pointer to calling instance of PSInterpreter
 *  @param[in] buf contains the characters to be output
 *  @param[in] len number of characters in buf
 *  @return number of processed characters (equals 'len') */
int GSDLLCALL PSInterpreter::output (void *inst, const char *buf, int len) {
	PSInterpreter *self = static_cast<PSInterpreter*>(inst);
	if (self && self->_actions) {
		const size_t MAXLEN = 512;    // maximal line length (longer lines are of no interest)
		const char *end = buf+len-1;  // last position of buf
		for (const char *first=buf, *last=buf; first <= end; last++, first=last) {
			// move first and last to begin and end of the next line, respectively
			while (last < end && *last != '\n')
				last++;
			size_t linelength = last-first+1;
			if (linelength > MAXLEN)  // skip long lines since they don't contain any relevant information
				continue;

			vector<char> &linebuf = self->_linebuf;  // just a shorter name...
			if ((*last == '\n' || !self->active()) || self->_inError) {
				if (linelength + linebuf.size() > 1) {  // prefix "dvi." plus final newline
					SplittedCharInputBuffer ib(linebuf.empty() ? 0 : &linebuf[0], linebuf.size(), first, linelength);
					BufferInputReader in(ib);
					if (self->_inError)
						self->_errorMessage += string(first, linelength);
					else {
						in.skipSpace();
						if (in.check("Unrecoverable error: ")) {
							self->_errorMessage.clear();
							while (!in.eof())
								self->_errorMessage += in.get();
							self->_inError = true;
						}
						else if (in.check("dvi."))
							self->callActions(in);
					}
				}
				linebuf.clear();
			}
			else { // no line end found =>
				// save remaining characters and prepend them to the next incoming chunk of characters
				if (linebuf.size() + linelength > MAXLEN)
					linebuf.clear();   // don't care for long lines
				else {
					size_t currsize = linebuf.size();
					linebuf.resize(currsize+linelength);
					memcpy(&linebuf[currsize], first, linelength);
				}
			}
		}
	}
	return len;
}


/** Evaluates a command emitted by Ghostscript and invokes the corresponding
 *  method of interface class PSActions.
 *  @param[in] in reader pointing to the next command */
void PSInterpreter::callActions (InputReader &in) {
	struct Operator {
		int pcount;       // number of parameters (< 0 : variable number of parameters)
		void (PSActions::*handler)(vector<double> &p);  // operation handler
	};
	static const unordered_map<string, Operator> operators {
		{"applyscalevals", { 3, &PSActions::applyscalevals}},
		{"clip",           { 0, &PSActions::clip}},
		{"clippath",       { 0, &PSActions::clippath}},
		{"closepath",      { 0, &PSActions::closepath}},
		{"curveto",        { 6, &PSActions::curveto}},
		{"eoclip",         { 0, &PSActions::eoclip}},
		{"eofill",         { 0, &PSActions::eofill}},
		{"fill",           { 0, &PSActions::fill}},
		{"grestore",       { 0, &PSActions::grestore}},
		{"grestoreall",    { 0, &PSActions::grestoreall}},
		{"gsave",          { 0, &PSActions::gsave}},
		{"initclip",       { 0, &PSActions::initclip}},
		{"lineto",         { 2, &PSActions::lineto}},
		{"makepattern",    {-1, &PSActions::makepattern}},
		{"moveto",         { 2, &PSActions::moveto}},
		{"newpath",        { 1, &PSActions::newpath}},
		{"querypos",       { 2, &PSActions::querypos}},
		{"raw",            {-1, nullptr}},
		{"restore",        { 1, &PSActions::restore}},
		{"rotate",         { 1, &PSActions::rotate}},
		{"save",           { 1, &PSActions::save}},
		{"scale",          { 2, &PSActions::scale}},
		{"setcmykcolor",   { 4, &PSActions::setcmykcolor}},
		{"setdash",        {-1, &PSActions::setdash}},
		{"setgray",        { 1, &PSActions::setgray}},
		{"sethsbcolor",    { 3, &PSActions::sethsbcolor}},
		{"setlinecap",     { 1, &PSActions::setlinecap}},
		{"setlinejoin",    { 1, &PSActions::setlinejoin}},
		{"setlinewidth",   { 1, &PSActions::setlinewidth}},
		{"setmatrix",      { 6, &PSActions::setmatrix}},
		{"setmiterlimit",  { 1, &PSActions::setmiterlimit}},
		{"setopacityalpha",{ 1, &PSActions::setopacityalpha}},
		{"setpagedevice",  { 0, &PSActions::setpagedevice}},
		{"setpattern",     {-1, &PSActions::setpattern}},
		{"setrgbcolor",    { 3, &PSActions::setrgbcolor}},
		{"shfill",         {-1, &PSActions::shfill}},
		{"stroke",         { 0, &PSActions::stroke}},
		{"translate",      { 2, &PSActions::translate}},
	};
	if (_actions) {
		in.skipSpace();
		auto it = operators.find(in.getWord());
		if (it != operators.end()) {
			if (!it->second.handler) { // raw string data received?
				_rawData.clear();
				in.skipSpace();
				while (!in.eof()) {
					_rawData.emplace_back(in.getString());
					in.skipSpace();
				}
			}
			else {
				// collect parameters
				vector<string> params;
				int pcount = it->second.pcount;
				if (pcount < 0) {       // variable number of parameters?
					in.skipSpace();
					while (!in.eof()) {  // read all available parameters
						params.emplace_back(in.getString());
						in.skipSpace();
					}
				}
				else {   // fix number of parameters
					for (int i=0; i < pcount; i++) {
						in.skipSpace();
						params.emplace_back(in.getString());
					}
				}
				// convert parameter strings to doubles
				vector<double> v(params.size());
				transform(params.begin(), params.end(), v.begin(), [](const string &str) {
					return stod(str);
				});
				// call operator handler
				(_actions->*it->second.handler)(v);
				_actions->executed();
			}
		}
	}
}


/** This callback function handles output from Ghostscript to stderr.
 *  @param[in] inst pointer to calling instance of PSInterpreter
 *  @param[in] buf contains the characters to be output
 *  @param[in] len number of chars in buf
 *  @return number of processed characters */
int GSDLLCALL PSInterpreter::error (void *inst, const char *buf, int len) {
	return len;
}

