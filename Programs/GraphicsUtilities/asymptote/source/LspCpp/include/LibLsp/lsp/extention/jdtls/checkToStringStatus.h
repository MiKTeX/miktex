#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/CodeActionParams.h"
#include "checkHashCodeEqualsStatus.h"
struct CheckToStringResponse {
	std::string type;
	std::vector<LspVariableBinding>  fields;
	 bool exists;
	 void swap(CheckToStringResponse& arg) noexcept
	 {
		 type.swap(arg.type);
		 fields.swap(arg.fields);
		 std::swap(exists, arg.exists);
	 }
};
MAKE_REFLECT_STRUCT(CheckToStringResponse,type,fields,exists)

DEFINE_REQUEST_RESPONSE_TYPE(java_checkToStringStatus,
	lsCodeActionParams, CheckToStringResponse ,"java/checkToStringStatus")


