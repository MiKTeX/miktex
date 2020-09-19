/*************************************************************************
** PsSpecialHandler.hpp                                                 **
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

#ifndef PSSPECIALHANDLER_HPP
#define PSSPECIALHANDLER_HPP

#include <memory>
#include <set>
#include <stack>
#include <string>
#include <vector>
#include "GraphicsPath.hpp"
#include "PSInterpreter.hpp"
#include "SpecialHandler.hpp"
#include "PSPattern.hpp"
#include "PSPreviewFilter.hpp"


class PSPattern;
class XMLElement;

class PsSpecialHandler : public SpecialHandler, protected PSActions {
	using Path = GraphicsPath<double>;
	using ColorSpace = Color::ColorSpace;

	/** Helper class storing the clipping paths currently present on the graphics context stack.
	 *  Since PS operator 'clippath' only delivers a linearly approximated version of the paths
	 *  that are sometimes too imprecise for direct usage in SVG, we keep the possibly curved
	 *  clipping paths and compute their intersections locally if necessary. */
	class ClippingStack {
		public:
			void pushEmptyPath ();
			void push (const Path &path, int saveID=-1);
			bool replace (const Path &path);
			void dup (int saveID=-1);
			void pop (int saveID=-1, bool grestore=false);
			void clear ();
			bool empty () const {return _stack.empty();}
			void setPrependedPath ();
			const Path* path () const;
			const Path* prependedPath () const;
			void removePrependedPath ();
			int topID () const {return _stack.empty() ? 0 : _stack.top().pathID;}

		private:
			struct Entry {
				std::shared_ptr<Path> path;  ///< pointer to current clipping path
				std::shared_ptr<Path> prependedPath=nullptr;
				int pathID;        ///< ID of current clipping path
				int saveID;        ///< if >=0, path was pushed by 'save', and saveID holds the ID of the PS memory object
				Entry () : Entry(-1) {}
				Entry (const Path &p, int pid, int sid) : path(std::make_shared<Path>(p)), pathID(pid), saveID(sid) {}
				explicit Entry (int sid) : path(nullptr), pathID(0), saveID(sid) {}
				Entry (const Entry &entry) =default;
				Entry (Entry &&entry) =default;
			};
			size_t _maxID=0;
			std::stack<Entry> _stack;  ///< stack holding the clipping information of the current graphics context
	};

	enum PsSection {PS_NONE, PS_HEADERS, PS_BODY};
	enum class FileType {EPS, PDF, SVG, BITMAP};

	public:
		PsSpecialHandler ();
		~PsSpecialHandler () override;
		const char* name () const override {return "ps";}
		const char* info () const override {return "dvips PostScript specials";}
		std::vector<const char*> prefixes() const override;
		void preprocess (const std::string &prefix, std::istream &is, SpecialActions &actions) override;
		bool process (const std::string &prefix, std::istream &is, SpecialActions &actions) override;
		void setDviScaleFactor (double dvi2bp) override {_previewFilter.setDviScaleFactor(dvi2bp);}
		void enterBodySection ();
		PSInterpreter& psInterpreter () {return _psi;}

	public:
		static bool COMPUTE_CLIPPATHS_INTERSECTIONS;
		static bool SHADING_SEGMENT_OVERLAP;
		static int SHADING_SEGMENT_SIZE;
		static double SHADING_SIMPLIFY_DELTA;
		static std::string BITMAP_FORMAT;

	protected:
		void initialize ();
		void initgraphics ();
		void moveToDVIPos ();
		void executeAndSync (std::istream &is, bool updatePos);
		void processHeaderFile (const char *fname);
		void imgfile (FileType type, const std::string &fname, const std::map<std::string,std::string> &attr);
		std::unique_ptr<XMLElement> createImageNode (FileType type, const std::string &fname, int pageno, BoundingBox bbox, bool clip);
		void dviBeginPage (unsigned int pageno, SpecialActions &actions) override;
		void dviEndPage (unsigned pageno, SpecialActions &actions) override;
		void clip (Path path, bool evenodd);
		void processSequentialPatchMesh (int shadingTypeID, ColorSpace cspace, VectorIterator<double> &it);
		void processLatticeTriangularPatchMesh (ColorSpace colorSpace, VectorIterator<double> &it);

		/// scale given value by current PS scale factors
		double scale (double v) const {return v*(_sx*(1-_cos*_cos) + _sy*_cos*_cos);}

		void applyscalevals (std::vector<double> &p) override  {_sx = p[0]; _sy = p[1]; _cos = p[2];}
		void clip (std::vector<double> &p) override            {clip(p, false);}
		void clip (std::vector<double> &p, bool evenodd);
		void clippath (std::vector<double> &p) override;
		void closepath (std::vector<double> &p) override;
		void curveto (std::vector<double> &p) override;
		void eoclip (std::vector<double> &p) override          {clip(p, true);}
		void eofill (std::vector<double> &p) override          {fill(p, true);}
		void fill (std::vector<double> &p, bool evenodd);
		void fill (std::vector<double> &p) override            {fill(p, false);}
		void grestore (std::vector<double> &p) override;
		void grestoreall (std::vector<double> &p) override;
		void gsave (std::vector<double> &p) override;
		void image (std::vector<double> &p) override;
		void initclip (std::vector<double> &p) override;
		void lineto (std::vector<double> &p) override;
		void makepattern (std::vector<double> &p) override;
		void moveto (std::vector<double> &p) override;
		void newpath (std::vector<double> &p) override;
		void querypos (std::vector<double> &p) override        {_currentpoint = DPair(p[0], p[1]);}
		void restore (std::vector<double> &p) override;
		void rotate (std::vector<double> &p) override;
		void save (std::vector<double> &p) override;
		void scale (std::vector<double> &p) override;
		void setblendmode (std::vector<double> &p) override    {_blendmode = int(p[0]);}
		void setcolorspace (std::vector<double> &p) override   {_patternEnabled = bool(p[0]);}
		void setcmykcolor (std::vector<double> &cmyk) override;
		void setdash (std::vector<double> &p) override;
		void setgray (std::vector<double> &p) override;
		void sethsbcolor (std::vector<double> &hsb) override;
		void setlinecap (std::vector<double> &p) override      {_linecap = uint8_t(p[0]);}
		void setlinejoin (std::vector<double> &p) override     {_linejoin = uint8_t(p[0]);}
		void setlinewidth (std::vector<double> &p) override    {_linewidth = scale(p[0] ? p[0] : 0.5);}
		void setmatrix (std::vector<double> &p) override;
		void setmiterlimit (std::vector<double> &p) override   {_miterlimit = p[0];}
		void setnulldevice (std::vector<double> &p) override;
		void setopacityalpha (std::vector<double> &p) override {_opacityalpha = p[0];}
		void setshapealpha (std::vector<double> &p) override   {_shapealpha = p[0];}
		void setpagedevice (std::vector<double> &p) override;
		void setpattern (std::vector<double> &p) override;
		void setrgbcolor (std::vector<double> &rgb) override;
		void shfill (std::vector<double> &p) override;
		void stroke (std::vector<double> &p) override;
		void translate (std::vector<double> &p) override;
		void executed () override;

	private:
		PSInterpreter _psi;
		SpecialActions *_actions=nullptr;
		PSPreviewFilter _previewFilter;  ///< filter to extract information generated by the preview package
		PsSection _psSection=PS_NONE;    ///< current section processed (nothing yet, headers, or body specials)
		XMLElement *_xmlnode=nullptr;    ///< if != 0, created SVG elements are appended to this node
		XMLElement *_savenode=nullptr;   ///< pointer to temporaryly store _xmlnode
		std::string _headerCode;    ///< collected literal PS header code
		Path _path;
		DPair _currentpoint;        ///< current PS position in bp units
		Color _currentcolor;        ///< current stroke/fill color
		double _sx, _sy;            ///< horizontal and vertical scale factors retrieved by operator "applyscalevals"
		double _cos;                ///< cosine of angle between (1,0) and transform(1,0)
		double _linewidth;          ///< current line width in bp units
		double _miterlimit;         ///< current miter limit in bp units
		double _opacityalpha;       ///< opacity level (0=fully transparent, ..., 1=opaque)
		double _shapealpha;         ///< shape opacity level (0=fully transparent, ..., 1=opaque)
		int _blendmode;             ///< blend mode used when overlaying colored areas
		uint8_t _linecap  : 2;      ///< current line cap (0=butt, 1=round, 2=projecting square)
		uint8_t _linejoin : 2;      ///< current line join (0=miter, 1=round, 2=bevel)
		double _dashoffset;         ///< current dash offset
		std::vector<double> _dashpattern;
		ClippingStack _clipStack;
		std::map<int, std::unique_ptr<PSPattern>> _patterns;
		PSTilingPattern *_pattern;  ///< current pattern
		bool _patternEnabled;       ///< true if active color space is a pattern
};

#endif
