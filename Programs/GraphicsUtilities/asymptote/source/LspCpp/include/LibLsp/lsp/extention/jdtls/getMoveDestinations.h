#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include <string>
#include "LibLsp/lsp/lsAny.h"
#include "LibLsp/lsp/CodeActionParams.h"

struct MoveKindInfo
{
	static   std::string moveResource()
	{
		return "moveResource";
	}
	static   std::string moveInstanceMethod()
	{
		return "moveInstanceMethod";
	}
	static   std::string moveStaticMember()
	{
		return "moveStaticMember";
	}
};

struct MoveParams {
	/**
	 * The supported move kind: moveResource, moveInstanceMethod, moveStaticMember,
	 * moveTypeToNewFile.
	 */
	std::string moveKind;
	/**
	 * The selected resource uris when the move operation is triggered.
	 */
	std::vector<std::string> sourceUris;
	/**
	 * The code action params when the move operation is triggered.
	 */
	boost::optional<lsCodeActionParams>  params;
	/**
	 * The possible destination: a folder/package, class, instanceDeclaration.
	 */
	lsp::Any destination;
	bool updateReferences;
	void swap(MoveParams& arg) noexcept
	{
		moveKind.swap(arg.moveKind);
		sourceUris.swap(arg.sourceUris);
		params.swap(arg.params);
		destination.swap(arg.destination);
		std::swap(updateReferences, arg.updateReferences);
	}
};
MAKE_REFLECT_STRUCT(MoveParams, moveKind, sourceUris, params, destination, updateReferences);

struct MoveDestinationsResponse {
	std::string errorMessage;
	std::vector<lsp::Any > destinations;
	MAKE_SWAP_METHOD(MoveDestinationsResponse, errorMessage, destinations);
};
MAKE_REFLECT_STRUCT(MoveDestinationsResponse, errorMessage, destinations);

DEFINE_REQUEST_RESPONSE_TYPE(java_getMoveDestinations, MoveParams, MoveDestinationsResponse, "java/getMoveDestinations");



