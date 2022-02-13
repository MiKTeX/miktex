#pragma once

#include "LibLsp/JsonRpc/serializer.h"
#include <vector>
#include "lsTextDocumentEdit.h"
#include "LibLsp/lsp/ResourceOperation.h"
#include "lsAny.h"

//A workspace edit represents changes to many resources managed in the workspace.
//The edit should either provide changes or documentChanges.
//If the client can handle versioned document edits and if documentChanges are present, the latter are preferred over changes.

//Since version 3.13.0 a workspace edit can contain resource operations(create, delete or rename files and folders) as well.
//If resource operations are present clients need to execute the operations in the order in which they are provided.
//So a workspace edit for example can consist of the following two changes : (1) create file a.txt and (2) a text document edit which insert text into file a.
//txt.An invalid sequence(e.g. (1) delete file a.txt and (2) insert text into file a.txt) will cause failure of the operation.
//How the client recovers from the failure is described by the client capability : workspace.workspaceEdit.failureHandling



struct lsChangeAnnotations
{
	lsChangeAnnotation id;
	MAKE_SWAP_METHOD(lsChangeAnnotations, id)
};
MAKE_REFLECT_STRUCT(lsChangeAnnotations, id)
struct lsWorkspaceEdit {
	// Holds changes to existing resources.
	// changes ? : { [uri:string]: TextEdit[]; };
	// std::unordered_map<lsDocumentUri, std::vector<lsTextEdit>> changes;

	// An array of `TextDocumentEdit`s to express changes to specific a specific
	// version of a text document. Whether a client supports versioned document
	// edits is expressed via `WorkspaceClientCapabilites.versionedWorkspaceEdit`.
	//
	boost::optional< std::map<std::string, std::vector<lsTextEdit> > >  changes;
	typedef std::pair < boost::optional<lsTextDocumentEdit>, boost::optional<lsp::Any> > Either;

	boost::optional <  std::vector< Either > > documentChanges;
	/**
	 * A map of change annotations that can be referenced in
	 * `AnnotatedTextEdit`s or create, rename and delete file / folder
	 * operations.
	 *
	 * Whether clients honor this property depends on the client capability
	 * `workspace.changeAnnotationSupport`.
	 *
	 * @since 3.16.0
	 */
	boost::optional< lsChangeAnnotations > changeAnnotations;

	MAKE_SWAP_METHOD(lsWorkspaceEdit, changes, documentChanges, changeAnnotations)
};
MAKE_REFLECT_STRUCT(lsWorkspaceEdit, changes, documentChanges, changeAnnotations)

extern void Reflect(Reader& visitor, lsWorkspaceEdit::Either& value);

