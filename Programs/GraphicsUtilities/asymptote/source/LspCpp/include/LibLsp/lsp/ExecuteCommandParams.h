#pragma once

#include "lsAny.h"
struct ExecuteCommandParams {
	/**
	 * The identifier of the actual command handler.
	 */

	std::string command;

	/**
	 * Arguments that the command should be invoked with.
	 * The arguments are typically specified when a command is returned from the server to the client.
	 * Example requests that return a command are textDocument/codeAction or textDocument/codeLens.
	 */
	boost::optional<std::vector<lsp::Any>>  arguments;
	
	MAKE_SWAP_METHOD(ExecuteCommandParams, command, arguments);
};
MAKE_REFLECT_STRUCT(ExecuteCommandParams,command,arguments)
