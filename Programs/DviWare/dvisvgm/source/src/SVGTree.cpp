/*************************************************************************
** SVGTree.cpp                                                          **
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
#include <algorithm>
#include <sstream>
#include "BoundingBox.h"
#include "DependencyGraph.h"
#include "DVIToSVG.h"
#include "Font.h"
#include "FontManager.h"
#include "SVGTree.h"
#include "XMLDocument.h"
#include "XMLNode.h"
#include "XMLString.h"

using namespace std;


// static class variables
bool SVGTree::CREATE_STYLE=true;
bool SVGTree::USE_FONTS=true;
bool SVGTree::CREATE_USE_ELEMENTS=false;
bool SVGTree::RELATIVE_PATH_CMDS=false;
bool SVGTree::MERGE_CHARS=true;
bool SVGTree::ADD_COMMENTS=false;
double SVGTree::ZOOM_FACTOR=1.0;


SVGTree::SVGTree () : _vertical(false), _font(0), _color(Color::BLACK), _matrix(1) {
	_xchanged = _ychanged = false;
	_fontnum = 0;
	reset();
}


/** Clears the SVG tree and initializes the root element. */
void SVGTree::reset () {
	_doc.clear();
	_root = new XMLElementNode("svg");
	_root->addAttribute("version", "1.1");
	_root->addAttribute("xmlns", "http://www.w3.org/2000/svg");
	_root->addAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
	_doc.setRootNode(_root);
	_page = _text = _span = _defs = 0;
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
	if (!_font.get() || _font.get()->color() == Color::BLACK)
		_color.set(c);
}


void SVGTree::setFont (int num, const Font *font) {
	_font.set(font);
	_fontnum = num;
	// set default color assigned to the font
	if (font->color() != Color::BLACK && _color.get() != font->color())
		_color.set(font->color());
}


/** Starts a new page.
 *  @param[in] pageno number of new page */
void SVGTree::newPage (int pageno) {
	_page = new XMLElementNode("g");
	if (pageno >= 0)
		_page->addAttribute("id", string("page")+XMLString(pageno));
	_root->append(_page);
	_text = _span = 0;
	while (!_pageContainerStack.empty())
		_pageContainerStack.pop();
}


void SVGTree::appendToDefs (XMLNode *node) {
	if (!_defs) {
		_defs = new XMLElementNode("defs");
		_root->prepend(_defs);
	}
	_defs->append(node);
}


void SVGTree::appendToPage (XMLNode *node) {
	if (_pageContainerStack.empty())
		_page->append(node);
	else
		_pageContainerStack.top()->append(node);
	if (node != _text) // if the appended node differs from the text element currently in use,
		_text = 0;      // then force creating a new text element for the following characters
}


void SVGTree::prependToPage (XMLNode *node) {
	if (_pageContainerStack.empty())
		_page->prepend(node);
	else
		_pageContainerStack.top()->prepend(node);
}


/** Appends a single charater to the current text node. If necessary, and depending on output mode
 *  and further output states, new XML elements (text, tspan, g, ...) are created.
 *  @param[in] c character to be added
 *  @param[in] x x coordinate
 *  @param[in] y y coordinate
 *  @param[in] font font to be used */
void SVGTree::appendChar (int c, double x, double y, const Font &font) {
	XMLElementNode *node=_span;
	if (USE_FONTS) {
		// changes of fonts and transformations require a new text element
		if (!MERGE_CHARS || !_text || _font.changed() || _matrix.changed() || _vertical.changed()) {
			newTextNode(x, y);
			node = _text;
			_color.changed(true);
		}
		if (MERGE_CHARS && (_xchanged || _ychanged || (_color.changed() && _color.get() != Color::BLACK))) {
			// if drawing position was explicitly changed, create a new tspan element
			_span = new XMLElementNode("tspan");
			if (_xchanged) {
				if (_vertical) {
					// align glyphs designed for horizontal layout properly
					if (const PhysicalFont *pf = dynamic_cast<const PhysicalFont*>(_font.get()))
						if (!pf->getMetrics()->verticalLayout())
							x += pf->scaledAscent()/2.5; // move vertical baseline to the right by strikethrough offset
				}
				_span->addAttribute("x", x);
				_xchanged = false;
			}
			if (_ychanged) {
				_span->addAttribute("y", y);
				_ychanged = false;
			}
			if (_color.get() != font.color()) {
					_span->addAttribute("fill", _color.get().svgColorString());
				_color.changed(false);
			}
			_text->append(_span);
			node = _span;
		}
		if (!node) {
			if (!_text)
				newTextNode(x, y);
			node = _text;
		}
		node->append(XMLString(font.unicode(c), false));
		if (!MERGE_CHARS && _color.get() != font.color()) {
			node->addAttribute("fill", _color.get().svgColorString());
			_color.changed(false);
		}
	}
	else {
		if (_color.changed() || _matrix.changed()) {
			bool set_color = (_color.changed() && _color.get() != Color::BLACK);
			bool set_matrix = (_matrix.changed() && !_matrix.get().isIdentity());
			if (set_color || set_matrix) {
				_span = new XMLElementNode("g");
				if (_color.get() != Color::BLACK)
					_span->addAttribute("fill", _color.get().svgColorString());
				if (!_matrix.get().isIdentity())
					_span->addAttribute("transform", _matrix.get().getSVG());
				appendToPage(_span);
				node = _span;
				_color.changed(false);
				_matrix.changed(false);
			}
			else if (_color.get() == Color::BLACK && _matrix.get().isIdentity())
				node = _span = 0;
		}
		if (!node)
			node = _pageContainerStack.empty() ? _page : _pageContainerStack.top();
		if (font.verticalLayout()) {
			// move glyph graphics so that its origin is located at the top center position
			GlyphMetrics metrics;
			font.getGlyphMetrics(c, _vertical, metrics);
			x -= metrics.wl;
			if (const PhysicalFont *pf = dynamic_cast<const PhysicalFont*>(&font)) {
				// Center glyph between top and bottom border of the TFM box.
				// This is just an approximation used until I find a way to compute
				// the exact location in vertical mode.
				GlyphMetrics exact_metrics;
				pf->getExactGlyphBox(c, exact_metrics, false);
				y += exact_metrics.h+(metrics.d-exact_metrics.h-exact_metrics.d)/2;
			}
			else
				y += metrics.d;
		}
		Matrix rotation(1);
		if (_vertical && !font.verticalLayout()) {
			// alphabetic text designed for horizontal mode
			// must be rotated by 90 degrees if in vertical mode
			rotation.translate(-x, -y);
			rotation.rotate(90);
			rotation.translate(x, y);
		}
		if (CREATE_USE_ELEMENTS) {
			ostringstream oss;
			oss << "#g" << FontManager::instance().fontID(_font) << '-' << c;
			XMLElementNode *use = new XMLElementNode("use");
			use->addAttribute("x", XMLString(x));
			use->addAttribute("y", XMLString(y));
			use->addAttribute("xlink:href", oss.str());
			if (!rotation.isIdentity())
				use->addAttribute("transform", rotation.getSVG());
			node->append(use);
		}
		else {
			Glyph glyph;
			const PhysicalFont *pf = dynamic_cast<const PhysicalFont*>(&font);
			if (pf && pf->getGlyph(c, glyph)) {
				double sx = pf->scaledSize()/pf->unitsPerEm();
				double sy = -sx;
				ostringstream oss;
				glyph.writeSVG(oss, RELATIVE_PATH_CMDS, sx, sy, x, y);
				XMLElementNode *glyph_node = new XMLElementNode("path");
				glyph_node->addAttribute("d", oss.str());
				if (!rotation.isIdentity())
					glyph_node->addAttribute("transform", rotation.getSVG());
				node->append(glyph_node);
			}
		}
	}
}


/** Creates a new text element. This is a helper function used by appendChar().
 *  @param[in] x current x coordinate
 *  @param[in] y current y coordinate */
void SVGTree::newTextNode (double x, double y) {
	_text = new XMLElementNode("text");
	_span = 0; // no tspan in text element yet
	if (USE_FONTS) {
		const Font *font = _font.get();
		if (CREATE_STYLE || !font)
			_text->addAttribute("class", string("f")+XMLString(_fontnum));
		else {
			_text->addAttribute("font-family", font->name());
			_text->addAttribute("font-size", XMLString(font->scaledSize()));
			if (font->color() != Color::BLACK)
				_text->addAttribute("fill", font->color().svgColorString());
		}
		if (_vertical) {
			_text->addAttribute("writing-mode", "tb");
			// align glyphs designed for horizontal layout properly
			if (const PhysicalFont *pf = dynamic_cast<const PhysicalFont*>(_font.get()))
				if (!pf->getMetrics()->verticalLayout()) { // alphabetic text designed for horizontal layout?
					x += pf->scaledAscent()/2.5; // move vertical baseline to the right by strikethrough offset
					_text->addAttribute("glyph-orientation-vertical", 90); // ensure rotation
				}
		}
	}
	_text->addAttribute("x", x);
	_text->addAttribute("y", y);
	if (!_matrix.get().isIdentity())
		_text->addAttribute("transform", _matrix.get().getSVG());
	appendToPage(_text);
	_vertical.changed(false);
	_font.changed(false);
	_matrix.changed(false);
	_xchanged = false;
	_ychanged = false;
}


void SVGTree::transformPage (const Matrix *usermatrix) {
	if (usermatrix && !usermatrix->isIdentity())
		_page->addAttribute("transform", usermatrix->getSVG());
}


/** Creates an SVG element for a single glyph.
 *  @param[in] c character number
 *  @param[in] font font to extract the glyph from
 *  @param[in] cb pointer to callback object for sending feedback to the glyph tracer (may be 0)
 *  @return pointer to element node if glyph exists, 0 otherwise */
static XMLElementNode* createGlyphNode (int c, const PhysicalFont &font, GFGlyphTracer::Callback *cb) {
	Glyph glyph;
	if (!font.getGlyph(c, glyph, cb) || (!SVGTree::USE_FONTS && !SVGTree::CREATE_USE_ELEMENTS))
		return 0;

	double sx=1.0, sy=1.0;
	double upem = font.unitsPerEm();
	XMLElementNode *glyph_node=0;
	if (SVGTree::USE_FONTS) {
		double extend = font.style() ? font.style()->extend : 1;
		glyph_node = new XMLElementNode("glyph");
		glyph_node->addAttribute("unicode", XMLString(font.unicode(c), false));
		glyph_node->addAttribute("horiz-adv-x", XMLString(font.hAdvance(c)*extend));
		glyph_node->addAttribute("vert-adv-y", XMLString(font.vAdvance(c)));
		string name = font.glyphName(c);
		if (!name.empty())
			glyph_node->addAttribute("glyph-name", name);
	}
	else {
		ostringstream oss;
		oss << 'g' << FontManager::instance().fontID(&font) << '-' << c;
		glyph_node = new XMLElementNode("path");
		glyph_node->addAttribute("id", oss.str());
		sx = font.scaledSize()/upem;
		sy = -sx;
	}
	ostringstream oss;
	glyph.writeSVG(oss, SVGTree::RELATIVE_PATH_CMDS, sx, sy);
	glyph_node->addAttribute("d", oss.str());
	return glyph_node;
}


static string font_info (const Font &font) {
	ostringstream oss;
	if (const NativeFont *nf = dynamic_cast<const NativeFont*>(&font)) {
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


void SVGTree::appendFontStyles (const set<const Font*> &fonts) {
	if (CREATE_STYLE && USE_FONTS && !fonts.empty() && _defs) {
		XMLElementNode *styleNode = new XMLElementNode("style");
		styleNode->addAttribute("type", "text/css");
		_root->insertAfter(styleNode, _defs);
		typedef map<int, const Font*> SortMap;
		SortMap sortmap;
		FORALL(fonts, set<const Font*>::const_iterator, it)
			if (!dynamic_cast<const VirtualFont*>(*it))   // skip virtual fonts
				sortmap[FontManager::instance().fontID(*it)] = *it;
		ostringstream style;
		// add font style definitions in ascending order
		FORALL(sortmap, SortMap::const_iterator, it) {
			style << "text.f"     << it->first << ' '
				<< "{font-family:" << it->second->name()
				<< ";font-size:"   << XMLString(it->second->scaledSize()) << "px";
			if (it->second->color() != Color::BLACK)
				style << ";fill:" << it->second->color().svgColorString();
			style << '}';
			if (ADD_COMMENTS) {
				string info = font_info(*it->second);
				if (!info.empty())
					style << " /* " << info << " */";
			}
			style << '\n';
		}
		XMLCDataNode *cdata = new XMLCDataNode(style.str());
		styleNode->append(cdata);
	}
}


/** Appends glyph definitions of a given font to the defs section of the SVG tree.
 *  @param[in] font font to be appended
 *  @param[in] chars codes of the characters whose glyph outlines should be appended
 *  @param[in] cb pointer to callback object for sending feedback to the glyph tracer (may be 0) */
void SVGTree::append (const PhysicalFont &font, const set<int> &chars, GFGlyphTracer::Callback *cb) {
	if (chars.empty())
		return;

	if (USE_FONTS) {
		if (ADD_COMMENTS) {
			string info = font_info(font);
			if (!info.empty())
				appendToDefs(new XMLCommentNode(string(" font: ")+info+" "));
		}
		XMLElementNode *fontNode = new XMLElementNode("font");
		string fontname = font.name();
		fontNode->addAttribute("id", fontname);
		fontNode->addAttribute("horiz-adv-x", XMLString(font.hAdvance()));
		appendToDefs(fontNode);

		XMLElementNode *faceNode = new XMLElementNode("font-face");
		faceNode->addAttribute("font-family", fontname);
		faceNode->addAttribute("units-per-em", XMLString(font.unitsPerEm()));
		if (font.type() != PhysicalFont::MF && !font.verticalLayout()) {
			faceNode->addAttribute("ascent", XMLString(font.ascent()));
			faceNode->addAttribute("descent", XMLString(font.descent()));
		}
		fontNode->append(faceNode);
		FORALL(chars, set<int>::const_iterator, i)
			fontNode->append(createGlyphNode(*i, font, cb));
	}
	else if (CREATE_USE_ELEMENTS && &font != font.uniqueFont()) {
		// If the same character is used in various sizes, we don't want to embed the complete (lengthy) path
		// descriptions multiple times. Because they would only differ by a scale factor, it's better to
		// reference the already embedded path together with a transformation attribute and let the SVG renderer
		// scale the glyphs properly. This is only necessary if we don't want to use font but path elements.
		FORALL(chars, set<int>::const_iterator, it) {
			ostringstream oss;
			XMLElementNode *use = new XMLElementNode("use");
			oss << 'g' << FontManager::instance().fontID(&font) << '-' << *it;
			use->addAttribute("id", oss.str());
			oss.str("");
			oss << "#g" << FontManager::instance().fontID(font.uniqueFont()) << '-' << *it;
			use->addAttribute("xlink:href", oss.str());
			double scale = font.scaledSize()/font.uniqueFont()->scaledSize();
			if (scale != 1.0) {
				oss.str("");
				oss << "scale(" << scale << ')';
				use->addAttribute("transform", oss.str());
			}
			appendToDefs(use);
		}
	}
	else {
		FORALL(chars, set<int>::const_iterator, i)
			appendToDefs(createGlyphNode(*i, font, cb));
	}
}


/** Pushes a new context element that will take all following nodes added to the page. */
void SVGTree::pushContextElement (XMLElementNode *node) {
	if (_pageContainerStack.empty())
		_page->append(node);
	else
		_pageContainerStack.top()->append(node);
	_pageContainerStack.push(node);
	_text = _span = 0;  // ensure the creation of a new text element for the following characters added
}


/** Pops the current context element and restored the previous one. */
void SVGTree::popContextElement () {
	if (!_pageContainerStack.empty()) {
		_pageContainerStack.pop();
		_text = _span = 0; // ensure the creation of a new text element for the following characters added
	}
}


/** Extracts the ID from a local URL reference like url(#abcde) */
inline string extract_id_from_url (const string &url) {
	return url.substr(5, url.length()-6);
}


/** Removes elements present in the SVH tree that are not required.
 *  For now, only clipPath elements are removed. */
void SVGTree::removeRedundantElements () {
	vector<XMLElementNode*> clipElements;
	if (!_defs || !_defs->getDescendants("clipPath", 0, clipElements))
		return;

	// collect dependencies between clipPath elements in the defs section of the SVG tree
	DependencyGraph<string> idTree;
	for (vector<XMLElementNode*>::iterator it=clipElements.begin(); it != clipElements.end(); ++it) {
		if (const char *id = (*it)->getAttributeValue("id")) {
			if (const char *url = (*it)->getAttributeValue("clip-path"))
				idTree.insert(extract_id_from_url(url), id);
			else
				idTree.insert(id);
		}
	}
	// collect elements that reference a clipPath (have a clip-path attribute)
	vector<XMLElementNode*> descendants;
	_page->getDescendants(0, "clip-path", descendants);
	// remove referenced IDs and their dependencies from the dependency graph
	for (vector<XMLElementNode*>::iterator it=descendants.begin(); it != descendants.end(); ++it) {
		string idref = extract_id_from_url((*it)->getAttributeValue("clip-path"));
		idTree.removeDependencyPath(idref);
	}
	descendants.clear();
	vector<string> ids;
	idTree.getKeys(ids);
	for (vector<string>::iterator it=ids.begin(); it != ids.end(); ++it) {
		XMLElementNode *node = _defs->getFirstDescendant("clipPath", "id", it->c_str());
		_defs->remove(node);
	}
}

