/*************************************************************************
** SpecialManager.cpp                                                   **
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

#include <iomanip>
#include <map>
#include <sstream>
#include "SpecialActions.hpp"
#include "SpecialHandler.hpp"
#include "SpecialManager.hpp"

using namespace std;

double SpecialActions::PROGRESSBAR_DELAY=1000;  // initial delay in seconds (values >= 1000 disable the progressbar)


SpecialManager& SpecialManager::instance() {
	static SpecialManager sm;
	return sm;
}


/** Remove all registered handlers. */
void SpecialManager::unregisterHandlers () {
	_handlerPool.clear();
	_handlersByPrefix.clear();
}


/** Registers a single special handler. This method doesn't check if a
 *  handler of the same class is already registered.
 *  @param[in] handler pointer to handler to be registered */
void SpecialManager::registerHandler (unique_ptr<SpecialHandler> handler) {
	if (handler) {
		// get array of prefixes this handler is responsible for
		for (const char *prefix : handler->prefixes())
			_handlersByPrefix[prefix] = handler.get();
		_handlerPool.emplace_back(std::move(handler));
	}
}


/** Registers several special handlers at once.
 *  If ignorelist == 0, all given handlers are registered. To exclude selected sets of
 *  specials, the corresponding names can be given separated by non alpha-numeric characters,
 *  e.g. "color, ps, em" or "color: ps em" etc.
 *  @param[in] handlers pointer to zero-terminated array of handlers to be registered
 *  @param[in] ignorelist list of special names to be ignored */
void SpecialManager::registerHandlers (vector<unique_ptr<SpecialHandler>> &handlers, const char *ignorelist) {
	if (handlers.empty())
		return;
	string ignorestr = ignorelist ? ignorelist : "";
	for (char &c : ignorestr)
		if (!isalnum(c))
			c = '%';
	ignorestr = "%"+ignorestr+"%";

	for (auto &handler : handlers)
		if (!handler->name() || ignorestr.find("%"+string(handler->name())+"%") == string::npos)
			registerHandler(std::move(handler));
}


/** Looks for a handler responsible for a given special prefix.
 *  @param[in] prefix the special prefix, e.g. "color" or "em"
 *  @return in case of success: pointer to handler, 0 otherwise */
SpecialHandler* SpecialManager::findHandlerByPrefix (const string &prefix) const {
	auto it = _handlersByPrefix.find(prefix);
	if (it != _handlersByPrefix.end())
		return it->second;
	return nullptr;
}


/** Looks for a handler with a given name.
 *  @param[in] name name of handler to look for, e.g. "papersize"
 *  @return in case of success: pointer to handler, 0 otherwise */
SpecialHandler* SpecialManager::findHandlerByName (const string &name) const {
	for (auto &handler : _handlerPool) {
		if (handler->name() == name)
			return handler.get();
	}
	return nullptr;
}


static string extract_prefix (istream &is) {
	int c;
	string prefix;
	while (isalnum(c=is.get()))
		prefix += char(c);
	if (ispunct(c)) // also add separation character to identifying prefix
		prefix += char(c);
	if (prefix == "ps:" && is.peek() == ':')
		prefix += char(is.get());
	return prefix;
}


void SpecialManager::preprocess (const string &special, SpecialActions &actions) const {
	istringstream iss(special);
	const string prefix = extract_prefix(iss);
	if (SpecialHandler *handler = findHandlerByPrefix(prefix))
		handler->preprocess(prefix, iss, actions);
}


/** Executes a special command.
 *  @param[in] special the special expression
 *  @param[in] dvi2bp factor to convert DVI units to PS points
 *  @param[in] actions actions the special handlers can perform
 *  @return true if the special could be processed successfully
 *  @throw SpecialException in case of errors during special processing */
bool SpecialManager::process (const string &special, double dvi2bp, SpecialActions &actions) const {
	istringstream iss(special);
	const string prefix = extract_prefix(iss);
	bool success=false;
	if (SpecialHandler *handler = findHandlerByPrefix(prefix)) {
		handler->setDviScaleFactor(dvi2bp);
		success = handler->process(prefix, iss, actions);
	}
	return success;
}


void SpecialManager::notifyPreprocessingFinished () const {
	for (auto &handler : _handlerPool)
		handler->dviPreprocessingFinished();
}


void SpecialManager::notifyBeginPage (unsigned pageno, SpecialActions &actions) const {
	for (auto &handler : _handlerPool)
		handler->dviBeginPage(pageno, actions);
}


void SpecialManager::notifyEndPage (unsigned pageno, SpecialActions &actions) const {
	for (auto &handler : _handlerPool)
		handler->dviEndPage(pageno, actions);
}


void SpecialManager::notifyPositionChange (double x, double y, SpecialActions &actions) const {
	for (auto &handler : _handlerPool)
		handler->dviMovedTo(x, y, actions);
}


void SpecialManager::writeHandlerInfo (ostream &os) const {
	ios::fmtflags osflags(os.flags());
	map<string,SpecialHandler*> sortmap;
	for (const auto &handler : _handlerPool)
		if (handler->name())
			sortmap[handler->name()] = handler.get();
	for (const auto &strhandlerpair : sortmap) {
		os << setw(10) << left << strhandlerpair.second->name() << ' ';
		if (strhandlerpair.second->info())
			os << strhandlerpair.second->info();
		os << endl;
	}
	os.flags(osflags);  // restore format flags
}
