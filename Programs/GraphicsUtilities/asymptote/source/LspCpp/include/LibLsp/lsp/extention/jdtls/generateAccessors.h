#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include <string>
#include "checkHashCodeEqualsStatus.h"
#include "resolveUnimplementedAccessors.h"


struct GenerateAccessorsParams {
	lsCodeActionParams context;
	std::vector<AccessorField>  accessors;


	MAKE_SWAP_METHOD(GenerateAccessorsParams, context, accessors)
};
MAKE_REFLECT_STRUCT(GenerateAccessorsParams, context, accessors)

DEFINE_REQUEST_RESPONSE_TYPE(java_generateAccessors, GenerateAccessorsParams, lsWorkspaceEdit, "java/generateAccessors");
