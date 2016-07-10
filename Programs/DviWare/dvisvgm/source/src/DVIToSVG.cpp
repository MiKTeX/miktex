/*************************************************************************
** DVIToSVG.cpp                                                         **
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
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <set>
#include <sstream>
#include "Calculator.h"
#include "DVIToSVG.h"
#include "DVIToSVGActions.h"
#include "Font.h"
#include "FontManager.h"
#include "GlyphTracerMessages.h"
#include "InputBuffer.h"
#include "InputReader.h"
#include "PageRanges.h"
#include "PageSize.h"
#include "PreScanDVIReader.h"
#include "SignalHandler.h"
#include "SVGOutput.h"

///////////////////////////////////
// special handlers

#include "BgColorSpecialHandler.h"
#include "ColorSpecialHandler.h"
#include "DvisvgmSpecialHandler.h"
#include "EmSpecialHandler.h"
#include "HtmlSpecialHandler.h"
#include "PapersizeSpecialHandler.h"
#include "PdfSpecialHandler.h"
#ifndef HAVE_LIBGS
	#include "NoPsSpecialHandler.h"
#endif
#ifndef DISABLE_GS
	#include "PsSpecialHandler.h"
#endif
#include "TpicSpecialHandler.h"

///////////////////////////////////

using namespace std;

/** 'a': trace all glyphs even if some of them are already cached
 *  'm': trace missing glyphs, i.e. glyphs not yet cached
 *   0 : only trace actually required glyphs */
char DVIToSVG::TRACE_MODE = 0;
bool DVIToSVG::COMPUTE_PROGRESS = false;


DVIToSVG::DVIToSVG (istream &is, SVGOutputBase &out) : DVIReader(is), _out(out)
{
	_pageHeight = _pageWidth = 0;
	_tx = _ty = 0;    // no cursor translation
	_pageByte = 0;
	_prevYPos = numeric_limits<double>::min();
	_actions = new DVIToSVGActions(*this, _svg);
}


DVIToSVG::~DVIToSVG () {
	delete _actions;
}


/** Starts the conversion process.
 *  @param[in] first number of first page to convert
 *  @param[in] last number of last page to convert
 *  @param[out] pageinfo (number of converted pages, number of total pages) */
void DVIToSVG::convert (unsigned first, unsigned last, pair<int,int> *pageinfo) {
	if (first > last)
		swap(first, last);
	if (first > numberOfPages()) {
		ostringstream oss;
		oss << "file contains only " << numberOfPages() << " page";
		if (numberOfPages() > 1)
			oss << 's';
		throw DVIException(oss.str());
	}
	last = min(last, numberOfPages());

	for (unsigned i=first; i <= last; ++i) {
		executePage(i);
		_svg.removeRedundantElements();
		embedFonts(_svg.rootNode());
		_svg.write(_out.getPageStream(currentPageNumber(), numberOfPages()));
		string fname = _out.filename(i, numberOfPages());
		Message::mstream(false, Message::MC_PAGE_WRITTEN) << "\noutput written to " << (fname.empty() ? "<stdout>" : fname) << '\n';
		_svg.reset();
		static_cast<DVIToSVGActions*>(_actions)->reset();
	}
	if (pageinfo) {
		pageinfo->first = last-first+1;
		pageinfo->second = numberOfPages();
	}
}


/** Convert DVI pages specified by a page range string.
 *  @param[in] rangestr string describing the pages to convert
 *  @param[out] pageinfo (number of converted pages, number of total pages) */
void DVIToSVG::convert (const string &rangestr, pair<int,int> *pageinfo) {
	PageRanges ranges;
	if (!ranges.parse(rangestr, numberOfPages()))
		throw MessageException("invalid page range format");

	Message::mstream(false, Message::MC_PAGE_NUMBER) << "pre-processing DVI file (format version "  << getDVIVersion() << ")\n";
	if (DVIToSVGActions *actions = dynamic_cast<DVIToSVGActions*>(_actions)) {
		PreScanDVIReader prescan(getInputStream(), actions);
		actions->setDVIReader(prescan);
		prescan.executeAllPages();
		actions->setDVIReader(*this);
		SpecialManager::instance().notifyPreprocessingFinished();
	}

	FORALL(ranges, PageRanges::ConstIterator, it)
		convert(it->first, it->second);
	if (pageinfo) {
		pageinfo->first = ranges.numberOfPages();
		pageinfo->second = numberOfPages();
	}
}


int DVIToSVG::executeCommand () {
	SignalHandler::instance().check();
	const streampos cmdpos = tell();
	int opcode = DVIReader::executeCommand();
	if (currState().v+_ty != _prevYPos) {
		_tx = _ty = 0;
		_prevYPos = currState().v;
	}
	if (COMPUTE_PROGRESS && inPage() && _actions) {
		size_t pagelen = numberOfPageBytes(currentPageNumber()-1);
		_pageByte += tell()-cmdpos;
		_actions->progress(_pageByte, pagelen);
	}
	return opcode;
}


/** This template method is called by parent class DVIReader before
 *  executing the BOP actions.
 *  @param[in] pageno physical page number (1 = first page)
 *  @param[in] c contains information about the page (page number etc.) */
void DVIToSVG::enterBeginPage (unsigned pageno, const vector<Int32> &c) {
	if (dynamic_cast<DVIToSVGActions*>(_actions)) {
		Message::mstream().indent(0);
		Message::mstream(false, Message::MC_PAGE_NUMBER) << "processing page " << pageno;
		if (pageno != (unsigned)c[0])  // Does page number shown on page differ from physical page number?
			Message::mstream(false) << " [" << c[0] << ']';
		Message::mstream().indent(1);
		_svg.appendToDoc(new XMLCommentNode(" This file was generated by dvisvgm " VERSION " "));
	}
}


/** This template method is called by DVIReader::cmdEop() after
 *  executing the EOP actions. */
void DVIToSVG::leaveEndPage (unsigned) {
	if (!dynamic_cast<DVIToSVGActions*>(_actions))
		return;

	// set bounding box and apply page transformations
	BoundingBox bbox = _actions->bbox();  // bounding box derived from the DVI commands executed
	if (_bboxFormatString == "min" || _bboxFormatString == "preview" || _bboxFormatString == "papersize") {
		Matrix matrix;
		getPageTransformation(matrix);
		bbox.transform(matrix);
	}
	else if (_bboxFormatString == "dvi") {
		// center page content
		double dx = (_pageWidth-bbox.width())/2;
		double dy = (_pageHeight-bbox.height())/2;
		bbox += BoundingBox(-dx, -dy, dx, dy);
	}
	else if (_bboxFormatString != "none") {
		istringstream iss(_bboxFormatString);
		StreamInputReader ir(iss);
		ir.skipSpace();
		if (isalpha(ir.peek())) {
			// set explicitly given page format
			PageSize size(_bboxFormatString);
			if (size.valid()) {
				// convention: DVI position (0,0) equals (1in, 1in) relative
				// to the upper left vertex of the page (see DVI specification)
				const double border = -72;
				bbox = BoundingBox(border, border, size.widthInBP()+border, size.heightInBP()+border);
			}
		}
		else { // set/modify bounding box by explicitly given values
			try {
				vector<Length> lengths;
				BoundingBox::extractLengths(_bboxFormatString, lengths);
				if (lengths.size() == 1 || lengths.size() == 2) {  // relative box size?
					// apply the page transformation and adjust the bbox afterwards
					Matrix matrix;
					getPageTransformation(matrix);
					bbox.transform(matrix);
				}
				bbox.set(lengths);
			}
			catch (const MessageException &e) {
			}
		}
	}
	if (bbox.width() == 0)
		Message::wstream(false) << "\npage is empty\n";
	if (_bboxFormatString != "none") {
		_svg.setBBox(bbox);
		const double bp2pt = 72.27/72;
		const double bp2mm = 25.4/72;
		Message::mstream(false) << '\n';
		Message::mstream(false, Message::MC_PAGE_SIZE) << "graphic size: " << XMLString(bbox.width()*bp2pt) << "pt"
			" x " << XMLString(bbox.height()*bp2pt) << "pt"
			" (" << XMLString(bbox.width()*bp2mm) << "mm"
			" x " << XMLString(bbox.height()*bp2mm) << "mm)";
		Message::mstream(false) << '\n';
	}
}


void DVIToSVG::getPageTransformation(Matrix &matrix) const {
	if (_transCmds.empty())
		matrix.set(1);  // unity matrix
	else {
		Calculator calc;
		if (_actions) {
			const double bp2pt = 72.27/72;
			BoundingBox &bbox = _actions->bbox();
			calc.setVariable("ux", bbox.minX()*bp2pt);
			calc.setVariable("uy", bbox.minY()*bp2pt);
			calc.setVariable("w",  bbox.width()*bp2pt);
			calc.setVariable("h",  bbox.height()*bp2pt);
		}
		calc.setVariable("pt", 1);
		calc.setVariable("in", 72.27);
		calc.setVariable("cm", 72.27/2.54);
		calc.setVariable("mm", 72.27/25.4);
		matrix.set(_transCmds, calc);
	}
}


static void collect_chars (map<const Font*, set<int> > &fm) {
	typedef const map<const Font*, set<int> > UsedCharsMap;
	FORALL(fm, UsedCharsMap::const_iterator, it) {
		if (it->first->uniqueFont() != it->first) {
			FORALL(it->second, set<int>::const_iterator, cit)
				fm[it->first->uniqueFont()].insert(*cit);
		}
	}
}


/** Adds the font information to the SVG tree.
 *  @param[in] svgElement the font nodes are added to this node */
void DVIToSVG::embedFonts (XMLElementNode *svgElement) {
	if (!svgElement)
		return;
	if (!_actions)  // no dvi actions => no chars written => no fonts to embed
		return;

	typedef map<const Font*, set<int> > UsedCharsMap;
	const DVIToSVGActions *svgActions = static_cast<DVIToSVGActions*>(_actions);
	UsedCharsMap &usedChars = svgActions->getUsedChars();

	collect_chars(usedChars);

	GlyphTracerMessages messages;
	set<const Font*> tracedFonts;  // collect unique fonts already traced
	FORALL(usedChars, UsedCharsMap::const_iterator, it) {
		const Font *font = it->first;
		if (const PhysicalFont *ph_font = dynamic_cast<const PhysicalFont*>(font)) {
			// Check if glyphs should be traced. Only trace the glyphs of unique fonts, i.e.
			// avoid retracing the same glyphs again if they are referenced in various sizes.
			if (TRACE_MODE != 0 && tracedFonts.find(ph_font->uniqueFont()) == tracedFonts.end()) {
				ph_font->traceAllGlyphs(TRACE_MODE == 'a', &messages);
				tracedFonts.insert(ph_font->uniqueFont());
			}
			if (font->path())  // does font file exist?
				_svg.append(*ph_font, it->second, &messages);
			else
				Message::wstream(true) << "can't embed font '" << font->name() << "'\n";
		}
		else
			Message::wstream(true) << "can't embed font '" << font->name() << "'\n";
	}
	_svg.appendFontStyles(svgActions->getUsedFonts());
}


/** Enables or disables processing of specials. If ignorelist == 0, all
 *  supported special handlers are loaded. To disable selected sets of specials,
 *  the corresponding prefixes can be given separated by non alpha-numeric characters,
 *  e.g. "color, ps, em" or "color: ps em" etc.
 *  A single "*" in the ignore list disables all specials.
 *  @param[in] ignorelist list of special prefixes to ignore
 *  @param[in] pswarning if true, shows warning about disabled PS support
 *  @return the SpecialManager that handles special statements */
void DVIToSVG::setProcessSpecials (const char *ignorelist, bool pswarning) {
	if (ignorelist && strcmp(ignorelist, "*") == 0) { // ignore all specials?
		SpecialManager::instance().unregisterHandlers();
	}
	else {
		// add special handlers
		SpecialHandler *handlers[] = {
			0,                           // placeholder for PsSpecialHandler
			new BgColorSpecialHandler,   // handles background color special
			new ColorSpecialHandler,     // handles color specials
			new DvisvgmSpecialHandler,   // handles raw SVG embeddings
			new EmSpecialHandler,        // handles emTeX specials
			new HtmlSpecialHandler,      // handles hyperref specials
			new PapersizeSpecialHandler, // handles papersize special
			new PdfSpecialHandler,       // handles pdf specials
			new TpicSpecialHandler,      // handles tpic specials
			0
		};
		SpecialHandler **p = handlers;
#ifndef DISABLE_GS
		if (Ghostscript().available())
			*p = new PsSpecialHandler;
		else
#endif
		{
#ifndef HAVE_LIBGS
			*p = new NoPsSpecialHandler; // dummy PS special handler that only prints warning messages
			if (pswarning) {
#ifdef DISABLE_GS
				Message::wstream() << "processing of PostScript specials has been disabled permanently\n";
#else
				Message::wstream() << "processing of PostScript specials is disabled (Ghostscript not found)\n";
#endif
			}
#endif
		}
		SpecialManager::instance().unregisterHandlers();
		SpecialManager::instance().registerHandlers(p, ignorelist);
	}
}


string DVIToSVG::getSVGFilename (unsigned pageno) const {
	return _out.filename(pageno, numberOfPages());
}


void DVIToSVG::moveRight (double dx) {
	DVIReader::moveRight(dx);
	if (_actions) {
		if (currState().d == WMODE_LR)
			_actions->moveToX(currState().h+_tx);
		else
			_actions->moveToY(currState().v+_ty);
	}
}


void DVIToSVG::moveDown (double dy) {
	DVIReader::moveDown(dy);
	if (_actions) {
		if (currState().d == WMODE_LR)
			_actions->moveToY(currState().v+_ty);
		else
			_actions->moveToX(currState().h+_tx);
	}
}


void DVIToSVG::dviPost (UInt16, UInt16 pages, UInt32 pw, UInt32 ph, UInt32, UInt32, UInt32, UInt32) {
	_pageHeight = ph; // height of tallest page in dvi units
	_pageWidth  = pw; // width of widest page in dvi units
}


void DVIToSVG::dviBop (const std::vector<Int32> &c, Int32) {
	_pageByte = 0;
	enterBeginPage(currentPageNumber(), c);
	if (_actions) {
		_actions->progress(0, 1);  // ensure that progress is called at 0%
		_actions->beginPage(currentPageNumber(), c);
	}
}


void DVIToSVG::dviEop () {
	if (_actions) {
		_actions->endPage(currentPageNumber());
		_pageByte = numberOfPageBytes(currentPageNumber()-1);
		_actions->progress(_pageByte, _pageByte); // ensure that progress is called at 100%
	}
	leaveEndPage(currentPageNumber());
}


void DVIToSVG::dviSetChar0 (UInt32 c, const Font *font) {
	if (_actions && !dynamic_cast<const VirtualFont*>(font))
		_actions->setChar(prevState().h+_tx, prevState().v+_ty, c, prevState().d != WMODE_LR, *font);
}


void DVIToSVG::dviSetChar (UInt32 c, const Font *font) {
	dviSetChar0(c, font);
}


void DVIToSVG::dviPutChar (UInt32 c, const Font *font) {
	dviSetChar0(c, font);
}


void DVIToSVG::dviSetRule (double height, double width) {
	if (_actions && height > 0 && width > 0)
		_actions->setRule(currState().h+_tx, currState().v+_ty, height, width);
}


void DVIToSVG::dviPutRule (double height, double width) {
	dviSetRule(height, width);
}


void DVIToSVG::dviPop () {
	if (_actions) {
		if (prevState().h != currState().h)
			_actions->moveToX(currState().h + _tx);
		if (prevState().v != currState().v)
			_actions->moveToY(currState().v + _ty);
		if (prevState().d != currState().d)
			_actions->setTextOrientation(currState().d != WMODE_LR);
	}
}


void DVIToSVG::dviFontNum (UInt32 fontnum, SetFontMode, const Font *font) {
	if (_actions && font && !dynamic_cast<const VirtualFont*>(font))
		_actions->setFont(FontManager::instance().fontID(fontnum), *font);  // all fonts get a recomputed ID
}


void DVIToSVG::dviDir (WritingMode dir) {
	if (_actions)
		_actions->setTextOrientation(dir != WMODE_LR);
}


void DVIToSVG::dviXXX (const std::string &str) {
	if (_actions)
		_actions->special(str, dvi2bp());
}


void DVIToSVG::dviXGlyphArray (std::vector<double> &dx, vector<double> &dy, vector<UInt16> &glyphs, const Font &font) {
	if (_actions) {
		for (size_t i=0; i < glyphs.size(); i++)
			_actions->setChar(prevState().h+dx[i]+_tx, prevState().v+dy[i]+_ty, glyphs[i], false, font);
	}
}


void DVIToSVG::dviXGlyphString (vector<double> &dx, vector<UInt16> &glyphs, const Font &font) {
	if (_actions) {
		for (size_t i=0; i < glyphs.size(); i++)
			_actions->setChar(prevState().h+dx[i]+_tx, prevState().v+_ty, glyphs[i], false, font);
	}
}


void DVIToSVG::dviXTextAndGlyphs (vector<double> &dx, vector<double> &dy, vector<UInt16>&, vector<UInt16> &glyphs, const Font &font) {
	dviXGlyphArray(dx, dy, glyphs, font);
}


