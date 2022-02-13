#pragma once


#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/lsFormattingOptions.h"
#include "LibLsp/lsp/lsTextDocumentIdentifier.h"
#include "LibLsp/lsp/lsRange.h"
#include "LibLsp/lsp/lsTextEdit.h"


struct lsDocumentOnTypeFormattingParams
{
  lsTextDocumentIdentifier textDocument;
  lsFormattingOptions options;

  lsPosition position;

	 /**
	  * The character that has been typed.
	  */

  std::string ch;

  MAKE_SWAP_METHOD(lsDocumentOnTypeFormattingParams,
	  textDocument,
	  position,
	  options, ch);
};
MAKE_REFLECT_STRUCT(lsDocumentOnTypeFormattingParams,
                    textDocument,
					position,
                    options,ch);

/**
 * The document range formatting request is sent from the client to the
 * server to format a given range in a document.
 *
 * Registration Options: TextDocumentRegistrationOptions
 */
DEFINE_REQUEST_RESPONSE_TYPE(td_onTypeFormatting,
	lsDocumentOnTypeFormattingParams, std::vector<lsTextEdit>, "textDocument/onTypeFormatting");

