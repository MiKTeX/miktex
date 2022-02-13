#pragma once

#include "LibLsp/JsonRpc/NotificationInMessage.h"


#include "workspaceFolders.h"




struct lsDidChangeConfigurationParams {
	/**
	  * The actual changed settings.
	  */
	lsp::Any settings;
	MAKE_SWAP_METHOD(lsDidChangeConfigurationParams, settings);
};

MAKE_REFLECT_STRUCT(lsDidChangeConfigurationParams, settings);

/**
 * A notification sent from the client to the server to signal the change of
 * configuration settings.
 */
DEFINE_NOTIFICATION_TYPE(Notify_WorkspaceDidChangeConfiguration, lsDidChangeConfigurationParams, "workspace/didChangeConfiguration");
