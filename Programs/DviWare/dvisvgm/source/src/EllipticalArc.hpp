/*************************************************************************
** EllipticalArc.hpp                                                    **
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

#pragma once

#include "Bezier.hpp"
#include "BoundingBox.hpp"
#include "Matrix.hpp"
#include "Pair.hpp"

class EllipticalArc {
	public:
		struct CenterParams {
			DPair center;      ///< absolute coordinates of the center point
			double startAngle; ///< angle of start point relative to semi-major axis (in radians)
			double deltaAngle; ///< difference between angles of the start and end point (>0 if sweep flag is set)
		};

	public:
		EllipticalArc (const DPair &start, double rx, double ry, double angle, bool laf, bool sweep, const DPair &end);
		EllipticalArc (const DPair &center, double rx, double ry, double rot, double startAngle, double deltaAngle);
		CenterParams getCenterParams () const;
		double rx () const {return _rx;}
		double ry () const {return _ry;}
		bool largeArc () const {return _largeArc;}
		bool sweepPositive () const {return _sweepPositive;}
		double rotationAngle () const {return _rotationAngle;}
		DPair startPoint () const {return _startPoint;}
		DPair endPoint () const {return _endPoint;}
		bool isStraightLine () const {return _rx < 1e-7 || _ry < 1e-7;}
		BoundingBox getBBox () const;
		void transform (const Matrix &matrix);
		std::vector<Bezier> approximate () const;

	private:
		double _rx, _ry;       ///< length of semi-major and semi-minor axes
		double _rotationAngle; ///< rotation angle around center (in radians)
		bool _largeArc;        ///< if true, the longer arc from start to end point is chosen, else the shorter one
		bool _sweepPositive;   ///< if true, arc is drawn in direction of positive angles, else the opposite direction
		DPair _startPoint, _endPoint;  ///< absolute coordinates of start and end point
};

