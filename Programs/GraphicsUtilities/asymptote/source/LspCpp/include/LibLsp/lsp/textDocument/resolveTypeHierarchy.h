#pragma once



#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

#include "LibLsp/lsp/symbol.h"
#include "typeHierarchy.h"

struct ResolveTypeHierarchyItemParams {
	/**
	 * The hierarchy item to resolve.
	 */

	TypeHierarchyItem item;

	/**
	 * The number of hierarchy levels to resolve. {@code 0} indicates no hierarchy level.
	 */
	boost::optional<int>  resolve;

	/**
	 * The direction of the type hierarchy resolution.
	 */

	TypeHierarchyDirection direction;
	MAKE_SWAP_METHOD(ResolveTypeHierarchyItemParams, item, resolve, direction)
};
MAKE_REFLECT_STRUCT(ResolveTypeHierarchyItemParams,item,resolve,direction)
DEFINE_REQUEST_RESPONSE_TYPE(typeHierarchy_resolve, ResolveTypeHierarchyItemParams, TypeHierarchyItem, "typeHierarchy/resolve")
