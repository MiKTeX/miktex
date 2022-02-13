#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include <string>
#include "LibLsp/lsp/CodeActionParams.h"
#include "getRefactorEdit.h"
#include "LibLsp/lsp/lsTextDocumentPositionParams.h"

struct  FindLinksParams {
	// Supported link types: superImplementation
	std::string type;
	lsTextDocumentPositionParams position;
	
	MAKE_SWAP_METHOD(FindLinksParams, type, position)
};
MAKE_REFLECT_STRUCT(FindLinksParams,type,position)

DEFINE_REQUEST_RESPONSE_TYPE(java_findLinks, FindLinksParams,lsp::Any, "java/findLinks");



