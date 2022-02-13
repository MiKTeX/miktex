#pragma once

#include "LibLsp/JsonRpc/NotificationInMessage.h"


#include <string>
#include <vector>
#include "WorkspaceSymbolParams.h"

#include "LibLsp/lsp/lsTextDocumentIdentifier.h"

DEFINE_NOTIFICATION_TYPE(java_projectConfigurationUpdate, lsTextDocumentIdentifier, "java/projectConfigurationUpdate");

