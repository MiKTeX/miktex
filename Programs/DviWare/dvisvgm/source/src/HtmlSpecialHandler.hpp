/*************************************************************************
** HtmlSpecialHandler.hpp                                               **
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

#ifndef HTMLSPECIALHANDLER_HPP
#define HTMLSPECIALHANDLER_HPP

#include <map>
#include <string>
#include "Color.hpp"
#include "SpecialHandler.hpp"

class SpecialActions;

class HtmlSpecialHandler : public SpecialHandler, public DVIEndPageListener, public DVIPositionListener
{
	struct NamedAnchor {
		NamedAnchor () : pageno(0), id(0), pos(0), referenced(false) {}
		NamedAnchor (unsigned pn, int i, double p, bool r=false) : pageno(pn), id(i), pos(p), referenced(r) {}
		unsigned pageno;  ///< page number where the anchor is located
		int id;           ///< unique numerical ID (< 0 if anchor is undefined yet)
		double pos;       ///< vertical position of named anchor (in PS point units)
		bool referenced;  ///< true if a reference to this anchor exists
	};

	enum class AnchorType {NONE, HREF, NAME};
	typedef std::map<std::string, NamedAnchor> NamedAnchors;

	public:
		HtmlSpecialHandler () : _active(false), _anchorType(AnchorType::NONE), _depthThreshold(0) {}
		void preprocess (const char *prefix, std::istream &is, SpecialActions &actions) override;
		bool process (const char *prefix, std::istream &is, SpecialActions &actions) override;
		const char* name () const override {return "html";}
		const char* info () const override {return "hyperref specials";}
		const char** prefixes () const override;

		static bool setLinkMarker (const std::string &marker);

	protected:
		void preprocessHrefAnchor (const std::string &uri);
		void preprocessNameAnchor (const std::string &name, SpecialActions &actions);
		void processHrefAnchor (std::string uri, SpecialActions &actions);
		void processNameAnchor (const std::string &name, SpecialActions &actions);
		void dviEndPage (unsigned pageno, SpecialActions &actions) override;
		void dviMovedTo (double x, double y, SpecialActions &actions) override;
		void closeAnchor (SpecialActions &actions);
		void markLinkedBox (SpecialActions &actions);

		enum class MarkerType {NONE, LINE, BOX, BGCOLOR};
		static MarkerType MARKER_TYPE;  ///< selects how to mark linked areas
		static Color LINK_BGCOLOR;      ///< background color if linkmark type == LT_BGCOLOR
		static Color LINK_LINECOLOR;    ///< line color if linkmark type is LM_LINE or LM_BOX
		static bool USE_LINECOLOR;      ///< if true, LINK_LINECOLOR is applied

	private:
		bool _active;
		AnchorType _anchorType;     ///< type of active anchor
		int _depthThreshold;        ///< break anchor box if the DVI stack depth underruns this threshold
		std::string _base;          ///< base URL that is prepended to all relative targets
		NamedAnchors _namedAnchors; ///< information about all named anchors processed
};

#endif
