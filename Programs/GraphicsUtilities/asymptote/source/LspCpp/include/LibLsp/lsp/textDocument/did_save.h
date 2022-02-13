#pragma once



#include "LibLsp/JsonRpc/NotificationInMessage.h"


namespace TextDocumentDidSave  {

  struct Params {
    // The document that was saved.
    lsTextDocumentIdentifier textDocument;

    // Optional the content when saved. Depends on the includeText value
    // when the save notifcation was requested.
    boost::optional<std::string>  text;

	MAKE_SWAP_METHOD(TextDocumentDidSave::Params, textDocument, text);
  };

};
MAKE_REFLECT_STRUCT(TextDocumentDidSave::Params, textDocument, text);

/**
 * The document save notification is sent from the client to the server when
 * the document for saved in the client.
 *
 * Registration Options: TextDocumentSaveRegistrationOptions
 */
DEFINE_NOTIFICATION_TYPE(Notify_TextDocumentDidSave, TextDocumentDidSave::Params, "textDocument/didSave");

