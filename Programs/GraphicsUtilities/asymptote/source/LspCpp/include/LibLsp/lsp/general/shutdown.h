#pragma once


#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include "LibLsp/JsonRpc/RequestInMessage.h"

/**
 * The shutdown request is sent from the client to the server. It asks the
 * server to shutdown, but to not exit (otherwise the response might not be
 * delivered correctly to the client). There is a separate exit notification
 * that asks the server to exit.
 */

DEFINE_REQUEST_RESPONSE_TYPE(td_shutdown, boost::optional<JsonNull>, boost::optional<lsp::Any>, "shutdown");

