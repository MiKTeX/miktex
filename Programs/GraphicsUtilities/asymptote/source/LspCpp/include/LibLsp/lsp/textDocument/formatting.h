#pragma once
#include "LibLsp/lsp/lsFormattingOptions.h"

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"


namespace  TextDocumentFormatting  {

  struct Params {
	/**
	 * The document to format.
	*/
    lsTextDocumentIdentifier textDocument;
	/**
	 * The format options.
	 */
    lsFormattingOptions options;
	MAKE_SWAP_METHOD(Params, textDocument, options);
  };

};
MAKE_REFLECT_STRUCT(TextDocumentFormatting::Params, textDocument, options);
/**
 * The document formatting request is sent from the client to the server to
 * format a whole document.
 *
 * Registration Options: TextDocumentRegistrationOptions
 */
DEFINE_REQUEST_RESPONSE_TYPE(td_formatting, TextDocumentFormatting::Params,
	std::vector<lsTextEdit>, "textDocument/formatting");

