#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"


namespace TextDocumentDocumentLink  {

  struct Params {
    // The document to provide document links for.
    lsTextDocumentIdentifier textDocument;
	MAKE_SWAP_METHOD(Params, textDocument)
  };

};
MAKE_REFLECT_STRUCT(TextDocumentDocumentLink::Params, textDocument);




// A document link is a range in a text document that links to an internal or
// external resource, like another text document or a web site.
struct lsDocumentLink {
  // The range this link applies to.
  lsRange range;
  // The uri this link points to. If missing a resolve request is sent later.
  boost::optional<lsDocumentUri> target;

  boost::optional<lsp::Any> data;
	
  MAKE_SWAP_METHOD(lsDocumentLink, range, target, data)
	
};
MAKE_REFLECT_STRUCT(lsDocumentLink, range, target,data);


DEFINE_REQUEST_RESPONSE_TYPE(td_links, TextDocumentDocumentLink::Params, lsDocumentLink, "textDocument/documentLink");


/**
 * The document link resolve request is sent from the client to the server to resolve the target of a given document link.
 */
DEFINE_REQUEST_RESPONSE_TYPE(td_linkResolve, std::vector<lsDocumentLink>, lsDocumentLink, "documentLink/resolve");

