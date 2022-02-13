#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/CodeActionParams.h"
#include "OverridableMethod.h"





DEFINE_REQUEST_RESPONSE_TYPE(java_listOverridableMethods, lsCodeActionParams, OverridableMethodsResponse, "java/listOverridableMethods");




