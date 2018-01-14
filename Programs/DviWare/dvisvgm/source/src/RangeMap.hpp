/*************************************************************************
** RangeMap.hpp                                                         **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2018 Martin Gieseking <martin.gieseking@uos.de>   **
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

#ifndef RANGEMAP_HPP
#define RANGEMAP_HPP

#include <algorithm>
#include <ostream>
#include <vector>


class RangeMap {
	class Range
	{
		friend class RangeMap;

		public:
			Range () : _min(0), _max(0), _minval(0) {}

			Range (uint32_t min, uint32_t max, uint32_t minval) : _min(min), _max(max), _minval(minval) {
				if (_min > _max)
					std::swap(_min, _max);
			}

			uint32_t min () const                  {return _min;}
			uint32_t max () const                  {return _max;}
			uint32_t minval () const               {return _minval;}
			uint32_t maxval () const               {return valueAt(_max);}
			uint32_t valueAt (uint32_t c) const    {return c-_min+_minval;}
			bool operator < (const Range &r) const {return _min < r._min;}
			std::ostream& write (std::ostream &os) const;

		protected:
			void min (uint32_t m)                 {_min = m;}
			void max (uint32_t m)                 {_max = m;}
			void setMinAndAdaptValue (uint32_t c) {_minval = valueAt(c); _min = c;}
			bool join (const Range &r);

		private:
			uint32_t _min, _max;
			uint32_t _minval;
	};

	using Ranges = std::vector<Range>;

	public:
		void addRange (uint32_t first, uint32_t last, uint32_t cid);
		bool valueExists (uint32_t c) const  {return lookup(c) >= 0;}
		uint32_t valueAt (uint32_t c) const;
		size_t size () const                 {return _ranges.size();}
		bool empty () const                  {return _ranges.empty();}
		void clear ()                        {_ranges.clear();}
		std::ostream& write (std::ostream &os) const;

	protected:
		void adaptNeighbors (Ranges::iterator it);
		int lookup (uint32_t c) const;
		const Range& rangeAt (size_t n) const {return _ranges[n];}

	private:
		Ranges _ranges;
};


inline std::ostream& operator << (std::ostream& os, const RangeMap &rangemap) {
	return rangemap.write(os);
}

#endif
