/*************************************************************************
** PDFToSVG.hpp                                                         **
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

#ifndef PDFTOSVG_HPP
#define PDFTOSVG_HPP

#include <fstream>
#include "ImageToSVG.hpp"

class PsSpecialHandler;

class PDFToSVG : public ImageToSVG {
	public:
		PDFToSVG (const std::string &fname, SVGOutputBase &out) : ImageToSVG(fname, out) {}
		bool isSinglePageFormat() const override {return false;}

		/** Returns the total number of pages in the PDF file. */
		int totalPageCount() override {
			if (_totalPageCount < 0) {
				_totalPageCount = psInterpreter().pdfPageCount(filename());
				if (_totalPageCount < 1)
					throw MessageException("can't retrieve number of pages from file " + filename());
			}
			return _totalPageCount;
		}

	protected:
		bool imageIsValid () const override {
			std::ifstream ifs(filename());
			if (ifs) {
				char buf[16];
				ifs.getline(buf, 16);
				return std::strncmp(buf, "%PDF-1.", 7) == 0;
			}
			return false;
		}
		std::string imageFormat () const override {return "PDF";}
		BoundingBox imageBBox () const override {return BoundingBox();}
		std::string psSpecialCmd () const override {return "pdffile=";}

	private:
		mutable int _totalPageCount = -1;
};

#endif

