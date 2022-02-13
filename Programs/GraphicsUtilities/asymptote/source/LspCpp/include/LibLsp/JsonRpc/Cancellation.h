#pragma once
#include <functional>
#include "lsRequestId.h"
#include <LibLsp/JsonRpc/NotificationInMessage.h>
using CancelMonitor = std::function<int()>;
namespace Cancellation
{

  struct Params {
	  /**
	   * The request id to cancel.
	   */
	  lsRequestId id;
  	
	  MAKE_SWAP_METHOD(Cancellation::Params, id);
  };

};
MAKE_REFLECT_STRUCT(Cancellation::Params, id);

DEFINE_NOTIFICATION_TYPE(Notify_Cancellation, Cancellation::Params,"$/cancelRequest");


