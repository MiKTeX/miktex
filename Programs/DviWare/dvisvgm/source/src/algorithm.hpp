/*************************************************************************
** algorithm.hpp                                                        **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2025 Martin Gieseking <martin.gieseking@uos.de>   **
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

#include <algorithm>
#include <iterator>
#include <numeric>

namespace algo {

template <class Container, class T, class BinaryOp>
T accumulate (const Container &c, T init, BinaryOp op) {
	return std::accumulate(std::begin(c), std::end(c), std::move(init), op);
}

template <class Container, class UnaryOp>
bool any_of (const Container &c, UnaryOp op) {
	return std::any_of(std::begin(c), std::end(c), op);
}

template <class Container, class UnaryOp>
bool all_of (const Container &c, UnaryOp op) {
	return std::all_of(std::begin(c), std::end(c), op);
}

template <class Container, class UnaryOp>
bool none_of (const Container &c, UnaryOp op) {
	return std::none_of(std::begin(c), std::end(c), op);
}

template <class Container, class T, class Compare>
bool binary_search (const Container &c, T val, Compare cmp) {
	return std::binary_search(std::begin(c), std::end(c), std::move(val), cmp);
}

template <class Container, class OutputIt>
OutputIt copy (const Container &c, OutputIt out) {
	return std::copy(std::begin(c), std::end(c), out);
}

template <class Container, class OutputIt, class UnaryOp>
OutputIt copy_if (const Container &c, OutputIt out, UnaryOp op) {
	return std::copy_if(std::begin(c), std::end(c), out, op);
}

template <class Container, class UnaryOp>
size_t count_if (const Container &c, UnaryOp op) {
	return std::count_if(std::begin(c), std::end(c), op);
}

template <class Container, class UnaryOp>
auto erase_if (Container &c, UnaryOp op) -> decltype(std::begin(std::declval<Container&>()))
{
	return c.erase(std::remove_if(std::begin(c), std::end(c), op), std::end(c));
}

template <class Container, class T>
auto find (Container &c, const T &val) -> decltype(std::begin(std::declval<Container&>()))
{
	return std::find(std::begin(c), std::end(c), val);
}

template <class Container, class UnaryOp>
auto find_if (Container &c, UnaryOp op) -> decltype(std::begin(std::declval<Container&>()))
{
	return std::find_if(std::begin(c), std::end(c), op);
}

template <class Container, class Generator>
void generate (Container &c, Generator generator) {
	std::generate(std::begin(c), std::end(c), generator);
}

template <class Container, class T>
auto lower_bound (Container &c, const T &val) -> decltype(std::begin(std::declval<Container&>()))
{
	return std::lower_bound(std::begin(c), std::end(c), val);
}

template <class Container, class T, class Compare>
auto lower_bound (Container &c, const T &val, Compare cmp) -> decltype(std::begin(std::declval<Container&>()))
{
	return std::lower_bound(std::begin(c), std::end(c), val, cmp);
}

template <class Container>
auto max_element (Container &c) -> decltype(std::begin(std::declval<Container&>()))
{
	return std::max_element(std::begin(c), std::end(c));
}

template <class Container, class Compare>
void sort (Container &c, Compare cmp) {
	return std::sort(std::begin(c), std::end(c), cmp);
}

template <class Container, class OutputIt, class UnaryOp>
OutputIt transform (const Container &c, OutputIt out, UnaryOp op) {
	return std::transform(std::begin(c), std::end(c), out, op);
}

} // namespace algo
