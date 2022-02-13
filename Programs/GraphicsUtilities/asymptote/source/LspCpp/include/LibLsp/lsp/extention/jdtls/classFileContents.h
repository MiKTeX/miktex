#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include <string>
#include "LibLsp/lsp/lsTextDocumentIdentifier.h"

DEFINE_REQUEST_RESPONSE_TYPE(java_classFileContents, lsTextDocumentIdentifier, std::string ,"java/classFileContents");

