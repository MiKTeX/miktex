#pragma once


#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/lsFormattingOptions.h"


struct lsTextDocumentRangeFormattingParams {
  lsTextDocumentIdentifier textDocument;
  lsRange range;
  lsFormattingOptions options;

  MAKE_SWAP_METHOD(lsTextDocumentRangeFormattingParams,
	  textDocument,
	  range,
	  options)
};
MAKE_REFLECT_STRUCT(lsTextDocumentRangeFormattingParams,
                    textDocument,
                    range,
                    options);

/**
 * The document range formatting request is sent from the client to the
 * server to format a given range in a document.
 *
 * Registration Options: TextDocumentRegistrationOptions
 */
DEFINE_REQUEST_RESPONSE_TYPE(td_rangeFormatting, lsTextDocumentRangeFormattingParams, std::vector<lsTextEdit>,
	"textDocument/rangeFormatting");



