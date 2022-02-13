#pragma once

#include "LibLsp/lsp/lsTextDocumentIdentifier.h"
#include "LibLsp/lsp/lsVersionedTextDocumentIdentifier.h"
#include "LibLsp/JsonRpc/RequestInMessage.h"
enum class HighlightingKind_clangD {
	Variable = 0,
	LocalVariable,
	Parameter,
	Function,
	Method,
	StaticMethod,
	Field,
	StaticField,
	Class,
	Interface,
	Enum,
	EnumConstant,
	Typedef,
	Type,
	Unknown,
	Namespace,
	TemplateParameter,
	Concept,
	Primitive,
	Macro,

	// This one is different from the other kinds as it's a line style
	// rather than a token style.
	InactiveCode,

	LastKind = InactiveCode
};
std::string toSemanticTokenType(HighlightingKind_clangD kind);

enum class HighlightingModifier_clangD {
	Declaration,
	Deprecated,
	Deduced,
	Readonly,
	Static,
	Abstract,
	DependentName,
	DefaultLibrary,

	FunctionScope,
	ClassScope,
	FileScope,
	GlobalScope,

	LastModifier = GlobalScope
};
std::string toSemanticTokenModifier(HighlightingModifier_clangD modifier);

enum  SemanticTokenType {
	ls_namespace=0,// 'namespace',
	/**
	 * Represents a generic type. Acts as a fallback for types which
	 * can't be mapped to a specific type like class or enum.
	 */
	 ls_type,// 'type',
	 ls_class,// 'class',
	 ls_enum,// 'enum',
	 ls_interface,// 'interface',
	 ls_struct,// 'struct',
	 ls_typeParameter,// 'typeParameter',
	 ls_parameter,// 'parameter',
	 ls_variable,// 'variable',
	 ls_property,// 'property',
	 ls_enumMember,// 'enumMember',
	 ls_event,// 'event',
	 ls_function,// 'function',
	 ls_method,// 'method',
	 ls_macro,// 'macro',
	 ls_keyword,// 'keyword',
	 ls_modifier,// 'modifier',
	 ls_comment,// 'comment',
	 ls_string,// 'string',
	 ls_number,// 'number',
	 ls_regexp,// 'regexp',
	 ls_operator,// 'operator'
	 lastKind = ls_operator
};
std::string to_string(SemanticTokenType);
unsigned toSemanticTokenType(std::vector<SemanticTokenType>& modifiers);

enum TokenType_JDT {
	PACKAGE_JDT=0,
	CLASS_JDT,
	INTERFACE_JDT,
	ENUM_JDT,
	ENUM_MEMBER_JDT,
	TYPE_JDT,
	TYPE_PARAMETER_JDT,
	ANNOTATION_JDT,
	ANNOTATION_MEMBER_JDT,
	METHOD_JDT,
	PROPERTY_JDT,
	VARIABLE_JDT,
	PARAMETER_JDT
};
std::string to_string(TokenType_JDT);

enum SemanticTokenModifier {
	ls_declaration=0,// 'declaration',
	ls_definition,// 'definition',
	ls_readonly,// 'readonly',
	ls_static,// 'static',
	ls_deprecated,// 'deprecated',
	ls_abstract,// 'abstract',
	ls_async,// 'async',
	ls_modification,// 'modification',
	ls_documentation,// 'documentation',
	ls_defaultLibrary,// 'defaultLibrary'
	LastModifier = ls_defaultLibrary
};
std::string to_string(SemanticTokenModifier);
unsigned  toSemanticTokenModifiers(std::vector<SemanticTokenModifier>&);
/// Specifies a single semantic token in the document.
/// This struct is not part of LSP, which just encodes lists of tokens as
/// arrays of numbers directly.
struct SemanticToken {
	/// token line number, relative to the previous token
	unsigned deltaLine = 0;
	/// token start character, relative to the previous token
	/// (relative to 0 or the previous token's start if they are on the same line)
	unsigned deltaStart = 0;
	/// the length of the token. A token cannot be multiline
	unsigned length = 0;
	/// will be looked up in `SemanticTokensLegend.tokenTypes`
	unsigned tokenType = 0;
	/// each set bit will be looked up in `SemanticTokensLegend.tokenModifiers`
	unsigned tokenModifiers = 0;
};
bool operator==(const SemanticToken&, const SemanticToken&);
struct  SemanticTokens{

	/**
	* Tokens in a file are represented as an array of integers. The position of each token is expressed relative to
	* the token before it, because most tokens remain stable relative to each other when edits are made in a file.
	*
	* ---
	* In short, each token takes 5 integers to represent, so a specific token `i` in the file consists of the following array indices:
	*  - at index `5*i`   - `deltaLine`: token line number, relative to the previous token
	*  - at index `5*i+1` - `deltaStart`: token start character, relative to the previous token (relative to 0 or the previous token's start if they are on the same line)
	*  - at index `5*i+2` - `length`: the length of the token. A token cannot be multiline.
	*  - at index `5*i+3` - `tokenType`: will be looked up in `SemanticTokensLegend.tokenTypes`. We currently ask that `tokenType` < 65536.
	*  - at index `5*i+4` - `tokenModifiers`: each set bit will be looked up in `SemanticTokensLegend.tokenModifiers`
	*
	* ---
	* ### How to encode tokens
	*
	* Here is an example for encoding a file with 3 tokens in a uint32 array:
	* ```
	*    { line: 2, startChar:  5, length: 3, tokenType: "property",  tokenModifiers: ["private", "static"] },
	*    { line: 2, startChar: 10, length: 4, tokenType: "type",      tokenModifiers: [] },
	*    { line: 5, startChar:  2, length: 7, tokenType: "class",     tokenModifiers: [] }
	* ```
	*
	* 1. First of all, a legend must be devised. This legend must be provided up-front and capture all possible token types.
	* For this example, we will choose the following legend which must be passed in when registering the provider:
	* ```
	*    tokenTypes: ['property', 'type', 'class'],
	*    tokenModifiers: ['private', 'static']
	* ```
	*
	* 2. The first transformation step is to encode `tokenType` and `tokenModifiers` as integers using the legend. Token types are looked
	* up by index, so a `tokenType` value of `1` means `tokenTypes[1]`. Multiple token modifiers can be set by using bit flags,
	* so a `tokenModifier` value of `3` is first viewed as binary `0b00000011`, which means `[tokenModifiers[0], tokenModifiers[1]]` because
	* bits 0 and 1 are set. Using this legend, the tokens now are:
	* ```
	*    { line: 2, startChar:  5, length: 3, tokenType: 0, tokenModifiers: 3 },
	*    { line: 2, startChar: 10, length: 4, tokenType: 1, tokenModifiers: 0 },
	*    { line: 5, startChar:  2, length: 7, tokenType: 2, tokenModifiers: 0 }
	* ```
	*
	* 3. The next step is to represent each token relative to the previous token in the file. In this case, the second token
	* is on the same line as the first token, so the `startChar` of the second token is made relative to the `startChar`
	* of the first token, so it will be `10 - 5`. The third token is on a different line than the second token, so the
	* `startChar` of the third token will not be altered:
	* ```
	*    { deltaLine: 2, deltaStartChar: 5, length: 3, tokenType: 0, tokenModifiers: 3 },
	*    { deltaLine: 0, deltaStartChar: 5, length: 4, tokenType: 1, tokenModifiers: 0 },
	*    { deltaLine: 3, deltaStartChar: 2, length: 7, tokenType: 2, tokenModifiers: 0 }
	* ```
	*
	* 4. Finally, the last step is to inline each of the 5 fields for a token in a single array, which is a memory friendly representation:
	* ```
	*    // 1st token,  2nd token,  3rd token
	*    [  2,5,3,0,3,  0,5,4,1,0,  3,2,7,2,0 ]
	* ```
	*/
	std::vector<int32_t> data;
	static std::vector<int32_t> encodeTokens(std::vector<SemanticToken>& tokens);

	/**
	 * An optional result id. If provided and clients support delta updating
	 * the client will include the result id in the next semantic token request.
	 * A server can then instead of computing all semantic tokens again simply
	 * send a delta.
	 */
   boost::optional<std::string> resultId;
   MAKE_SWAP_METHOD(SemanticTokens, data, resultId)
};
MAKE_REFLECT_STRUCT(SemanticTokens, data, resultId)

/// Body of textDocument/semanticTokens/full request.
struct SemanticTokensParams {
	/// The text document.
	lsTextDocumentIdentifier textDocument;
	MAKE_REFLECT_STRUCT(SemanticTokensParams, textDocument)
};
MAKE_REFLECT_STRUCT(SemanticTokensParams, textDocument)


/// Body of textDocument/semanticTokens/full/delta request.
/// Requests the changes in semantic tokens since a previous response.
struct SemanticTokensDeltaParams {
	/// The text document.
	lsTextDocumentIdentifier textDocument;
	/**
	 * The result id of a previous response. The result Id can either point to
	 * a full response or a delta response depending on what was received last.
	 */
	std::string previousResultId;

	MAKE_REFLECT_STRUCT(SemanticTokensDeltaParams, textDocument, previousResultId)
};
MAKE_REFLECT_STRUCT(SemanticTokensDeltaParams, textDocument, previousResultId)

/// Describes a a replacement of a contiguous range of semanticTokens.
struct SemanticTokensEdit {
	// LSP specifies `start` and `deleteCount` which are relative to the array
	// encoding of the previous tokens.
	// We use token counts instead, and translate when serializing this struct.
	unsigned startToken = 0;
	unsigned deleteTokens = 0;
	std::vector<int32_t> tokens; // encoded as a flat integer array

	MAKE_REFLECT_STRUCT(SemanticTokensEdit, startToken, deleteTokens, tokens)
};
MAKE_REFLECT_STRUCT(SemanticTokensEdit, startToken, deleteTokens, tokens)


/// This models LSP SemanticTokensDelta | SemanticTokens, which is the result of
/// textDocument/semanticTokens/full/delta.
struct SemanticTokensOrDelta {
	boost::optional<std::string >  resultId;
	/// Set if we computed edits relative to a previous set of tokens.
	boost::optional< std::vector<SemanticTokensEdit> > edits;
	/// Set if we computed a fresh set of tokens.
	/// Set if we computed edits relative to a previous set of tokens.
	boost::optional<std::vector<int32_t>> tokens; // encoded as integer array
	MAKE_REFLECT_STRUCT(SemanticTokensOrDelta, resultId, edits, tokens)
};
MAKE_REFLECT_STRUCT(SemanticTokensOrDelta, resultId, edits, tokens)

       
struct SemanticTokensLegend {
	std::vector<std::string> tokenTypes;
	std::vector<std::string> tokenModifiers;
	MAKE_REFLECT_STRUCT(SemanticTokensLegend, tokenTypes, tokenModifiers)
};
MAKE_REFLECT_STRUCT(SemanticTokensLegend, tokenTypes, tokenModifiers)

DEFINE_REQUEST_RESPONSE_TYPE(td_semanticTokens_full, SemanticTokensParams,boost::optional<SemanticTokens >,"textDocument/semanticTokens/full")
DEFINE_REQUEST_RESPONSE_TYPE(td_semanticTokens_full_delta, SemanticTokensDeltaParams, boost::optional<SemanticTokensOrDelta >, "textDocument/semanticTokens/full/delta")