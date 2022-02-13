#pragma once
#include <LibLsp/lsp/general/lsClientCapabilities.h>
#include <LibLsp/lsp/utils.h>
#include <memory>
#include <vector>
#include <string>

class ClientPreferences
{
public:

	std::shared_ptr<lsWorkspaceClientCapabilites>  workspace;
	lsTextDocumentClientCapabilities textDocument ;
	
	ClientPreferences(const lsClientCapabilities& capabilities)
	{
		v3supported = capabilities.textDocument.has_value();
		if (v3supported)
			textDocument = capabilities.textDocument.value();
		if(capabilities.workspace)
		{
			workspace = std::make_shared<lsWorkspaceClientCapabilites>(capabilities.workspace.value());
		}
	}
	
	bool v3supported=false;

	bool isSignatureHelpSupported() {
	
		return v3supported && (textDocument.signatureHelp);
	}
	bool  isWorkspaceDidChangeConfigurationSupported() const
	{
		return workspace && isDynamicRegistrationSupported(workspace->didChangeConfiguration);
	}
	bool isWorkspaceFoldersSupported() {
		return workspace != nullptr && isTrue(workspace->workspaceFolders);
	}

	bool isCompletionDynamicRegistered() {
		return v3supported && isDynamicRegistrationSupported(textDocument.completion);
	}

	bool isCompletionSnippetsSupported() {
		//@formatter:off
		if(!v3supported || !textDocument.completion)
		{
			return false;
		}
		const auto& completion = textDocument.completion.value();
		if(completion.completionItem)
		{
			return isTrue(completion.completionItem.value().snippetSupport);
		}
		return false;
	}

	bool isV3Supported() {
		return v3supported;
	}

	bool isFormattingDynamicRegistrationSupported() {
		return v3supported && isDynamicRegistrationSupported(textDocument.formatting);
	}

	bool isRangeFormattingDynamicRegistrationSupported() {
		return v3supported && isDynamicRegistrationSupported(textDocument.rangeFormatting);
	}

	bool isOnTypeFormattingDynamicRegistrationSupported() {
		return v3supported && isDynamicRegistrationSupported(textDocument.onTypeFormatting);
	}

	bool isCodeLensDynamicRegistrationSupported() {
		return v3supported && isDynamicRegistrationSupported(textDocument.codeLens);
	}

	bool isSignatureHelpDynamicRegistrationSupported() {
		return v3supported && isDynamicRegistrationSupported(textDocument.signatureHelp);
	}
	template<typename  T>
	static bool isDynamicRegistrationSupported(boost::optional<T>& capability)
	{
		if(capability)
			return (capability.value().dynamicRegistration.value());
		return false;
	}
	
	bool isTrue(const boost::optional<bool>& value)
	{
		return  value.get_value_or(false);
	}

	bool isRenameDynamicRegistrationSupported() {
		return v3supported && isDynamicRegistrationSupported(textDocument.rename);
	}

	bool isExecuteCommandDynamicRegistrationSupported() {
		return v3supported && workspace != nullptr && isDynamicRegistrationSupported(workspace->executeCommand);
	}

	bool isWorkspaceSymbolDynamicRegistered() {
		return v3supported && workspace != nullptr && isDynamicRegistrationSupported(workspace->symbol);
	}

	bool isWorkspaceChangeWatchedFilesDynamicRegistered() {
		return v3supported && workspace != nullptr && isDynamicRegistrationSupported(workspace->didChangeWatchedFiles);
	}

	bool isDocumentSymbolDynamicRegistered() {
		return v3supported && isDynamicRegistrationSupported(textDocument.documentSymbol);
	}

	bool isCodeActionDynamicRegistered() {
		return v3supported && isDynamicRegistrationSupported(textDocument.codeAction);
	}

	bool isDefinitionDynamicRegistered() {
		return v3supported && isDynamicRegistrationSupported(textDocument.definition);
	}

	bool isTypeDefinitionDynamicRegistered() {
		return v3supported && isDynamicRegistrationSupported(textDocument.typeDefinition);
	}

	bool isHoverDynamicRegistered() {
		return v3supported && isDynamicRegistrationSupported(textDocument.hover);
	}

	bool isReferencesDynamicRegistered() {
		return v3supported && isDynamicRegistrationSupported(textDocument.references);
	}

	bool isDocumentHighlightDynamicRegistered() {
		return v3supported && isDynamicRegistrationSupported(textDocument.documentHighlight);
	}

	bool isFoldgingRangeDynamicRegistered() {
		return v3supported && isDynamicRegistrationSupported(textDocument.foldingRange);
	}

	bool isImplementationDynamicRegistered() {
		return v3supported && isDynamicRegistrationSupported(textDocument.implementation);
	}

	bool isSelectionRangeDynamicRegistered() {
		return v3supported && isDynamicRegistrationSupported(textDocument.selectionRange);
	}

	bool isWillSaveRegistered() {
		return v3supported && isTrue(textDocument.synchronization.willSave);
	}

	bool isWillSaveWaitUntilRegistered() {
		return v3supported && isTrue(textDocument.synchronization.willSaveWaitUntil);
	}

	bool isWorkspaceApplyEditSupported() {
		return workspace != nullptr && isTrue(workspace->applyEdit);
	}

	bool isSupportsCompletionDocumentationMarkdown() {

		if (!v3supported || !textDocument.completion)
		{
			return false;
		}
		const auto& completion = textDocument.completion.value();
		if (completion.completionItem)
		{
			auto& documentationFormat = completion.completionItem.value().documentationFormat;
			if(documentationFormat)
			{
				auto& data = documentationFormat.value();
				for(auto& it : data)
				{
					if(it == "markdown")
					{
						return true;
					}
				}
			}
		}
		return false;
		
	}


	bool isWorkspaceEditResourceChangesSupported() {
		if(!workspace) return false;
		
		if(workspace->workspaceEdit)
		{
			return isTrue(workspace->workspaceEdit.value().resourceChanges);
		}
		return false;
	}
	static  bool contains(const std::vector<std::string>& v, const std::string& target)
	{
		for(auto& it : v)
		{
			if(it == target) return true;
		}
		return false;
	}
	bool isResourceOperationSupported() const
	{
		if (!workspace) return false;
		if (!workspace->workspaceEdit)
		{
			return false;
		}
		auto& it = (workspace->workspaceEdit.value());
		if(!it.resourceOperations) return false;
		const auto& resourceOperations = it.resourceOperations.value();
		return contains(resourceOperations, "create") && contains(resourceOperations, "rename") && contains(resourceOperations, "delete");
		
	}

	/**
	 * {@code true} if the client has explicitly set the
	 * {@code textDocument.documentSymbol.hierarchicalDocumentSymbolSupport} to
	 * {@code true} when initializing the LS. Otherwise, {@code false}.
	 */
	bool isHierarchicalDocumentSymbolSupported() {
		if(!v3supported || !textDocument.documentSymbol) return false;
		return  isTrue(textDocument.documentSymbol.value().hierarchicalDocumentSymbolSupport);

	}

	bool isSemanticHighlightingSupported() {
		//@formatter:off
		if (!v3supported || !textDocument.semanticHighlightingCapabilities) return false;
		return  isTrue(textDocument.semanticHighlightingCapabilities.value().semanticHighlighting);
		//@formatter:on
	}

	/**
	 * {@code true} if the client has explicitly set the
	 * {@code textDocument.codeAction.codeActionLiteralSupport.codeActionKind.valueSet}
	 * value. Otherwise, {@code false}.
	 */
	bool isSupportedCodeActionKind(const std::string& kind) {
		if (!v3supported || !textDocument.codeAction) return false;
		//@formatter:off
		const auto& codeAction = textDocument.codeAction.value();
		if(codeAction.codeActionLiteralSupport)
		{
			const auto& codeActionKind = codeAction.codeActionLiteralSupport.value().codeActionKind;
			if(codeActionKind)
			{
				const auto& valueSet = codeActionKind.value().valueSet;
				if(valueSet)
				{
					for(auto& k : valueSet.value())
					{
						if(lsp::StartsWith(kind,k))
						{
							return true;
						}
					}
				}
			}
		}
		return false;

		//@formatter:on
	}

	/**
	 * {@code true} if the client has explicitly set the
	 * {@code textDocument.publishDiagnostics.tagSupport} to
	 * {@code true} when initializing the LS. Otherwise, {@code false}.
	 */
	bool isDiagnosticTagSupported() {
		if (!v3supported || !textDocument.publishDiagnostics) return false;
		const auto& publishDiagnostics = textDocument.publishDiagnostics.value();
		if(publishDiagnostics.tagSupport)
		{
			isTagSupported(publishDiagnostics.tagSupport);
		}
		return false;
	}

	bool isTagSupported(const boost::optional < std::pair<boost::optional<bool>,
		boost::optional<DiagnosticsTagSupport> > >& tagSupport) {
		if(tagSupport)
		{
			auto &v = tagSupport.value();
			if (v.first)
			{
				return v.first.value();
			}
			if (v.second) {
				return !v.second.value().valueSet.empty();
			}
		}
		return false;
	}

	bool isCallHierarchyDynamicRegistered() {
		return v3supported && isDynamicRegistrationSupported(textDocument.callHierarchy);
	}

};

