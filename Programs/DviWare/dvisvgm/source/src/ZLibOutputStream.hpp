/*************************************************************************
** ZLibOutputStream.hpp                                                 **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2018 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef ZLIBOUTPUTSTREAM_HPP
#define ZLIBOUTPUTSTREAM_HPP

#include <ostream>
#include <zlib.h>

class ZLibOutputStream;

class ZLibOutputBuffer : public std::streambuf
{
	friend class ZLibOutputStream;
	public:
		~ZLibOutputBuffer () {close();}

		/** Opens the buffer object for writing.
		 *  @param[in] fname name of gzip file
		 *  @param[in] zipLevel compression level (1-9)
		 *  @return true on success */
		bool open (const char *fname, int zipLevel) {
			if (_opened)
				return false;
			zipLevel = std::max(1, std::min(9, zipLevel));
			std::string modestr = "wb0";
			modestr[2] += zipLevel;
			_gzfile = gzopen(fname, modestr.c_str());
			return _opened = (_gzfile != nullptr);
		}

		/** Closes the buffer object. */
		bool close () {
			if (!_opened)
				return false;
			sync();
			_opened = false;
			return gzclose(_gzfile) == Z_OK;
		}

		int overflow (int c) override {
			if (!_opened)
				return EOF;
			if (c != EOF) {
				*pptr() = c;
				pbump(1);
			}
			return (flush() == EOF) ? EOF : c;
		}

		int sync () override {
			return (pptr() && pptr() > pbase() && flush() == EOF) ? -1 : 0;
		}

	protected:
		ZLibOutputBuffer () : _gzfile(nullptr), _opened(false) {
			setp(_buffer, _buffer+SIZE-1);
		}

		/** Forces to write the buffer data to the output file. */
		int flush () {
			int w = pptr()-pbase();
			if (gzwrite(_gzfile, pbase(), w) != w)
				return EOF;
			pbump(-w);
			return w;
		}

	private:
		static constexpr int SIZE = 512;
		gzFile _gzfile;
		char _buffer[SIZE];
		bool _opened;
};


class ZLibOutputStream : public virtual std::ios, public std::ostream
{
	public:
		ZLibOutputStream () : std::ostream(&_buf) {init(&_buf);}
		ZLibOutputStream (const char *fname, int zipLevel) : ZLibOutputStream() {open(fname, zipLevel);}
		ZLibOutputStream (const std::string &fname, int zipLevel) : ZLibOutputStream(fname.c_str(), zipLevel) {}
		~ZLibOutputStream () {close();}
		ZLibOutputBuffer* rdbuf() {return &_buf;}

		/** Opens the output stream for writing.
		 *  @param[in] fname name of gzip file
		 *  @param[in] zipLevel compression level (1-9) */
		void open (const char *name, int zipLevel) {
			if (!_buf.open(name, zipLevel))
				clear(rdstate()|std::ios::badbit);
		}

		void close () {
			if (!_buf.close())
				clear(rdstate()|std::ios::badbit);
		}

	private:
		ZLibOutputBuffer _buf;
};

#endif
