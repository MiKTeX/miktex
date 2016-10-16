/*************************************************************************
** SignalHandler.hpp                                                    **
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

#ifndef SIGNALHANDLER_HPP
#define SIGNALHANDLER_HPP

#include <exception>

struct SignalException : public std::exception {
};


class SignalHandler
{
	public:
		~SignalHandler ();
		static SignalHandler& instance ();
		bool start ();
		void stop ();
		void check ();
		void trigger (bool notify);
		bool active () const {return _active;}

	protected:
		SignalHandler () : _active(false) {}
		static void callback (int signal);

	private:
		bool _active;       ///< true if listening for signals
		static bool _break; ///< true if signal has been caught
};

#endif
