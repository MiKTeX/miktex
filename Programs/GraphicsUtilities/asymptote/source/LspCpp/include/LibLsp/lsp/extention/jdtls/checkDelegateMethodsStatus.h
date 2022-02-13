#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/CodeActionParams.h"
#include "checkHashCodeEqualsStatus.h"
#include "checkConstructorsStatus.h"

struct  LspDelegateField {
	LspVariableBinding field;
	std::vector<LspMethodBinding> delegateMethods;

	MAKE_SWAP_METHOD(LspDelegateField, field, delegateMethods);
};
MAKE_REFLECT_STRUCT(LspDelegateField, field, delegateMethods);


struct CheckDelegateMethodsResponse {
	std::vector<LspDelegateField> delegateFields;

	MAKE_SWAP_METHOD(CheckDelegateMethodsResponse, delegateFields)
};
MAKE_REFLECT_STRUCT(CheckDelegateMethodsResponse, delegateFields)

DEFINE_REQUEST_RESPONSE_TYPE(java_checkDelegateMethodsStatus,
	lsCodeActionParams, CheckDelegateMethodsResponse, "java/checkDelegateMethodsStatus");


