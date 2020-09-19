/*************************************************************************
** HyperlinkManager.hpp                                                 **
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

#ifndef HYPERLINKMANAGER_HPP
#define HYPERLINKMANAGER_HPP

#include <string>
#include <unordered_map>
#include "Color.hpp"
#include "SpecialActions.hpp"

class SpecialActions;

class HyperlinkManager {
	struct NamedAnchor {
		NamedAnchor () : pageno(0), id(0), pos(0), referenced(false) {}
		NamedAnchor (unsigned pn, int i, double p, bool r=false) : pageno(pn), id(i), pos(p), referenced(r) {}
		unsigned pageno;  ///< page number where the anchor is located
		int id;           ///< unique numerical ID (< 0 if anchor is undefined yet)
		double pos;       ///< vertical position of named anchor (in PS point units)
		bool referenced;  ///< true if a reference to this anchor exists
	};

	enum class AnchorType {NONE, HREF, NAME};
	enum class ColorSource {DEFAULT, LINKMARKER, STATIC};
	using NamedAnchors = std::unordered_map<std::string, NamedAnchor>;

   public:
		HyperlinkManager (const HyperlinkManager&) =delete;
		HyperlinkManager (HyperlinkManager&&) =delete;
		void addHrefAnchor (const std::string &uri);
		void addNameAchor (const std::string &name, int pageno);
		void setActiveNameAnchor (const std::string &name, SpecialActions &actions);
		void closeAnchor (SpecialActions &actions);
		void checkNewLine (SpecialActions &actions);
		void createLink (std::string uri, SpecialActions &actions);
		void createViews (unsigned pageno, SpecialActions &actions);
		void setBaseUrl (std::string &base) {_base = base;}
		void setLineWidth (double w) {_linewidth = w;}
		static HyperlinkManager& instance ();
		static bool setLinkMarker (const std::string &marker);
		static void setDefaultLinkColor (Color color);

	protected:
		HyperlinkManager () =default;
		void markLinkedBox (SpecialActions &actions);

		enum class MarkerType {NONE, LINE, BOX, BGCOLOR};
		static MarkerType MARKER_TYPE;  ///< selects how to mark linked areas
		static Color LINK_BGCOLOR;      ///< background color if linkmark type == LT_BGCOLOR
		static Color LINK_LINECOLOR;    ///< line color if linkmark type is LM_LINE or LM_BOX
		static ColorSource COLORSOURCE; ///< if true, LINK_LINECOLOR is applied

   private:
		AnchorType _anchorType=AnchorType::NONE;  ///< type of active anchor
		int _depthThreshold=0;      ///< break anchor box if the DVI stack depth underruns this threshold
		double _linewidth=-1;       ///< line width of link marker (-1 => compute individual value per link)
		std::string _base;          ///< base URL that is prepended to all relative targets
		NamedAnchors _namedAnchors; ///< information about all named anchors processed
};

#endif
