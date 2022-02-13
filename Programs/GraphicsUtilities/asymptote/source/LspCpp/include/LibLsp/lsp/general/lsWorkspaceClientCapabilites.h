#pragma once
#include "LibLsp/lsp/method_type.h"


#include <stdexcept>

#include "LibLsp/JsonRpc/message.h"
#include "LibLsp/lsp/lsDocumentUri.h"
#include "LibLsp/lsp/lsAny.h"
#include "LibLsp/lsp/extention/jdtls/searchSymbols.h"

/**
 * Capabilities specific to `WorkspaceEdit`s
 */

//New in version 3.13: ResourceOperationKind and FailureHandlingKind and the client capability workspace.workspaceEdit.
//resourceOperations as well as workspace.workspaceEdit.failureHandling.

//The capabilities of a workspace edit has evolved over the time.
//Clients can describe their support using the following client capability :

struct lschangeAnnotationSupport
{
	/**
	 * Whether the client groups edits with equal labels into tree nodes,
	 * for instance all edits labelled with "Changes in Strings" would
	 * be a tree node.
	 */
	boost::optional<bool> groupsOnLabel;
	MAKE_SWAP_METHOD(lschangeAnnotationSupport, groupsOnLabel)
};
MAKE_REFLECT_STRUCT(lschangeAnnotationSupport, groupsOnLabel)

struct WorkspaceEditCapabilities {
	/**
	 * The client supports versioned document changes in `WorkspaceEdit`s
	 */
	boost::optional<bool>  documentChanges;

	/**
	 * The client supports resource changes
	 * in `WorkspaceEdit`s.
	 *
	 * @deprecated Since LSP introduces resource operations, use {link #resourceOperations}
	 */

	boost::optional<bool> resourceChanges;

	/**
	 * The resource operations the client supports. Clients should at least
	 * support 'create', 'rename' and 'delete' files and folders.
	 *
	 * @since 3.13.0
	 */
	boost::optional< std::vector<std::string> > resourceOperations;

	/**
	 * The failure handling strategy of a client if applying the workspace edit
	 * fails.
	 *
	 * See {@link FailureHandlingKind} for allowed values.
	 */
	boost::optional<std::string > failureHandling;

	/**
	 * Whether the client normalizes line endings to the client specific
	 * setting.
	 * If set to `true` the client will normalize line ending characters
	 * in a workspace edit to the client specific new line character(s).
	 *
	 * @since 3.16.0
	 */
	boost::optional<bool> normalizesLineEndings;;

	/**
	 * Whether the client in general supports change annotations on text edits,
	 * create file, rename file and delete file changes.
	 *
	 * @since 3.16.0
	 */
	boost::optional<lschangeAnnotationSupport> changeAnnotationSupport;
	
	MAKE_SWAP_METHOD(WorkspaceEditCapabilities, documentChanges, resourceChanges, resourceOperations, failureHandling, normalizesLineEndings, changeAnnotationSupport)

};
MAKE_REFLECT_STRUCT(WorkspaceEditCapabilities,documentChanges, resourceChanges, resourceOperations, failureHandling, normalizesLineEndings, changeAnnotationSupport)


struct DynamicRegistrationCapabilities {
	// Did foo notification supports dynamic registration.
	boost::optional<bool> dynamicRegistration;

	MAKE_SWAP_METHOD(DynamicRegistrationCapabilities,
		dynamicRegistration);
};

MAKE_REFLECT_STRUCT(DynamicRegistrationCapabilities,
	dynamicRegistration);



// Workspace specific client capabilities.
struct SymbolKindCapabilities
{
	boost::optional< std::vector<lsSymbolKind> >  valueSet;

	MAKE_SWAP_METHOD(SymbolKindCapabilities, valueSet)


};
MAKE_REFLECT_STRUCT(SymbolKindCapabilities, valueSet)




struct SymbolCapabilities :public DynamicRegistrationCapabilities {
	/**
	 * Specific capabilities for the `SymbolKind` in the `workspace/symbol` request.
	 */
	boost::optional<SymbolKindCapabilities>  symbolKind;

	MAKE_SWAP_METHOD(SymbolCapabilities,
		symbolKind, dynamicRegistration)
};
MAKE_REFLECT_STRUCT(SymbolCapabilities,
	symbolKind, dynamicRegistration)


struct lsFileOperations
{
	/**
 * Whether the client supports dynamic registration for file
 * requests/notifications.
 */
	boost::optional<bool> dynamicRegistration ;

	/**
	 * The client has support for sending didCreateFiles notifications.
	 */
	boost::optional<bool>didCreate ;

	/**
	 * The client has support for sending willCreateFiles requests.
	 */
	boost::optional<bool>willCreate ;

	/**
	 * The client has support for sending didRenameFiles notifications.
	 */
	boost::optional<bool>didRename ;

	/**
	 * The client has support for sending willRenameFiles requests.
	 */
	boost::optional<bool>willRename ;

	/**
	 * The client has support for sending didDeleteFiles notifications.
	 */
	boost::optional<bool>didDelete ;

	/**
	 * The client has support for sending willDeleteFiles requests.
	 */
	boost::optional<bool> willDelete ;
	MAKE_SWAP_METHOD(lsFileOperations, dynamicRegistration, didCreate, willCreate,
		didRename, willRename, didDelete, willDelete)
};
MAKE_REFLECT_STRUCT(lsFileOperations, dynamicRegistration, didCreate, willCreate,
	didRename, willRename, didDelete, willDelete)

struct lsWorkspaceClientCapabilites {
  // The client supports applying batch edits to the workspace.
  boost::optional<bool> applyEdit;

 

  // Capabilities specific to `WorkspaceEdit`s
  boost::optional<WorkspaceEditCapabilities> workspaceEdit;



  // Capabilities specific to the `workspace/didChangeConfiguration`
  // notification.
  boost::optional<DynamicRegistrationCapabilities> didChangeConfiguration;

  // Capabilities specific to the `workspace/didChangeWatchedFiles`
  // notification.
  boost::optional<DynamicRegistrationCapabilities> didChangeWatchedFiles;

  // Capabilities specific to the `workspace/symbol` request.
  boost::optional<SymbolCapabilities> symbol;

  // Capabilities specific to the `workspace/executeCommand` request.
  boost::optional<DynamicRegistrationCapabilities> executeCommand;


  /**
 * The client has support for workspace folders.
 *
 * Since 3.6.0
 */
  boost::optional<bool> workspaceFolders;

  /**
   * The client supports `workspace/configuration` requests.
   *
   * Since 3.6.0
   */
  boost::optional<bool> configuration;


  /**
		 * Capabilities specific to the semantic token requests scoped to the
		 * workspace.
		 *
		 * @since 3.16.0
		 */
  boost::optional<DynamicRegistrationCapabilities> semanticTokens ;

  /**
   * Capabilities specific to the code lens requests scoped to the
   * workspace.
   *
   * @since 3.16.0
   */
  boost::optional<DynamicRegistrationCapabilities> codeLens ;

  /**
   * The client has support for file requests/notifications.
   *
   * @since 3.16.0
   */
  boost::optional<lsFileOperations> fileOperations;
	
  MAKE_SWAP_METHOD(lsWorkspaceClientCapabilites,
	  applyEdit,
	  workspaceEdit,
	  didChangeConfiguration,
	  didChangeWatchedFiles,
	  symbol,executeCommand, workspaceFolders,
	  configuration, semanticTokens, codeLens, fileOperations)
};

MAKE_REFLECT_STRUCT(lsWorkspaceClientCapabilites,
                    applyEdit,
                    workspaceEdit,
                    didChangeConfiguration,
                    didChangeWatchedFiles,
                    symbol,
                    executeCommand,workspaceFolders,
	configuration, semanticTokens, codeLens, fileOperations)




