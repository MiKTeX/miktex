#pragma once

#include "LibLsp/JsonRpc/NotificationInMessage.h"
/**
 * A notification to ask the server to exit its process.
 */
DEFINE_NOTIFICATION_TYPE(Notify_Exit, optional<JsonNull>, "exit");
