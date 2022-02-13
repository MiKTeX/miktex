#pragma once

#include "LibLsp/JsonRpc/serializer.h"
#include <string>


struct  WorkspaceSymbolParams
{
	std::string query;
	MAKE_SWAP_METHOD(WorkspaceSymbolParams, query);
};
MAKE_REFLECT_STRUCT(WorkspaceSymbolParams, query);

