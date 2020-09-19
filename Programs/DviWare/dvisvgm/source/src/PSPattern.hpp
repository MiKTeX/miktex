/*************************************************************************
** PSPattern.hpp                                                        **
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

#ifndef PSPATTERN_HPP
#define PSPATTERN_HPP

#include <memory>
#include <set>
#include <string>
#include "BoundingBox.hpp"
#include "Color.hpp"
#include "Matrix.hpp"
#include "XMLNode.hpp"


class SpecialActions;
class SVGTree;
class XMLElement;

class PSPattern {
	public:
		virtual ~PSPattern () =default;
		virtual int psID () const {return _id;}
		virtual std::string svgID () const;
		virtual void apply (SpecialActions &actions);

	protected:
		explicit PSPattern (int id) : _id(id) {}
		virtual std::unique_ptr<XMLElement> createPatternNode () const =0;

	private:
		int _id;  ///< PostSCript ID of this pattern
};


class PSTilingPattern : public PSPattern {
	public:
		virtual XMLElement* getContainerNode ()     {return _groupNode.get();}
		void apply (SpecialActions &actions) override;

	protected:
		PSTilingPattern (int id, BoundingBox &bbox, Matrix &matrix, double xstep, double ystep);
		std::unique_ptr<XMLElement> createPatternNode () const override;
		virtual std::unique_ptr<XMLElement> createClipNode () const;
		virtual std::unique_ptr<XMLElement> createGroupNode () const;
		virtual XMLElement* getGroupNode () const    {return _groupNodePtr;}
		virtual void setGroupNode (std::unique_ptr<XMLElement> node);

	private:
		BoundingBox _bbox;           ///< bounding box of the tile graphics
		Matrix _matrix;              ///< tile transformation
		double _xstep, _ystep;       ///< horizontal and vertical distance between neighboured tiles
		mutable std::unique_ptr<XMLElement> _groupNode;  ///< group containing the drawing elements
		XMLElement *_groupNodePtr; ///< keeps a pointer to the group node even after moving _groupNode to the SVGTree
};


class PSColoredTilingPattern final : public PSTilingPattern {
	public:
		PSColoredTilingPattern (int id, BoundingBox &bbox, Matrix &matrix, double xstep, double ystep);
};


class PSUncoloredTilingPattern final : public PSTilingPattern {
	public:
		PSUncoloredTilingPattern (int id, BoundingBox &bbox, Matrix &matrix, double xstep, double ystep);
		std::string svgID () const override;
		void setColor (Color color) {_currentColor = color;}
		void apply (SpecialActions &actions) override;

	protected:
		std::unique_ptr<XMLElement> createClipNode () const override;

	private:
		std::set<Color> _colors;  ///< colors this pattern has already been drawn with
		Color _currentColor;      ///< current color to be applied
		bool _applied;  ///< has pattern with current group node already been applied to the SVG tree?
};

#endif
