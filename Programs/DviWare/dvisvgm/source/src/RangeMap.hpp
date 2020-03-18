/*************************************************************************
** RangeMap.hpp                                                         **
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

#ifndef RANGEMAP_HPP
#define RANGEMAP_HPP

#include <algorithm>
#include <ostream>
#include <vector>


class RangeMap {
	class Range {
		friend class RangeMap;
		public:
			Range () =default;

			Range (uint32_t min, uint32_t max, uint32_t minval) : _min(min), _max(max), _minval(minval) {
				if (_min > _max)
					std::swap(_min, _max);
			}

			uint32_t min () const                  {return _min;}
			uint32_t max () const                  {return _max;}
			uint32_t minval () const               {return _minval;}
			uint32_t maxval () const               {return valueAt(_max);}
			uint32_t valueAt (uint32_t c) const    {return c-_min+_minval;}
			uint32_t numKeys () const              {return _max-_min+1;}
			bool operator < (const Range &r) const {return _min < r._min;}
			std::ostream& write (std::ostream &os) const;

		protected:
			void min (uint32_t m)                 {_min = m;}
			void max (uint32_t m)                 {_max = m;}
			void setMinAndAdaptValue (uint32_t c) {_minval = valueAt(c); _min = c;}
			bool join (const Range &r);

		private:
			uint32_t _min=0, _max=0;
			uint32_t _minval=0;
	};

	using Ranges = std::vector<Range>;

	class Iterator {
		friend class RangeMap;
		public:
			void operator ++ () {
				if (_currentKey < _rangeIterator->max())
					++_currentKey;
				else {
					if (++_rangeIterator != _rangeMap._ranges.end())
						_currentKey = _rangeIterator->min();
				}
			}

			void operator -- () {
				if (_currentKey > _rangeIterator->min())
					--_currentKey;
				else {
					if (_rangeIterator != _rangeMap._ranges.begin())
						_currentKey = (--_rangeIterator)->max();
				}
			}

			std::pair<uint32_t,uint32_t> operator * () const {
				return std::pair<uint32_t,uint32_t>(_currentKey, _rangeMap.valueAt(_currentKey));
			}

			bool operator == (const Iterator &it) const {
				bool ret = (_rangeIterator == it._rangeIterator);
				if (ret && _rangeIterator != _rangeMap._ranges.end())
					ret = (_currentKey == it._currentKey);
				return ret;
			}

			bool operator != (const Iterator &it) const {
				bool ret = (_rangeIterator != it._rangeIterator);
				if (!ret && _rangeIterator != _rangeMap._ranges.end())
					ret = _currentKey != it._currentKey;
				return ret;
			}

		protected:
			Iterator (const RangeMap &rangeMap, Ranges::const_iterator it) : _rangeMap(rangeMap), _rangeIterator(it), _currentKey() {
				if (_rangeIterator != _rangeMap._ranges.end())
					_currentKey = _rangeIterator->min();
			}

		private:
			const RangeMap &_rangeMap;
			Ranges::const_iterator _rangeIterator;
			uint32_t _currentKey;
	};

	public:
		void addRange (uint32_t first, uint32_t last, uint32_t cid);
		bool valueExists (uint32_t c) const  {return lookup(c) >= 0;}
		uint32_t valueAt (uint32_t c) const;
		uint32_t minKey () const               {return _ranges.empty() ? 0 : _ranges.front().min();}
		uint32_t maxKey () const               {return _ranges.empty() ? 0 : _ranges.back().max();}
		size_t numRanges () const              {return _ranges.size();}
		size_t numValues () const;
		bool empty () const                    {return _ranges.empty();}
		void clear ()                          {_ranges.clear();}
		const Range& getRange (size_t n) const {return _ranges[n];}
		Iterator begin () const                {return Iterator(*this, _ranges.begin());}
		Iterator end () const                  {return Iterator(*this, _ranges.end());}
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
