#pragma once
#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include <LibLsp/lsp/lsCodeAction.h>

#include "LibLsp/lsp/CodeActionParams.h"



DEFINE_REQUEST_RESPONSE_TYPE(java_organizeImports, lsCodeActionParams, lsWorkspaceEdit, "java/organizeImports");


