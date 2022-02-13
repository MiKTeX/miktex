#pragma once


#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/lsTextDocumentIdentifier.h"

namespace WillSaveTextDocumentParams {

	/**
	 * Represents reasons why a text document is saved.
	 */
	enum class TextDocumentSaveReason {

		/**
		 * Manually triggered, e.g. by the user pressing save, by starting debugging,
		 * or by an API call.
		 */
		Manual=(1),

		/**
		 * Automatic after a delay.
		 */
		 AfterDelay=(2),

		 /**
		  * When the editor lost focus.
		  */
		  FocusOut=(3)
	};
	
  struct Params {
	  /**
	   * The document that will be saved.
	   */
    lsTextDocumentIdentifier textDocument;

   /*
   * A reason why a text document is saved.
   */

	  boost::optional<TextDocumentSaveReason>  reason;

		MAKE_SWAP_METHOD(Params, textDocument, reason);
  };

};
MAKE_REFLECT_TYPE_PROXY(WillSaveTextDocumentParams::TextDocumentSaveReason);

MAKE_REFLECT_STRUCT(WillSaveTextDocumentParams::Params, textDocument, reason);

/**
 * The document save notification is sent from the client to the server when
 * the document for saved in the client.
 *
 * Registration Options: TextDocumentSaveRegistrationOptions
 */
DEFINE_NOTIFICATION_TYPE(td_willSave, WillSaveTextDocumentParams::Params, "textDocument/willSave");

/**
 * The document will save request is sent from the client to the server before the document is actually saved.
 * The request can return an array of TextEdits which will be applied to the text document before it is saved.
 * Please note that clients might drop results if computing the text edits took too long or if a server constantly fails on this request.
 * This is done to keep the save fast and reliable.
 *
 * Registration Options: TextDocumentRegistrationOptions
 */
DEFINE_REQUEST_RESPONSE_TYPE(td_willSaveWaitUntil,
	WillSaveTextDocumentParams::Params, std::vector<lsTextEdit>, "textDocument/willSaveWaitUntil");

