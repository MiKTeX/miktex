#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include "LibLsp/lsp/lsTextDocumentIdentifier.h"
#include "LibLsp/lsp/lsRange.h"
#include "LibLsp/lsp/lsAny.h"

namespace TextDocumentDocumentLink
{

struct Params
{
    // The document to provide document links for.
    lsTextDocumentIdentifier textDocument;
    MAKE_SWAP_METHOD(Params, textDocument)
};

}; // namespace TextDocumentDocumentLink
MAKE_REFLECT_STRUCT(TextDocumentDocumentLink::Params, textDocument);

// A document link is a range in a text document that links to an internal or
// external resource, like another text document or a web site.
struct lsDocumentLink
{
    // The range this link applies to.
    lsRange range;
    // The uri this link points to. If missing a resolve request is sent later.
    optional<lsDocumentUri> target;

    optional<lsp::Any> data;

    MAKE_SWAP_METHOD(lsDocumentLink, range, target, data)
};
MAKE_REFLECT_STRUCT(lsDocumentLink, range, target, data);

DEFINE_REQUEST_RESPONSE_TYPE(
    td_links, TextDocumentDocumentLink::Params, std::vector<lsDocumentLink>, "textDocument/documentLink"
);

/**
 * The document link resolve request is sent from the client to the server to resolve the target of a given document link.
 */
DEFINE_REQUEST_RESPONSE_TYPE(td_linkResolve, lsDocumentLink, lsDocumentLink, "documentLink/resolve");
