#pragma once
#include "LibLsp/lsp/location_type.h"


enum class lsSymbolKind : uint8_t {
	Unknown = 0,

	File = 1,
	Module = 2,
	Namespace = 3,
	Package = 4,
	Class = 5,
	Method = 6,
	Property = 7,
	Field = 8,
	Constructor = 9,
	Enum = 10,
	Interface = 11,
	Function = 12,
	Variable = 13,
	Constant = 14,
	String = 15,
	Number = 16,
	Boolean = 17,
	Array = 18,
	Object = 19,
	Key = 20,
	Null = 21,
	EnumMember = 22,
	Struct = 23,
	Event = 24,
	Operator = 25,

	// For C++, this is interpreted as "template parameter" (including
	// non-type template parameters).
	TypeParameter = 26,

	// cquery extensions
	// See also https://github.com/Microsoft/language-server-protocol/issues/344
	// for new SymbolKind clang/Index/IndexSymbol.h clang::index::SymbolKind
	TypeAlias = 252,
	Parameter = 253,
	StaticMethod = 254,
	Macro = 255,
};
MAKE_REFLECT_TYPE_PROXY(lsSymbolKind);

typedef  lsSymbolKind SymbolKind;

// A document highlight kind.
enum class lsDocumentHighlightKind {
  // A textual occurrence.
  Text = 1,
  // Read-access of a symbol, like reading a variable.
  Read = 2,
  // Write-access of a symbol, like writing to a variable.
  Write = 3
};
MAKE_REFLECT_TYPE_PROXY(lsDocumentHighlightKind);

// A document highlight is a range inside a text document which deserves
// special attention. Usually a document highlight is visualized by changing
// the background color of its range.
struct lsDocumentHighlight {
  // The range this highlight applies to.
  lsRange range;

  // The highlight kind, default is DocumentHighlightKind.Text.
  boost::optional<lsDocumentHighlightKind>  kind ;

  MAKE_SWAP_METHOD(lsDocumentHighlight, range, kind)
};
MAKE_REFLECT_STRUCT(lsDocumentHighlight, range, kind);

struct lsSymbolInformation {

/**
 * The name of this symbol.
 */
  std::string name;
  /**
   * The kind of this symbol.
   */
  lsSymbolKind kind;
  /**
* Indicates if this symbol is deprecated.
*/
  boost::optional<bool> deprecated;
  /**
   * The location of this symbol. The location's range is used by a tool
   * to reveal the location in the editor. If the symbol is selected in the
   * tool the range's start information is used to position the cursor. So
   * the range usually spans more then the actual symbol's name and does
   * normally include things like visibility modifiers.
   *
   * The range doesn't have to denote a node range in the sense of a abstract
   * syntax tree. It can therefore not be used to re-construct a hierarchy of
   * the symbols.
   */
  lsLocation location;
  /**
 * The name of the symbol containing this symbol. This information is for
 * user interface purposes (e.g. to render a qualifier in the user interface
 * if necessary). It can't be used to re-infer a hierarchy for the document
 * symbols.
 */
  boost::optional<std::string>  containerName;


  MAKE_SWAP_METHOD(lsSymbolInformation, name, kind, deprecated, location, containerName);
};
MAKE_REFLECT_STRUCT(lsSymbolInformation, name, kind, deprecated, location, containerName);


struct lsDocumentSymbol {
	/**
	 * The name of this symbol.
	 */

	std::string name;

	/**
	 * The kind of this symbol.
	 */
	
	lsSymbolKind kind = lsSymbolKind::Unknown;

	/**
	 * The range enclosing this symbol not including leading/trailing whitespace but everything else
	 * like comments. This information is typically used to determine if the clients cursor is
	 * inside the symbol to reveal in the symbol in the UI.
	 */
	
	lsRange range;

	/**
	 * The range that should be selected and revealed when this symbol is being picked, e.g the name of a function.
	 * Must be contained by the `range`.
	 */
	
	lsRange selectionRange;

	/**
	 * More detail for this symbol, e.g the signature of a function. If not provided the
	 * name is used.
	 */
	boost::optional< std::string >  detail;

	/**
	 * Indicates if this symbol is deprecated.
	 */
	boost::optional< bool > deprecated;

	/**
	 * Children of this symbol, e.g. properties of a class.
	 */
	boost::optional < std::vector<lsDocumentSymbol> > children;

	//internal use
	int flags=0;

	MAKE_SWAP_METHOD(lsDocumentSymbol, name, kind, range, selectionRange, detail, deprecated, children, flags);
};
MAKE_REFLECT_STRUCT(lsDocumentSymbol, name, kind, range, selectionRange, detail, deprecated, children, flags);