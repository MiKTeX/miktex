#pragma once

#include "LibLsp/JsonRpc/message.h"
#include "OverridableMethod.h"
#include "LibLsp/lsp/CodeActionParams.h"
#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
struct AddOverridableMethodParams {
	lsCodeActionParams context;
	std::vector<OverridableMethod> overridableMethods;

	MAKE_SWAP_METHOD(AddOverridableMethodParams, context, overridableMethods);
};

MAKE_REFLECT_STRUCT(AddOverridableMethodParams, context, overridableMethods);

DEFINE_REQUEST_RESPONSE_TYPE(java_addOverridableMethods, AddOverridableMethodParams, lsWorkspaceEdit, "java/addOverridableMethods");


