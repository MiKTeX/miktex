/*************************************************************************
** SpecialManager.hpp                                                   **
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

#ifndef SPECIALMANAGER_HPP
#define SPECIALMANAGER_HPP

#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "SpecialHandler.hpp"

class SpecialActions;

class SpecialManager {
	private:
		using HandlerPool = std::vector<std::unique_ptr<SpecialHandler>>;
		using HandlerMap = std::unordered_map<std::string,SpecialHandler*>;

	public:
		SpecialManager (const SpecialManager &) =delete;
		static SpecialManager& instance ();
		void registerHandler (std::unique_ptr<SpecialHandler> handler);
		void registerHandlers (std::vector<std::unique_ptr<SpecialHandler>> &handlers, const char *ignorelist);
		void unregisterHandlers ();
		void preprocess (const std::string &special, SpecialActions &actions) const;
		bool process (const std::string &special, double dvi2bp, SpecialActions &actions) const;
		void notifyPreprocessingFinished () const;
		void notifyBeginPage (unsigned pageno, SpecialActions &actions) const;
		void notifyEndPage (unsigned pageno, SpecialActions &actions) const;
		void notifyPositionChange (double x, double y, SpecialActions &actions) const;
		void writeHandlerInfo (std::ostream &os) const;
		SpecialHandler* findHandlerByName (const std::string &name) const;

	protected:
		SpecialManager () =default;
		SpecialHandler* findHandlerByPrefix (const std::string &prefix) const;

	private:
		HandlerPool _handlerPool;      ///< stores pointers to all handlers
		HandlerMap _handlersByPrefix;  ///< pointers to handlers for corresponding prefixes
};

#endif
