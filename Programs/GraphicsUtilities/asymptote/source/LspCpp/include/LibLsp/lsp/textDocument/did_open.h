#pragma once


#include "LibLsp/JsonRpc/NotificationInMessage.h"

#include "LibLsp/lsp/lsTextDocumentItem.h"

// Open, view, change, close file
namespace TextDocumentDidOpen {

  struct Params {
    lsTextDocumentItem textDocument;

 

   /**
  * Legacy property to support protocol version 1.0 requests.
  */
    boost::optional<std::string> text;
  	
   MAKE_SWAP_METHOD(TextDocumentDidOpen::Params, textDocument, text);
  
  };

}
MAKE_REFLECT_STRUCT(TextDocumentDidOpen::Params, textDocument, text);

/**
 * The document open notification is sent from the client to the server to
 * signal newly opened text documents. The document's truth is now managed
 * by the client and the server must not try to read the document's truth
 * using the document's uri.
 *
 * Registration Options: TextDocumentRegistrationOptions
 */;


DEFINE_NOTIFICATION_TYPE(Notify_TextDocumentDidOpen, TextDocumentDidOpen::Params, "textDocument/didOpen");

