#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"



#include <string>
#include <vector>
#include "WorkspaceSymbolParams.h"
#include "LibLsp/lsp/method_type.h"
#include "LibLsp/lsp/textDocument/code_action.h"
#include "LibLsp/lsp/lsFormattingOptions.h"

namespace
RefactorProposalUtility
{
	extern  const char* APPLY_REFACTORING_COMMAND_ID;
	extern  const char* EXTRACT_VARIABLE_ALL_OCCURRENCE_COMMAND;
	extern  const char* EXTRACT_VARIABLE_COMMAND;
	extern  const char* EXTRACT_CONSTANT_COMMAND;
	extern  const char* EXTRACT_METHOD_COMMAND;
	extern  const char* EXTRACT_FIELD_COMMAND;
	extern  const char* CONVERT_VARIABLE_TO_FIELD_COMMAND;
	extern  const char* MOVE_FILE_COMMAND;
	extern  const char* MOVE_INSTANCE_METHOD_COMMAND;
	extern  const char* MOVE_STATIC_MEMBER_COMMAND;
	extern  const char* MOVE_TYPE_COMMAND;
};


struct RenamePosition {
	lsDocumentUri uri;
	int offset = 0;
	int length = 0;
	void swap(RenamePosition& arg) noexcept
	{
		uri.swap(arg.uri);
		std::swap(offset, arg.offset);
		std::swap(length, arg.length);
	}
};
MAKE_REFLECT_STRUCT(RenamePosition, uri, offset, length);

struct GetRefactorEditParams
{
	std::string command;
	std::vector<lsp::Any>  commandArguments;
	lsCodeActionParams context;
	boost::optional<lsFormattingOptions> options;
	MAKE_SWAP_METHOD(GetRefactorEditParams, command, context, options);
};
MAKE_REFLECT_STRUCT(GetRefactorEditParams, command, context, options);




struct RefactorWorkspaceEdit {
	/**
	 * The workspace edit this code action performs.
	 */
	 lsWorkspaceEdit edit;
	/**
	 * A command this code action executes. If a code action provides a edit and a
	 * command, first the edit is executed and then the command.
	 */
	
	boost::optional<std::string> errorMessage;
	
	boost::optional < lsCommandWithAny > command;
	
	MAKE_SWAP_METHOD(RefactorWorkspaceEdit, edit, command, errorMessage)
};
MAKE_REFLECT_STRUCT(RefactorWorkspaceEdit,edit,command,errorMessage)

DEFINE_REQUEST_RESPONSE_TYPE(java_getRefactorEdit, GetRefactorEditParams, RefactorWorkspaceEdit, "java/getRefactorEdit");

