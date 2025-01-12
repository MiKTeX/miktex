/*************************************************************************
** FontMap.hpp                                                          **
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

#ifndef FONTMAP_HPP
#define FONTMAP_HPP

#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "FontStyle.hpp"
#include "MapLine.hpp"
#include "MessageException.hpp"

class FilePath;
struct FontEncoding;
class Subfont;

class FontMap {
	public:
		struct Entry {
			explicit Entry (const MapLine &mapline, Subfont *subfont=nullptr);
			Entry (const Entry &entry) =delete;
			Entry (Entry &&entry) =default;
			Entry& operator = (Entry &&entry) =default;
			std::string fontname; ///< target font name
			std::string encname;  ///< name of font encoding
			Subfont *subfont;
			int fontindex;        ///< index of font in multi-font file
			bool locked;
			FontStyle style;
		};

	public:
		enum class Mode {APPEND, REMOVE, REPLACE};
		enum class FirstIncludeMode {OFF, ACTIVE, DONE};

		static FontMap& instance ();
		bool read (const std::string &fname, Mode mode, std::vector<std::string> *includedFilesRef=nullptr);
		bool read (const std::string &fname, char modechar, std::vector<std::string> *includedFilesRef=nullptr);
		bool read (const std::string &fname_seq, bool warn=false);
		void readdir (const std::string &dirname);
		bool apply (const MapLine &mapline, Mode mode);
		bool apply (const MapLine &mapline, char modechar);
		bool append (const MapLine &mapline);
		bool replace (const MapLine &mapline);
		bool remove (const MapLine &mapline);
		void lockFont (const std::string &fontname);
		void clear (bool unlocked_only=false);
		std::ostream& write (std::ostream &os) const;
		const Entry* lookup (const std::string &fontname) const;

	protected:
		FontMap () =default;
		void include (std::string line, const FilePath &includingFile, std::vector<std::string> &includedFiles);
		void includefirst (std::string line, const FilePath &includingFile, std::vector<std::string> &includedFiles);

	private:
		std::unordered_map<std::string,std::unique_ptr<Entry>> _entries;
		FirstIncludeMode _firstincludeMode = FirstIncludeMode::OFF;
};

class FontMapException : public MapLineException {
	public:
		enum class Cause {UNSPECIFIED, FILE_ACCESS_ERROR};

		FontMapException (const std::string &msg, Cause cause) : MapLineException(msg), _cause(cause) {}
		explicit FontMapException (const std::string &msg) : FontMapException(msg, Cause::UNSPECIFIED) {}
		Cause cause () const {return _cause;}

	private:
		Cause _cause = FontMapException::Cause::UNSPECIFIED;
};

#endif
