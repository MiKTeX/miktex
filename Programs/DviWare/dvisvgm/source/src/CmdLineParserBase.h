/*************************************************************************
** CmdLineParserBase.h                                                  **
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

#ifndef DVISVGM_CMDLINEPARSERBASE_H
#define DVISVGM_CMDLINEPARSERBASE_H

#include <string>
#include <vector>

class InputReader;

class CmdLineParserBase
{
	protected:
		struct Option;

		struct OptionHandler {
			virtual ~OptionHandler () {}
			virtual void operator () (CmdLineParserBase *obj, InputReader &ir, const Option &opt, bool longopt) const=0;
		};

		template <typename T>
		class OptionHandlerImpl : public OptionHandler {
			protected:
				typedef void (T::*LocalHandler)(InputReader &ir, const Option &opt, bool longopt);

			public:
				OptionHandlerImpl (LocalHandler handler) : _handler(handler) {}

				void operator () (CmdLineParserBase *obj, InputReader &ir, const Option &opt, bool longopt) const {
					if (T *tobj = dynamic_cast<T*>(obj))
						(tobj->*_handler)(ir, opt, longopt);
				}

			private:
				LocalHandler _handler;
		};

		enum ArgMode {ARG_NONE, ARG_OPTIONAL, ARG_REQUIRED};

		struct Option {
			~Option () {delete handler;}
			char shortname;
			const char *longname;
			ArgMode argmode;  // mode of option argument
			const OptionHandler *handler;
		};

	public:
		virtual void parse (int argc, char **argv, bool printErrors=true);
		virtual void help (int mode=0, void (*out)(const char*)=0) const;
		virtual int numFiles () const       {return _files.size();}
		virtual const char* file (size_t n) {return n < _files.size() ? _files[n].c_str() : 0;}
//		virtual void status () const;
		virtual bool error () const         {return _error;}

	protected:
		CmdLineParserBase () : _printErrors(true), _error(false) {}
		CmdLineParserBase (const CmdLineParserBase &cmd) : _printErrors(true), _error(false) {}
		virtual ~CmdLineParserBase () {}
		virtual void init ();
		virtual void error (const Option &opt, bool longopt, const char *msg) const;
		virtual const Option* options (size_t *numopts) const =0;
		virtual const char** helplines (size_t *numlines) const =0;
		bool checkArgPrefix (InputReader &ir, const Option &opt, bool longopt) const;
		bool checkNoArg (InputReader &ir, const Option &opt, bool longopt) const;
		bool getIntArg (InputReader &ir, const Option &opt, bool longopt, int &arg) const;
		bool getUIntArg (InputReader &ir, const Option &opt, bool longopt, unsigned &arg) const;
		bool getDoubleArg (InputReader &ir, const Option &opt, bool longopt, double &arg) const;
		bool getStringArg (InputReader &ir, const Option &opt, bool longopt, std::string &arg) const;
		bool getBoolArg (InputReader &ir, const Option &opt, bool longopt, bool &arg) const;
		bool getCharArg (InputReader &ir, const Option &opt, bool longopt, char &arg) const;
		const Option* option (char shortname) const;
		const Option* option (const std::string &longname) const;

	private:
		bool _printErrors;    ///< if true, print error messages
		mutable bool _error;  ///< error occured while parsing options
		std::vector<std::string> _files;  ///< filename parameters
};

#endif
