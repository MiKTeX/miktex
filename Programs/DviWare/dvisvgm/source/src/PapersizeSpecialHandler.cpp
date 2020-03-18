/*************************************************************************
** PapersizeSpecialHandler.cpp                                          **
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

#include "Message.hpp"
#include "PapersizeSpecialHandler.hpp"
#include "SpecialActions.hpp"

using namespace std;

void PapersizeSpecialHandler::preprocess (const string&, std::istream &is, SpecialActions &actions) {
	string params;
	is >> params;
	Length w, h;
	const size_t splitpos = params.find(',');
	try {
		if (splitpos == string::npos) {
			w.set(params);
			h.set(params);
		}
		else {
			w.set(params.substr(0, splitpos));
			h.set(params.substr(splitpos+1));
		}
		storePaperSize(actions.getCurrentPageNumber(), w, h);
	}
	catch (UnitException &e) { // ignore invalid length units for now
	}
}


bool PapersizeSpecialHandler::process (const string&, std::istream&, SpecialActions&) {
	return true;
}


/** Records a paper size for a given page number for later processing. This function doesn't
 *  assign them to the page. */
void PapersizeSpecialHandler::storePaperSize (unsigned pageno, Length width, Length height) {
	DoublePair whpair(width.bp(), height.bp());
	if (_pageSizes.empty() || _pageSizes.back().second != whpair) {
		if (!_pageSizes.empty() && _pageSizes.back().first == pageno)
			_pageSizes.back().second = whpair;
		else
			_pageSizes.emplace_back(PageSize(pageno, whpair));
	}
}


/** Applies the previously recorded size to a given page. */
void PapersizeSpecialHandler::applyPaperSize (unsigned pageno, SpecialActions &actions) {
	// find page n >= pageno that contains a papersize special
	auto lb_it = lower_bound(_pageSizes.begin(), _pageSizes.end(), PageSize(pageno, DoublePair()),
		[](const PageSize &ps1, const PageSize &ps2) {
			// order PageSize objects by page number
			return ps1.first < ps2.first;
		});
	auto it = _pageSizes.end();
	if (lb_it != _pageSizes.end() && lb_it->first == pageno)
		it = lb_it;                        // if current page contains a papersize special, use it
	else if (lb_it != _pageSizes.begin()) // no papersize special on current page?
		it = lb_it-1;                      // => use the one on the nearest preceding page
	if (it == _pageSizes.end())
		Message::wstream(true) << "no valid papersize special found\n";
	else {
		DoublePair size = it->second;
		const double border = -72;  // DVI standard: coordinates of upper left paper corner are (-72bp, -72bp)
		actions.bbox() = BoundingBox(border, border, size.first+border, size.second+border);
	}
}


void PapersizeSpecialHandler::dviEndPage (unsigned pageno, SpecialActions &actions) {
	if (actions.getBBoxFormatString() == "papersize")
		applyPaperSize(pageno, actions);
}


vector<const char*> PapersizeSpecialHandler::prefixes() const {
	vector<const char*> pfx {"papersize="};
	return pfx;
}
