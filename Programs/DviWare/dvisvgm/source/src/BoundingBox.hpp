/*************************************************************************
** BoundingBox.hpp                                                      **
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

#ifndef BOUNDINGBOX_HPP
#define BOUNDINGBOX_HPP

#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include "Length.hpp"
#include "MessageException.hpp"
#include "Pair.hpp"


class Matrix;
class XMLElement;


struct BoundingBoxException : MessageException {
	explicit BoundingBoxException (const std::string &msg) : MessageException(msg) {}
};


class BoundingBox {
	public:
		BoundingBox ();
		BoundingBox (double ulxx, double ulyy, double lrxx, double lryy);
		BoundingBox (const DPair &p1, const DPair &p2);
		BoundingBox (const Length &ulxx, const Length &ulyy, const Length &lrxx, const Length &lryy);
		explicit BoundingBox (const std::string &boxstr);
		void set (const std::string &boxstr);
		void set (const std::vector<Length> &lengths);
		void embed (double x, double y);
		void embed (const BoundingBox &bb);
		void embed (const DPair &p) {embed(p.x(), p.y());}
		void embed (const DPair &c, double r);

		static std::vector<Length> extractLengths (std::string boxstr);

		template <typename T>
		void embed (const Pair<T> &p) {embed(p.x(), p.y());}

		void expand (double m);
		bool  intersect (const BoundingBox &bbox);
		double minX () const        {return _ulx;}
		double minY () const        {return _uly;}
		double maxX () const        {return _lrx;}
		double maxY () const        {return _lry;}
		double width () const       {return _lrx-_ulx;}
		double height () const      {return _lry-_uly;}
		bool valid() const          {return _valid;}
		bool locked() const         {return _locked;}
		void lock ()                {_locked = true;}
		void unlock ()              {_locked = false;}
		void invalidate ()          {_valid = false;}
		void operator += (const BoundingBox &bbox);
		bool operator == (const BoundingBox &bbox) const;
		bool operator != (const BoundingBox &bbox) const {return !(*this == bbox);}
		void scale (double sx, double sy);
		void transform (const Matrix &tm);
		std::string toSVGViewBox () const;
		std::ostream& write (std::ostream &os) const;
		std::unique_ptr<XMLElement> createSVGRect () const;

	private:
		double _ulx, _uly; ///< coordinates of upper left vertex (in PS point units)
		double _lrx, _lry; ///< coordinates of lower right vertex (in PS point units)
		bool _valid : 1;   ///< true if the box coordinates are properly set
		bool _locked : 1;  ///< if true, the box data is read-only
};


inline std::ostream& operator << (std::ostream &os, const BoundingBox &bbox) {
	return bbox.write(os);
}

#endif
