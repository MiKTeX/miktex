/*************************************************************************
** NumericRanges.hpp                                                    **
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

#ifndef NUMERICRANGES_HPP
#define NUMERICRANGES_HPP

#include <algorithm>
#include <list>
#include <utility>

template <class T>
class NumericRanges {
	public:
		using Range = std::pair<T,T>;
		using Container = std::list<Range>;
		using ConstIterator = typename Container::const_iterator;

	public:
		void addRange (T value)          {addRange(value, value);}
		void addRange (T first, T last);
		bool valueExists (T value) const;
		size_t size () const             {return _ranges.size();}
		ConstIterator begin () const     {return _ranges.begin();}
		ConstIterator end () const       {return _ranges.end();}
		const Container& ranges () const {return _ranges;}

	private:
		Container _ranges;
};


/** Adds a numeric range to the collection.
 *  @param[in] first left bound of new range
 *  @param[in] last right bound of new range */
template <class T>
void NumericRanges<T>::addRange (T first, T last) {
	if (first > last)
		std::swap(first, last);
	auto it = _ranges.begin();
	while (it != _ranges.end() && first > it->first+1 && first > it->second+1)
		++it;
	if (it == _ranges.end() || last < it->first-1 || first > it->second+1)
		it = _ranges.insert(it, Range(first, last));
	else if ((first < it->first && last >= it->first-1) || (first <= it->second+1 && last > it->second)) {
		it->first = std::min(it->first, first);
		it->second = std::max(it->second, last);
	}
	// merge adjacent ranges
	if (it != _ranges.end()) {
		auto l = it;
		auto r = it;
		if (l == _ranges.begin())
			l = _ranges.end();
		else
			--l;
		++r;
		bool l_modified = false;
		bool r_modified = false;
		if (l != _ranges.end() && l->second >= it->first-1) {
			l->first = std::min(l->first, it->first);
			l->second = std::max(l->second, it->second);
			l_modified = true;
		}
		if (r != _ranges.end() && r->first <= it->second+1) {
			r->first = std::min(r->first, it->first);
			r->second = std::max(r->second, it->second);
			r_modified = true;
		}
		if (l_modified || r_modified) {
			_ranges.erase(it);
			if (l_modified && r_modified && l->second >= r->first-1) {
				l->first = std::min(l->first, r->first);
				l->second = std::max(l->second, r->second);
				_ranges.erase(r);
			}
		}
	}
}


template <class T>
bool NumericRanges<T>::valueExists (T value) const {
	auto it = std::lower_bound(_ranges.begin(), _ranges.end(), Range(value, 0),
		[](const Range &r1, const Range &r2) {
			return r1.first < r2.first;
		});
	return (it != _ranges.end() && it->first <= value && it->second >= value);
}


#endif
