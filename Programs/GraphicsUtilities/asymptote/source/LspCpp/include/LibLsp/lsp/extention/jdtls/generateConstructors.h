#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include <string>
#include "checkHashCodeEqualsStatus.h"
#include "checkConstructorsStatus.h"


struct GenerateConstructorsParams {
	lsCodeActionParams context;
	std::vector<LspMethodBinding> constructors;
	std::vector< LspVariableBinding >fields;
	MAKE_SWAP_METHOD(GenerateConstructorsParams, context, fields)
};
MAKE_REFLECT_STRUCT(GenerateConstructorsParams, context, fields)

DEFINE_REQUEST_RESPONSE_TYPE(java_generateConstructors, GenerateConstructorsParams, lsWorkspaceEdit, "java/generateConstructors");

