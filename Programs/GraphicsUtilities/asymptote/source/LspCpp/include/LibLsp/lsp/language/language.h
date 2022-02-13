#pragma once

#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/NotificationInMessage.h"

#include "LibLsp/lsp/lsDocumentUri.h"
#include "LibLsp/lsp/lsCommand.h"

#ifdef _WIN32
#include <ppltasks.h>
#endif

struct  StatusReport {
	
	std::string ToString() const
	{
		std::string info;
		info += "type:" + type + "\n";
		info += "message:" + message + "\n";
		return info;
	}
	/**
	 * The message type. See {
	 *
	 */

	std::string  type;
	/**
	 * The actual message
	 *
	 */

	std::string  message;
	MAKE_SWAP_METHOD(StatusReport, type, message);
};
MAKE_REFLECT_STRUCT(StatusReport, type, message);

/**
 * The show message notification is sent from a server to a client to ask
 * the client to display a particular message in the user interface.
 */
DEFINE_NOTIFICATION_TYPE(lang_status, StatusReport, "language/status");


enum class MessageType {

	/**
	 * An error message.
	 */
	Error=(1),

	/**
	 * A warning message.
	 */
	 Warning=(2),

	 /**
	  * An information message.
	  */
	  Info=(3),

	  /**
	   * A log message.
	   */
	   Log=(4)
};
MAKE_REFLECT_TYPE_PROXY(MessageType);


struct ActionableNotification {


	
	/**
	 * The message type. See {
	 *
	 */

	 MessageType severity;
	/**
	 * The actual message
	 *
	 */

	 std::string message;

	/**
	 * Optional data
	 *
	 */

	boost::optional<lsp::Any> data;


	/**
	 * Optional commands
	 *
	 */

	 std::vector<lsCommandWithAny> commands;

	 MAKE_SWAP_METHOD(ActionableNotification, severity, message, data, commands)
};
MAKE_REFLECT_STRUCT(ActionableNotification, severity, message, data, commands)


/**
 * The actionable notification is sent from a server to a client to ask the
 * client to display a particular message in the user interface, and possible
 * commands to execute. The commands must be implemented on the client side.
 */
DEFINE_NOTIFICATION_TYPE(lang_actionableNotification, ActionableNotification, "language/actionableNotification");



struct  ProgressReport {
	std::string ToString() const;

	std::string id;


	std::string task;


	std::string subTask;


	std::string status;

	 int totalWork = 0;


	 int workDone = 0;


	 bool complete = false;
	 MAKE_SWAP_METHOD(ProgressReport, id, task, subTask, status, workDone, complete);
};


MAKE_REFLECT_STRUCT(ProgressReport, id, task, subTask, status, workDone, complete);

/**
 * The progress report notification is sent from a server to be handled by the
 * client.
 */
DEFINE_NOTIFICATION_TYPE(lang_progressReport, ProgressReport, "language/progressReport");

enum EventType {
	/**
	 * classpath updated event.
	 */
	ClasspathUpdated = (100),

	/**
	 * projects imported event.
	 */
	 ProjectsImported = (200)
};

struct EventNotification
{
	int eventType;
	lsp::Any data;
	std::string ToString() const;
	MAKE_SWAP_METHOD(EventNotification, eventType, data)
};
MAKE_REFLECT_STRUCT(EventNotification, eventType, data);

DEFINE_NOTIFICATION_TYPE(lang_eventNotification, EventNotification, "language/eventNotification");