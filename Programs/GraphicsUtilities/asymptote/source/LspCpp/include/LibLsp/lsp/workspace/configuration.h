#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/lsDocumentUri.h"


struct ConfigurationItem {
	/**
	 * The scope to get the configuration section for.
	 */
	lsDocumentUri scopeUri;

	/**
	 * The configuration section asked for.
	 */
	std::string section;
	MAKE_SWAP_METHOD(ConfigurationItem, scopeUri, section);
};
MAKE_REFLECT_STRUCT(ConfigurationItem, scopeUri, section);
struct ConfigurationParams
{
	std::vector<ConfigurationItem> items;
	MAKE_SWAP_METHOD(ConfigurationParams, items)
};

MAKE_REFLECT_STRUCT(ConfigurationParams, items);

/**
 * The workspace/configuration request is sent from the server to the client to fetch
 * configuration settings from the client. The request can fetch n configuration settings
 * in one roundtrip. The order of the returned configuration settings correspond to the
 * order of the passed ConfigurationItems (e.g. the first item in the response is the
 * result for the first configuration item in the params).
 */
DEFINE_REQUEST_RESPONSE_TYPE(WorkspaceConfiguration, ConfigurationParams,std::vector<lsp::Any>, "workspace/configuration");