#pragma once


#include "LibLsp/JsonRpc/NotificationInMessage.h"
/**
 * The initialized notification is sent from the client to the server after
 * the client received the result of the initialize request but before the
 * client is sending any other request or notification to the server. The
 * server can use the initialized notification for example to dynamically
 * register capabilities.
 */
DEFINE_NOTIFICATION_TYPE(Notify_InitializedNotification, JsonNull, "initialized");

