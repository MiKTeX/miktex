#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include <string>
#include "LibLsp/lsp/CodeActionParams.h"
#include "getMoveDestinations.h"
#include "getRefactorEdit.h"


DEFINE_REQUEST_RESPONSE_TYPE(java_move, MoveParams, RefactorWorkspaceEdit, "java/move");



