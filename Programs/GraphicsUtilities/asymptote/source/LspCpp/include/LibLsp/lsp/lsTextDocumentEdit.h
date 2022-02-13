#pragma once

#include "LibLsp/JsonRpc/serializer.h"
#include <vector>
#include "LibLsp/JsonRpc/message.h"
#include "lsVersionedTextDocumentIdentifier.h"
#include "lsTextEdit.h"


struct lsTextDocumentEdit {
		// The text document to change.
		lsVersionedTextDocumentIdentifier textDocument;
	
		/**
		 * The edits to be applied.
		 *
		 * @since 3.16.0 - support for AnnotatedTextEdit. This is guarded by the
		 * client capability `workspace.workspaceEdit.changeAnnotationSupport`
		 */
		// The edits to be applied.
		std::vector< lsAnnotatedTextEdit > edits;
		MAKE_SWAP_METHOD(lsTextDocumentEdit, textDocument, edits);
};
MAKE_REFLECT_STRUCT(lsTextDocumentEdit, textDocument, edits);