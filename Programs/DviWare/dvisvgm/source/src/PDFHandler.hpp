/*************************************************************************
** PDFHandler.hpp                                                       **
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

#ifndef PDFHANDLER_HPP
#define PDFHANDLER_HPP

#include <map>
#include <string>
#include "BoundingBox.hpp"
#include "Process.hpp"
#include "SVGTree.hpp"

class XMLElement;

class PDFHandler {
	public:
		struct ObjID {
			ObjID () : num(-1), gen(0) {}
			ObjID (int number, int generation) : num(number), gen(generation) {}
			ObjID (int number, int generation, std::string filename) : num(number), gen(generation), fname(std::move(filename)) {}
			bool valid () const {return num >= 0;}
			int num, gen;  ///< object and generation number
			std::string fname;
		};

	protected:
		struct ClipPathData {
			ClipPathData (std::string cpid, SVGElement *group) : id(std::move(cpid)), groupElement(group) {}
			std::string id;
			SVGElement *groupElement;
		};

	public:
		PDFHandler () =default;
		explicit PDFHandler (SVGTree &svg) : _svg(&svg) {}
		PDFHandler (const PDFHandler &page) =delete;
		PDFHandler (PDFHandler &&page) =delete;
		~PDFHandler ();
		void assignSVGTree (SVGTree &svg) {_svg = &svg;}
		const BoundingBox& bbox () const {return _bbox;}
		std::unique_ptr<SVGElement> convert (const std::string &fname, int pageno, std::unique_ptr<SVGElement> context=nullptr);
		static std::string mutoolVersion ();
		static bool available () {return !mutoolVersion().empty();}
		static int numberOfPages (const std::string &fname);

	protected:
		static std::string mutool (const std::string &cmd, bool readFromStderr=false);
		static std::string mutool (const std::string &cmd, const SearchPattern &pattern, bool readFromStderr=false);
		static std::string mtExtract (const std::string &fname, const SearchPattern &pattern);
		static std::string mtShow (const std::string &fname, const std::string &path, char fmtmode= 'b');
		static std::string mtShow (const std::string &fname, const std::string &path, const SearchPattern &pattern, char fmtmode= 'b');
		std::string mtShow (const std::string &path, char fmtmode= 'b') const {return mtShow(_fname, path, fmtmode);}
		std::string mtShow (const std::string &path, const SearchPattern &pattern, char fmtmode= 'b') const {
			return mtShow(_fname, path, pattern, fmtmode);
		}
		void initFile (const std::string &fname);
		void finishFile ();
		void initPage (int pageno, std::unique_ptr<SVGElement> context);
		std::unique_ptr<SVGElement> finishPage ();
		void collectObjects ();
		void elementOpened (XMLElement *trcElement);
		void elementClosed (XMLElement *trcElement);
		void doPage (XMLElement *trcPageElement);
		void doClipPath (XMLElement *trcClipPathElement);
		void doClipStrokePath (XMLElement *trcClipStrokePathElement);
		void doClipText (XMLElement *trcClipTextElement);
		void doPopClip (XMLElement *trcPopClipElement);
		void doFillPath (XMLElement *trcFillPathElement);
		void doStrokePath (XMLElement *trcStrokePathElement);
		void doFillImage (XMLElement *trcFillImageElement);
		void doFillText (XMLElement *trcFillTextElement);
		void doOpenTile (XMLElement *trcTileElement);
		void doCloseTile (XMLElement *trcTileElement);

	private:
		std::string _fname;
		int _pageno=0;
		int _numPages=0;
		std::map<int,std::string> _extractedFiles;
		std::map<std::string,ObjID> _objDict;    ///< object names => object IDs
		std::vector<std::string> _imgSeq;
		NativeFont *_currentFont=nullptr;        ///< currently selected font
		size_t _numClipPath=0; ///< number of clipping paths processed
		size_t _numImages=0;   ///< number of embedded images already processed
		size_t _numPatterns=0; ///< number of patterns processed
		std::stack<ClipPathData> _clipPathStack;
		double _x=0, _y=0;     ///< current character position
		BoundingBox _bbox;     ///< bounding box of current/last page processed
		SVGTree *_svg=nullptr;
		XMLElement *_context=nullptr; ///< node holding the generated page contents
};

#endif
