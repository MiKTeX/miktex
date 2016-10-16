/*************************************************************************
** PSPattern.hpp                                                        **
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

#ifndef PSPATTERN_HPP
#define PSPATTERN_HPP

#include <set>
#include <string>
#include "BoundingBox.hpp"
#include "Color.hpp"
#include "Matrix.hpp"


class SpecialActions;
class SVGTree;
class XMLElementNode;

class PSPattern
{
	public:
		virtual ~PSPattern () =default;
		virtual int psID () const {return _id;}
		virtual std::string svgID () const;
		virtual void apply (SpecialActions &actions);

	protected:
		PSPattern (int id) : _id(id) {}
		virtual XMLElementNode* createPatternNode () const =0;

	private:
		int _id;  ///< PostSCript ID of this pattern
};


class PSTilingPattern : public PSPattern
{
	public:
		~PSTilingPattern ();
		virtual XMLElementNode* getContainerNode ()     {return _groupNode;}
		void apply (SpecialActions &actions) override;

	protected:
		PSTilingPattern (int id, BoundingBox &bbox, Matrix &matrix, double xstep, double ystep);
		XMLElementNode* createPatternNode () const override;
		virtual XMLElementNode* createClipNode () const;
		virtual XMLElementNode* createGroupNode () const;
		virtual XMLElementNode* getGroupNode () const    {return _groupNode;}
		virtual void setGroupNode (XMLElementNode *node) {_groupNode = node;}

	private:
		BoundingBox _bbox;           ///< bounding box of the tile graphics
		Matrix _matrix;              ///< tile transformation
		double _xstep, _ystep;       ///< horizontal and vertical distance between neighboured tiles
		XMLElementNode *_groupNode;  ///< group containing the drawing elements
};


class PSColoredTilingPattern : public PSTilingPattern
{
	public:
		PSColoredTilingPattern (int id, BoundingBox &bbox, Matrix &matrix, double xstep, double ystep);
};


class PSUncoloredTilingPattern : public PSTilingPattern
{
	public:
		PSUncoloredTilingPattern (int id, BoundingBox &bbox, Matrix &matrix, double xstep, double ystep);
		~PSUncoloredTilingPattern ();
		std::string svgID () const override;
		void setColor (Color color) {_currentColor = color;}
		void apply (SpecialActions &actions) override;

	protected:
		XMLElementNode* createClipNode () const override;

	private:
		std::set<Color> _colors;  ///< colors this pattern has already been drawn with
		Color _currentColor;      ///< current color to be applied
		bool _applied;  ///< has pattern with current group node already been applied to the SVG tree?
};

#endif
