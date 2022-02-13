#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include "LibLsp/lsp/CodeActionParams.h"
#include "checkHashCodeEqualsStatus.h"

struct  LspMethodBinding {
	 std::string bindingKey;
	 std::string name;
	std::vector< std::string> parameters;

	MAKE_SWAP_METHOD(LspMethodBinding, bindingKey, name, parameters);
};
MAKE_REFLECT_STRUCT(LspMethodBinding, bindingKey, name, parameters);


struct CheckConstructorsResponse {
	std::vector<LspMethodBinding> constructors;
	std::vector<LspVariableBinding>  fields;
	MAKE_SWAP_METHOD(CheckConstructorsResponse, constructors, fields)
};
MAKE_REFLECT_STRUCT(CheckConstructorsResponse, constructors,fields)

DEFINE_REQUEST_RESPONSE_TYPE(java_checkConstructorsStatus, lsCodeActionParams, CheckConstructorsResponse,"java/checkConstructorsStatus")




