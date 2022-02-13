#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/lsWorkspaceEdit.h"
#include "LibLsp/lsp/lsTextDocumentPositionParams.h"

/**
 * The rename request is sent from the client to the server to do a
 * workspace wide rename of a symbol.
 *
 * Registration Options: TextDocumentRegistrationOptions
 */
struct PrepareRenameResult{
	/**
	 * The range of the string to rename
	 */
	
	 lsRange range;

/**
 * A placeholder text of the string content to be renamed.
 */

	std::string placeholder;
	
	MAKE_SWAP_METHOD(PrepareRenameResult, range, placeholder)
	
};
MAKE_REFLECT_STRUCT(PrepareRenameResult,range,placeholder)



typedef  std::pair< boost::optional< lsRange>, boost::optional<PrepareRenameResult>> TextDocumentPrepareRenameResult;
extern void  Reflect(Reader& visitor, TextDocumentPrepareRenameResult& value);


DEFINE_REQUEST_RESPONSE_TYPE(td_prepareRename, 
	lsTextDocumentPositionParams, TextDocumentPrepareRenameResult, "textDocument/prepareRename");