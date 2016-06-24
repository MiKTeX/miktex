/*************************************************************************
** RangeMap.h                                                           **
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

#ifndef DVISVGM_RANGEMAP_H
#define DVISVGM_RANGEMAP_H

#include <algorithm>
#include <ostream>
#include <vector>
#include "types.h"


class RangeMap {
	class Range
	{
		friend class RangeMap;

		public:
			Range () : _min(0), _max(0), _minval(0) {}

			Range (UInt32 min, UInt32 max, UInt32 minval) : _min(min), _max(max), _minval(minval) {
				if (_min > _max)
					std::swap(_min, _max);
			}

			UInt32 min () const                    {return _min;}
			UInt32 max () const                    {return _max;}
			UInt32 minval () const                 {return _minval;}
			UInt32 maxval () const                 {return valueAt(_max);}
			UInt32 valueAt (UInt32 c) const        {return c-_min+_minval;}
			bool operator < (const Range &r) const {return _min < r._min;}
			std::ostream& write (std::ostream &os) const;

		protected:
			void min (UInt32 m)                 {_min = m;}
			void max (UInt32 m)                 {_max = m;}
			void setMinAndAdaptValue (UInt32 c) {_minval = valueAt(c); _min = c;}
			bool join (const Range &r);

		private:
			UInt32 _min, _max;
			UInt32 _minval;
	};

	typedef std::vector<Range> Ranges;

	public:
		void addRange (UInt32 first, UInt32 last, UInt32 cid);
		bool valueExists (UInt32 c) const         {return lookup(c) >= 0;}
		UInt32 valueAt (UInt32 c) const;
		size_t size () const                      {return _ranges.size();}
		bool empty () const                       {return _ranges.empty();}
		void clear ()                             {_ranges.clear();}
		std::ostream& write (std::ostream &os) const;

	protected:
		void adaptNeighbors (Ranges::iterator it);
		int lookup (UInt32 c) const;
		const Range& rangeAt (size_t n) const     {return _ranges[n];}

	private:
		Ranges _ranges;
};


inline std::ostream& operator << (std::ostream& os, const RangeMap &rangemap) {
	return rangemap.write(os);
}

#endif
