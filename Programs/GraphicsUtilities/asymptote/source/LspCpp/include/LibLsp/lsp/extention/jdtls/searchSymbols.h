#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"


#include <string>
#include <vector>
#include "WorkspaceSymbolParams.h"
#include "LibLsp/lsp/method_type.h"
#include "LibLsp/lsp/symbol.h"


struct SearchSymbolParams :public WorkspaceSymbolParams
{
	boost::optional<std::string>  projectName;
	boost::optional< bool >sourceOnly;
	boost::optional< int> maxResults;
	MAKE_SWAP_METHOD(SearchSymbolParams, query, projectName, sourceOnly, maxResults);
};
MAKE_REFLECT_STRUCT(SearchSymbolParams, query, projectName, sourceOnly, maxResults);


DEFINE_REQUEST_RESPONSE_TYPE(java_searchSymbols, SearchSymbolParams, std::vector<lsSymbolInformation>, "java/searchSymbols");



