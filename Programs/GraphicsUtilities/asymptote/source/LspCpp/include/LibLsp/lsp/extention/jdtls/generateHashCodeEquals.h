#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include <LibLsp/lsp/lsCodeAction.h>

#include "LibLsp/lsp/CodeActionParams.h"
#include "checkHashCodeEqualsStatus.h"

struct GenerateHashCodeEqualsParams {
	lsCodeActionParams context;
	std::vector<LspVariableBinding>  fields;
	bool regenerate= false;
	void swap(GenerateHashCodeEqualsParams& arg) noexcept
	{
		context.swap(arg.context);
		fields.swap(arg.fields);
		std::swap(regenerate, arg.regenerate);
	}
};
MAKE_REFLECT_STRUCT(GenerateHashCodeEqualsParams, context, fields, regenerate);

DEFINE_REQUEST_RESPONSE_TYPE(java_generateHashCodeEquals, GenerateHashCodeEqualsParams, lsWorkspaceEdit, "java/generateHashCodeEquals")


