#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include <string>
#include "checkHashCodeEqualsStatus.h"


struct GenerateToStringParams {
	lsCodeActionParams context;
	std::vector< LspVariableBinding >fields;
	
	MAKE_SWAP_METHOD(GenerateToStringParams, context, fields)
	
};
MAKE_REFLECT_STRUCT(GenerateToStringParams, context, fields)

DEFINE_REQUEST_RESPONSE_TYPE(java_generateToString, GenerateToStringParams, lsWorkspaceEdit, "java/generateToString");
