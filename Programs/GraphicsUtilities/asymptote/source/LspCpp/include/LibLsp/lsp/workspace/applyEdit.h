#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/lsDocumentUri.h"
#include "LibLsp/lsp/lsWorkspaceEdit.h"

struct  ApplyWorkspaceEditParams
{
	/**
	 * The edits to apply.
	 */

	lsWorkspaceEdit edit;

	/**
	 * An optional label of the workspace edit. This label is
	 * presented in the user interface for example on an undo
	 * stack to undo the workspace edit.
	 */
	std::string label;

	MAKE_SWAP_METHOD(ApplyWorkspaceEditParams, edit, label)
};
/**
 * The workspace/applyEdit request is sent from the server to the client to modify resource on the client side.
 */
MAKE_REFLECT_STRUCT(ApplyWorkspaceEditParams, edit, label);



struct  ApplyWorkspaceEditResponse
{
	bool applied;
	boost::optional<std::string> failureReason;
	MAKE_SWAP_METHOD(ApplyWorkspaceEditResponse, applied, failureReason)
};
MAKE_REFLECT_STRUCT(ApplyWorkspaceEditResponse, applied, failureReason);


DEFINE_REQUEST_RESPONSE_TYPE(WorkspaceApply, ApplyWorkspaceEditParams, ApplyWorkspaceEditResponse, "workspace/applyEdit");
