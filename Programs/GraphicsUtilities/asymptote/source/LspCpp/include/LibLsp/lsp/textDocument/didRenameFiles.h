#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include <string>
#include "LibLsp/lsp/CodeActionParams.h"
#include "LibLsp/lsp/lsWorkspaceEdit.h"


class FileRenameEvent {
public:
	std::string oldUri;
	std::string newUri;

	FileRenameEvent() {
	}

	FileRenameEvent(std::string oldUri, std::string newUri) {
		this->oldUri = oldUri;
		this->newUri = newUri;
	}
	MAKE_SWAP_METHOD(FileRenameEvent, oldUri, newUri);
};
MAKE_REFLECT_STRUCT(FileRenameEvent, oldUri, newUri);

class FileRenameParams {
public:
	std::vector <FileRenameEvent> files;

	FileRenameParams() {
	}

	FileRenameParams(std::vector<FileRenameEvent>& files) {
		this->files = files;
	}
	MAKE_SWAP_METHOD(FileRenameParams, files);
};
MAKE_REFLECT_STRUCT(FileRenameParams, files);


DEFINE_REQUEST_RESPONSE_TYPE(td_didRenameFiles, FileRenameParams, boost::optional<lsWorkspaceEdit>, "java/didRenameFiles");


DEFINE_REQUEST_RESPONSE_TYPE(td_willRenameFiles, FileRenameParams, boost::optional<lsWorkspaceEdit>, "java/willRenameFiles");
