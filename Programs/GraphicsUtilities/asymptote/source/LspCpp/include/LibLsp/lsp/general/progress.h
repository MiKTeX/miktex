#pragma once

#include "LibLsp/JsonRpc/NotificationInMessage.h"
#include "LibLsp/lsp/lsAny.h"
//The base protocol offers also support to report progress in a generic fashion.
//This mechanism can be used to report any kind of progress including work done
//progress(usually used to report progress in the user interface using a progress bar)
//and partial result progress to support streaming of results.
struct  ProgressParams
{
	std::pair<boost::optional<std::string> , boost::optional<int> > token;
	lsp::Any value;
	MAKE_SWAP_METHOD(ProgressParams, token, value)
};
MAKE_REFLECT_STRUCT(ProgressParams, token, value)
DEFINE_NOTIFICATION_TYPE(Notify_Progress, ProgressParams, "$/progress");
