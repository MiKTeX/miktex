/*************************************************************************
** PDFHandler.cpp                                                       **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2025 Martin Gieseking <martin.gieseking@uos.de>   **
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
#include <fstream>
#include <iterator>
#include <limits>
#include <sstream>
#include "FilePath.hpp"
#include "FileSystem.hpp"
#include "FontEngine.hpp"
#include "FontManager.hpp"
#include "GraphicsPath.hpp"
#include "Color.hpp"
#include "Matrix.hpp"
#include "Opacity.hpp"
#include "PDFHandler.hpp"
#include "Process.hpp"
#include "SVGElement.hpp"
#include "SVGTree.hpp"
#include "Unicode.hpp"
#include "XMLParser.hpp"
#include "XXHashFunction.hpp"
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/PathNameUtil>
#define EXPATH_(x) MiKTeX::Util::PathNameUtil::ToLengthExtendedPathName(x)
#endif

using namespace std;

template <typename T>
static T parse_value (const string &str) {
	T value{};
	istringstream iss(str);
	iss >> value;
	return value;
}

template<>
string parse_value (const string &str) {
	return str;
}

template<>
vector<double> parse_value (const string &str) {
	vector<double> vec;
	istringstream iss(str);
	while (iss) {
		double val;
		if (iss >> val)
			vec.push_back(val);
	}
	return vec;
}


istream& operator >> (istream &is, PDFHandler::ObjID &objId) {
	is >> objId.num;
	is >> objId.gen;
	is >> ws;
	if (is.peek() == 'R')
		is.get();
	return is;
}


template <typename T>
static vector<T> parse_pdf_array (const string &str) {
	vector<T> vec;
	istringstream iss(str);
	if (iss.get() == '[') {
		while (iss) {
			T val;
			if (iss >> val)
				vec.push_back(val);
		}
	}
	return vec;
}

template <typename T>
static map<string, T> parse_pdf_dict (const string &str) {
	map<string, T> dict;
	istringstream iss(str);
	if (iss.get() == '<' && iss.get() == '<') {
		string name;
		T val;
		while (iss) {
			iss >> ws >> name >> val;
			if (iss)
				dict.emplace(name, val);
		}
	}
	return dict;
}


template <typename T>
static T parse_attr_value (const XMLElement *elem, const string &attr) {
	T ret{};
	if (const char *value = elem->getAttributeValue(attr))
		ret = parse_value<T>(value);
	return ret;
}

template<>
Matrix parse_attr_value (const XMLElement *elem, const string &attr) {
	Matrix matrix(1);
	if (const char *val = elem->getAttributeValue(attr))
		matrix = Matrix(val);
	return matrix;
}

///////////////////////////////////////////////////////////////////////////////


PDFHandler::~PDFHandler () {
	finishFile();
}


string PDFHandler::mutoolVersion () {
	static string version;
	if (version.empty())
		version = mutool("-v", SearchPattern("([0-9.]+)", "$1"), true);
	return version;
}


/** Returns the total number of pages of a PDF file. */
int PDFHandler::numberOfPages (const string &fname) {
	return parse_value<int>(mtShow("\""+fname+"\"", "trailer/Root/Pages/Count"));
}


/** Converts a single page of a PDF file to SVG. If no context element is given,
 *  the SVG page contents are added to a page group element of the SVG tree.
 *  Otherwise, they are added to the context element which is not inserted into
 *  the SVG tree but returned for further processing.
 *  @param[in] fname PDF filename
 *  @param[in] pageno number of page to convert
 *  @param[in] context element the generated page contents are appended to
 *  @return the context element holding the page contents */
unique_ptr<SVGElement> PDFHandler::convert (const string &fname, int pageno, unique_ptr<SVGElement> context) {
	if (_fname != fname)
		initFile(fname);
	if (!_svg || pageno < 1 || pageno > _numPages)
		return context;
	initPage(pageno, std::move(context));

	// create trace XML and convert it to SVG
	XMLParser xmlParser;
	auto openfunc = std::bind(&PDFHandler::elementOpened, this, std::placeholders::_1);
	auto closefunc = std::bind(&PDFHandler::elementClosed, this, std::placeholders::_1);
	xmlParser.setNotifyFuncs(openfunc, closefunc);
	xmlParser.setRootElement(nullptr);
	string xmlfname = FileSystem::tmpdir()+FilePath(fname, FilePath::PT_FILE).filename()+"-"+ to_string(_pageno)+".xml";
	mutool("draw -Ftrace -o"+xmlfname+" "+_fname+" "+to_string(_pageno));
#if defined(MIKTEX_WINDOWS)
	ifstream ifs(EXPATH_(xmlfname));
#else
	ifstream ifs(xmlfname);
#endif
	xmlParser.parse(ifs);
	ifs.close();
	// remove trace XML
	if (!PhysicalFont::KEEP_TEMP_FILES)
		FileSystem::remove(xmlfname);
	return finishPage();
}


void PDFHandler::initFile (const string &fname) {
	finishFile();
	_fname = FilePath(fname, FilePath::PT_FILE).absolute();
	_fname = "\"" + _fname + "\"";
	_numPages = parse_value<int>(mtShow("trailer/Root/Pages/Count"));
	// extract image and font files from the PDF
	string cwd = FileSystem::getcwd();
	string tmpdir = FileSystem::tmpdir();
	FileSystem::chdir(tmpdir);
	string out = mtExtract(_fname, SearchPattern(R"(extracting\s+([a-z]+-\d+\.\S+))", "$1\n"));
	FileSystem::chdir(cwd);
	for (const string &extrfname : util::split(out, "\n")) {
		auto dashpos = extrfname.rfind('-');
		if (dashpos != string::npos) {
			int objnum;
			istringstream iss(extrfname.substr(dashpos+1));
			iss >> objnum;
			_extractedFiles.emplace(objnum, extrfname);
		}
	}
}


void PDFHandler::finishFile () {
	if (!PhysicalFont::KEEP_TEMP_FILES) {
		// remove extracted image and font files
		for (auto &entry : _extractedFiles)
			FileSystem::remove(FileSystem::tmpdir() + entry.second);
	}
	_fname.clear();
	_numPages = 0;
}


void PDFHandler::initPage (int pageno, unique_ptr<SVGElement> context) {
	_pageno = pageno;
	if (!context)
		_svg->newPage(_pageno);
	else {
		_context = context.get();
		_svg->pushPageContext(std::move(context));
	}
	// collect sequence of images referenced on current page
	collectObjects();
	_imgSeq.clear();
	SearchPattern pattern{R"((/[a-zA-Z0-9]+)\s+Do)", "$1\n"};
	string content = mtShow("pages/" + to_string(_pageno) + "/Contents", pattern);
	if (content.empty())
		content = mtShow("pages/" + to_string(_pageno) + "/Contents/*", pattern);
	for (const string &entry : util::split(content, "\n")) {
		if (!entry.empty())
			_imgSeq.push_back(entry);
	}
}


unique_ptr<SVGElement> PDFHandler::finishPage () {
	if (!_context) {
		// add glyph outlines of used characters
		for (auto &fontchars: FontManager::instance().getUsedChars()) {
			if (auto font = font_cast<const PhysicalFont *>(fontchars.first))
				_svg->append(*font, fontchars.second);
		}
		_svg->appendFontStyles(FontManager::instance().getUsedFonts());
	}
	// pop clipping paths still active
	while (!_clipPathStack.empty()) {
		_svg->popPageContext();
		if (_clipPathStack.top().groupElement->empty())
			XMLElement::detach(_clipPathStack.top().groupElement);
		_clipPathStack.pop();
	}
	_pageno = 0;
	_x = _y = 0;
	_currentFont = nullptr;
	_objDict.clear();
	auto context = util::static_unique_ptr_cast<SVGElement>(XMLElement::detach(_context));
	if (context)
		_svg->popPageContext();
	_context = nullptr;
	return context;
}


/** Called by the XMLParser after an opening tag of an element has been processed.
 *  @param[in] trcElement opened element of trace XML */
void PDFHandler::elementOpened (XMLElement *trcElement) {
	const string &name = trcElement->name();
	if (name == "tile")
		doOpenTile(trcElement);  // start defining a tiling pattern
}


/** Called by the XMLParser after a closing tag of an element has been processed.
 *  @param[in] trcElement closed element of trace XML */
void PDFHandler::elementClosed (XMLElement *trcElement) {
	const string &name = trcElement->name();
	struct Handler {
		const char *name;
		void (PDFHandler::*func)(XMLElement*);
	} handlers[10] = {
		{"page", &PDFHandler::doPage},
		{"stroke_path", &PDFHandler::doStrokePath},
		{"fill_path", &PDFHandler::doFillPath},
		{"fill_image", &PDFHandler::doFillImage},
		{"fill_text", &PDFHandler::doFillText},
		{"clip_path", &PDFHandler::doClipPath},
		{"clip_stroke_path", &PDFHandler::doClipStrokePath},
		{"clip_text", &PDFHandler::doClipText},
		{"pop_clip", &PDFHandler::doPopClip},
		{"tile", &PDFHandler::doCloseTile},
	};
	auto it = find_if(begin(handlers), end(handlers), [&name](const Handler &handler) {
		return handler.name == name;
	});
	if (it != end(handlers))
		(this->*it->func)(trcElement);
	else
		return;
	XMLElement::detach(trcElement);  // remove element from XML tree, it's no longer needed
}


void PDFHandler::doPage (XMLElement *trcPageElement) {
	auto vec = parse_attr_value<vector<double>>(trcPageElement, "mediabox");
	if (vec.size() == 4)
		_bbox = BoundingBox(vec[0], vec[1], vec[2], vec[3]);
}


/** Returns a color object from color data given in the trace XML.
 *  @param[in] colorspace currently supported: DeviceGray, DeviceRGB, DeviceCMYK
 *  @param[in] value color components depending on color space */
static Color to_color (const string &colorspace, const string &value) {
	Color color;
	if (colorspace == "DeviceGray")
		color.setGray(parse_value<double>(value));
	else if (colorspace == "DeviceRGB") {
		auto comp = util::split(value, " ");
		if (comp.size() == 3)
			color.setRGB(parse_value<double>(comp[0]), parse_value<double>(comp[1]), parse_value<double>(comp[2]));
	}
	else if (colorspace == "DeviceCMYK") {
		auto comp = util::split(value, " ");
		if (comp.size() == 4)
			color.setCMYK(parse_value<double>(comp[0]), parse_value<double>(comp[1]), parse_value<double>(comp[2]), parse_value<double>(comp[3]));
	}
	return color;
}


static double matrix_extent (const Matrix &m) {
	// signed area of the parallelogram spanned by the scale/rotation components of the matrix
	// v1 x v2 = det(v1,v2) = sin(v1,v2) * |v1| * |v2|
	double area = m.get(0,0)*m.get(1,1) - m.get(0,1)*m.get(1,0);
	return sqrt(abs(area));
}


static double matrix_max_extent (const Matrix &matrix) {
	double max1 = max(abs(matrix.get(0,0)), abs(matrix.get(0,1)));
	double max2 = max(abs(matrix.get(1,0)), abs(matrix.get(1,1)));
	return max(max1, max2);
}


/** Creates a GraphicsPath object from a sequence of command elements
 *  (moveto, lineto, curveto, closepath) read from the trace XML file.
 *  @param[in] srcPathElement parent of the command elements */
static GraphicsPath<double> create_path (XMLElement *srcPathElement) {
	GraphicsPath<double> path;
	for (const XMLNode *child : *srcPathElement) {
		if (const XMLElement *cmd = child->toElement()) {
			string name = cmd->name();
			if (name == "moveto")
				path.moveto(parse_attr_value<double>(cmd, "x"), parse_attr_value<double>(cmd, "y"));
			else if (name == "lineto")
				path.lineto(parse_attr_value<double>(cmd, "x"), parse_attr_value<double>(cmd, "y"));
			else if (name == "curveto")
				path.cubicto(
					parse_attr_value<double>(cmd, "x1"), parse_attr_value<double>(cmd, "y1"),
					parse_attr_value<double>(cmd, "x2"), parse_attr_value<double>(cmd, "y2"),
					parse_attr_value<double>(cmd, "x3"), parse_attr_value<double>(cmd, "y3"));
			else if (name == "closepath")
				path.closepath();
		}
	}
	if (!path.empty())
		path.removeRedundantCommands();
	return path;
}


/** Creates a new SVG path element from XML drawing commands.
 *  @param[in] srcPathElement element holding the drawing commands
 *  @param[in] stroke create a stroke path if true, a fill path otherwise
 *  @param[out] bbox if not 0, gets the bounding box of the resulting path */
static unique_ptr<SVGElement> create_path_element (XMLElement *srcPathElement, bool stroke, BoundingBox *bbox=nullptr) {
	unique_ptr<SVGElement> pathElement;
	auto path = create_path(srcPathElement);
	if (!path.empty()) {
		auto matrix = parse_attr_value<Matrix>(srcPathElement, "transform");
		path.transform(matrix);
		if (bbox)
			*bbox = path.computeBBox();
		pathElement = util::make_unique<SVGElement>("path");
		ostringstream oss;
		path.writeSVG(oss, SVGTree::RELATIVE_PATH_CMDS);
		pathElement->addAttribute("d", oss.str());
		string colorspace = parse_attr_value<string>(srcPathElement, "colorspace");
		string color = parse_attr_value<string>(srcPathElement, "color");
		if (!stroke) {  // create path for filling?
			if (parse_attr_value<string>(srcPathElement, "winding") == "evenodd")
				pathElement->setFillRule(SVGElement::FR_EVENODD);
			if (!colorspace.empty())
				pathElement->setFillColor(to_color(colorspace, color));
		}
		else {  // create path for stroking
			if (!colorspace.empty())
				pathElement->setStrokeColor(to_color(colorspace, color));
			pathElement->setNoFillColor();
			double linewidth=1;
			if (const char *valstr = srcPathElement->getAttributeValue("linewidth")) {
				double lwScale = matrix_extent(matrix);
				linewidth = parse_value<double>(valstr);
				pathElement->setStrokeWidth(linewidth * lwScale);
			}
			double miterlimit=0;
			SVGElement::LineJoin linejoin=SVGElement::LJ_MITER;
			if (const char *valstr = srcPathElement->getAttributeValue("miterlimit"))
				pathElement->setStrokeMiterLimit(miterlimit = parse_value<double>(valstr));
			if (const char *valstr = srcPathElement->getAttributeValue("linejoin")) {
				linejoin = SVGElement::LJ_MITER;
				switch (parse_value<int>(valstr)) {
					case 1: linejoin = SVGElement::LJ_ROUND; break;
					case 2: linejoin = SVGElement::LJ_BEVEL; break;
				}
				pathElement->setStrokeLineJoin(linejoin);
			}
			if (const char *valstr = srcPathElement->getAttributeValue("linecap")) {
				SVGElement::LineCap captype = SVGElement::LC_BUTT;
				switch (parse_value<int>(valstr)) {
					case 1: captype = SVGElement::LC_ROUND; break;
					case 2: captype = SVGElement::LC_SQUARE; break;
				}
				pathElement->setStrokeLineCap(captype);
			}
			if (const char *valstr = srcPathElement->getAttributeValue("dash")) {
				auto offset = parse_attr_value<double>(srcPathElement, "dash_phase");
				pathElement->setStrokeDash(valstr, offset);
			}
			if (bbox) {
				double extent = (linewidth != 0 ? linewidth : 1.0) * matrix_max_extent(matrix);
				if (linejoin == SVGElement::LJ_MITER && miterlimit > 1)
					extent *= miterlimit;
				bbox->expand(extent);
			}
		}
	}
	return pathElement;
}


void PDFHandler::doFillPath (XMLElement *trcFillPathElement) {
	if (auto pathElement = create_path_element(trcFillPathElement, false)) {
		if (XMLElement *parent = trcFillPathElement->parent()->toElement()) {
			if (parent->name() == "group") {
				if (const char *valstr = parent->getAttributeValue("blendmode"))
					pathElement->setFillOpacity(Opacity::blendMode(valstr));
				if (const char *valstr = parent->getAttributeValue("alpha"))
					pathElement->setFillOpacity(OpacityAlpha(parse_value<double>(valstr)));
			}
		}
		_svg->appendToPage(std::move(pathElement));
	}
}


void PDFHandler::doStrokePath (XMLElement *trcStrokePathElement) {
	if (auto pathElement = create_path_element(trcStrokePathElement, true)) {
		if (XMLElement *parent = trcStrokePathElement->parent()->toElement()) {
			if (parent->name() == "group") {
				Opacity opacity;
				if (const char *valstr = parent->getAttributeValue("blendmode"))
					opacity.setBlendMode(Opacity::blendMode(valstr));
				if (const char *valstr = parent->getAttributeValue("alpha"))
					opacity.strokealpha().setConstAlpha(parse_value<double>(valstr));
				pathElement->setStrokeOpacity(opacity);
			}
		}
		_svg->appendToPage(std::move(pathElement));
	}
}


void PDFHandler::doClipPath (XMLElement *trcClipPathElement) {
	if (auto pathElement = create_path_element(trcClipPathElement, false)) {
		string id = "cp"+to_string(_numClipPath++);
		auto clipPathElement = util::make_unique<SVGElement>("clipPath");
		clipPathElement->addAttribute("id", id);
		auto groupElement = util::make_unique<SVGElement>("g");
		_clipPathStack.emplace(std::move(id), groupElement.get());
		groupElement->setClipPathUrl(_clipPathStack.top().id);
		clipPathElement->append(std::move(pathElement));
		_svg->appendToDefs(std::move(clipPathElement));
		_svg->pushPageContext(std::move(groupElement));
	}
}


void PDFHandler::doClipStrokePath (XMLElement *trcClipStrokePathElement) {
	BoundingBox pathbox;
	if (auto pathElement = create_path_element(trcClipStrokePathElement, true, &pathbox)) {
		pathElement->setStrokeColor(Color::WHITE);
		string id = "cm"+to_string(_numClipPath++);
		auto maskElement = util::make_unique<SVGElement>("mask");
		maskElement->addAttribute("id", id);
		maskElement->addAttribute("x", pathbox.minX());
		maskElement->addAttribute("y", pathbox.minY());
		maskElement->addAttribute("width", pathbox.width());
		maskElement->addAttribute("height", pathbox.height());
		maskElement->addAttribute("maskUnits", "userSpaceOnUse");
		maskElement->append(std::move(pathElement));
		auto groupElement = util::make_unique<SVGElement>("g");
		groupElement->setMaskUrl(id);
		_clipPathStack.emplace(std::move(id), groupElement.get());
		_svg->appendToDefs(std::move(maskElement));
		_svg->pushPageContext(std::move(groupElement));
	}
}


void PDFHandler::doClipText (XMLElement *trcClipTextElement) {
	string id = "cp"+to_string(_numClipPath++);
	auto clipPathElement = util::make_unique<SVGElement>("clipPath");
	clipPathElement->addAttribute("id", id);
	auto groupElement = util::make_unique<SVGElement>("g");
	_clipPathStack.emplace(std::move(id), groupElement.get());
	groupElement->setClipPathUrl(_clipPathStack.top().id);
	SVGElement *cpElementPtr = clipPathElement.get();
	_svg->pushPageContext(std::move(clipPathElement));
	doFillText(trcClipTextElement);
	_svg->popPageContext();
	_svg->appendToDefs(XMLElement::detach(cpElementPtr));  // move clipPath element from page to defs section
	_svg->pushPageContext(std::move(groupElement));
}


void PDFHandler::doPopClip (XMLElement*) {
	if (!_clipPathStack.empty()) {
		_svg->popPageContext();
		if (_clipPathStack.top().groupElement->empty())
			XMLElement::detach(_clipPathStack.top().groupElement);
		_clipPathStack.pop();
	}
}


static unique_ptr<SVGElement> create_use_element (double x, double y, const string &refID) {
	auto useElement = util::make_unique<SVGElement>("use");
	useElement->addAttribute("x", x);
	useElement->addAttribute("y", y);
	useElement->addAttribute("xlink:href", "#"+refID);
	return useElement;
}


void PDFHandler::doFillImage (XMLElement *trcFillImageElement) {
	if (_numImages < _imgSeq.size()) {
		auto it = _objDict.find(_imgSeq[_numImages]);
		if (it != _objDict.end()) {
			string imgID = "img"+to_string(it->second.num)+"-"+XXH32HashFunction(_fname).digestString();
			auto w = parse_attr_value<double>(trcFillImageElement, "width");
			auto h = parse_attr_value<double>(trcFillImageElement, "height");
			auto alpha = parse_attr_value<double>(trcFillImageElement, "alpha");
			if (w == 0 || h == 0 || alpha == 0)
				return;
			// add base64 encoding of the image to the defs section if it hasn't been referenced yet
			auto last = _imgSeq.begin()+int(_numImages);
			if (find(_imgSeq.begin(), last, it->first) == last) {
				string fname = it->second.fname;
				auto symbolElement = util::make_unique<SVGElement>("symbol");
				symbolElement->addAttribute("id", imgID);
				symbolElement->addAttribute("viewBox", BoundingBox(0, 0, w, h).svgViewBoxString());
				auto imgElement = util::make_unique<SVGElement>("image");
				imgElement->addAttribute("width", parse_attr_value<double>(trcFillImageElement, "width"));
				imgElement->addAttribute("height", parse_attr_value<double>(trcFillImageElement, "height"));
				imgElement->addAttribute("@@xlink:href", "data:"+util::mimetype(fname)+";base64,"+fname);
				symbolElement->append(std::move(imgElement));
				_svg->appendToDefs(std::move(symbolElement));
			}
			// add reference to image
			auto useElement = create_use_element(0, 0, imgID);
			useElement->setOpacity(OpacityAlpha(alpha));
			Matrix matrix = parse_attr_value<Matrix>(trcFillImageElement, "transform");
			matrix.rmultiply(ScalingMatrix(1/w, 1/h));
			useElement->setTransform(matrix);
			if (!_clipPathStack.empty())
				useElement->setClipPathUrl(_clipPathStack.top().id);
			_svg->appendToPage(std::move(useElement));
		}
		_numImages++;
	}
}


static string to_utf8 (const string &str) {
	if (str.empty())
		return "";
	if (str.front() != '&' || str.back() != ';')
		return str;
	string ent = str.substr(1, str.length()-2);
	if (ent == "amp")  return Unicode::utf8('&');
	if (ent == "apos") return Unicode::utf8('\'');
	if (ent == "gt")   return Unicode::utf8('>');
	if (ent == "lt")   return Unicode::utf8('<');
	if (ent == "quot") return Unicode::utf8('"');
	if (ent[0] == '#') {
		try {
			int32_t cp;
			cp = (ent[1] == 'x' ? stoi(ent.substr(2), nullptr, 16) : stoi(ent.substr(1)));
			return Unicode::utf8(cp);
		}
		catch (exception&) {}
	}
	return "";
}


static string strip_subset_prefix (const std::string &fontname) {
	auto pos = fontname.find('+');
	if (pos != string::npos)
		return fontname.substr(pos+1);
	return fontname;
}


static string compose_utf8_char (const XMLElement *charElement, int glyph) {
	string utf8 = to_utf8(parse_attr_value<string>(charElement, "unicode"));
	string nonlig;
	for (XMLElement *elem=charElement->nextElement(); elem && !elem->hasAttribute("glyph"); elem=elem->nextElement())
		nonlig += to_utf8(parse_attr_value<string>(elem, "unicode"));
	if (!nonlig.empty()) {
		if (uint32_t lig_cp = Unicode::toLigature(utf8+nonlig)) // known Unicode ligature?
			utf8 = Unicode::utf8(lig_cp);
		else if (glyph <= 0x1900)  // unknown ligature fitting in Private Use Zone?
			utf8 = Unicode::utf8(0xE000+glyph);
		// TODO: handle unknown ligatures with glyph indices > 0x1900
	}
	return utf8;
}


void PDFHandler::doFillText (XMLElement *trcFillTextElement) {
	for (const XMLNode *textchild : *trcFillTextElement) {
		if (const XMLElement *spanElement = textchild->toElement()) {
			auto trm = parse_attr_value<vector<double>>(spanElement, "trm");
			if (trm.size() < 4 || trm[0] == 0)
				continue;
			auto fontname = parse_attr_value<string>(spanElement, "font");
			auto it = _objDict.find(fontname);
			if (it == _objDict.end()) {
				// try to lookup font without subfont prefix
				fontname = strip_subset_prefix(fontname);
				it = _objDict.find(fontname);
			}
			string filename;
			if (it != _objDict.end())
				filename = it->second.fname;
			if (filename.empty())
				filename = "sys://"+fontname;
			double ptsize = matrix_extent({trm[0], trm[1], 0, trm[2], trm[3]});
			ptsize = round(100*ptsize)/100;
			int fontID = FontManager::instance().registerFont(filename, std::move(fontname), ptsize);
			if (fontID >= 0) {
				auto font = font_cast<NativeFont*>(FontManager::instance().getFontById(fontID));
				if (font != _currentFont) {
					_svg->setFont(FontManager::instance().fontID(font), *font);
					_currentFont = font;
					_svg->setX(0);
					_svg->setY(0);
					_x = _y = numeric_limits<double>::max();
				}
				Matrix fontMatrix({trm[0]/ptsize, -trm[2]/ptsize, 0, trm[1]/ptsize, -trm[3]/ptsize});
				Matrix matrix = parse_attr_value<Matrix>(trcFillTextElement, "transform");
				matrix.rmultiply(fontMatrix);
				_svg->setMatrix(matrix);
				fontMatrix.invert();
				string colorspace = parse_attr_value<string>(trcFillTextElement, "colorspace");
				string colorval = parse_attr_value<string>(trcFillTextElement, "color");
				_svg->setFillColor(to_color(colorspace, colorval));
				for (const XMLNode *spanchild : *spanElement) {
					const XMLElement *charElement = spanchild->toElement();
					if (!charElement || charElement->name() != "g" || !charElement->hasAttribute("glyph"))
						continue;
					int glyph;
					if (font->isCIDFont())
						glyph = parse_attr_value<int>(charElement, "glyph");
					else {
						glyph = font->charIndexByName(parse_attr_value<string>(charElement, "glyph"));
						if (glyph == 0)
							glyph = parse_attr_value<int>(charElement, "glyph");
					}
					// determine code point of current character
					string utf8;
					if (charElement->hasAttribute("unicode"))
						utf8 = to_utf8(parse_attr_value<string>(charElement, "unicode"));
					if (utf8.empty())
						utf8 = compose_utf8_char(charElement, glyph);
					if (glyph == 0 || utf8.empty())
						continue;
					DPair p(parse_attr_value<double>(charElement, "x"), parse_attr_value<double>(charElement, "y"));
					p = fontMatrix * p;
					if (abs(_x-p.x()) > 0.2) {_x = p.x(); _svg->setX(_x);}
					if (abs(_y-p.y()) > 0.2) {_y = p.y(); _svg->setY(_y);}
					font->mapCharToUnicode(glyph, Unicode::utf8ToCodepoint(utf8));
					if (font->verticalLayout())
						_y += font->charHeight(glyph);
					else
						_x += font->charWidth(glyph);
					_svg->appendChar(glyph, p.x(), p.y());
					FontManager::instance().addUsedChar(*font, glyph);
				}
			}
		}
	}
}


void PDFHandler::doOpenTile (XMLElement *trcTileElement) {
	auto patternSymbol = util::make_unique<SVGElement>("symbol");
	patternSymbol->addAttribute("id", "tile"+ to_string(_numPatterns));
	patternSymbol->addAttribute("style", "overflow:visible");
	_svg->pushPageContext(std::move(patternSymbol));
}


static unique_ptr<SVGElement> rect_path_elem (const vector<double> &coords) {
	GraphicsPath<double> path;
	path.moveto(coords[0], coords[1]);
	path.lineto(coords[2], coords[1]);
	path.lineto(coords[2], coords[3]);
	path.lineto(coords[0], coords[3]);
	path.closepath();
	ostringstream oss;
	path.writeSVG(oss, SVGTree::RELATIVE_PATH_CMDS);
	auto pathElement = util::make_unique<SVGElement>("path");
	pathElement->addAttribute("d", oss.str());
	return pathElement;
}


void PDFHandler::doCloseTile (XMLElement *trcTileElement) {
	_svg->popPageContext();  // definition of symbol finished
	auto xstep = parse_attr_value<double>(trcTileElement, "xstep");
	auto ystep = parse_attr_value<double>(trcTileElement, "ystep");
	if (xstep == 0 || ystep == 0)
		return;
	auto view = parse_attr_value<vector<double>>(trcTileElement, "view");
	auto patternElement = util::make_unique<SVGElement>("pattern");
	patternElement->addAttribute("id", "pat"+ to_string(_numPatterns));
	patternElement->addAttribute("x", 0);
	patternElement->addAttribute("y", 0);
	patternElement->addAttribute("width", xstep);
	patternElement->addAttribute("height", ystep);
	patternElement->addAttribute("patternUnits", "userSpaceOnUse");

	SVGElement *contextElement = patternElement.get();
	view.resize(4);
	if (view[0] > 0 || view[2] > xstep || view[1] > 0 || view[3] > ystep) {
		auto clipPathElement = util::make_unique<SVGElement>("clipPath");
		clipPathElement->addAttribute("id", "patcp"+to_string(_numPatterns));
		auto pathElement = rect_path_elem(view);
		clipPathElement->append(std::move(pathElement));
		patternElement->append(std::move(clipPathElement));

		auto clipGroupElement = util::make_unique<SVGElement>("g");
		clipGroupElement->setClipPathUrl("patcp"+to_string(_numPatterns));
		contextElement = clipGroupElement.get();
		patternElement->append(std::move(clipGroupElement));
	}
	Matrix matrix = parse_attr_value<Matrix>(trcTileElement, "transform");
	auto transformGroupElement = util::make_unique<SVGElement>("g");
	transformGroupElement->setTransform(matrix);
	// Most SVG renderers don't support overflow:visible on patterns. Thus, parts of
	// the pattern graphics that lie outside the tile area are clipped. To work around
	// this, we place the pattern graphics at all 4 corners of the tile rectangle. This
	// way, the outside parts of all quadrants are drawn inside as well so that adjacent
	// tiles are enabled to compose the pattern. If the pattern graphics falls completely
	// inside the tile area, the additional drawings are empty and thus redundant. For now,
	// we keep them anyway.
	Matrix invmatrix{matrix.get(0,0), matrix.get(0,1), 0, matrix.get(1,0), matrix.get(1,1)};
	invmatrix.invert();
	for (int i=0; i < 2; i++) {
		for (int j=0; j < 2; j++) {
			DPair p = invmatrix * DPair(xstep*i, ystep*j);
			transformGroupElement->append(create_use_element(p.x(), p.y(), "tile"+to_string(_numPatterns)));
		}
	}
	contextElement->append(std::move(transformGroupElement));
	_svg->appendToPage(std::move(patternElement));
	auto area = parse_attr_value<vector<double>>(trcTileElement, "area");
	area.resize(4);
	auto pathElement = rect_path_elem(area);
	pathElement->setFillPatternUrl("pat"+to_string(_numPatterns));
	pathElement->setTransform(matrix);
	_svg->appendToPage(std::move(pathElement));
	_numPatterns++;
}


void PDFHandler::collectObjects () {
	string tmpdir = FileSystem::tmpdir();
	_objDict = parse_pdf_dict<ObjID>(mtShow("pages/" + to_string(_pageno) + "/Resources/XObject"));
	// replace referenced font IDs by actual IDs used for extracted fonts
	for (auto &entry : _objDict) {
		// store filenames of non-font object in object map
		auto fnameIt = _extractedFiles.find(entry.second.num);
		entry.second.fname = fnameIt != _extractedFiles.end() ? tmpdir+fnameIt->second : "";
	}
	for (auto &entry : _extractedFiles) {
		if (entry.second.substr(0, 5) == "font-") {
			string filepath = tmpdir+entry.second;  // path to font file
			string psFontname = FontEngine::instance().getPSName(filepath);
			string fontname = mtShow(to_string(entry.first) + "/FontName", SearchPattern(R"(/((\w|[+-])+))", "$1"));
			if (!psFontname.empty() && fontname.find('+') == string::npos)
				fontname = std::move(psFontname);
			_objDict.emplace(fontname, ObjID(entry.first, 0, std::move(filepath)));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

string PDFHandler::mutool (const string &cmd, bool readFromStderr) {
	string out;
	Process("mutool", cmd).run(&out, readFromStderr ? Process::PF_STDERR : Process::PF_STDOUT);
	return out;
}


string PDFHandler::mutool (const string &cmd, const SearchPattern &pattern, bool readFromStderr) {
	string out;
	Process("mutool", cmd).run(&out, pattern, readFromStderr ? Process::PF_STDERR : Process::PF_STDOUT);
	return out;
}


string PDFHandler::mtExtract (const string &fname, const SearchPattern &pattern) {
	return mutool("extract -a "+fname, pattern);
}


/** Calls "mutool show" to retrieve select data from a PDF file.
 *  @param[in] fname name of PDF file
 *  @param[in] path path expression locating the requested data
 *  @param[in] fmtmode flag specifying the output format
 *  @return mutool output, result of the query */
string PDFHandler::mtShow (const string &fname, const string &path, char fmtmode) {
	string cmd = "show -";
	cmd += fmtmode;
	cmd += " " + fname + " " + path;
	return mutool(cmd);
}


string PDFHandler::mtShow (const string &fname, const string &path, const SearchPattern &pattern, char fmtmode) {
	string cmd = "show -";
	cmd += fmtmode;
	cmd += " " + fname + " " + path;
	return mutool(cmd, pattern);
}

#if 0
int main (int argc, char *argv[]) {
	if (argc < 2)
		return 0;
	SVGTree::USE_FONTS=true;
	SVGTree::FONT_FORMAT = FontWriter::FontFormat::SVG;
	SVGTree svg;
	PDFHandler page(svg);
	page.convert(argv[1], argc > 2 ? stoi(argv[2]) : 1);
	svg.write(cout);
}
#endif
