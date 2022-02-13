#pragma once



#include "LibLsp/JsonRpc/NotificationInMessage.h"




namespace TextDocumentDidClose  {

  struct Params {
    lsTextDocumentIdentifier textDocument;
	void swap(Params& arg) noexcept
	{
		textDocument.swap(arg.textDocument);
	}
  	
  };

};

MAKE_REFLECT_STRUCT(TextDocumentDidClose::Params, textDocument);

/**
 * The document close notification is sent from the client to the server
 * when the document got closed in the client. The document's truth now
 * exists where the document's uri points to (e.g. if the document's uri is
 * a file uri the truth now exists on disk).
 *
 * Registration Options: TextDocumentRegistrationOptions
 */
DEFINE_NOTIFICATION_TYPE(Notify_TextDocumentDidClose, TextDocumentDidClose::Params, "textDocument/didClose");
