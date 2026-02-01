/*************************************************************************
** PsSpecialHandlerProxy.cpp                                            **
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

#include <config.h>
#include <memory>
#include "Message.hpp"
#ifndef DISABLE_GS
	#include "PsSpecialHandler.hpp"
#endif
#ifndef HAVE_LIBGS
	#include "NoPsSpecialHandler.hpp"
#endif
#include "PsSpecialHandlerProxy.hpp"
#include "SpecialManager.hpp"
#include "utility.hpp"

using namespace std;

inline unique_ptr<SpecialHandler> createPsSpecialHandler () {
#ifndef DISABLE_GS
#ifndef HAVE_LIBGS
	if (Ghostscript().available())
#endif
		return util::make_unique<PsSpecialHandler>();
#endif
#ifndef HAVE_LIBGS
	return util::make_unique<NoPsSpecialHandler>();
#endif
}


/** Replaces this handler proxy with the actual PS special handler. */
SpecialHandler* PsSpecialHandlerProxy::replaceHandler () const {
	auto psSpecialHandler = createPsSpecialHandler();
	if (_pswarning) {
#ifdef DISABLE_GS
		Message::wstream() << "processing of PostScript specials was permanently disabled\n";
#else
		if (!psSpecialHandler->name())
			Message::wstream() << "processing of PostScript specials is disabled (Ghostscript not found)\n";
#endif
	}
	SpecialHandler *handlerPtr = psSpecialHandler.get();
	SpecialManager::instance().unregisterHandler(this);
	SpecialManager::instance().registerHandler(std::move(psSpecialHandler));
	return handlerPtr;
}


void PsSpecialHandlerProxy::preprocess (const string &prefix, istream &is, SpecialActions &actions) {
	replaceHandler()->preprocess(prefix, is, actions);
}


bool PsSpecialHandlerProxy::process (const string &prefix, istream &is, SpecialActions &actions) {
	return replaceHandler()->process(prefix, is, actions);
}


const char* PsSpecialHandlerProxy::info() const {
	return createPsSpecialHandler()->info();
}


vector<const char *> PsSpecialHandlerProxy::prefixes() const {
	vector<const char*> pfx {"header=", "pdffile=", "psfile=", "PSfile=", "ps:", "ps::", "!", "\"", "pst:", "PST:"};
	return pfx;
}
