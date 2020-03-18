/*************************************************************************
** Ghostscript.hpp                                                      **
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

#ifndef GHOSTSCRIPT_HPP
#define GHOSTSCRIPT_HPP

#include <config.h>
#include <string>

#if defined(DISABLE_GS)
	#include "iapi.h"
#elif defined(HAVE_LIBGS)
	#include <ghostscript/iapi.h>
#else
	#include "DLLoader.hpp"
	#include "iapi.h"
#endif

#if defined(_WIN32) && !defined(_Windows)
	#define _Windows
#endif

#if defined(DISABLE_GS)
struct Ghostscript {
	using Stdin  = int (GSDLLCALLPTR)(void *caller, char *buf, int len);
	using Stdout = int (GSDLLCALLPTR)(void *caller, const char *str, int len);
	using Stderr = int (GSDLLCALLPTR)(void *caller, const char *str, int len);

	Ghostscript () {}
	Ghostscript (int argc, const char **argv, void *caller=0) {}
	bool init (int argc, const char **argv, void *caller=0) {return false;}
	bool available () {return false;}
	bool revision (gsapi_revision_t *r) {return false;}
	int revision () {return 0;}
	std::string revisionstr () {return "";}
	int set_stdio (Stdin in, Stdout out, Stderr err) {return 0;}
	int run_string_begin (int user_errors, int *pexit_code) {return 0;}
	int run_string_continue (const char *str, unsigned int length, int user_errors, int *pexit_code) {return 0;}
	int run_string_end (int user_errors, int *pexit_code) {return 0;}
	int exit () {return 0;}
	const char* error_name (int code) {return 0;}
};

#else

/** Wrapper class of (a subset of) the Ghostscript API. */
class Ghostscript
#if !defined(HAVE_LIBGS)
: public DLLoader
#endif
{
	public:
		using Stdin  = int (GSDLLCALLPTR)(void *caller, char *buf, int len);
		using Stdout = int (GSDLLCALLPTR)(void *caller, const char *str, int len);
		using Stderr = int (GSDLLCALLPTR)(void *caller, const char *str, int len);

	public:
		Ghostscript ();
		Ghostscript (int argc, const char **argv, void *caller=nullptr);
		Ghostscript (const Ghostscript &gs) =delete;
		~Ghostscript ();
		bool init (int argc, const char **argv, void *caller=nullptr);
		bool available ();
		bool revision (gsapi_revision_t *r);
		int revision ();
		std::string revisionstr ();
		int set_stdio (Stdin in, Stdout out, Stderr err);
		int run_string_begin (int user_errors, int *pexit_code);
		int run_string_continue (const char *str, unsigned int length, int user_errors, int *pexit_code);
		int run_string_end (int user_errors, int *pexit_code);
		int exit ();
		const char* error_name (int code);

		static std::string LIBGS_NAME;

	protected:
		int init_with_args (int argc, char **argv);
		int new_instance (void **psinst, void *caller);
		void delete_instance ();

	private:
		void *_inst = nullptr; ///< Ghostscript handle needed to call the gsapi_foo functions
};

#endif  // DISABLE_GS

#endif
