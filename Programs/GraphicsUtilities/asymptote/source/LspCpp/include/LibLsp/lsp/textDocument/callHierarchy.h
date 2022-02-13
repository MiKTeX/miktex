#pragma once

#include "LibLsp/lsp/lsAny.h"
#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include "LibLsp/lsp/symbol.h"
#include "LibLsp/lsp/lsTextDocumentPositionParams.h"
#include "LibLsp/lsp/lsRange.h"

enum class SymbolTag { Deprecated = 1 };
MAKE_REFLECT_TYPE_PROXY(SymbolTag)

struct CallHierarchyPrepareParams
{
	lsTextDocumentIdentifier textDocument;
	lsPosition position;

	MAKE_SWAP_METHOD(CallHierarchyPrepareParams,
		textDocument,
		position)
};
MAKE_REFLECT_STRUCT(CallHierarchyPrepareParams,
	textDocument,
	position)



	/// Represents programming constructs like functions or constructors
	/// in the context of call hierarchy.
struct CallHierarchyItem {
	/// The name of this item.
	std::string name;

	/// The kind of this item.
	SymbolKind kind;

	/// Tags for this item.
	boost::optional<std::vector<SymbolTag>>  tags;

	/// More detaill for this item, e.g. the signature of a function.
	boost::optional<std::string>  detail;

	/// The resource identifier of this item.
	lsDocumentUri uri;

	/**
	 * The range enclosing this symbol not including leading/trailing whitespace
	 * but everything else, e.g. comments and code.
	 */
	lsRange range;

	/**
	 * The range that should be selected and revealed when this symbol is being
	 * picked, e.g. the name of a function. Must be contained by the
	 * [`range`](#CallHierarchyItem.range).
	 */
	lsRange selectionRange;

	/**
	 * A data entry field that is preserved between a call hierarchy prepare and
	 * incoming calls or outgoing calls requests.
	 */
	boost::optional<lsp::Any>  data;
	MAKE_SWAP_METHOD(CallHierarchyItem, name, kind, tags, detail, uri, range, selectionRange, data)
};
MAKE_REFLECT_STRUCT(CallHierarchyItem, name, kind, tags, detail, uri, range, selectionRange, data)



/// The parameter of a `callHierarchy/incomingCalls` request.
struct CallHierarchyIncomingCallsParams {
	CallHierarchyItem item;
	MAKE_SWAP_METHOD(CallHierarchyIncomingCallsParams,item)
};
MAKE_REFLECT_STRUCT(CallHierarchyIncomingCallsParams, item)


/// Represents an incoming call, e.g. a caller of a method or constructor.
struct CallHierarchyIncomingCall {
	/// The item that makes the call.
	CallHierarchyItem from;

	/// The range at which the calls appear.
	/// This is relative to the caller denoted by `From`.
	std::vector<lsRange> fromRanges;
	MAKE_SWAP_METHOD(CallHierarchyIncomingCall, from, fromRanges)
};
MAKE_REFLECT_STRUCT(CallHierarchyIncomingCall, from, fromRanges)




/// The parameter of a `callHierarchy/outgoingCalls` request.
struct CallHierarchyOutgoingCallsParams {
	CallHierarchyItem item;
	MAKE_SWAP_METHOD(CallHierarchyOutgoingCallsParams, item)
};
MAKE_REFLECT_STRUCT(CallHierarchyOutgoingCallsParams, item)

/// Represents an outgoing call, e.g. calling a getter from a method or
/// a method from a constructor etc.
struct CallHierarchyOutgoingCall {
	/// The item that is called.
	CallHierarchyItem to;

	/// The range at which this item is called.
	/// This is the range relative to the caller, and not `To`.
	std::vector<lsRange> fromRanges;
	MAKE_SWAP_METHOD(CallHierarchyOutgoingCall, to, fromRanges)
};
MAKE_REFLECT_STRUCT(CallHierarchyOutgoingCall, to, fromRanges)


DEFINE_REQUEST_RESPONSE_TYPE(td_prepareCallHierarchy, CallHierarchyPrepareParams,
	boost::optional<std::vector<CallHierarchyItem>>, "textDocument/prepareCallHierarchy")

DEFINE_REQUEST_RESPONSE_TYPE(td_incomingCalls, CallHierarchyIncomingCallsParams,
	boost::optional<std::vector<CallHierarchyIncomingCall>>, "callHierarchy/incomingCalls")

DEFINE_REQUEST_RESPONSE_TYPE(td_outgoingCalls, CallHierarchyOutgoingCallsParams,
	boost::optional<std::vector<CallHierarchyOutgoingCall>>, "callHierarchy/CallHierarchyOutgoingCall")