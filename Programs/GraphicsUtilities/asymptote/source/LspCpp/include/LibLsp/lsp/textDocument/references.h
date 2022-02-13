#pragma once


#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/symbol.h"


namespace  TextDocumentReferences {

  struct lsReferenceContext {
    // Include the declaration of the current symbol.
    boost::optional<bool>  includeDeclaration;
   MAKE_REFLECT_STRUCT(lsReferenceContext,
	   includeDeclaration)
  };
  struct Params {
    lsTextDocumentIdentifier textDocument;
    lsPosition position;
    lsReferenceContext context;
	MAKE_SWAP_METHOD(Params,
		textDocument,
		position,
		context)

  };

};
MAKE_REFLECT_STRUCT(TextDocumentReferences::lsReferenceContext,
                    includeDeclaration);
MAKE_REFLECT_STRUCT(TextDocumentReferences::Params,
                    textDocument,
                    position,
                    context);




/**
 * The references request is sent from the client to the server to resolve
 * project-wide references for the symbol denoted by the given text document
 * position.
 *
 * Registration Options: TextDocumentRegistrationOptions
 */
DEFINE_REQUEST_RESPONSE_TYPE(td_references, TextDocumentReferences::Params, 
    std::vector<lsLocation>, "textDocument/references");
