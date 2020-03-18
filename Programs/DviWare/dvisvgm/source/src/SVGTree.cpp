/*************************************************************************
** SVGTree.cpp                                                          **
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

#include <algorithm>
#include <array>
#include <cstring>
#include <sstream>
#include "BoundingBox.hpp"
#include "DVIToSVG.hpp"
#include "FileSystem.hpp"
#include "Font.hpp"
#include "FontManager.hpp"
#include "FontWriter.hpp"
#include "SVGCharHandlerFactory.hpp"
#include "SVGTree.hpp"
#include "XMLDocument.hpp"
#include "XMLNode.hpp"
#include "XMLString.hpp"

using namespace std;

// static class variables
bool SVGTree::CREATE_CSS=true;
bool SVGTree::USE_FONTS=true;
FontWriter::FontFormat SVGTree::FONT_FORMAT = FontWriter::FontFormat::SVG;
bool SVGTree::CREATE_USE_ELEMENTS=false;
bool SVGTree::RELATIVE_PATH_CMDS=false;
bool SVGTree::MERGE_CHARS=true;
bool SVGTree::ADD_COMMENTS=false;
double SVGTree::ZOOM_FACTOR=1.0;


SVGTree::SVGTree () : _charHandler(SVGCharHandlerFactory::createHandler()) {
	reset();
}


/** Clears the SVG tree and initializes the root element. */
void SVGTree::reset () {
	_doc.clear();
	auto rootNode = util::make_unique<XMLElement>("svg");
	rootNode->addAttribute("version", "1.1");
	rootNode->addAttribute("xmlns", "http://www.w3.org/2000/svg");
	rootNode->addAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
	_root = rootNode.get();
	_doc.setRootNode(std::move(rootNode));
	_page = _defs = nullptr;
	_styleCDataNode = nullptr;
}


/** Sets the bounding box of the document.
 *  @param[in] bbox bounding box in PS point units */
void SVGTree::setBBox (const BoundingBox &bbox) {
	if (ZOOM_FACTOR >= 0) {
		_root->addAttribute("width", XMLString(bbox.width()*ZOOM_FACTOR)+"pt");
		_root->addAttribute("height", XMLString(bbox.height()*ZOOM_FACTOR)+"pt");
	}
	_root->addAttribute("viewBox", bbox.toSVGViewBox());
}


void SVGTree::setColor (const Color &c) {
	const Font *font = _charHandler->getFont();
	if (!font || font->color() == Color::BLACK)
		_charHandler->setColor(c);
}


void SVGTree::setFont (int num, const Font &font) {
	_charHandler->setFont(font, num);
	// set default color assigned to the font
	if (font.color() != Color::BLACK && getColor() != font.color())
		setColor(font.color());
}


bool SVGTree::setFontFormat (string formatstr) {
	size_t pos = formatstr.find(',');
	string opt;
	if (pos != string::npos) {
		opt = formatstr.substr(pos+1);
		formatstr = formatstr.substr(0, pos);
	}
	FontWriter::FontFormat format = FontWriter::toFontFormat(formatstr);
	if (format == FontWriter::FontFormat::UNKNOWN)
		return false;
	FONT_FORMAT = format;
	FontWriter::AUTOHINT_FONTS = (opt == "autohint" || opt == "ah");
	return true;
}


/** Starts a new page.
 *  @param[in] pageno number of new page */
void SVGTree::newPage (int pageno) {
	auto pageNode = util::make_unique<XMLElement>("g");
	if (pageno >= 0)
		pageNode->addAttribute("id", string("page")+XMLString(pageno));
	_charHandler->setInitialContextNode(pageNode.get());
	_page = pageNode.get();
	_root->append(std::move(pageNode));
	_defsContextStack = stack<XMLElement*>();
	_pageContextStack = stack<XMLElement*>();
}


void SVGTree::appendToDefs (unique_ptr<XMLNode> node) {
	if (!_defs) {
		auto defsNode = util::make_unique<XMLElement>("defs");
		_defs = defsNode.get();
		_root->prepend(std::move(defsNode));
	}
	XMLElement *parent = _defsContextStack.empty() ? _defs : _defsContextStack.top();
	parent->append(std::move(node));
}


void SVGTree::appendToPage (unique_ptr<XMLNode> node) {
	XMLElement *parent = _pageContextStack.empty() ? _page : _pageContextStack.top();
	parent->append(std::move(node));
	_charHandler->setInitialContextNode(parent);
}


void SVGTree::prependToPage (unique_ptr<XMLNode> node) {
	if (_pageContextStack.empty())
		_page->prepend(std::move(node));
	else
		_pageContextStack.top()->prepend(std::move(node));
}


void SVGTree::transformPage (const Matrix &usermatrix) {
	if (!usermatrix.isIdentity())
		_page->addAttribute("transform", usermatrix.toSVG());
}


/** Creates an SVG element for a single glyph.
 *  @param[in] c character number
 *  @param[in] font font to extract the glyph from
 *  @param[in] cb pointer to callback object for sending feedback to the glyph tracer (may be 0)
 *  @return pointer to element node if glyph exists, 0 otherwise */
static unique_ptr<XMLElement> createGlyphNode (int c, const PhysicalFont &font, GFGlyphTracer::Callback *cb) {
	Glyph glyph;
	if (!font.getGlyph(c, glyph, cb) || (!SVGTree::USE_FONTS && !SVGTree::CREATE_USE_ELEMENTS))
		return nullptr;

	double sx=1.0, sy=1.0;
	double upem = font.unitsPerEm();
	unique_ptr<XMLElement> glyphNode;
	if (SVGTree::USE_FONTS) {
		double extend = font.style() ? font.style()->extend : 1;
		glyphNode = util::make_unique<XMLElement>("glyph");
		glyphNode->addAttribute("unicode", XMLString(font.unicode(c), false));
		glyphNode->addAttribute("horiz-adv-x", XMLString(font.hAdvance(c)*extend));
		glyphNode->addAttribute("vert-adv-y", XMLString(font.vAdvance(c)));
		string name = font.glyphName(c);
		if (!name.empty())
			glyphNode->addAttribute("glyph-name", name);
	}
	else {
		glyphNode = util::make_unique<XMLElement>("path");
		glyphNode->addAttribute("id", "g"+to_string(FontManager::instance().fontID(&font))+"-"+to_string(c));
		sx = font.scaledSize()/upem;
		sy = -sx;
	}
	ostringstream oss;
	glyph.writeSVG(oss, SVGTree::RELATIVE_PATH_CMDS, sx, sy);
	glyphNode->addAttribute("d", oss.str());
	return glyphNode;
}


static string font_info (const Font &font) {
	ostringstream oss;
	if (auto nf = dynamic_cast<const NativeFont*>(&font)) {
		oss << nf->familyName() << ' ' << nf->styleName() << "; " << nf->filename();
		if (nf->style()) {
			if (nf->style()->bold != 0)
				oss << ", bold:" << XMLString(nf->style()->bold) << "pt";
			if (nf->style()->extend != 1)
				oss << ", extent:" << XMLString(nf->style()->extend);
			if (nf->style()->slant != 0)
				oss << ", slant:" << XMLString(nf->style()->slant);
		}
	}
	return oss.str();
}


void SVGTree::appendFontStyles (const unordered_set<const Font*> &fonts) {
	if (CREATE_CSS && USE_FONTS && !fonts.empty() && _page) {
		map<int, const Font*> sortmap;
		for (const Font *font : fonts)
			if (!dynamic_cast<const VirtualFont*>(font))   // skip virtual fonts
				sortmap[FontManager::instance().fontID(font)] = font;
		ostringstream style;
		// add font style definitions in ascending order
		for (auto &idfontpair : sortmap) {
			if (CREATE_CSS) {
				style << "text.f"     << idfontpair.first << ' '
					<< "{font-family:" << idfontpair.second->name()
					<< ";font-size:"   << XMLString(idfontpair.second->scaledSize()) << "px";
				if (idfontpair.second->color() != Color::BLACK)
					style << ";fill:" << idfontpair.second->color().svgColorString();
				style << '}';
				if (ADD_COMMENTS) {
					string info = font_info(*idfontpair.second);
					if (!info.empty())
						style << " /* " << info << " */";
				}
				style << '\n';
			}
		}
		styleCDataNode()->append(style.str());
	}
}


/** Appends glyph definitions of a given font to the defs section of the SVG tree.
 *  @param[in] font font to be appended
 *  @param[in] chars codes of the characters whose glyph outlines should be appended
 *  @param[in] callback pointer to callback object for sending feedback to the glyph tracer (may be 0) */
void SVGTree::append (const PhysicalFont &font, const set<int> &chars, GFGlyphTracer::Callback *callback) {
	if (chars.empty())
		return;

	if (USE_FONTS) {
		if (FONT_FORMAT != FontWriter::FontFormat::SVG) {
			ostringstream style;
			FontWriter fontWriter(font);
			if (fontWriter.writeCSSFontFace(FONT_FORMAT, chars, style, callback))
				styleCDataNode()->append(style.str());
		}
		else {
			if (ADD_COMMENTS) {
				string info = font_info(font);
				if (!info.empty())
					appendToDefs(util::make_unique<XMLComment>(string(" font: ")+info+" "));
			}
			auto fontNode = util::make_unique<XMLElement>("font");
			string fontname = font.name();
			fontNode->addAttribute("id", fontname);
			fontNode->addAttribute("horiz-adv-x", XMLString(font.hAdvance()));

			auto faceNode = util::make_unique<XMLElement>("font-face");
			faceNode->addAttribute("font-family", fontname);
			faceNode->addAttribute("units-per-em", XMLString(font.unitsPerEm()));
			if (!font.verticalLayout()) {
				faceNode->addAttribute("ascent", XMLString(font.ascent()));
				faceNode->addAttribute("descent", XMLString(font.descent()));
			}
			fontNode->append(std::move(faceNode));
			for (int c : chars)
				fontNode->append(createGlyphNode(c, font, callback));
			appendToDefs(std::move(fontNode));
		}
	}
	else if (CREATE_USE_ELEMENTS && &font != font.uniqueFont()) {
		// If the same character is used in various sizes, we don't want to embed the complete (lengthy) path
		// descriptions multiple times. Because they would only differ by a scale factor, it's better to
		// reference the already embedded path together with a transformation attribute and let the SVG renderer
		// scale the glyphs properly. This is only necessary if we don't want to use font but path elements.
		for (int c : chars) {
			auto useNode = util::make_unique<XMLElement>("use");
			useNode->addAttribute("id", "g"+to_string(FontManager::instance().fontID(&font))+"-"+to_string(c));
			useNode->addAttribute("xlink:href", "#g"+to_string(FontManager::instance().fontID(font.uniqueFont()))+"-"+to_string(c));
			double scale = font.scaledSize()/font.uniqueFont()->scaledSize();
			if (scale != 1.0)
				useNode->addAttribute("transform", "scale("+XMLString(scale)+")");
			appendToDefs(std::move(useNode));
		}
	}
	else {
		for (int c : chars)
			appendToDefs(createGlyphNode(c, font, callback));
	}
}


void SVGTree::pushDefsContext (unique_ptr<XMLElement> node) {
	XMLElement *nodePtr = node.get();
	if (_defsContextStack.empty())
		appendToDefs(std::move(node));
	else
		_defsContextStack.top()->append(std::move(node));
	_defsContextStack.push(nodePtr);
}


void SVGTree::popDefsContext () {
	if (!_defsContextStack.empty())
		_defsContextStack.pop();
}


/** Pushes a new context element that will take all following nodes added to the page. */
void SVGTree::pushPageContext (unique_ptr<XMLElement> node) {
	XMLElement *nodePtr = node.get();
	if (_pageContextStack.empty())
		_page->append(std::move(node));
	else
		_pageContextStack.top()->append(std::move(node));
	_pageContextStack.push(nodePtr);
	_charHandler->setInitialContextNode(nodePtr);
}


/** Pops the current context element and restored the previous one. */
void SVGTree::popPageContext () {
	if (!_pageContextStack.empty())
		_pageContextStack.pop();
	_charHandler->setInitialContextNode(_page);
}


XMLCData* SVGTree::styleCDataNode () {
	if (!_styleCDataNode) {
		auto styleNode = util::make_unique<XMLElement>("style");
		styleNode->addAttribute("type", "text/css");
		auto cdataNode = util::make_unique<XMLCData>();
		_styleCDataNode = cdataNode.get();
		styleNode->append(std::move(cdataNode));
		_root->insertBefore(std::move(styleNode), _page);
	}
	return _styleCDataNode;
}
