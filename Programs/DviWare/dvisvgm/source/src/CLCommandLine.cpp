/*************************************************************************
** CLCommandLine.cpp                                                    **
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

#include <algorithm>
#include <cstring>
#include <unordered_map>
#include "CLCommandLine.hpp"
#include "version.hpp"

using namespace CL;
using namespace std;


CommandLine::CommandLine (const char *summary, const char *usage, const char *copyright)
	: _summary(summary), _usage(usage), _copyright(copyright)
{
}


void CommandLine::parse (int argc, char **argv) {
	bool filesOnly = false;
	for (int i=1; i < argc; i++) {
		istringstream iss(argv[i]);
		if (filesOnly || iss.peek() != '-')
			_files.emplace_back(argv[i]);
		else {
			iss.get();  // skip dash
			if (iss.peek() < 0)
				_singleDashParsed = true;
			else if (iss.peek() != '-')
				parseShortOption(iss, argc, argv, i);
			else {
				iss.get();             // skip dash
				if (iss.peek() == EOF) // "--" only, no following option name?
					filesOnly = true;   // treat all following options as filenames
				else
					parseLongOption(iss);
			}
		}
	}
}


static void error (const Option &option, bool shortname, const string &msg) {
	ostringstream oss;
	oss << "option ";
	if (shortname)
		oss << '-' << option.shortName();
	else
		oss << "--" << option.longName();
	oss << ": " << msg;
	throw CommandLineException(oss.str());
}


static void type_error (const Option &option, bool shortname) {
	ostringstream oss;
	switch (option.argMode()) {
		case Option::ArgMode::NONE:
			error(option, shortname, "no argument expected");
			break;
		case Option::ArgMode::OPTIONAL:
			oss << "optional argument ";
			if (!option.argName().empty())
				oss << "'" << option.argName() << "' ";
			oss << "must be of "+option.typeString()+" type";
			error(option, shortname, oss.str());
			break;
		default:
			oss << option.typeString()+" argument ";
			if (!option.argName().empty())
				oss << "'" << option.argName() << "' ";
			oss << "expected";
			error(option, shortname, oss.str());
	}
}


void CommandLine::parseShortOption (istringstream &iss, int argc, char **argv, int &argn) {
	bool combined = false;
	do {
		char shortname = static_cast<char>(iss.get());
		if (!isalnum(shortname))
			throw CommandLineException(string("syntax error: -")+shortname);
		if (Option *option = lookupOption(shortname)) {
			if (!combined || option->argMode() == Option::ArgMode::NONE) {
				if (option->argMode() == Option::ArgMode::REQUIRED && strlen(argv[argn]) == 2) { // required argument separated by whitespace?
					if (argn+1 < argc && argv[argn+1][0] != '-') {
						iss.clear();            // reset error flags
						iss.str(argv[++argn]);  // continue parsing with next command-line field
					}
				}
				if (!option->parse(iss, false))
					type_error(*option, true);
				if (option->argMode() == Option::ArgMode::NONE)
					combined = true;
			}
			else
				throw CommandLineException(string("option -")+ shortname + " must be given separately");
		}
		else
			throw CommandLineException(string("unknown option -")+shortname);
	}
	while (combined && !iss.eof());
}


void CommandLine::parseLongOption (istream &is) {
	string optname;
	while (isalnum(is.peek()) || (!optname.empty() && is.peek() == '-'))
		optname += char(is.get());
	if (optname.empty())
		throw CommandLineException("unexpected character '" + string(1, is.peek()) + "' after '--'");
	vector<Option*> options = lookupOption(optname);
	if (options.empty())
		throw CommandLineException("unknown option --"+optname);
	else if (options.size() == 1) {
		if (!options[0]->parse(is, true))
			type_error(*options[0], false);
	}
	else {  // is partially given option ambiguous?
		sort(options.begin(), options.end(), [](const Option *opt1, const Option *opt2) {
			return opt1->longName() < opt2->longName();
		});
		string msg = "option --" + optname + " is ambiguous (";
		for (const Option *opt : options) {
			if (opt != options[0])
				msg += ", ";
			msg += opt->longName();
		}
		msg += ')';
		throw CommandLineException(msg);
	}
}


/** Returns all options that match the given long name. */
vector<Option*> CommandLine::lookupOption (const string &optname) const {
	vector<Option*> matches;
	int len = optname.length();
	for (OptSectPair optsect : options()) {
		if (optsect.first->longName() == optname) {  // exact match?
			matches.clear();
			matches.push_back(optsect.first);
			break;
		}
		if (optsect.first->longName().substr(0, len) == optname)  // partial match?
			matches.push_back(optsect.first);
	}
	return matches;
}


/** Returns the option that match the given short name. */
Option* CommandLine::lookupOption (char optchar) const {
	auto it = find_if(options().begin(), options().end(), [=](const OptSectPair &optsect) {
		return optsect.first->shortName() == optchar;
	});
	return (it != options().end()) ? it->first : nullptr;
}


/** Prints help text including summary of options.
 *  @param[in] os output stream the help text is printed to
 *  @param[in] mode output mode (0=section, 1=sorted by short options, 2=sorted by long options) */
void CommandLine::help (ostream &os, int mode) const {
	os << PROGRAM_NAME << ' '<< PROGRAM_VERSION << "\n\n";
	os << _summary << "\n\n";
	// print usage info
	string usage = _usage;
	int count=0;
	while (!usage.empty()) {
		size_t pos = usage.find('\n');
		os << (count++ == 0 ? "Usage: " : "       ") << PROGRAM_NAME << ' ' << usage.substr(0, pos) << '\n';
		if (pos != string::npos)
			usage = usage.substr(pos+1);
		else
			usage.clear();
	}
	if (mode > 0)
		os << '\n';

	// compute width of first column of help output
	unordered_map<Option*, pair<string,string>> linecols;
	size_t col1width=0;
	for (const OptSectPair &ospair : options()) {
		size_t pos;
		string line = ospair.first->helpline();
		if ((pos = line.find('\t')) != string::npos) {
			linecols.emplace(ospair.first, pair<string,string>(line.substr(0, pos), line.substr(pos+1)));
			col1width = max(col1width, pos);
		}
	}
	if (mode > 0) {
		bool (*isless)(const OptSectPair&, const OptSectPair&) = [](const OptSectPair &p1, const OptSectPair &p2) {
			return p1.first->longName() < p2.first->longName();
		};
		if (mode == 1) {
			isless = [](const OptSectPair &p1, const OptSectPair &p2) {
				char c1 = p1.first->shortName(), c2 = p2.first->shortName();
				if (c1 == c2)
					return p1.first->longName() < p2.first->longName();
				if (tolower(c1) == tolower(c2))
					return c1 > c2;  // lower case before upper case letters
				return tolower(c1) < tolower(c2);
			};
		}
		sort(options().begin(), options().end(), isless);
	}

	// print summary of options
	col1width += 2;
	int sectno=-1;
	for (const OptSectPair &ospair : options()) {
		if (mode == 0 && ospair.second != sectno) {  // first option in current section?
			sectno = ospair.second;
			if (section(sectno))
				os << '\n' << section(sectno) << ":\n";
		}
		Option *opt = ospair.first;
		os << "  " << setw(col1width) << left << linecols[opt].first;
		os << setw(0) << linecols[opt].second << '\n';
	}
	os << '\n' << _copyright << '\n';
}
