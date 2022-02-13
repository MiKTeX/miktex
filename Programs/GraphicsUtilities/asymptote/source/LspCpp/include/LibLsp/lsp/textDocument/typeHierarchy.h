#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include "LibLsp/lsp/lsTextDocumentPositionParams.h"
#include "LibLsp/lsp/symbol.h"
#include "LibLsp/lsp/lsAny.h"
/**
	 * The {@code textDocument/typeHierarchy} request is sent from the client to the
	 * server to retrieve a {@link TypeHierarchyItem type hierarchy item} based on
	 * the {@link TypeHierarchyParams cursor position in the text document}. This
	 * request would also allow to specify if the item should be resolved and
	 * whether sub- or supertypes are to be resolved. If no type hierarchy item can
	 * be found under the given text document position, resolves to {@code null}.
	 *
	 * <p>
	 * <b>Note:</b> the <a href=
	 * "https://github.com/Microsoft/vscode-languageserver-node/pull/426">{@code textDocument/typeHierarchy}
	 * language feature</a> is not yet part of the official LSP specification.
	 */
enum class TypeHierarchyDirection :  uint32_t{

	/**
	 * Flag for retrieving/resolving the subtypes. Value: {@code 0}.
	 */
	Children = 0,

	/**
	 * Flag to use when retrieving/resolving the supertypes. Value: {@code 1}.
	 */
	 Parents =1,

	 /**
	  * Flag for resolving both the super- and subtypes. Value: {@code 2}.
	  */
	  Both=2
};
void Reflect(Reader& reader, TypeHierarchyDirection& value);


void Reflect(Writer& writer, TypeHierarchyDirection& value);

struct TypeHierarchyParams :public lsTextDocumentPositionParams
{
	boost::optional<int>  resolve;
	boost::optional<TypeHierarchyDirection> direction ;
	
	MAKE_SWAP_METHOD(TypeHierarchyParams, textDocument, position, resolve, direction)
};
MAKE_REFLECT_STRUCT(TypeHierarchyParams,  textDocument, position, resolve, direction);





/**
 * Representation of an item that carries type information (such as class, interface, enumeration, etc) with additional parentage details.
 */

struct  TypeHierarchyItem {
	/**
	 * The human readable name of the hierarchy item.
	 */

	std::string name;

	/**
	 * Optional detail for the hierarchy item. It can be, for instance, the signature of a function or method.
	 */
	boost::optional<std::string>
	 detail;

	/**
	 * The kind of the hierarchy item. For instance, class or interface.
	 */

	SymbolKind kind;

	/**
	 * {@code true} if the hierarchy item is deprecated. Otherwise, {@code false}. It is {@code false} by default.
	 */
	
	boost::optional<bool> deprecated;

	/**
	 * The URI of the text document where this type hierarchy item belongs to.
	 */

	lsDocumentUri uri;

	/**
	 * The range enclosing this type hierarchy item not including leading/trailing whitespace but everything else
	 * like comments. This information is typically used to determine if the clients cursor is inside the type
	 * hierarchy item to reveal in the symbol in the UI.
	 *
	 * @see TypeHierarchyItem#selectionRange
	 */

	lsRange range;

	/**
	 * The range that should be selected and revealed when this type hierarchy item is being picked, e.g the name of a function.
	 * Must be contained by the the {@link TypeHierarchyItem#getRange range}.
	 *
	 * @see TypeHierarchyItem#range
	 */

	lsRange selectionRange;

	/**
	 * If this type hierarchy item is resolved, it contains the direct parents. Could be empty if the item does not have any
	 * direct parents. If not defined, the parents have not been resolved yet.
	 */
	boost::optional< std::vector<TypeHierarchyItem> >  parents;

	/**
	 * If this type hierarchy item is resolved, it contains the direct children of the current item.
	 * Could be empty if the item does not have any descendants. If not defined, the children have not been resolved.
	 */
	boost::optional< std::vector<TypeHierarchyItem> >  children;

	/**
 * An boost::optional data field can be used to identify a type hierarchy item in a resolve request.
 */
	boost::optional<lsp::Any> data;

	MAKE_SWAP_METHOD(TypeHierarchyItem, name, detail, kind, deprecated, uri, range, selectionRange, parents, children, data)
};
MAKE_REFLECT_STRUCT(TypeHierarchyItem, name, detail, kind, deprecated, uri, range, selectionRange, parents, children, data);



DEFINE_REQUEST_RESPONSE_TYPE(td_typeHierarchy, TypeHierarchyParams, TypeHierarchyItem, "textDocument/typeHierarchy");