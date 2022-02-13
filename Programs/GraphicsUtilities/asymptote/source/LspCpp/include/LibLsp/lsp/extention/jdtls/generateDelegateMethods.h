#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include <string>
#include "checkHashCodeEqualsStatus.h"
#include "checkConstructorsStatus.h"

struct  LspDelegateEntry {
	LspVariableBinding field;
	LspMethodBinding delegateMethod;
	MAKE_SWAP_METHOD(LspDelegateEntry, field, delegateMethod);
};
MAKE_REFLECT_STRUCT(LspDelegateEntry, field, delegateMethod);


struct GenerateDelegateMethodsParams {
	lsCodeActionParams context;
	std::vector<LspDelegateEntry> delegateEntries;
	MAKE_SWAP_METHOD(GenerateDelegateMethodsParams, context, delegateEntries)
};
MAKE_REFLECT_STRUCT(GenerateDelegateMethodsParams, context, delegateEntries)

DEFINE_REQUEST_RESPONSE_TYPE(java_generateDelegateMethods, GenerateDelegateMethodsParams, lsWorkspaceEdit, "java/generateDelegateMethods");

