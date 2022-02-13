#pragma once
#include "LibLsp/lsp/method_type.h"


#include <stdexcept>
#include "LibLsp/JsonRpc/message.h"
#include "LibLsp/lsp/lsDocumentUri.h"
#include "LibLsp/lsp/lsAny.h"
#include "LibLsp/lsp/extention/jdtls/searchSymbols.h"
#include "lsWorkspaceClientCapabilites.h"
#include "LibLsp/lsp/lsp_completion.h"
#include "LibLsp/lsp/lsp_diagnostic.h"


struct  WorkDoneProgressOptions
{
	boost::optional<bool>workDoneProgress;
	MAKE_SWAP_METHOD(WorkDoneProgressOptions, workDoneProgress);
};
MAKE_REFLECT_STRUCT(WorkDoneProgressOptions, workDoneProgress);

// Completion options.
struct lsCompletionOptions:WorkDoneProgressOptions
{
  // The server provides support to resolve additional
  // information for a completion item.
  boost::optional<bool>  resolveProvider = false;

  //
   // Most tools trigger completion request automatically without explicitly requesting
   // it using a keyboard shortcut (e.g. Ctrl+Space). Typically they do so when the user
   // starts to type an identifier. For example if the user types `c` in a JavaScript file
   // code complete will automatically pop up present `console` besides others as a
   // completion item. Characters that make up identifiers don't need to be listed here.
   //
   // If code complete should automatically be trigger on characters not being valid inside
   // an identifier (for example `.` in JavaScript) list them in `triggerCharacters`.
   //
  // https://github.com/Microsoft/language-server-protocol/issues/138.
  boost::optional< std::vector<std::string> > triggerCharacters ;

  //
   // The list of all possible characters that commit a completion. This field can be used
   // if clients don't support individual commmit characters per completion item. See
   // `ClientCapabilities.textDocument.completion.completionItem.commitCharactersSupport`
   //
  boost::optional< std::vector<std::string> > allCommitCharacters;

  MAKE_SWAP_METHOD(lsCompletionOptions, workDoneProgress, resolveProvider, triggerCharacters, allCommitCharacters);
};
MAKE_REFLECT_STRUCT(lsCompletionOptions, workDoneProgress, resolveProvider, triggerCharacters,allCommitCharacters);



// Save options.
struct lsSaveOptions {
  // The client is supposed to include the content on save.
  bool includeText = false;
  void swap(lsSaveOptions& arg)noexcept
  {
	  auto temp = includeText;
	  includeText = arg.includeText;
	  arg.includeText = temp;
  }
};
MAKE_REFLECT_STRUCT(lsSaveOptions, includeText);

// Signature help options.
struct lsSignatureHelpOptions  : WorkDoneProgressOptions {
  // The characters that trigger signature help automatically.
  // NOTE: If updating signature help tokens make sure to also update
  // WorkingFile::FindClosestCallNameInBuffer.
  std::vector<std::string> triggerCharacters;
  MAKE_SWAP_METHOD(lsSignatureHelpOptions, workDoneProgress, triggerCharacters);
};
MAKE_REFLECT_STRUCT(lsSignatureHelpOptions, workDoneProgress,  triggerCharacters);

// Defines how the host (editor) should sync document changes to the language
// server.
enum class lsTextDocumentSyncKind {
  // Documents should not be synced at all.
  None = 0,

  // Documents are synced by always sending the full content
  // of the document.
  Full = 1,

  // Documents are synced by sending the full content on open.
  // After that only incremental updates to the document are
  // send.
  Incremental = 2
};

#if _WIN32
MAKE_REFLECT_TYPE_PROXY(lsTextDocumentSyncKind)
#else
//#pragma clang diagnostic push
//#pragma clang diagnostic ignored "-Wunused-function"
MAKE_REFLECT_TYPE_PROXY(lsTextDocumentSyncKind)
//#pragma clang diagnostic pop
#endif

struct lsTextDocumentSyncOptions {
  // Open and close notifications are sent to the server.
  boost::optional<bool>  openClose ;
  // Change notificatins are sent to the server. See TextDocumentSyncKind.None,
  // TextDocumentSyncKind.Full and TextDocumentSyncKindIncremental.
  boost::optional< lsTextDocumentSyncKind> change ;
  // Will save notifications are sent to the server.
  boost::optional<bool> willSave;
  // Will save wait until requests are sent to the server.
  boost::optional<bool> willSaveWaitUntil;
  // Save notifications are sent to the server.
  boost::optional<lsSaveOptions> save;

  MAKE_SWAP_METHOD(lsTextDocumentSyncOptions,
	  openClose,
	  change,
	  willSave,
	  willSaveWaitUntil,
	  save);
};
MAKE_REFLECT_STRUCT(lsTextDocumentSyncOptions,
                    openClose,
                    change,
                    willSave,
                    willSaveWaitUntil,
                    save);

struct SynchronizationCapabilities {
	// Whether text document synchronization supports dynamic registration.
	boost::optional<bool> dynamicRegistration;

	// The client supports sending will save notifications.
	boost::optional<bool> willSave;

	// The client supports sending a will save request and
	// waits for a response providing text edits which will
	// be applied to the document before it is saved.
	boost::optional<bool> willSaveWaitUntil;

	// The client supports did save notifications.
	boost::optional<bool> didSave;

	MAKE_SWAP_METHOD(SynchronizationCapabilities,
		dynamicRegistration,
		willSave,
		willSaveWaitUntil,
		didSave);
};
MAKE_REFLECT_STRUCT(SynchronizationCapabilities,
	dynamicRegistration,
	willSave,
	willSaveWaitUntil,
	didSave);

struct CompletionItemKindCapabilities
{
	boost::optional<std::vector<lsCompletionItemKind> >valueSet;
	MAKE_SWAP_METHOD(CompletionItemKindCapabilities, valueSet);
};
MAKE_REFLECT_STRUCT(CompletionItemKindCapabilities, valueSet);

struct CompletionItemCapabilities {
	// Client supports snippets as insert text.
	//
	// A snippet can define tab stops and placeholders with `$1`, `$2`
	// and `${3:foo}`. `$0` defines the final tab stop, it defaults to
	// the end of the snippet. Placeholders with equal identifiers are linked,
	// that is typing in one will update others too.
	boost::optional<bool> snippetSupport;

        // Client supports commit characters on a completion item.

	boost::optional<bool> commitCharactersSupport;


        // Client supports the following content formats for the documentation
        // property. The order describes the preferred format of the client.

	boost::optional< std::vector<std::string> > documentationFormat;

        // Client supports the deprecated property on a completion item.

	boost::optional<bool> deprecatedSupport;

	//
	 // Client supports the preselect property on a completion item.
	 //
	boost::optional<bool> preselectSupport;

	MAKE_SWAP_METHOD(CompletionItemCapabilities,
		snippetSupport,
		commitCharactersSupport,
		documentationFormat,
		deprecatedSupport, preselectSupport);
};
MAKE_REFLECT_STRUCT(CompletionItemCapabilities,
	snippetSupport,
	commitCharactersSupport,
	documentationFormat,
	deprecatedSupport, preselectSupport);


//
 // Capabilities specific to the `textDocument/completion`
 //
struct CompletionCapabilities {
	// Whether completion supports dynamic registration.
	boost::optional<bool> dynamicRegistration;



	// The client supports the following `CompletionItem` specific
	// capabilities.
	boost::optional<CompletionItemCapabilities> completionItem;

	//
	 // The client supports the following `CompletionItemKind` specific
	 // capabilities.
	 //
	boost::optional<CompletionItemKindCapabilities> completionItemKind;

	//
	 // The client supports sending additional context information for a
	 // `textDocument/completion` request.
	 //
	boost::optional<bool> contextSupport;


	MAKE_SWAP_METHOD(CompletionCapabilities,
		dynamicRegistration,
		completionItem, completionItemKind);
};

MAKE_REFLECT_STRUCT(CompletionCapabilities,
	dynamicRegistration,
	completionItem , completionItemKind);


struct HoverCapabilities:public DynamicRegistrationCapabilities
{
	//
 // Client supports the following content formats for the content
 // property. The order describes the preferred format of the client.
 //
 // See {@link MarkupKind} for allowed values.
 //
	boost::optional<std::vector<std::string>> contentFormat;

	MAKE_SWAP_METHOD(HoverCapabilities, dynamicRegistration, contentFormat);
};
MAKE_REFLECT_STRUCT(HoverCapabilities, dynamicRegistration, contentFormat);

//
 // Client capabilities specific to parameter information.
 //
struct	ParameterInformationCapabilities {
	//
	 // The client supports processing label offsets instead of a
	 // simple label string.
	 //
	 // Since 3.14.0
	 //
	boost::optional<bool> labelOffsetSupport;

	MAKE_SWAP_METHOD(ParameterInformationCapabilities, labelOffsetSupport);
};
MAKE_REFLECT_STRUCT(ParameterInformationCapabilities, labelOffsetSupport)


struct SignatureInformationCapabilities {
	//
	 // Client supports the following content formats for the documentation
	 // property. The order describes the preferred format of the client.
	 //
	 // See {@link MarkupKind} for allowed values.
	 //
	std::vector<std::string> documentationFormat;

	//
	 // Client capabilities specific to parameter information.
	 //
	 ParameterInformationCapabilities parameterInformation;

	 MAKE_SWAP_METHOD(SignatureInformationCapabilities, documentationFormat, parameterInformation)
};
MAKE_REFLECT_STRUCT(SignatureInformationCapabilities,documentationFormat, parameterInformation)

struct SignatureHelpCapabilities :public DynamicRegistrationCapabilities
{
	//
	 // The client supports the following `SignatureInformation`
	 // specific properties.
	 //
	boost::optional< SignatureInformationCapabilities  > signatureInformation;

	MAKE_SWAP_METHOD(SignatureHelpCapabilities, dynamicRegistration, signatureInformation)
};
MAKE_REFLECT_STRUCT(SignatureHelpCapabilities, dynamicRegistration, signatureInformation)

struct DocumentSymbolCapabilities :public DynamicRegistrationCapabilities {
	//
	 // Specific capabilities for the `SymbolKind`.
	 //
	boost::optional<SymbolKindCapabilities>  symbolKind;

	//
	 // The client support hierarchical document symbols.
	 //
	 boost::optional<bool> hierarchicalDocumentSymbolSupport;

	 MAKE_SWAP_METHOD(DocumentSymbolCapabilities, dynamicRegistration, symbolKind, hierarchicalDocumentSymbolSupport)
};
MAKE_REFLECT_STRUCT(DocumentSymbolCapabilities, dynamicRegistration, symbolKind, hierarchicalDocumentSymbolSupport)

struct DeclarationCapabilities:public DynamicRegistrationCapabilities{
	//
	 // The client supports additional metadata in the form of declaration links.
	 //
	boost::optional<bool>linkSupport;

	MAKE_SWAP_METHOD(DeclarationCapabilities, dynamicRegistration, linkSupport);
};
MAKE_REFLECT_STRUCT(DeclarationCapabilities, dynamicRegistration, linkSupport)


struct CodeActionKindCapabilities
{
	//
	 // The code action kind values the client supports. When this
	 // property exists the client also guarantees that it will
	 // handle values outside its set gracefully and falls back
	 // to a default value when unknown.
	 //
	 // See {@link CodeActionKind} for allowed values.
	 //
	boost::optional< std::vector< std::string> >valueSet;

	MAKE_SWAP_METHOD(CodeActionKindCapabilities, valueSet)
};
MAKE_REFLECT_STRUCT(CodeActionKindCapabilities,valueSet)

struct CodeActionLiteralSupportCapabilities
{
	boost::optional<CodeActionKindCapabilities> codeActionKind;

	MAKE_SWAP_METHOD(CodeActionLiteralSupportCapabilities, codeActionKind)
};
MAKE_REFLECT_STRUCT(CodeActionLiteralSupportCapabilities, codeActionKind)

struct CodeActionCapabilities:public DynamicRegistrationCapabilities{
	//
 // The client support code action literals as a valid
 // response of the `textDocument/codeAction` request.
 //
	boost::optional<CodeActionLiteralSupportCapabilities> codeActionLiteralSupport;

	MAKE_SWAP_METHOD(CodeActionCapabilities, dynamicRegistration, codeActionLiteralSupport)
};
MAKE_REFLECT_STRUCT(CodeActionCapabilities,dynamicRegistration,codeActionLiteralSupport)

struct RenameCapabilities :public DynamicRegistrationCapabilities {
	//
 // The client support code action literals as a valid
 // response of the `textDocument/codeAction` request.
 //
	boost::optional<bool> prepareSupport;

	MAKE_SWAP_METHOD(RenameCapabilities, dynamicRegistration, prepareSupport)
};
MAKE_REFLECT_STRUCT(RenameCapabilities, dynamicRegistration, prepareSupport)

struct  DiagnosticsTagSupport {
	/**
	 * The tags supported by the client.
	 */
	std::vector<DiagnosticTag> valueSet;
	MAKE_SWAP_METHOD(DiagnosticsTagSupport, valueSet)
};
MAKE_REFLECT_STRUCT(DiagnosticsTagSupport, valueSet)

struct PublishDiagnosticsClientCapabilities :public DynamicRegistrationCapabilities {
	/**
 * The client support code action literals as a valid
 * response of the `textDocument/codeAction` request.
 */
	boost::optional<bool> relatedInformation;

	/**
	 * Client supports the tag property to provide meta data about a diagnostic.
	 * Clients supporting tags have to handle unknown tags gracefully.
	 *
	 * This property had been added and implemented as boolean before it was
	 * added to the specification as {@link DiagnosticsTagSupport}. In order to
	 * keep this implementation compatible with intermediate clients (including
	 * vscode-language-client < 6.0.0) we add an either type here.
	 *
	 * Since 3.15
	 */
	boost::optional < std::pair<boost::optional<bool>, boost::optional<DiagnosticsTagSupport> > >  tagSupport;

	/**
	 * Whether the client interprets the version property of the
	 * `textDocument/publishDiagnostics` notification's parameter.
	 *
	 * Since 3.15.0
	 */
	boost::optional<bool> versionSupport;

	/**
 * Client supports a codeDescription property
 *
 * @since 3.16.0
 */
	boost::optional<bool> codeDescriptionSupport ;

	/**
	 * Whether code action supports the `data` property which is
	 * preserved between a `textDocument/publishDiagnostics` and
	 * `textDocument/codeAction` request.
	 *
	 * @since 3.16.0
	 */
	boost::optional<bool> dataSupport ;


	MAKE_SWAP_METHOD(PublishDiagnosticsClientCapabilities, dynamicRegistration, relatedInformation, tagSupport,versionSupport,codeDescriptionSupport,dataSupport)
};
MAKE_REFLECT_STRUCT(PublishDiagnosticsClientCapabilities, dynamicRegistration, relatedInformation, tagSupport, versionSupport, codeDescriptionSupport, dataSupport)


struct FoldingRangeCapabilities :public DynamicRegistrationCapabilities {
	//
	 // The maximum number of folding ranges that the client prefers to receive per document. The value serves as a
	 // hint, servers are free to follow the limit.
	 //
	boost::optional<int> rangeLimit;

	//
	 // If set, the client signals that it only supports folding complete lines. If set, client will
	 // ignore specified `startCharacter` and `endCharacter` properties in a FoldingRange.
	 //
	boost::optional<bool> lineFoldingOnly;
	MAKE_SWAP_METHOD(FoldingRangeCapabilities, dynamicRegistration, rangeLimit, lineFoldingOnly)
};
MAKE_REFLECT_STRUCT(FoldingRangeCapabilities, dynamicRegistration, rangeLimit,lineFoldingOnly)


struct SemanticHighlightingCapabilities :public DynamicRegistrationCapabilities {
	//
 // The client support code action literals as a valid
 // response of the `textDocument/codeAction` request.
 //
	boost::optional<bool> semanticHighlighting;

	MAKE_SWAP_METHOD(SemanticHighlightingCapabilities, dynamicRegistration, semanticHighlighting)
};
MAKE_REFLECT_STRUCT(SemanticHighlightingCapabilities, dynamicRegistration, semanticHighlighting)

struct SemanticTokensClientCapabilitiesRequestsFull {

	//
	// The client will send the `textDocument/semanticTokens/full/delta` request if
	// the server provides a corresponding handler.
	//
	bool delta = false;
	MAKE_SWAP_METHOD(SemanticTokensClientCapabilitiesRequestsFull, delta)
};
MAKE_REFLECT_STRUCT(SemanticTokensClientCapabilitiesRequestsFull, delta)

struct SemanticTokensClientCapabilities :  public DynamicRegistrationCapabilities
{
	//export  TokenFormat = 'relative';
	struct lsRequests
	{
		//
		 // The client will send the `textDocument/semanticTokens/range` request
		 // if the server provides a corresponding handler.
		 //
		boost::optional<std::pair< boost::optional<bool>,
		boost::optional<SemanticTokensClientCapabilitiesRequestsFull>>>  range;
		//
		 // The client will send the `textDocument/semanticTokens/full` request
		 // if the server provides a corresponding handler.
		 //
		boost::optional<std::pair< boost::optional<bool>, boost::optional<lsp::Any>>> full;
		MAKE_SWAP_METHOD(lsRequests, range, full)
	};

	lsRequests requests;
	//
	 // The token types that the client supports.
	 //
	std::vector<std::string> tokenTypes;

	//
	 // The token modifiers that the client supports.
	 //
	std::vector<std::string> tokenModifiers;
	//
	 // The formats the clients supports.
	 //
	std::vector<std::string> formats;
	//
	 // Whether the client supports tokens that can overlap each other.
	 //
	boost::optional < bool >overlappingTokenSupport;

	//
	 // Whether the client supports tokens that can span multiple lines.
	 //
	boost::optional < bool > multilineTokenSupport;

	MAKE_SWAP_METHOD(SemanticTokensClientCapabilities, dynamicRegistration,requests, tokenTypes, tokenModifiers,
		formats, overlappingTokenSupport, multilineTokenSupport)

};
MAKE_REFLECT_STRUCT(SemanticTokensClientCapabilities::lsRequests, range,full)
MAKE_REFLECT_STRUCT(SemanticTokensClientCapabilities, dynamicRegistration, requests, tokenTypes, tokenModifiers,
	formats, overlappingTokenSupport, multilineTokenSupport)

// Text document specific client capabilities.
struct lsTextDocumentClientCapabilities {

	SynchronizationCapabilities synchronization;


  // Capabilities specific to the `textDocument/completion`
  boost::optional<CompletionCapabilities> completion;



  // Capabilities specific to the `textDocument/hover`
  boost::optional<HoverCapabilities> hover;

  // Capabilities specific to the `textDocument/signatureHelp`
  boost::optional<SignatureHelpCapabilities> signatureHelp;

  // Capabilities specific to the `textDocument/references`
  boost::optional<DynamicRegistrationCapabilities> references;





  // Capabilities specific to the `textDocument/documentHighlight`
  boost::optional<DynamicRegistrationCapabilities> documentHighlight;

  // Capabilities specific to the `textDocument/documentSymbol`
  boost::optional<DocumentSymbolCapabilities> documentSymbol;

  // Capabilities specific to the `textDocument/formatting`
  boost::optional<DynamicRegistrationCapabilities> formatting;

  // Capabilities specific to the `textDocument/rangeFormatting`
  boost::optional<DynamicRegistrationCapabilities> rangeFormatting;

  // Capabilities specific to the `textDocument/onTypeFormatting`
  boost::optional<DynamicRegistrationCapabilities> onTypeFormatting;


  //
 // Capabilities specific to the `textDocument/declaration`
 //
 // Since 3.14.0
 //
  boost::optional< DeclarationCapabilities> declaration;


  typedef  DeclarationCapabilities DefinitionCapabilities;
  // Capabilities specific to the `textDocument/definition`
  boost::optional<DefinitionCapabilities> definition;



  //
// Capabilities specific to the `textDocument/typeDefinition`
//
// Since 3.6.0
//
  typedef  DeclarationCapabilities TypeDefinitionCapabilities;
  boost::optional<TypeDefinitionCapabilities>  typeDefinition;


  typedef  DeclarationCapabilities ImplementationCapabilities;
  // Capabilities specific to the `textDocument/implementation`
  boost::optional<ImplementationCapabilities> implementation;


  // Capabilities specific to the `textDocument/codeAction`
  boost::optional<CodeActionCapabilities> codeAction;


  // Capabilities specific to the `textDocument/codeLens`
  boost::optional<DynamicRegistrationCapabilities> codeLens;

  // Capabilities specific to the `textDocument/documentLink`
  boost::optional<DynamicRegistrationCapabilities> documentLink;

  //
 // Capabilities specific to the `textDocument/documentColor` and the
 // `textDocument/colorPresentation` request.
 //
 // Since 3.6.0
 //
  boost::optional<DynamicRegistrationCapabilities> colorProvider;

  // Capabilities specific to the `textDocument/rename`
  boost::optional<RenameCapabilities> rename;

//
// Capabilities specific to `textDocument/publishDiagnostics`.
//
  boost::optional<PublishDiagnosticsClientCapabilities> publishDiagnostics;

  //
// Capabilities specific to `textDocument/foldingRange` requests.
//
// Since 3.10.0
//
  boost::optional< FoldingRangeCapabilities > foldingRange;


  //
   // Capabilities specific to {@code textDocument/semanticHighlighting}.
   //
  boost::optional< SemanticHighlightingCapabilities >  semanticHighlightingCapabilities;

  //
   // Capabilities specific to {@code textDocument/typeHierarchy}.
   //
  boost::optional< DynamicRegistrationCapabilities >  typeHierarchyCapabilities;



  //
// Capabilities specific to `textDocument/selectionRange` requests
//

  boost::optional< DynamicRegistrationCapabilities > selectionRange;

  //
	 // Capabilities specific to the `textDocument/linkedEditingRange` request.
	 //
	 // @since 3.16.0
	 //
  boost::optional< DynamicRegistrationCapabilities > linkedEditingRange;

  //
   // Capabilities specific to the various call hierarchy requests.
   //
   // @since 3.16.0
   //
  boost::optional< DynamicRegistrationCapabilities > callHierarchy;

  //
   // Capabilities specific to the various semantic token requests.
   //
   // @since 3.16.0
   //
  boost::optional< SemanticTokensClientCapabilities > semanticTokens;

  //
   // Capabilities specific to the `textDocument/moniker` request.
   //
   // @since 3.16.0
   //
  boost::optional< DynamicRegistrationCapabilities >  moniker;

  MAKE_SWAP_METHOD(lsTextDocumentClientCapabilities,
	  synchronization,
	  completion,
	  hover,
	  signatureHelp,
	  implementation,
	  references,
	  documentHighlight,
	  documentSymbol,
	  formatting,
	  rangeFormatting,
	  onTypeFormatting,
	  declaration,
	  definition, typeDefinition, implementation,
	  codeAction,
	  codeLens,
	  documentLink, colorProvider,
	  rename, publishDiagnostics, foldingRange,
	  semanticHighlightingCapabilities, typeHierarchyCapabilities,
	  callHierarchy, selectionRange , linkedEditingRange, semanticTokens, moniker)
};


MAKE_REFLECT_STRUCT(lsTextDocumentClientCapabilities,
	synchronization,
	completion,
	hover,
	signatureHelp,
	implementation,
	references,
	documentHighlight,
	documentSymbol,
	formatting,
	rangeFormatting,
	onTypeFormatting,
	declaration,
	definition, typeDefinition, implementation,
	codeAction,
	codeLens,
	documentLink, colorProvider,
	rename, publishDiagnostics, foldingRange,
	semanticHighlightingCapabilities, typeHierarchyCapabilities,
	callHierarchy, selectionRange, linkedEditingRange, semanticTokens, moniker)
