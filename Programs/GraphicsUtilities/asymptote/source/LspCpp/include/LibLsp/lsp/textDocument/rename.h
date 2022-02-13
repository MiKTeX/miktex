#pragma once


#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/lsWorkspaceEdit.h"
#include "LibLsp/lsp/lsTextDocumentIdentifier.h"

namespace TextDocumentRename  {

  struct Params {
    // The document to format.
    lsTextDocumentIdentifier textDocument;

    // The position at which this request was sent.
    lsPosition position;

    // The new name of the symbol. If the given name is not valid the
    // request must return a [ResponseError](#ResponseError) with an
    // appropriate message set.
    std::string newName;
	MAKE_SWAP_METHOD(Params,
		textDocument,
		position,
		newName);
  };

};
MAKE_REFLECT_STRUCT(TextDocumentRename::Params,
                    textDocument,
                    position,
                    newName);
/**
 * The rename request is sent from the client to the server to do a
 * workspace wide rename of a symbol.
 *
 * Registration Options: TextDocumentRegistrationOptions
 */
DEFINE_REQUEST_RESPONSE_TYPE(td_rename, TextDocumentRename::Params, lsWorkspaceEdit, "textDocument/rename");

