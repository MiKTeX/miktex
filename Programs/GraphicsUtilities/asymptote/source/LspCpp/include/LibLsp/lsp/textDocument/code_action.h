#pragma once
#include "LibLsp/lsp/method_type.h"
#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include "LibLsp/lsp/lsTextDocumentIdentifier.h"
#include "LibLsp/lsp/CodeActionParams.h"

namespace  QuickAssistProcessor {

	extern const char* SPLIT_JOIN_VARIABLE_DECLARATION_ID;//$NON-NLS-1$
	extern const char* CONVERT_FOR_LOOP_ID;// ;// "org.eclipse.jdt.ls.correction.convertForLoop.assist"; //$NON-NLS-1$
	extern const char* ASSIGN_TO_LOCAL_ID ;// "org.eclipse.jdt.ls.correction.assignToLocal.assist"; //$NON-NLS-1$
	extern const char* ASSIGN_TO_FIELD_ID ;// "org.eclipse.jdt.ls.correction.assignToField.assist"; //$NON-NLS-1$
	extern const char* ASSIGN_PARAM_TO_FIELD_ID ;// "org.eclipse.jdt.ls.correction.assignParamToField.assist"; //$NON-NLS-1$
	extern const char* ASSIGN_ALL_PARAMS_TO_NEW_FIELDS_ID ;// "org.eclipse.jdt.ls.correction.assignAllParamsToNewFields.assist"; //$NON-NLS-1$
	extern const char* ADD_BLOCK_ID ;// "org.eclipse.jdt.ls.correction.addBlock.assist"; //$NON-NLS-1$
	extern const char* EXTRACT_LOCAL_ID ;// "org.eclipse.jdt.ls.correction.extractLocal.assist"; //$NON-NLS-1$
	extern const char* EXTRACT_LOCAL_NOT_REPLACE_ID ;// "org.eclipse.jdt.ls.correction.extractLocalNotReplaceOccurrences.assist"; //$NON-NLS-1$
	extern const char* EXTRACT_CONSTANT_ID ;// "org.eclipse.jdt.ls.correction.extractConstant.assist"; //$NON-NLS-1$
	extern const char* INLINE_LOCAL_ID ;// "org.eclipse.jdt.ls.correction.inlineLocal.assist"; //$NON-NLS-1$
	extern const char* CONVERT_LOCAL_TO_FIELD_ID ;// "org.eclipse.jdt.ls.correction.convertLocalToField.assist"; //$NON-NLS-1$
	extern const char* CONVERT_ANONYMOUS_TO_LOCAL_ID ;// "org.eclipse.jdt.ls.correction.convertAnonymousToLocal.assist"; //$NON-NLS-1$
	extern const char* CONVERT_TO_STRING_BUFFER_ID ;// "org.eclipse.jdt.ls.correction.convertToStringBuffer.assist"; //$NON-NLS-1$
	extern const char* CONVERT_TO_MESSAGE_FORMAT_ID ;// "org.eclipse.jdt.ls.correction.convertToMessageFormat.assist"; //$NON-NLS-1$;
	extern const char* EXTRACT_METHOD_INPLACE_ID ;// "org.eclipse.jdt.ls.correction.extractMethodInplace.assist"; //$NON-NLS-1$;

	extern const char* CONVERT_ANONYMOUS_CLASS_TO_NESTED_COMMAND ;// "convertAnonymousClassToNestedCommand";
};
/**
 * The code action request is sent from the client to the server to compute
 * commands for a given text document and range. These commands are
 * typically code fixes to either fix problems or to beautify/refactor code.
 *
 * Registration Options: TextDocumentRegistrationOptions
 */

DEFINE_REQUEST_RESPONSE_TYPE(td_codeAction, lsCodeActionParams, std::vector<lsCommandWithAny>, "textDocument/codeAction");
