#pragma once


#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include <regex>
#include "LibLsp/lsp/lsp_completion.h"
#include "LibLsp/lsp/lsTextDocumentPositionParams.h"


// How a completion was triggered
enum class lsCompletionTriggerKind {
  // Completion was triggered by typing an identifier (24x7 code
  // complete), manual invocation (e.g Ctrl+Space) or via API.
  Invoked = 1,

  // Completion was triggered by a trigger character specified by
  // the `triggerCharacters` properties of the `CompletionRegistrationOptions`.
  TriggerCharacter = 2
};
MAKE_REFLECT_TYPE_PROXY(lsCompletionTriggerKind);


// Contains additional information about the context in which a completion
// request is triggered.
struct lsCompletionContext {
  // How the completion was triggered.
  lsCompletionTriggerKind triggerKind = lsCompletionTriggerKind::Invoked;

  // The trigger character (a single character) that has trigger code complete.
  // Is undefined if `triggerKind !== CompletionTriggerKind.TriggerCharacter`
  boost::optional<std::string> triggerCharacter;

  MAKE_SWAP_METHOD(lsCompletionContext, triggerKind, triggerCharacter);
};
MAKE_REFLECT_STRUCT(lsCompletionContext, triggerKind, triggerCharacter);

struct lsCompletionParams : lsTextDocumentPositionParams {
  // The completion context. This is only available it the client specifies to
  // send this using
  // `ClientCapabilities.textDocument.completion.contextSupport === true`
  boost::optional<lsCompletionContext> context;
	
  MAKE_SWAP_METHOD(lsCompletionParams, textDocument, position, context);
	
};
MAKE_REFLECT_STRUCT(lsCompletionParams, textDocument, position, context);









namespace TextDocumentComplete{
	
	typedef  std::pair< boost::optional<std::vector<lsCompletionItem>>, boost::optional<CompletionList> > Either;
	
};
extern  void Reflect(Reader& visitor, TextDocumentComplete::Either& value);

/**
 * The Completion request is sent from the client to the server to compute
 * completion items at a given cursor position. Completion items are
 * presented in the IntelliSense user interface. If computing complete
 * completion items is expensive servers can additional provide a handler
 * for the resolve completion item request. This request is sent when a
 * completion item is selected in the user interface.
 *
 * Registration Options: CompletionRegistrationOptions
 */
DEFINE_REQUEST_RESPONSE_TYPE(td_completion, lsCompletionParams, CompletionList , "textDocument/completion")







