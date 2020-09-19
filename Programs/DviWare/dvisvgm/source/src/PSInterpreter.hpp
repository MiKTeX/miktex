/*************************************************************************
** PSInterpreter.hpp                                                    **
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

#ifndef PSINTERPRETER_HPP
#define PSINTERPRETER_HPP

#include <cstring>
#include <istream>
#include <string>
#include <vector>
#include "BoundingBox.hpp"
#include "Ghostscript.hpp"
#include "InputReader.hpp"
#include "MessageException.hpp"


struct PSException : public MessageException {
	explicit PSException (const std::string &msg) : MessageException(msg) {}
};


/** This interface provides the template methods called by PSInterpreter when executing a PS snippet.
 *  Each method corresponds to a PostScript operator of the same name. */
struct PSActions {
	virtual ~PSActions () =default;
	virtual void applyscalevals (std::vector<double> &p) =0;
	virtual void clip (std::vector<double> &p) =0;
	virtual void clippath (std::vector<double> &p) =0;
	virtual void closepath (std::vector<double> &p) =0;
	virtual void curveto (std::vector<double> &p) =0;
	virtual void eoclip (std::vector<double> &p) =0;
	virtual void eofill (std::vector<double> &p) =0;
	virtual void fill (std::vector<double> &p) =0;
	virtual void gsave (std::vector<double> &p) =0;
	virtual void grestore (std::vector<double> &p) =0;
	virtual void grestoreall (std::vector<double> &p) =0;
	virtual void image (std::vector<double> &p) =0;
	virtual void initclip (std::vector<double> &p) =0;
	virtual void lineto (std::vector<double> &p) =0;
	virtual void makepattern (std::vector<double> &p) =0;
	virtual void moveto (std::vector<double> &p) =0;
	virtual void newpath (std::vector<double> &p) =0;
	virtual void querypos (std::vector<double> &p) =0;
	virtual void restore (std::vector<double> &p) =0;
	virtual void rotate (std::vector<double> &p) =0;
	virtual void save (std::vector<double> &p) =0;
	virtual void scale (std::vector<double> &p) =0;
	virtual void setblendmode (std::vector<double> &p) =0;
	virtual void setcolorspace (std::vector<double> &p) =0;
	virtual void setcmykcolor (std::vector<double> &cmyk) =0;
	virtual void setdash (std::vector<double> &p) =0;
	virtual void setgray (std::vector<double> &p) =0;
	virtual void sethsbcolor (std::vector<double> &hsb) =0;
	virtual void setlinecap (std::vector<double> &p) =0;
	virtual void setlinejoin (std::vector<double> &p) =0;
	virtual void setlinewidth (std::vector<double> &p) =0;
	virtual void setmatrix (std::vector<double> &p) =0;
	virtual void setmiterlimit (std::vector<double> &p) =0;
	virtual void setnulldevice (std::vector<double> &p) =0;
	virtual void setopacityalpha (std::vector<double> &p) =0;
	virtual void setshapealpha (std::vector<double> &p) =0;
	virtual void setpagedevice (std::vector<double> &p) =0;
	virtual void setpattern (std::vector<double> &p) =0;
	virtual void setrgbcolor (std::vector<double> &rgb) =0;
	virtual void shfill (std::vector<double> &p) =0;
	virtual void stroke (std::vector<double> &p) =0;
	virtual void translate (std::vector<double> &p) =0;
	virtual void executed () {}  // triggered if one of the above PS operators has been executed
};

class PSFilter;

struct PSDeviceInfo {
	std::string name;
	std::string description;
};

/** This class provides methods to execute chunks of PostScript code and calls
 *  several template methods on invocation of selected PS operators (see PSActions). */
class PSInterpreter {
	enum Mode {PS_NONE, PS_RUNNING, PS_QUIT};

	public:
		explicit PSInterpreter (PSActions *actions=nullptr);
		PSInterpreter (const PSInterpreter &psi) =delete;
		bool execute (const char *str, size_t len, bool flush=true);
		bool execute (const char *str, bool flush=true)        {return execute(str, std::strlen(str), flush);}
		bool execute (const std::string &str, bool flush=true) {return execute(str.c_str(), flush);}
		bool execute (std::istream &is, bool flush=true);
		bool executeRaw (const std::string &str, int n);
		bool active () const                   {return _mode != PS_QUIT;}
		void limit (size_t max_bytes)          {_bytesToRead = max_bytes;}
		void setFilter (PSFilter *filter)      {_filter = filter;}
		PSActions* setActions (PSActions *actions);
		int pdfPageCount (const std::string &fname);
		BoundingBox pdfPageBox (const std::string &fname, int pageno);
		const std::vector<std::string>& rawData () const {return _rawData;}
		bool setImageDevice (const std::string &deviceStr);
		static std::vector<PSDeviceInfo> getImageDeviceInfos ();
		static void listImageDeviceInfos (std::ostream &os);
		static bool imageDeviceKnown (std::string deviceStr);

	protected:
		void init ();
		// callback functions
		static int GSDLLCALL input (void *inst, char *buf, int len);
		static int GSDLLCALL output (void *inst, const char *buf, int len);
		static int GSDLLCALL error (void *inst, const char *buf, int len);

		void checkStatus (int status);
		void callActions (InputReader &cib);

	private:
		Ghostscript _gs;
		Mode _mode;                        ///< current execution mode
		PSActions *_actions=nullptr;       ///< actions to be performed
		PSFilter *_filter=nullptr;         ///< active filter used to process PS code
		size_t _bytesToRead=0;             ///< if > 0, maximal number of bytes to be processed by following calls of execute()
		std::vector<char> _linebuf;
		std::string _errorMessage;         ///< text of error message
		bool _inError=false;               ///< true if scanning error message
		bool _initialized=false;           ///< true if PSInterpreter has been completely initialized
		std::vector<std::string> _rawData; ///< raw data received
		static const char *PSDEFS;         ///< initial PostScript definitions
};

#endif
