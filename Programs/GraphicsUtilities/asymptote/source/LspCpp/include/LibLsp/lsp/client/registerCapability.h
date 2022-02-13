#pragma once
#include "LibLsp/JsonRpc/message.h"
#include "LibLsp/lsp/lsDocumentUri.h"

/**
 * General parameters to register for a capability.
 */

struct Registration {
	static  Registration Create(const std::string& method);
	/**
	 * The id used to register the request. The id can be used to deregister
	 * the request again.
	 */
	std::string id;

	/**
	 * The method / capability to register for.
	 */

	std::string method;
	
	MAKE_SWAP_METHOD(Registration, id, method);
};


MAKE_REFLECT_STRUCT(Registration, id, method);

/**
 * The client/registerCapability request is sent from the server to the client to register
 * for a new capability on the client side. Not all clients need to support dynamic
 * capability registration. A client opts in via the dynamicRegistration property on the
 * specific client capabilities. A client can even provide dynamic registration for
 * capability A but not for capability B (see TextDocumentClientCapabilities as an example).
 */
struct RegistrationParams
{
	std::vector<Registration> registrations;
	MAKE_SWAP_METHOD(RegistrationParams, registrations);
};
/**
 * The client/registerCapability request is sent from the server to the client
 * to register for a new capability on the client side.
 * Not all clients need to support dynamic capability registration.
 * A client opts in via the ClientCapabilities.dynamicRegistration property
 */
MAKE_REFLECT_STRUCT(RegistrationParams, registrations);

DEFINE_REQUEST_RESPONSE_TYPE(Req_ClientRegisterCapability, RegistrationParams,JsonNull, "client/registerCapability");