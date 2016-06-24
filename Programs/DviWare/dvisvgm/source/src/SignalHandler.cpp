/*************************************************************************
** SignalHandler.cpp                                                    **
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

#include <config.h>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include "SignalHandler.h"

using namespace std;

bool SignalHandler::_break = false;


SignalHandler::~SignalHandler() {
	stop();
}


/** Returns the singleton handler object. */
SignalHandler& SignalHandler::instance() {
	static SignalHandler handler;
	return handler;
}


/** Starts listening to CTRL-C signals.
 *  @return true if handler was activated. */
bool SignalHandler::start () {
	if (!_active) {
		_break = false;
		if (signal(SIGINT, SignalHandler::callback) != SIG_ERR) {
			_active = true;
			return true;
		}
	}
	return false;
}


/** Stops listening for CTRL-C signals. */
void SignalHandler::stop () {
	if (_active) {
		signal(SIGINT, SIG_DFL);
		_active = false;
	}
}


/** Checks for incoming signals and throws an exception if CTRL-C was caught.
 *  @throw SignalException */
void SignalHandler::check() {
	if (_break)
		throw SignalException();
}


void SignalHandler::trigger (bool notify) {
	_break = true;
	if (notify)
		check();
}


/** This function is called on CTRL-C signals. */
void SignalHandler::callback (int) {
	_break = true;
}

