#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/lsMarkedString.h"
#include "LibLsp/lsp/lsTextDocumentPositionParams.h"
#include "LibLsp/lsp/lsRange.h"
/**
 * The hover request is sent from the client to the server to request hover
 * information at a given text document position.
 *
 * Registration Options: TextDocumentRegistrationOptions
 */

namespace TextDocumentHover
{
typedef optional<std::vector<std::pair<optional<std::string>, optional<lsMarkedString>>>> Left;
typedef std::pair<Left, optional<MarkupContent>> Either;
struct Result
{
    /**
                 * The hover's content as markdown
                 */
    Either contents;

    /**
                 * An optional range
                 */
    optional<lsRange> range;

    MAKE_SWAP_METHOD(Result, contents, range)
};
} // namespace TextDocumentHover
MAKE_REFLECT_STRUCT(TextDocumentHover::Result, contents, range);

extern void Reflect(Reader& visitor, std::pair<optional<std::string>, optional<lsMarkedString>>& value);
extern void Reflect(Reader& visitor, TextDocumentHover::Either& value);

DEFINE_REQUEST_RESPONSE_TYPE(td_hover, lsTextDocumentPositionParams, TextDocumentHover::Result, "textDocument/hover")

//struct Rsp_TextDocumentHover : ResponseMessage< TextDocumentHover::Result, Rsp_TextDocumentHover> {
//
//};
//MAKE_REFLECT_STRUCT(Rsp_TextDocumentHover,
//      jsonrpc,
//      id,
//      result);

//MAKE_REFLECT_STRUCT_OPTIONALS_MANDATORY(Rsp_TextDocumentHover,
//                                        jsonrpc,
//                                                                               id,
//                                        result);
