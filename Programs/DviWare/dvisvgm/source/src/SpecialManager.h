/*************************************************************************
** SpecialManager.h                                                     **
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

#ifndef DVISVGM_SPECIALMANAGER_H
#define DVISVGM_SPECIALMANAGER_H

#include <map>
#include <ostream>
#include <string>
#include <vector>
#include "SpecialHandler.h"

class SpecialActions;

class SpecialManager
{
	private:
		typedef std::vector<SpecialHandler*> HandlerPool;
		typedef std::map<std::string,SpecialHandler*> HandlerMap;
		typedef HandlerMap::iterator Iterator;
		typedef HandlerMap::const_iterator ConstIterator;

	public:
		~SpecialManager ();
		static SpecialManager& instance ();
		void registerHandler (SpecialHandler *handler);
		void registerHandlers (SpecialHandler **handlers, const char *ignorelist);
		void unregisterHandlers ();
		void preprocess (const std::string &special, SpecialActions &actions) const;
		bool process (const std::string &special, double dvi2bp, SpecialActions &actions) const;
		void notifyPreprocessingFinished () const;
		void notifyBeginPage (unsigned pageno, SpecialActions &actions) const;
		void notifyEndPage (unsigned pageno, SpecialActions &actions) const;
		void notifyPositionChange (double x, double y, SpecialActions &actions) const;
		void writeHandlerInfo (std::ostream &os) const;

	protected:
		SpecialManager () {}
		SpecialManager (const SpecialManager &) {}
		SpecialHandler* findHandler (const std::string &prefix) const;

	private:
		HandlerPool _pool;     ///< stores pointers to all handlers
		HandlerMap _handlers;  ///< pointers to handlers for corresponding prefixes
		std::vector<DVIPreprocessingListener*> _preprocListeners;
		std::vector<DVIBeginPageListener*> _beginPageListeners;
		std::vector<DVIEndPageListener*> _endPageListeners;
		std::vector<DVIPositionListener*> _positionListeners;
};

#endif
