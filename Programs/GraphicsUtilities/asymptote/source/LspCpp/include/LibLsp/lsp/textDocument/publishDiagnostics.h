#pragma once

#include "LibLsp/JsonRpc/NotificationInMessage.h"
#include "LibLsp/lsp/lsp_diagnostic.h"

// Diagnostics
namespace TextDocumentPublishDiagnostics{
  struct Params {
    // The URI for which diagnostic information is reported.
    lsDocumentUri uri;

    // An array of diagnostic information items.
    std::vector<lsDiagnostic> diagnostics;
	MAKE_SWAP_METHOD(Params,uri,diagnostics);
  };

  
};
MAKE_REFLECT_STRUCT(TextDocumentPublishDiagnostics::Params,
	uri,
	diagnostics);

/**
 * Diagnostics notifications are sent from the server to the client to
 * signal results of validation runs.
 */
DEFINE_NOTIFICATION_TYPE(Notify_TextDocumentPublishDiagnostics, TextDocumentPublishDiagnostics::Params, "textDocument/publishDiagnostics");

