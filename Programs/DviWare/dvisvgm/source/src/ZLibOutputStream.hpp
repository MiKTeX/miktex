/*************************************************************************
** ZLibOutputStream.hpp                                                 **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2019 Martin Gieseking <martin.gieseking@uos.de>   **
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

#include <fstream>
#include <ostream>
#include <vector>
#include <zlib.h>
#include "MessageException.hpp"

#ifdef _WIN32
#  include <fcntl.h>
#  include <io.h>
#endif

struct ZLibException : public MessageException {
	ZLibException (const std::string &msg) : MessageException(msg) {}
};

enum ZLibCompressionFormat {ZLIB_DEFLATE=0, ZLIB_GZIP=16};

class ZLibOutputBuffer : public std::streambuf {
	public:
		ZLibOutputBuffer () {}

		ZLibOutputBuffer (std::streambuf *sbuf, ZLibCompressionFormat format, int zipLevel) {
			open(sbuf, format, zipLevel);
		}

		~ZLibOutputBuffer () {
			close();
		}

		/** Opens the buffer for writing.
		 *  @param[in] sink stream buffer taking the compressed data
		 *  @param[in] format compression format (deflate or gzip)
		 *  @param[in] zipLevel compression level (1-9)
		 *  @return true if buffer is ready for writing */
		bool open (std::streambuf *sink, ZLibCompressionFormat format, int zipLevel) {
			if (sink) {
				_inbuf.reserve(4096);
				_zbuf.resize(4096);
				_zstream.zalloc = Z_NULL;
				_zstream.zfree = Z_NULL;
				_zstream.opaque = Z_NULL;
				zipLevel = std::max(1, std::min(9, zipLevel));
				if (deflateInit2(&_zstream, zipLevel, Z_DEFLATED, 15+format, 8, Z_DEFAULT_STRATEGY) != Z_OK)
					throw ZLibException("failed to initialize deflate compression");
				_sink = sink;
				_opened = true;
			}
			return _opened;
		}

		/** Flushes the remaining data, finishes the compression process, and
		 *  closes the buffer so that further output doesn't reach the sink. */
		void close () {
			close(true);
		}

		int_type overflow (int_type c) override {
			if (c == traits_type::eof()) {
				close();
			}
			else {
				if (_inbuf.size() == _inbuf.capacity())
					flush(Z_NO_FLUSH);
				_inbuf.push_back(c);
			}
			return c;
		}

		int sync () override {
			flush(Z_NO_FLUSH);
			return 0;
		}

	protected:
		/** Compresses the chunk of data present in the input buffer
		 *  and writes it to the assigned output stream.
		 *  @param[in] flushmode flush mode of deflate function (Z_NO_FLUSH or Z_FINISH)
		 *  @throws ZLibException if compression failed */
		void flush (int flushmode) {
			if (_opened) {
				_zstream.avail_in = _inbuf.size();
				_zstream.next_in = _inbuf.data();
				do {
					_zstream.avail_out = _zbuf.size();
					_zstream.next_out = _zbuf.data();
					int ret = deflate(&_zstream, flushmode);
					if (ret == Z_STREAM_ERROR) {
						close(false);
						throw ZLibException("stream error during data compression");
					}
					auto have = _zbuf.size()-_zstream.avail_out;
					_sink->sputn(reinterpret_cast<char*>(_zbuf.data()), have);
				} while (_zstream.avail_out == 0);
			}
			_inbuf.clear();
		}

		/** Closes the buffer so that further output doesn't reach the sink.
		 *  @param[in] finish if true, flushes the remaining data and finishes the compression process */
		void close (bool finish) {
			if (_opened) {
				if (finish)
					flush(Z_FINISH);
				deflateEnd(&_zstream);
				_sink = nullptr;
				_opened = false;
			}
		}

	private:
		z_stream _zstream;
		std::streambuf *_sink = nullptr;  ///< target buffer where the compressded data is flushed to
		std::vector<Bytef> _inbuf;  ///< buffer holding a chunk of data to be compressed
		std::vector<Bytef> _zbuf;   ///< buffer holding a chunk of compressed data
		bool _opened = false;       ///< true if ready to process the incoming data correctly
};


class ZLibOutputStream : private ZLibOutputBuffer, public std::ostream {
	public:
		ZLibOutputStream () : std::ostream(this) {}

		ZLibOutputStream (std::ostream &os, ZLibCompressionFormat format, int zipLevel)
			: ZLibOutputBuffer(os.rdbuf(), format, zipLevel), std::ostream(this) {}

		~ZLibOutputStream () {close();}

		bool open (std::ostream &os, ZLibCompressionFormat format, int zipLevel) {
			ZLibOutputBuffer::close();
			return ZLibOutputBuffer::open(os.rdbuf(), format, zipLevel);
		}

		void close () {
			ZLibOutputBuffer::close();
		}
};


class ZLibOutputFileStream : public ZLibOutputStream {
	public:
		ZLibOutputFileStream (const std::string &fname, ZLibCompressionFormat format, int zipLevel)
			: _ofs(fname, std::ios::binary)
		{
			if (_ofs) {
				if (_ofs.rdbuf())
					open(_ofs, format, zipLevel);
				else
					_ofs.close();
			}
		}

		~ZLibOutputFileStream () {close();}

	private:
		std::ofstream _ofs;
};

#endif
