#pragma once

#include "LibLsp/JsonRpc/NotificationInMessage.h"
#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"

// Show a message to the user.
enum class lsMessageType : int { Error = 1, Warning = 2, Info = 3, Log = 4 };
MAKE_REFLECT_TYPE_PROXY(lsMessageType)
struct MessageParams {
/**
  * The message type.
 */
	
	lsMessageType type = lsMessageType::Error;
	
/**
 * The actual message.
 */
	std::string message;
	
	void swap(MessageParams& arg) noexcept {
		lsMessageType temp = type;
		type = arg.type;
		arg.type = temp;
		message.swap(arg.message);
	}
};
MAKE_REFLECT_STRUCT(MessageParams, type, message)

 /**
  * The log message notification is send from the server to the client to ask
  * the client to log a particular message.
  */
DEFINE_NOTIFICATION_TYPE(Notify_LogMessage, MessageParams, "window/logMessage")


/**
 * The show message notification is sent from a server to a client to ask
 * the client to display a particular message in the user interface.
 */
DEFINE_NOTIFICATION_TYPE(Notify_ShowMessage, MessageParams, "window/showMessage")



/**
 * The show message request is sent from a server to a client to ask the client to display a particular message in the
 * user class. In addition to the show message notification the request allows to pass actions and to wait for an
 * answer from the client.
 */
struct MessageActionItem {
	/**
	 * A short title like 'Retry', 'Open Log' etc.
	 */

	std::string title;
	MAKE_SWAP_METHOD(MessageActionItem, title)
};
MAKE_REFLECT_STRUCT(MessageActionItem, title);


struct ShowMessageRequestParams :public MessageParams {
	/**
	 * The message action items to present.
	 */
	std::vector<MessageActionItem> actions;
	
	MAKE_SWAP_METHOD(ShowMessageRequestParams, type, message, actions)

};
MAKE_REFLECT_STRUCT(ShowMessageRequestParams, type, message, actions)

/**
* The show message request is sent from a server to a client to ask the
* client to display a particular message in the user interface. In addition
* to the show message notification the request allows to pass actions and
* to wait for an answer from the client.
*/

DEFINE_REQUEST_RESPONSE_TYPE(WindowShowMessage, ShowMessageRequestParams, MessageActionItem, "window/showMessage")



