/*************************************************************************
** Subfont.hpp                                                          **
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

#ifndef SUBFONT_HPP
#define SUBFONT_HPP

#include <istream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "MessageException.hpp"

class Subfont;


/** Represents a collection of subfont mappings as defined in a .sfd file, and
 *  encapsulates the evaluation of these files. */
class SubfontDefinition {
	using Subfonts = std::map<std::string, std::unique_ptr<Subfont>>;
	public:
		SubfontDefinition (const SubfontDefinition &sfd) =delete;
		static SubfontDefinition* lookup (const std::string &name);
//		int getIDs (std::vector<std::string> &ids) const;
		const std::string& name() const {return _sfname;}
		std::string filename() const    {return _sfname+".sfd";}
		Subfont* subfont (const std::string &id) const;
		int subfonts (std::vector<Subfont*> &sfs) const;
		const char* path () const;

	protected:
		SubfontDefinition (std::string name, const char *fpath);

	private:
		std::string _sfname; ///< name of subfont
		Subfonts _subfonts;  ///< all subfonts defined in the corresponding .sfd file
};


/** Represents a single subfont mapping defined in a SubfontDefinition (.sfd file). */
class Subfont {
	friend class SubfontDefinition;
	public:
		Subfont (const Subfont &sf) =delete;
		const std::string& id () const {return _id;}
		uint16_t decode (unsigned char c);

	protected:
		Subfont (SubfontDefinition &sfd, const std::string &id) : _sfd(sfd), _id(id), _mapping(0) {}
		bool read ();

	private:
		SubfontDefinition &_sfd;        ///< SubfontDefinition where this Subfont belongs to
		const std::string &_id;         ///< id of this subfont as specified in the .sfd file
		std::vector<uint16_t> _mapping; ///< the character mapping table with 256 entries
};


class SubfontException : public MessageException {
	public:
		SubfontException (const std::string &msg, std::string fname, int lineno=0)
			: MessageException(msg), _fname(std::move(fname)), _lineno(lineno) {}

		const char* filename () const {return _fname.c_str();}
		int lineno () const           {return _lineno;}

	private:
		std::string _fname;
		int _lineno;
};

#endif
