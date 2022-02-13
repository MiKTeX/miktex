#pragma once

#include "LibLsp/JsonRpc/serializer.h"
#include <vector>
#include "lsDocumentUri.h"
#include "LibLsp/lsp/lsAny.h"
#include "LibLsp/lsp/lsTextEdit.h"
struct ResourceOperation {
	std::string kind;
	virtual  ~ResourceOperation() = default;
	
	MAKE_SWAP_METHOD(ResourceOperation, kind);
};
MAKE_REFLECT_STRUCT(ResourceOperation, kind);
extern void Reflect(Writer& visitor, ResourceOperation* value);
struct CreateFileOptions{

	/**
	 * Overwrite existing file. Overwrite wins over `ignoreIfExists`
	 */
	boost::optional<bool>  overwrite = false;

	/**
	 * Ignore if exists.
	 */
	boost::optional< bool> ignoreIfExists =false;
	
	MAKE_SWAP_METHOD(CreateFileOptions, overwrite, ignoreIfExists)
};
MAKE_REFLECT_STRUCT(CreateFileOptions, overwrite, ignoreIfExists)
struct lsCreateFile :public ResourceOperation {

	/**
	 * The resource to create.
	 */
	lsCreateFile();
	lsDocumentUri uri;

	/**
	 * Additional options
	 */
	boost::optional<CreateFileOptions>  options;


	/**
	 * An optional annotation identifer describing the operation.
	 *
	 * @since 3.16.0
	 */
	boost::optional<lsChangeAnnotationIdentifier> annotationId;
	
	MAKE_SWAP_METHOD(lsCreateFile, kind, uri, options, annotationId)
};
MAKE_REFLECT_STRUCT(lsCreateFile, kind, uri,options, annotationId)


struct DeleteFileOptions {
	/**
	 * Delete the content recursively if a folder is denoted.
	 */
	boost::optional<bool>  recursive = false;

	/**
	 * Ignore the operation if the file doesn't exist.
	 */
	boost::optional<bool> ignoreIfNotExists = false;


	MAKE_SWAP_METHOD(DeleteFileOptions, recursive, ignoreIfNotExists);
};

MAKE_REFLECT_STRUCT(DeleteFileOptions, recursive, ignoreIfNotExists)

struct lsDeleteFile :public ResourceOperation {
	/**
	 * The file to delete.
	 */
	lsDeleteFile();
	lsDocumentUri uri;

	/**
	 * Delete options.
	 */
	boost::optional<DeleteFileOptions>  options;

	MAKE_SWAP_METHOD(lsDeleteFile, kind, uri, options);
};
MAKE_REFLECT_STRUCT(lsDeleteFile, kind, uri,options);

typedef  CreateFileOptions RenameFileOptions;
struct lsRenameFile :public ResourceOperation {
	/**
	 * The old (existing) location.
	 */
	lsRenameFile();
	lsDocumentUri oldUri;

	/**
	 * The new location.
	 */

	lsDocumentUri newUri;

	/**
	 * Rename options.
	 */
	boost::optional<RenameFileOptions>  options;

	/**
	 * An optional annotation identifer describing the operation.
	 *
	 * @since 3.16.0
	 */
	boost::optional<lsChangeAnnotationIdentifier> annotationId;
	
	MAKE_SWAP_METHOD(lsRenameFile, kind, oldUri, newUri, options, annotationId)
};
MAKE_REFLECT_STRUCT(lsRenameFile, kind, oldUri, newUri, options, annotationId);


extern  ResourceOperation* GetResourceOperation(lsp::Any& lspAny);