#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include <LibLsp/lsp/lsCodeAction.h>

#include "LibLsp/lsp/CodeActionParams.h"

struct LspVariableBinding {
	std::string bindingKey;
	std::string name;
	std::string type;
	bool isField;
	void swap(LspVariableBinding& arg) noexcept
	{
		bindingKey.swap(arg.bindingKey);
		name.swap(arg.name);
		type.swap(arg.type);
		std::swap(isField, arg.isField);
	}
};
MAKE_REFLECT_STRUCT(LspVariableBinding, bindingKey, name, type, isField)

struct CheckHashCodeEqualsResponse {
	 std::string type;
	 std::vector<LspVariableBinding>  fields;
	 std::vector<std::string> existingMethods;
	 MAKE_SWAP_METHOD(CheckHashCodeEqualsResponse, type, fields, type, existingMethods)
};
MAKE_REFLECT_STRUCT(CheckHashCodeEqualsResponse, type, fields, type, existingMethods)

DEFINE_REQUEST_RESPONSE_TYPE(java_checkHashCodeEqualsStatus,
	lsCodeActionParams, CheckHashCodeEqualsResponse, "java/checkHashCodeEqualsStatus")


