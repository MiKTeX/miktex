#pragma once

#include "WorkspaceSymbolParams.h"
#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"





enum class  BuildWorkspaceStatus : uint8_t{

	FAILED, SUCCEED, WITH_ERROR, CANCELLED,
};
MAKE_REFLECT_TYPE_PROXY(BuildWorkspaceStatus)

DEFINE_REQUEST_RESPONSE_TYPE(java_buildWorkspace, bool, BuildWorkspaceStatus, "java/buildWorkspace");