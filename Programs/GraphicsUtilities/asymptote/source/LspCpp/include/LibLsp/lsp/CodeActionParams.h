#pragma once
#include "LibLsp/lsp/method_type.h"
#include "LibLsp/JsonRpc/message.h"
#include "LibLsp/lsp/lsp_diagnostic.h"
#include "LibLsp/lsp/workspace/execute_command.h"
#include "LibLsp/lsp/lsTextDocumentIdentifier.h"
#include "LibLsp/lsp/lsCodeAction.h"
namespace  JDT
{
	namespace CodeActionKind {


		/**
		 * Base kind for quickfix actions: 'quickfix'
		 */
		extern  const char* QuickFix;

		/**
		 * Base kind for refactoring actions: 'refactor'
		 */
		extern const char* Refactor;

		/**
		 * Base kind for refactoring extraction actions: 'refactor.extract'
		 *
		 * Example extract actions:
		 *
		 * - Extract method - Extract function - Extract variable - Extract interface
		 * from class - ...
		 */
		extern	const char* RefactorExtract;

		/**
		 * Base kind for refactoring inline actions: 'refactor.inline'
		 *
		 * Example inline actions:
		 *
		 * - Inline function - Inline variable - Inline constant - ...
		 */
		extern const char* RefactorInline;

		/**
		 * Base kind for refactoring rewrite actions: 'refactor.rewrite'
		 *
		 * Example rewrite actions:
		 *
		 * - Convert JavaScript function to class - Add or remove parameter -
		 * Encapsulate field - Make method static - Move method to base class - ...
		 */
		extern const char* RefactorRewrite;

		/**
		 * Base kind for source actions: `source`
		 *
		 * Source code actions apply to the entire file.
		 */
		extern const char* Source ;

		/**
		 * Base kind for an organize imports source action: `source.organizeImports`
		 */
		extern const char* SourceOrganizeImports;

		extern  const char* COMMAND_ID_APPLY_EDIT;
	};


}
struct lsCodeActionContext {
	// An array of diagnostics.
	std::vector<lsDiagnostic> diagnostics;
	/**
	 * Requested kind of actions to return.
	 *
	 * Actions not of this kind are filtered out by the client before being shown. So servers
	 * can omit computing them.
	 *
	 * See {@link CodeActionKind} for allowed values.
	 */
	boost::optional<std::vector<std::string>> only;

	MAKE_SWAP_METHOD(lsCodeActionContext,
		diagnostics, only);
};
MAKE_REFLECT_STRUCT(lsCodeActionContext,
	diagnostics, only);


// Params for the CodeActionRequest
struct lsCodeActionParams {
	// The document in which the command was invoked.
	lsTextDocumentIdentifier textDocument;
	// The range for which the command was invoked.
	lsRange range;
	// Context carrying additional information.
	lsCodeActionContext context;

	MAKE_SWAP_METHOD(lsCodeActionParams,
		textDocument,
		range,
		context);
};
MAKE_REFLECT_STRUCT(lsCodeActionParams,
                    textDocument,
                    range,
                    context);








