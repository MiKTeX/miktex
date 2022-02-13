#pragma once

#include "LibLsp/lsp/ExecuteCommandParams.h"

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/JsonRpc/NotificationInMessage.h"



/**
 * The workspace/executeCommand request is sent from the client to the
 * server to trigger command execution on the server. In most cases the
 * server creates a WorkspaceEdit structure and applies the changes to the
 * workspace using the request workspace/applyEdit which is sent from the
 * server to the client.
 *
 * Registration Options: ExecuteCommandRegistrationOptions
 */
DEFINE_REQUEST_RESPONSE_TYPE(wp_executeCommand, ExecuteCommandParams, lsp::Any, "workspace/executeCommand");


DEFINE_NOTIFICATION_TYPE(Notify_sendNotification, ExecuteCommandParams, "workspace/notify")