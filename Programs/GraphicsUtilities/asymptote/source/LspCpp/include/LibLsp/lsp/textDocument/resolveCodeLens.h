#pragma once


#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "code_lens.h"

DEFINE_REQUEST_RESPONSE_TYPE(codeLens_resolve, lsCodeLens, lsCodeLens, "codeLens/resolve")

