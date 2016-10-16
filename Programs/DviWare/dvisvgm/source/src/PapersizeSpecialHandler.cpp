#include "Length.hpp"
#include "Message.hpp"
#include "PapersizeSpecialHandler.hpp"
#include "SpecialActions.hpp"

using namespace std;

void PapersizeSpecialHandler::preprocess (const char*, std::istream &is, SpecialActions &actions) {
	string params;
	is >> params;
	Length w, h;
	size_t splitpos = params.find(',');
	if (splitpos == string::npos) {
		w.set(params);
		h.set(params);
	}
	else {
		w.set(params.substr(0, splitpos));
		h.set(params.substr(splitpos+1));
	}
	DoublePair whpair(w.bp(), h.bp());
	if (_pageSizes.empty() || _pageSizes.back().second != whpair) {
		unsigned pageno = actions.getCurrentPageNumber();
		if (!_pageSizes.empty() && _pageSizes.back().first == pageno)
			_pageSizes.back().second = whpair;
		else
			_pageSizes.emplace_back(PageSize(pageno, whpair));
	}
}


bool PapersizeSpecialHandler::process (const char *, std::istream &, SpecialActions&) {
	return true;
}


void PapersizeSpecialHandler::dviEndPage (unsigned pageno, SpecialActions &actions) {
	if (actions.getBBoxFormatString() != "papersize")
		return;

	// find number of page with size change not lower than the current one
	typedef vector<PageSize>::iterator Iterator;
	Iterator lb_it = lower_bound(_pageSizes.begin(), _pageSizes.end(), PageSize(pageno, DoublePair()),
		[](const PageSize &ps1, const PageSize &ps2) {
			// order PageSize objects by page number
			return ps1.first < ps2.first;
		});
	Iterator it = _pageSizes.end();
	if (lb_it != _pageSizes.end() && lb_it->first == pageno)
		it = lb_it;
	else if (lb_it != _pageSizes.begin())
		it = lb_it-1;
	if (it == _pageSizes.end())
		Message::wstream(true) << "no valid papersize special found\n";
	else {
		DoublePair size = it->second;
		const double border = -72;
		actions.bbox() = BoundingBox(border, border, size.first+border, size.second+border);
	}
}


const char** PapersizeSpecialHandler::prefixes () const {
	static const char *pfx[] = {"papersize=", 0};
	return pfx;
}
