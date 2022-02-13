#pragma once


#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/symbol.h"
#include "LibLsp/lsp/lsTextDocumentIdentifier.h"
 /**
  * The document symbol request is sent from the client to the server to list all symbols found in a given text document.
  */
struct lsDocumentSymbolParams {
  lsTextDocumentIdentifier textDocument;
  MAKE_SWAP_METHOD(lsDocumentSymbolParams, textDocument)
};
MAKE_REFLECT_STRUCT(lsDocumentSymbolParams, textDocument);



struct  TextDocumentDocumentSymbol{
	typedef  std::pair< boost::optional<lsSymbolInformation>  , boost::optional<lsDocumentSymbol> > Either;
};
void Reflect(Reader& visitor, TextDocumentDocumentSymbol::Either& value);



/**
 * The document symbol request is sent from the client to the server to list all
 * symbols found in a given text document.
 *
 * Registration Options: {@link TextDocumentRegistrationOptions}
 *
 * <p>
 * <b>Caveat</b>: although the return type allows mixing the
 * {@link DocumentSymbol} and {@link SymbolInformation} instances into a list do
 * not do it because the clients cannot accept a heterogeneous list. A list of
 * {@code DocumentSymbol} instances is only a valid return value if the
 * {@link DocumentSymbolCapabilities#getHierarchicalDocumentSymbolSupport()
 * textDocument.documentSymbol.hierarchicalDocumentSymbolSupport} is
 * {@code true}. More details on this difference between the LSP and the LSP4J
 * can be found <a href="https://github.com/eclipse/lsp4j/issues/252">here</a>.
 * </p>
 */

//DEFINE_REQUEST_RESPONSE_TYPE(td_symbol, 
//	lsDocumentSymbolParams,
//	std::vector<TextDocumentDocumentSymbol::Either> );
//

DEFINE_REQUEST_RESPONSE_TYPE(td_symbol, 
	lsDocumentSymbolParams,
	std::vector< lsDocumentSymbol >,"textDocument/documentSymbol" );



