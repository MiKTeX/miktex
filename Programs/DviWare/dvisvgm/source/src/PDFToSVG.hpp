/*************************************************************************
** PDFToSVG.hpp                                                         **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2024 Martin Gieseking <martin.gieseking@uos.de>   **
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
#include "PDFHandler.hpp"


class PDFToSVG : public ImageToSVG {
	public:
		PDFToSVG (const std::string &fname, SVGOutputBase &out);
		bool isSinglePageFormat() const override {return false;}
		int totalPageCount() const override;
		void convert (int pageno) override;

	protected:
		void checkGSAndFileFormat () override;
		bool imageIsValid () const override;
		std::string imageFormat () const override {return "PDF";}
		BoundingBox imageBBox () const override {return {};}
		std::string psSpecialCmd () const override {return "pdffile=";}

	private:
		mutable int _totalPageCount = -1;
		PDFHandler _pdfHandler;
		bool _useGS = true;
};

#endif
