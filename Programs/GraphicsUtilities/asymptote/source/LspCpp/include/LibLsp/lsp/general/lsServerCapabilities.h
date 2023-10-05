#pragma once
#include "LibLsp/lsp/method_type.h"


#include <stdexcept>
#include "LibLsp/JsonRpc/message.h"
#include "LibLsp/lsp/lsDocumentUri.h"
#include "LibLsp/lsp/lsAny.h"
#include "InitializeParams.h"
#include "LibLsp/lsp/textDocument/SemanticTokens.h"


extern void Reflect(Reader&, std::pair<optional<lsTextDocumentSyncKind>, optional<lsTextDocumentSyncOptions> >&);

//
 // Code Action options.
 //
struct  CodeActionOptions : WorkDoneProgressOptions {
        //
         // CodeActionKinds that this server may return.
         //
         // The list of kinds may be generic, such as `CodeActionKind.Refactor`, or the server
         // may list out every specific kind they provide.
         //
        typedef  std::string CodeActionKind;
         std::vector<CodeActionKind> codeActionKinds;

         MAKE_SWAP_METHOD(CodeActionOptions, workDoneProgress, codeActionKinds);
};
MAKE_REFLECT_STRUCT(CodeActionOptions, workDoneProgress, codeActionKinds)
struct CodeLensOptions : WorkDoneProgressOptions {
        //
         // Code lens has a resolve provider as well.
         //
        optional<bool> resolveProvider ;
        MAKE_SWAP_METHOD(CodeLensOptions, workDoneProgress, resolveProvider);
};
MAKE_REFLECT_STRUCT(CodeLensOptions, workDoneProgress, resolveProvider)


// Format document on type options
struct lsDocumentOnTypeFormattingOptions :WorkDoneProgressOptions {
        // A character on which formatting should be triggered, like `}`.
        std::string firstTriggerCharacter;

        // More trigger characters.
        std::vector<std::string> moreTriggerCharacter;
        MAKE_SWAP_METHOD(lsDocumentOnTypeFormattingOptions, workDoneProgress,
                firstTriggerCharacter,
                moreTriggerCharacter);
};
MAKE_REFLECT_STRUCT(lsDocumentOnTypeFormattingOptions, workDoneProgress,
        firstTriggerCharacter,
        moreTriggerCharacter);
struct RenameOptions : WorkDoneProgressOptions {
        //
         // Renames should be checked and tested before being executed.
         //
        optional<bool> prepareProvider;
        MAKE_SWAP_METHOD(RenameOptions, workDoneProgress, prepareProvider);
};
MAKE_REFLECT_STRUCT(RenameOptions,workDoneProgress,prepareProvider)

struct DocumentFilter{
        //
         // A language id, like `typescript`.
         //
        optional<std::string> language;
        //
         // A Uri [scheme](#Uri.scheme), like `file` or `untitled`.
         //
        optional<std::string>scheme;
        //
         // A glob pattern, like `*.{ts,js}`.
         //
         // Glob patterns can have the following syntax:
         // - `*` to match one or more characters in a path segment
         // - `?` to match on one character in a path segment
         // - `**` to match any number of path segments, including none
         // - `{}` to group sub patterns into an OR expression. (e.g. `**/*.{ts,js}
         //   matches all TypeScript and JavaScript files)
         // - `[]` to declare a range of characters to match in a path segment
         //   (e.g., `example.[0-9]` to match on `example.0`, `example.1`,...)
         // - `[!...]` to negate a range of characters to match in a path segment
         //   (e.g., `example.[!0-9]` to match on `example.a`, `example.b`, but
         //   not `example.0`)
         //
        optional<std::string>pattern;
        MAKE_SWAP_METHOD(DocumentFilter, language, scheme, pattern)
};
MAKE_REFLECT_STRUCT(DocumentFilter, language, scheme, pattern)

//A document selector is the combination of one or more document filters.
using DocumentSelector = std::vector<DocumentFilter>;

// Document link options
struct lsDocumentLinkOptions :WorkDoneProgressOptions {
        // Document links have a resolve provider as well.
        optional<bool> resolveProvider;
        MAKE_SWAP_METHOD(lsDocumentLinkOptions, workDoneProgress, resolveProvider);
};
MAKE_REFLECT_STRUCT(lsDocumentLinkOptions, workDoneProgress,resolveProvider);

// Execute command options.
struct lsExecuteCommandOptions : WorkDoneProgressOptions {
        // The commands to be executed on the server
        std::vector<std::string> commands;
        MAKE_SWAP_METHOD(lsExecuteCommandOptions, workDoneProgress, commands);
};
MAKE_REFLECT_STRUCT(lsExecuteCommandOptions, workDoneProgress, commands);


struct TextDocumentRegistrationOptions
{
//
 // A document selector to identify the scope of the registration. If set to null
 // the document selector provided on the client side will be used.
 //
        optional<DocumentSelector>  documentSelector;

        MAKE_SWAP_METHOD(TextDocumentRegistrationOptions, documentSelector);
};
MAKE_REFLECT_STRUCT(TextDocumentRegistrationOptions, documentSelector);

//
 // Static registration options to be returned in the initialize request.
 //
struct StaticRegistrationOptions :public TextDocumentRegistrationOptions
{
        //
         // The id used to register the request. The id can be used to deregister
         // the request again. See also Registration#id.
         //
        optional<std::string> id;
        MAKE_SWAP_METHOD(StaticRegistrationOptions, documentSelector, id)
};
MAKE_REFLECT_STRUCT(StaticRegistrationOptions, documentSelector,id)

//
 // The server supports workspace folder.
 //
 // Since 3.6.0
 //

struct WorkspaceFoldersOptions {
        //
         // The server has support for workspace folders
         //
        optional<bool>  supported;

        //
         // Whether the server wants to receive workspace folder
         // change notifications.
         //
         // If a string is provided, the string is treated as an ID
         // under which the notification is registered on the client
         // side. The ID can be used to unregister for these events
         // using the `client/unregisterCapability` request.
         //
        optional<std::pair<  optional<std::string>, optional<bool> > > changeNotifications;
        MAKE_SWAP_METHOD(WorkspaceFoldersOptions, supported, changeNotifications);
};
MAKE_REFLECT_STRUCT(WorkspaceFoldersOptions, supported, changeNotifications);

//
 // A pattern kind describing if a glob pattern matches a file a folder or
 // both.
 //
 // @since 3.16.0
 //
enum lsFileOperationPatternKind
{
        file,
        folder
};
MAKE_REFLECT_TYPE_PROXY(lsFileOperationPatternKind)

//
 // Matching options for the file operation pattern.
 //
 // @since 3.16.0
 //
struct lsFileOperationPatternOptions {

        //
         // The pattern should be matched ignoring casing.
         //
        optional<bool> ignoreCase;
        MAKE_SWAP_METHOD(lsFileOperationPatternOptions, ignoreCase)
};
MAKE_REFLECT_STRUCT(lsFileOperationPatternOptions, ignoreCase)
//
 // A pattern to describe in which file operation requests or notifications
 // the server is interested in.
 //
 // @since 3.16.0
 //
struct lsFileOperationPattern {
        //
         // The glob pattern to match. Glob patterns can have the following syntax:
         // - `*` to match one or more characters in a path segment
         // - `?` to match on one character in a path segment
         // - `**` to match any number of path segments, including none
         // - `{}` to group sub patterns into an OR expression. (e.g. `**/*.{ts,js}`
         //   matches all TypeScript and JavaScript files)
         // - `[]` to declare a range of characters to match in a path segment
         //   (e.g., `example.[0-9]` to match on `example.0`, `example.1`,...)
         // - `[!...]` to negate a range of characters to match in a path segment
         //   (e.g., `example.[!0-9]` to match on `example.a`, `example.b`, but
         //   not `example.0`)
         //
        std::string glob;

        //
         // Whether to match files or folders with this pattern.
         //
         // Matches both if undefined.
         //
        optional<lsFileOperationPatternKind> matches;

        //
         // Additional options used during matching.
         //
        optional<lsFileOperationPatternOptions> options ;
        MAKE_SWAP_METHOD(lsFileOperationPattern, glob, matches, options)
};
MAKE_REFLECT_STRUCT(lsFileOperationPattern, glob, matches, options)
//
 // A filter to describe in which file operation requests or notifications
 // the server is interested in.
 //
 // @since 3.16.0
 //
struct lsFileOperationFilter {

        //
         // A Uri like `file` or `untitled`.
         //
        optional<std::string>  scheme;

        //
         // The actual file operation pattern.
         //
        optional<lsFileOperationPattern>   pattern;
        MAKE_SWAP_METHOD(lsFileOperationFilter, scheme, pattern)
};
MAKE_REFLECT_STRUCT(lsFileOperationFilter, scheme, pattern)
//
 // The options to register for file operations.
 //
 // @since 3.16.0
 //
struct lsFileOperationRegistrationOptions {
        //
         // The actual filters.
         //
        optional<std::vector<lsFileOperationFilter>> filters;
        MAKE_SWAP_METHOD(lsFileOperationRegistrationOptions, filters)
};
MAKE_REFLECT_STRUCT(lsFileOperationRegistrationOptions, filters)

struct WorkspaceServerCapabilities {
        //
         // The server supports workspace folder.
         //
         // Since 3.6.0
         //
        WorkspaceFoldersOptions workspaceFolders;


        //
         // The server is interested in file notifications/requests.
         //
         // @since 3.16.0
         //
        struct  lsFileOperations
        {
                //
                 // The server is interested in receiving didCreateFiles
                 // notifications.
                 //
                optional<lsFileOperationRegistrationOptions> didCreate;

                //
                 // The server is interested in receiving willCreateFiles requests.
                 //
                optional<lsFileOperationRegistrationOptions> willCreate;

                //
                 // The server is interested in receiving didRenameFiles
                 // notifications.
                 //
                optional<lsFileOperationRegistrationOptions> didRename;

                //
                 // The server is interested in receiving willRenameFiles requests.
                 //
                optional<lsFileOperationRegistrationOptions> willRename;

                //
                 // The server is interested in receiving didDeleteFiles file
                 // notifications.
                 //
                optional<lsFileOperationRegistrationOptions> didDelete;

                //
                 // The server is interested in receiving willDeleteFiles file
                 // requests.
                 //
                optional<lsFileOperationRegistrationOptions> willDelete;
                MAKE_SWAP_METHOD(lsFileOperations, didCreate, willCreate, didRename, willRename, didDelete, willDelete)
        };
        optional<lsFileOperations>fileOperations;


        MAKE_SWAP_METHOD(WorkspaceServerCapabilities, workspaceFolders, fileOperations)
};
MAKE_REFLECT_STRUCT(WorkspaceServerCapabilities, workspaceFolders, fileOperations)
MAKE_REFLECT_STRUCT(WorkspaceServerCapabilities::lsFileOperations, didCreate, willCreate, didRename, willRename, didDelete, willDelete)

//
 // Semantic highlighting server capabilities.
 //
 // <p>
 // <b>Note:</b> the <a href=
 // "https://github.com/Microsoft/vscode-languageserver-node/pull/367">{@code textDocument/semanticHighlighting}
 // language feature</a> is not yet part of the official LSP specification.
 //

struct SemanticHighlightingServerCapabilities {
        //
         // A "lookup table" of semantic highlighting <a href="https://manual.macromates.com/en/language_grammars">TextMate scopes</a>
         // supported by the language server. If not defined or empty, then the server does not support the semantic highlighting
         // feature. Otherwise, clients should reuse this "lookup table" when receiving semantic highlighting notifications from
         // the server.
         //
         std::vector< std::vector<std::string> > scopes;
         MAKE_SWAP_METHOD(SemanticHighlightingServerCapabilities, scopes)
};
MAKE_REFLECT_STRUCT(SemanticHighlightingServerCapabilities, scopes)

struct SemanticTokensServerFull
{
        //
        // The server supports deltas for full documents.
        //
        bool delta =false;
        MAKE_SWAP_METHOD(SemanticTokensServerFull, delta)
};
MAKE_REFLECT_STRUCT(SemanticTokensServerFull, delta)
struct SemanticTokensWithRegistrationOptions
{
        SemanticTokensLegend legend;

        //
         // Server supports providing semantic tokens for a specific range
         // of a document.
         //
        optional< std::pair< optional<bool>, optional<lsp::Any> > >  range;

        //
         // Server supports providing semantic tokens for a full document.
         //
        optional< std::pair< optional<bool>,
        optional<SemanticTokensServerFull> > >  full;

        //
         // A document selector to identify the scope of the registration. If set to null
         // the document selector provided on the client side will be used.
         //
        optional < std::vector<DocumentFilter> > documentSelector;
        //
         // The id used to register the request. The id can be used to deregister
         // the request again. See also Registration#id.
         //
        optional<std::string> id;
        MAKE_SWAP_METHOD(SemanticTokensWithRegistrationOptions, legend, range, full, documentSelector, id)
};
MAKE_REFLECT_STRUCT(SemanticTokensWithRegistrationOptions, legend, range, full, documentSelector ,id)

using  DocumentColorOptions = WorkDoneProgressOptions;
using  FoldingRangeOptions = WorkDoneProgressOptions;
struct lsServerCapabilities {
        // Defines how text documents are synced. Is either a detailed structure
        // defining each notification or for backwards compatibility the

        // TextDocumentSyncKind number.
        optional< std::pair<optional<lsTextDocumentSyncKind>,
        optional<lsTextDocumentSyncOptions> >> textDocumentSync;

        // The server provides hover support.
        optional<bool>  hoverProvider;

        // The server provides completion support.
        optional < lsCompletionOptions > completionProvider;

        // The server provides signature help support.
        optional < lsSignatureHelpOptions > signatureHelpProvider;

        // The server provides goto definition support.
        optional< std::pair< optional<bool>, optional<WorkDoneProgressOptions> > > definitionProvider;


  //
   // The server provides Goto Type Definition support.
   //
   // Since 3.6.0
   //
        optional< std::pair< optional<bool>, optional<StaticRegistrationOptions> > > typeDefinitionProvider ;

        // The server provides implementation support.
        optional< std::pair< optional<bool>, optional<StaticRegistrationOptions> > >  implementationProvider ;

        // The server provides find references support.
        optional< std::pair< optional<bool>, optional<WorkDoneProgressOptions> > > referencesProvider ;

        // The server provides document highlight support.
        optional< std::pair< optional<bool>, optional<WorkDoneProgressOptions> > > documentHighlightProvider ;

        // The server provides document symbol support.
        optional< std::pair< optional<bool>, optional<WorkDoneProgressOptions> > > documentSymbolProvider ;

        // The server provides workspace symbol support.
        optional< std::pair< optional<bool>, optional<WorkDoneProgressOptions> > > workspaceSymbolProvider ;

        // The server provides code actions.
        optional< std::pair< optional<bool>, optional<CodeActionOptions> > > codeActionProvider ;

        // The server provides code lens.
        optional<CodeLensOptions> codeLensProvider;

        // The server provides document formatting.
        optional< std::pair< optional<bool>, optional<WorkDoneProgressOptions> > > documentFormattingProvider ;

        // The server provides document range formatting.
        optional< std::pair< optional<bool>, optional<WorkDoneProgressOptions> > > documentRangeFormattingProvider ;

        // The server provides document formatting on typing.
        optional<lsDocumentOnTypeFormattingOptions> documentOnTypeFormattingProvider;

        // The server provides rename support.
        optional< std::pair< optional<bool>, optional<RenameOptions> > >  renameProvider;


        // The server provides document link support.
        optional<lsDocumentLinkOptions > documentLinkProvider;


        //
         // The server provides color provider support.
         //
         // @since 3.6.0
         //
        optional< std::pair< optional<bool>, optional<DocumentColorOptions> > >  colorProvider;


        //
                 // The server provides folding provider support.
                 //
                 // @since 3.10.0
                 //
        optional <  std::pair< optional<bool>, optional<FoldingRangeOptions> >   > foldingRangeProvider;

        // The server provides execute command support.
        optional < lsExecuteCommandOptions >executeCommandProvider;


        //
         // Workspace specific server capabilities
         //
        optional< WorkspaceServerCapabilities > workspace;

        //
         // Semantic highlighting server capabilities.
         //

         optional<  SemanticHighlightingServerCapabilities >semanticHighlighting;

        //
         // Server capability for calculating super- and subtype hierarchies.
         // The LS supports the type hierarchy language feature, if this capability is set to {@code true}.
         //
         // <p>
         // <b>Note:</b> the <a href=
         // "https://github.com/Microsoft/vscode-languageserver-node/pull/426">{@code textDocument/typeHierarchy}
         // language feature</a> is not yet part of the official LSP specification.
         //

         optional< std::pair< optional<bool>,
        optional<StaticRegistrationOptions> > > typeHierarchyProvider;

        //
         // The server provides Call Hierarchy support.
         //

         optional< std::pair< optional<bool>,
        optional<StaticRegistrationOptions> > > callHierarchyProvider;

        //
         // The server provides selection range support.
         //
         // Since 3.15.0
         //
         optional< std::pair< optional<bool>,
        optional<StaticRegistrationOptions> > > selectionRangeProvider;

         //
          // The server provides linked editing range support.
          //
          // Since 3.16.0
          //
         optional< std::pair< optional<bool>,
                 optional<StaticRegistrationOptions> > > linkedEditingRangeProvider;


         //
          // The server provides semantic tokens support.
          //
          // Since 3.16.0
          //
         optional < SemanticTokensWithRegistrationOptions> semanticTokensProvider;

         //
          // Whether server provides moniker support.
          //
          // Since 3.16.0
          //
         optional< std::pair< optional<bool>,
                 optional<StaticRegistrationOptions> > >  monikerProvider;

        optional<lsp::Any> experimental;


        MAKE_SWAP_METHOD(lsServerCapabilities,
                textDocumentSync,
                hoverProvider,
                completionProvider,
                signatureHelpProvider,
                definitionProvider,
                typeDefinitionProvider,
                implementationProvider,
                referencesProvider,
                documentHighlightProvider,
                documentSymbolProvider,
                workspaceSymbolProvider,
                codeActionProvider,
                codeLensProvider,
                documentFormattingProvider,
                documentRangeFormattingProvider,
                documentOnTypeFormattingProvider,
                renameProvider,
                documentLinkProvider,
                executeCommandProvider,
                workspace,
                semanticHighlighting,
                typeHierarchyProvider,
                callHierarchyProvider,
                selectionRangeProvider,
                experimental, colorProvider, foldingRangeProvider,
                linkedEditingRangeProvider, monikerProvider, semanticTokensProvider)

};
MAKE_REFLECT_STRUCT(lsServerCapabilities,
        textDocumentSync,
        hoverProvider,
        completionProvider,
        signatureHelpProvider,
        definitionProvider,
        typeDefinitionProvider,
        implementationProvider,
        referencesProvider,
        documentHighlightProvider,
        documentSymbolProvider,
        workspaceSymbolProvider,
        codeActionProvider,
        codeLensProvider,
        documentFormattingProvider,
        documentRangeFormattingProvider,
        documentOnTypeFormattingProvider,
        renameProvider,
        documentLinkProvider,
        executeCommandProvider,
        workspace,
        semanticHighlighting,
        typeHierarchyProvider,
        callHierarchyProvider,
        selectionRangeProvider,
        experimental, colorProvider, foldingRangeProvider,
        linkedEditingRangeProvider, monikerProvider, semanticTokensProvider)
