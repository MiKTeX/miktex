/*************************************************************************
** PDFToSVG.cpp                                                         **
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

#if defined(MIKTEX)
#include <config.h>
#endif
#include <cstdlib>
#include "Message.hpp"
#include "PDFToSVG.hpp"
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/PathNameUtil>
#define EXPATH_(x) MiKTeX::Util::PathNameUtil::ToLengthExtendedPathName(x)
#endif

using namespace std;


PDFToSVG::PDFToSVG (const string &fname, SVGOutputBase &out) : ImageToSVG(fname, out) {
	_useGS = (gsVersion() > 0 && gsVersion() < 10010);
	if (const char *pdfproc = getenv("DVISVGM_PDF_PROC")) {
		if (strcmp(pdfproc, "gs") == 0)
			_useGS = true;
		else if (strcmp(pdfproc, "mutool") == 0)
			_useGS = false;
	}
	if (!_useGS)
		_pdfHandler.assignSVGTree(_svg);
}


void PDFToSVG::checkGSAndFileFormat () {
	if (_useGS)
		ImageToSVG::checkGSAndFileFormat();
	else {
		if (!PDFHandler::available()) {
			ostringstream oss;
			if (gsVersion() > 0) {
				oss << "To process PDF files, either Ghostscript < 10.01.0 or mutool is required.\n";
				oss << "The installed Ghostscript version " << Ghostscript().revisionstr() << " isn't supported.\n";
				throw MessageException(oss.str());
			}
		}
	}
	if (!imageIsValid())
		throw MessageException("invalid "+imageFormat()+" file");
}


/** Returns the total number of pages in the PDF file. */
int PDFToSVG::totalPageCount () const {
	if (_totalPageCount < 0) {
		if (_useGS)
			_totalPageCount = psInterpreter().pdfPageCount(filename());
		else
			_totalPageCount = PDFHandler::numberOfPages(filename());
		if (_totalPageCount < 1)
			throw MessageException("can't retrieve number of pages from file " + filename());
	}
	return _totalPageCount;
}


bool PDFToSVG::imageIsValid () const {
#if defined(MIKTEX_WINDOWS)
	ifstream ifs(EXPATH_(filename()));
#else
	ifstream ifs(filename());
#endif
	if (ifs) {
		char buf[16];
		ifs.getline(buf, 16);
		return std::strncmp(buf, "%PDF-1.", 7) == 0;
	}
	return false;
}


void PDFToSVG::convert (int pageno) {
	if (_useGS)
		ImageToSVG::convert(pageno);
	else {
		Message::mstream().indent(0);
		Message::mstream(false, Message::MC_PAGE_NUMBER) << "processing PDF file\n";
		Message::mstream().indent(1);
		_pdfHandler.convert(filename(), pageno);
		embed(_pdfHandler.bbox());
		writeSVG(pageno);
	}
}