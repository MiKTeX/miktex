/*************************************************************************
** RangeMap.cpp                                                         **
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

#include <numeric>
#include "RangeMap.hpp"

using namespace std;


/** Tries to merge range r into this one. This is only possible if the ranges
 *  touch or overlap and if the assigned values match at the junction points.
 *  @param[in] r range to join
 *  @return true if join was successful */
bool RangeMap::Range::join (const Range &r) {
	// check most common cases first
	if (_max+1 < r._min || _min-1 > r._max)  // disjoint ranges?
		return false;
	if (r._min-1 == _max) {                  // does r touch *this on the right?
		if (valueAt(r._min) == r._minval) {
			_max = r._max;
			return true;
		}
		return false;
	}
	if (r._max+1 == _min) {                  // does r touch *this on the left
		if (r.valueAt(_min) == _minval) {
			setMinAndAdaptValue(r._min);
			return true;
		}
		return false;
	}
	// the following cases should be pretty rare
	if (r._min <= _min && r._max >= _max) {  // does r overlap *this on both sides?
		*this = r;
		return true;
	}
	if (r._min < _min) {                     // left overlap only?
		if (r.valueAt(_min) == _minval) {
			_min = r._min;
			_minval = r._minval;
			return true;
		}
		return false;
	}
	if (r._max > _max) {                     // right overlap only?
		if (valueAt(r._min) == r._minval) {
			_max = r._max;
			return true;
		}
		return false;
	}
	// r completely inside *this
	return valueAt(r._min) == r._minval;
}


/** Adds a new number range. The range describes a mapping from c to v(c), where
 *  \f$c \in [cmin,cmax]\f$ and \f$v(cmin):=vmin, v(c):=vmin+c-cmin\f$.
 *  @param[in] cmin smallest number in the range
 *  @param[in] cmax largest number in the range
 *  @param[in] vmin map value of cmin */
void RangeMap::addRange (uint32_t cmin, uint32_t cmax, uint32_t vmin) {
	if (cmin > cmax)
		swap(cmin, cmax);

	Range range(cmin, cmax, vmin);
	if (_ranges.empty())
		_ranges.emplace_back(std::move(range));
	else {
		// check for simple cases that can be handled pretty fast
		Range &lrange = *_ranges.begin();
		Range &rrange = *_ranges.rbegin();
		if (cmin > rrange.max()) {       // non-overlapping range at end of vector?
			if (!rrange.join(range))
				_ranges.emplace_back(std::move(range));
		}
		else if (cmax < lrange.min()) {  // non-overlapping range at begin of vector?
			if (!lrange.join(range))
				_ranges.emplace(_ranges.begin(), std::move(range));
		}
		else {
			// ranges overlap and/or must be inserted somewhere inside the vector
			auto it = lower_bound(_ranges.begin(), _ranges.end(), range);
			const bool at_end = (it == _ranges.end());
			if (at_end)
				--it;
			if (!it->join(range) && (it == _ranges.begin() || !(it-1)->join(range))) {
				if (it->min() < cmin && it->max() > cmax) { // new range completely inside an existing range?
					//split existing range
					uint32_t itmax = it->max();
					it->max(cmin-1);
					it = _ranges.emplace(it+1, Range(cmax+1, itmax, it->valueAt(cmax+1)));
				}
				else if (at_end)        // does new range overlap right side of last range in vector?
					it = _ranges.end();  // => append new range at end of vector
				it = _ranges.emplace(it, std::move(range));
			}
			adaptNeighbors(it);  // resolve overlaps
		}
	}
}


/** Adapts the left and right neighbor elements of a newly inserted range.
 *  The new range could overlap ranges in the neighborhood so that those must be
 *  adapted or removed. All ranges in the range vector are ordered ascendingly, i.e.
 *  [min_1, max_1],...,[min_n, max_n] where min_i < min_j for all i < j.
 *  @param[in] it pointer to the newly inserted range */
void RangeMap::adaptNeighbors (Ranges::iterator it) {
	if (it != _ranges.end()) {
		// adapt left neighbor
		if (it != _ranges.begin() && it->min() <= (it-1)->max()) {
			auto lit = it-1;  // points to left neighbor
			bool left_neighbor_valid = (it->min() > 0 && it->min()-1 >= lit->min());
			if (left_neighbor_valid)      // is adapted left neighbor valid?
				lit->max(it->min()-1);     // => assign new max value
			if (!left_neighbor_valid || it->join(*lit))
				it = _ranges.erase(lit);
		}
		// remove right neighbors completely overlapped by *it
		auto rit = it+1;    // points to right neighbor
		while (rit != _ranges.end() && it->max() >= rit->max()) { // complete overlap?
			_ranges.erase(rit);
			rit = it+1;
		}
		// adapt rightmost range partially overlapped by *it
		if (rit != _ranges.end()) {
			if (it->max() >= rit->min())
				rit->setMinAndAdaptValue(it->max()+1);
			// try to merge right neighbor into *this
			if (it->join(*rit))
				_ranges.erase(rit); // remove merged neighbor
		}
	}
}


/** Finds the index of the range that contains a given value c.
 *  @param[in] c find range that contains this value
 *  @return index of the range found, or -1 if range was not found */
int RangeMap::lookup (uint32_t c) const {
	// simple binary search
	int left=0, right=_ranges.size()-1;
	while (left <= right) {
		int mid = (left+right)/2;
		if (c < _ranges[mid].min())
			right = mid-1;
		else if (c > _ranges[mid].max())
			left = mid+1;
		else
			return mid;
	}
	return -1;
}


uint32_t RangeMap::valueAt (uint32_t c) const {
	int pos = lookup(c);
	return pos < 0 ? 0 : _ranges[pos].valueAt(c);
}


/** Returns the number of values mapped. */
size_t RangeMap::numValues () const {
	return std::accumulate(_ranges.begin(), _ranges.end(), 0, [](size_t sum, const Range &range) {
		return sum+range.max()-range.min()+1;
	});
}


ostream& RangeMap::Range::write (ostream& os) const {
	return os << '[' << _min << ',' << _max << "] => " << _minval;
}


ostream& RangeMap::write (ostream& os) const {
	for (const Range &range : _ranges)
		range.write(os) << '\n';
	return os;
}
