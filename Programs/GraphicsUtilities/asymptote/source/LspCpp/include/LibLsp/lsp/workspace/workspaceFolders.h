#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/lsDocumentUri.h"
struct WorkspaceFolder {
	/**
	 * The associated URI for this workspace folder.
	 */

	lsDocumentUri uri;

	/**
	 * The name of the workspace folder. Defaults to the uri's basename.
	 */
	std::string name;

	MAKE_SWAP_METHOD(WorkspaceFolder, uri, name)
};
MAKE_REFLECT_STRUCT(WorkspaceFolder, uri, name);


/**
 * The workspace/workspaceFolders request is sent from the server to the client
 * to fetch the current open list of workspace folders.
 *
 * @return null in the response if only a single file is open in the tool,
 *         an empty array if a workspace is open but no folders are configured,
 *         the workspace folders otherwise.
 */
DEFINE_REQUEST_RESPONSE_TYPE(WorkspaceFolders, 
	boost::optional<JsonNull>, boost::optional<std::vector< WorkspaceFolder>>, "workspace/workspaceFolders");

