#pragma once
#include "LibLsp/JsonRpc/NotificationInMessage.h"
#include "LibLsp/lsp/workspace/workspaceFolders.h"
/**
 * The workspace folder change event.
 */
struct WorkspaceFoldersChangeEvent {
	/**
	 * The array of added workspace folders
	 */

	std::vector<WorkspaceFolder> added;

	/**
	 * The array of the removed workspace folders
	 */

	std::vector <WorkspaceFolder> removed;
	MAKE_SWAP_METHOD(WorkspaceFoldersChangeEvent, added, removed);
	
};
MAKE_REFLECT_STRUCT(WorkspaceFoldersChangeEvent, added, removed);

struct DidChangeWorkspaceFoldersParams {
	/**
	 * The actual workspace folder change event.
	 */

	WorkspaceFoldersChangeEvent event;
	
	MAKE_SWAP_METHOD(DidChangeWorkspaceFoldersParams, event);
};
MAKE_REFLECT_STRUCT(DidChangeWorkspaceFoldersParams, event);


/**
 * The workspace/didChangeWorkspaceFolders notification is sent from the client
 * to the server to inform the server about workspace folder configuration changes.
 * The notification is sent by default if both ServerCapabilities/workspaceFolders
 * and ClientCapabilities/workspace/workspaceFolders are true; or if the server has
 * registered to receive this notification it first.
 */
DEFINE_NOTIFICATION_TYPE(Notify_WorkspaceDidChangeWorkspaceFolders, 
	DidChangeWorkspaceFoldersParams, "workspace/didChangeWorkspaceFolders");




