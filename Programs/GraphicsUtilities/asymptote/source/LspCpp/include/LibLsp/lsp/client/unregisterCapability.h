#pragma once

#include "LibLsp/lsp/lsDocumentUri.h"

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

/**
 * General parameters to unregister a capability.
 */

struct Unregistration {
	/**
	 * The id used to unregister the request or notification. Usually an id
	 * provided during the register request.
	 */

	std::string id;

	/**
	 * The method / capability to unregister for.
	 */

	std::string method;
	
	MAKE_SWAP_METHOD(Unregistration, id, method);
};
MAKE_REFLECT_STRUCT(Unregistration, id, method);
/**
 * The client/unregisterCapability request is sent from the server to the client to unregister
 * a previously registered capability.
 */
struct UnregistrationParams
{
	std::vector<Unregistration> unregisterations;
	MAKE_SWAP_METHOD(UnregistrationParams, unregisterations);
};

MAKE_REFLECT_STRUCT(UnregistrationParams, unregisterations);

DEFINE_REQUEST_RESPONSE_TYPE(Req_ClientUnregisterCapability, UnregistrationParams,JsonNull, "client/unregisterCapability");