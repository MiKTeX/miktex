#pragma once
#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include <string>
#include "LibLsp/lsp/lsWorkspaceEdit.h"
#include "LibLsp/lsp/ExecuteCommandParams.h"
namespace  buildpath
{
//	static  const char* EDIT_ORGNIZEIMPORTS = "java.edit.organizeImports";
//	static  const char* RESOLVE_SOURCE_ATTACHMENT = "java.project.resolveSourceAttachment";

//	static  const char* UPDATE_SOURCE_ATTACHMENT = "java.project.updateSourceAttachment";

//	static  const char* ADD_TO_SOURCEPATH = "java.project.addToSourcePath";

//	static  const char* REMOVE_FROM_SOURCEPATH = "java.project.removeFromSourcePath";

//	static  const char* LIST_SOURCEPATHS = "java.project.listSourcePaths";
	struct Result {
		bool status;
		std::string message;
	};



}


DEFINE_REQUEST_RESPONSE_TYPE(java_executeCommand, ExecuteCommandParams, lsWorkspaceEdit, "java/executeCommand");
