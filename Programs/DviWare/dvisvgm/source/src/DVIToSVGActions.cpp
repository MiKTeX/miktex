/*************************************************************************
** DVIToSVGActions.cpp                                                  **
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

#include <cstring>
#include <ctime>
#include "BoundingBox.hpp"
#include "DVIToSVG.hpp"
#include "DVIToSVGActions.hpp"
#include "Font.hpp"
#include "FontManager.hpp"
#include "GlyphTracerMessages.hpp"
#include "System.hpp"
#include "utility.hpp"

using namespace std;


void DVIToSVGActions::reset() {
	_usedChars.clear();
	_usedFonts.clear();
	_bbox = BoundingBox();
	_currentFontNum = -1;
	_bgcolor = Color::TRANSPARENT;
}


/** Change current horizontal position.
 *  @param[in] x new horizontal position
 *  @param[in] forceSVGMove if true, creates an explicit position change in the SVG tree */
void DVIToSVGActions::moveToX (double x, bool forceSVGMove) {
	SpecialManager::instance().notifyPositionChange(getX(), getY(), *this);
	if (forceSVGMove)
		_svg.setX(x);
}


/** Change current vertical position.
 *  @param[in] y new vertical position
 *  @param[in] forceSVGMove if true, creates an explicit position change in the SVG tree */
void DVIToSVGActions::moveToY (double y, bool forceSVGMove) {
	SpecialManager::instance().notifyPositionChange(getX(), getY(), *this);
	if (forceSVGMove)
		_svg.setY(y);
}


FilePath DVIToSVGActions::getSVGFilePath (unsigned pageno) const {
	FilePath path;
	if (auto dvi2svg = dynamic_cast<DVIToSVG*>(_dvireader))
		path = dvi2svg->getSVGFilePath(pageno);
	return path;
}


string DVIToSVGActions::getBBoxFormatString () const {
	string boxstr;
	if (auto dvi2svg = dynamic_cast<DVIToSVG*>(_dvireader))
		boxstr = dvi2svg->getUserBBoxString();
	return boxstr;
}


/** This method is called when a "set char" command was found in the DVI file.
 *  It draws a character of the current font.
 *  @param[in] x horizontal position of left bounding box edge
 *  @param[in] y vertical position of the character's baseline
 *  @param[in] c character code relative to the current font
 *  @param[in] vertical true if we're in vertical mode
 *  @param[in] font font to be used */
void DVIToSVGActions::setChar (double x, double y, unsigned c, bool vertical, const Font &font) {
	if (_outputLocked)
		return;

	// If we use SVG fonts there is no need to record all font name/char/size combinations
	// because the SVG font mechanism handles this automatically. It's sufficient to
	// record font names and chars. The various font sizes can be ignored here.
	// For a given font object, Font::uniqueFont() returns the same unique font object for
	// all fonts with the same name.
	_usedChars[SVGTree::USE_FONTS ? font.uniqueFont() : &font].insert(c);

	// However, we record all required fonts
	_usedFonts.insert(&font);
	_svg.appendChar(c, x, y);

	static string fontname;
	GlyphTracerMessages callback(fontname != font.name(), false);
	fontname = font.name();

	GlyphMetrics metrics;
	font.getGlyphMetrics(c, vertical, metrics);
	auto pf = dynamic_cast<const PhysicalFont*>(&font);
	if (PhysicalFont::EXACT_BBOX && pf) {
		GlyphMetrics exact_metrics;
		pf->getExactGlyphBox(c, exact_metrics, vertical, &callback);
		if (vertical) {
			// move top of bbox to upper margin of glyph (just an approximation yet)
			y += (metrics.d-exact_metrics.h-exact_metrics.d)/2;
		}
		metrics = exact_metrics;
	}
	BoundingBox bbox(x-metrics.wl, y-metrics.h, x+metrics.wr, y+metrics.d);

	// update bounding box
	if (!getMatrix().isIdentity())
		bbox.transform(getMatrix());
	embed(bbox);
#if 0
	XMLElement *rect = new XMLElement("rect");
	rect->addAttribute("x", x-metrics.wl);
	rect->addAttribute("y", y-metrics.h);
	rect->addAttribute("width", metrics.wl+metrics.wr);
	rect->addAttribute("height", metrics.h+metrics.d);
	rect->addAttribute("fill", "none");
	rect->addAttribute("stroke", "red");
	rect->addAttribute("stroke-width", "0.5");
	_svg.appendToPage(rect);
	if (metrics.d > 0) {
		XMLElement *line = new XMLElement("line");
		line->addAttribute("x1", x-metrics.wl);
		line->addAttribute("y1", y);
		line->addAttribute("x2", x+metrics.wr);
		line->addAttribute("y2", y);
		line->addAttribute("stroke", "blue");
		line->addAttribute("stroke-width", "0.5");
		_svg.appendToPage(line);
	}
	if (metrics.wl > 0) {
		XMLElement *line = new XMLElement("line");
		line->addAttribute("x1", x);
		line->addAttribute("y1", y-metrics.h);
		line->addAttribute("x2", x);
		line->addAttribute("y2", y+metrics.d);
		line->addAttribute("stroke", "blue");
		line->addAttribute("stroke-width", "0.5");
		_svg.appendToPage(line);
	}
#endif
}


/** This method is called when a "set rule" or "put rule" command was found in the
 *  DVI file. It draws a solid unrotated rectangle.
 *  @param[in] x horizontal position of left edge
 *  @param[in] y vertical position of bottom(!) edge
 *  @param[in] height length of the vertical edges
 *  @param[in] width length of the horizontal edges */
void DVIToSVGActions::setRule (double x, double y, double height, double width) {
	if (_outputLocked)
		return;

	// (x,y) is the lower left corner of the rectangle
	auto rect = util::make_unique<XMLElement>("rect");
	rect->addAttribute("x", x);
	rect->addAttribute("y", y-height);
	rect->addAttribute("height", height);
	rect->addAttribute("width", width);
	if (!getMatrix().isIdentity())
		rect->addAttribute("transform", getMatrix().toSVG());
	if (getColor() != Color::BLACK)
		rect->addAttribute("fill", _svg.getColor().svgColorString());
	_svg.appendToPage(std::move(rect));

	// update bounding box
	BoundingBox bb(x, y-height, x+width, y);
	if (!getMatrix().isIdentity())
		bb.transform(getMatrix());
	embed(bb);
}


/** This method is called when a "set font" command was found in the DVI file. The
 *  font must be previously defined.
 *  @param[in] num unique number of the font in the DVI file (not necessarily equal to the DVI font number)
 *  @param[in] font pointer to the font object (always represents a physical font and never a virtual font) */
void DVIToSVGActions::setFont (int num, const Font &font) {
	_currentFontNum = num;
	_svg.setFont(num, font);
}


/** This method is called when a "special" command was found in the DVI file.
 *  @param[in] spc the special expression
 *  @param[in] dvi2bp factor to scale DVI units to PS points
 *  @param[in] preprocessing if true, the DVI file is being pre-processed */
void DVIToSVGActions::special (const string &spc, double dvi2bp, bool preprocessing) {
	try {
		if (preprocessing)
			SpecialManager::instance().preprocess(spc, *this);
		else
			SpecialManager::instance().process(spc, dvi2bp, *this);
		// @@ output message in case of unsupported specials?
	}
	catch (const SpecialException &e) {
		Message::estream(true) << "error in special: " << e.what() << '\n';
	}
}


/** This method is called when a "begin of page (bop)" command was found in the DVI file.
 *  @param[in] pageno physical page number
 *  @param[in] c array with 10 components representing \\count0 ... \\count9. c[0] contains the
 *               current (printed) page number (may differ from page count) */
void DVIToSVGActions::beginPage (unsigned pageno, const vector<int32_t>&) {
	SpecialManager::instance().notifyBeginPage(pageno, *this);
	_svg.newPage(++_pageCount);
	_bbox = BoundingBox();  // clear bounding box
	_boxes.clear();
}


/** This method is called when an "end of page (eop)" command was found in the DVI file. */
void DVIToSVGActions::endPage (unsigned pageno) {
	SpecialManager::instance().notifyEndPage(pageno, *this);
	Matrix matrix = _dvireader->getPageTransformation();
	_svg.transformPage(matrix);
	if (_bgcolor != Color::TRANSPARENT) {
		// create a rectangle filled with the background color
		auto rect = util::make_unique<XMLElement>("rect");
		rect->addAttribute("x", _bbox.minX());
		rect->addAttribute("y", _bbox.minY());
		rect->addAttribute("width", _bbox.width());
		rect->addAttribute("height", _bbox.height());
		rect->addAttribute("fill", _bgcolor.svgColorString());
		_svg.prependToPage(std::move(rect));
	}
}


void DVIToSVGActions::setBgColor (const Color &color) {
	_bgcolor = color;
}


void DVIToSVGActions::embed(const BoundingBox &bbox) {
	_bbox.embed(bbox);
	for (auto &strboxpair : _boxes)
		strboxpair.second.embed(bbox);
}


void DVIToSVGActions::embed(const DPair& p, double r) {
	if (r == 0)
		_bbox.embed(p);
	else
		_bbox.embed(p, r);
	for (auto &strboxpair : _boxes)
		strboxpair.second.embed(p, r);
}


BoundingBox& DVIToSVGActions::bbox(const string& name, bool reset) {
	BoundingBox &box = _boxes[name];
	if (reset)
		box = BoundingBox();
	return box;
}


/** This method is called by subprocesses like the PS handler when
 *  a computation step has finished. Since the total number of steps
 *  can't be determined in advance, we don't show a percent value but
 *  a rotating dash. */
void DVIToSVGActions::progress (const char *id) {
	if (PROGRESSBAR_DELAY < 1000) {
		static double time=System::time();
		// slow down updating the progress indicator to prevent flickering
		if (System::time() - time > 0.1) {
			progress(0, 0, id);
			time = System::time();
		}
	}
}


/** Returns the number of digits of a given integer. */
static int digits (int n) {
	if (n == 0)
		return 1;
	if (n > 0)
		return util::ilog10(n)+1;
	return util::ilog10(-n)+2;
}


/** Draws a simple progress indicator.
 *  @param[in] current current iteration step (of 'total' steps)
 *  @param[in] total total number of iteration steps
 *  @param[in] id ID of the subprocess providing the information */
void DVIToSVGActions::progress (size_t current, size_t total, const char *id) {
	static double time=0;
	static bool draw=false; // show progress indicator?
	static const char *prev_id=nullptr;
	if (current == 0 && total > 0) {
		time = System::time();
		draw = false;
		Message::mstream() << '\n';
	}
	// don't show the progress indicator before the given time has elapsed
	if (!draw && System::time()-time > PROGRESSBAR_DELAY) {
		draw = true;
		Terminal::cursor(false);
	}
	if (draw && (System::time() - time > 0.1 || (total > 0 && current == total) || prev_id != id)) {
		static int step = -1;   // >=0: rotating dash
		static size_t prev_current=0, prev_total=1;
		const char *tips = "-\\|/";
		if (total == 0) {
			current = prev_current;
			total = prev_total;
			step = (step+1) % 4;
		}
		else {
			prev_current = current;
			prev_total = total;
			step = -1;
		}
		// adapt length of progress indicator to terminal width
		int cols = Terminal::columns();
		int width = (cols == 0 || cols > 60) ? 50 : 49-60+cols;
		double factor = double(current)/double(total);
		int length = int(width*factor);
		Message::mstream(false, Message::MC_PROGRESS)
			<< '[' << string(length, '=')
			<< (factor < 1.0 ? (step < 0 ? ' ' : tips[step]) : '=')
			<< string(width-length, ' ')
			<< "] " << string(3-digits(int(100*factor)), ' ') << int(100*factor)
			<< "%\r";
		// overprint indicator when finished
		if (factor == 1.0) {
			Message::estream().clearline();
			Terminal::cursor(true);
		}
		time = System::time();
		prev_id = id;
	}
}
