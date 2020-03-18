/*************************************************************************
** SignalHandler.cpp                                                    **
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

#include <config.h>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include "SignalHandler.hpp"
#include "utility.hpp"

using namespace std;

volatile bool SignalHandler::_break = false;

/** Helper class that encapsulates the system-specific parts of
 *  registering and handling CTRL-C (SIGINT) events. */
class SignalHandler::Impl {
	using HandlerFunc = void (*)(int);
	public:
		Impl ();
		bool setSigintHandler (HandlerFunc handler);
		bool restoreSigintHandler ();

	private:
#ifdef HAVE_SIGACTION
		struct sigaction _currentSigaction;
		struct sigaction _origSigaction;
#else
		HandlerFunc _origHandlerFunc;
#endif
};


SignalHandler::SignalHandler ()
	: _impl(util::make_unique<SignalHandler::Impl>())
{
}


SignalHandler::~SignalHandler () {
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
		if (_impl->setSigintHandler(callback))
			_active = true;
	}
	return _active;
}


/** Stops listening for CTRL-C signals. */
void SignalHandler::stop () {
	if (_active) {
		_impl->restoreSigintHandler();
		_active = false;
	}
}


/** Checks for incoming signals and throws an exception if CTRL-C was caught.
 *  @throw SignalException */
void SignalHandler::check () {
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


// Prefer sigaction() to signal() due to its more reliable interface and behavior.
// Use signal() as fallback on systems that don't provide sigaction().

#ifdef HAVE_SIGACTION

SignalHandler::Impl::Impl () {
	_origSigaction.sa_handler = nullptr;
}


bool SignalHandler::Impl::setSigintHandler (HandlerFunc handler) {
	sigemptyset(&_currentSigaction.sa_mask);
	_currentSigaction.sa_handler = handler;
	_currentSigaction.sa_flags = SA_SIGINFO;
	return (sigaction(SIGINT, &_currentSigaction, &_origSigaction) == 0);
}


bool SignalHandler::Impl::restoreSigintHandler () {
	if (_origSigaction.sa_handler == nullptr)
		return false;
	return (sigaction(SIGINT, &_origSigaction, nullptr) == 0);
}

#else  // !HAVE_SIGACTION

SignalHandler::Impl::Impl () : _origHandlerFunc(SIG_ERR) {
}


bool SignalHandler::Impl::setSigintHandler (HandlerFunc handler) {
	_origHandlerFunc = signal(SIGINT, handler);
	return _origHandlerFunc != SIG_ERR;
}


bool SignalHandler::Impl::restoreSigintHandler () {
	if (_origHandlerFunc == SIG_ERR)
		return false;
	return (signal(SIGINT, _origHandlerFunc) != SIG_ERR);
}

#endif
