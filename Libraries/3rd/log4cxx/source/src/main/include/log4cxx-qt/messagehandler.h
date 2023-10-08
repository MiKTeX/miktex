/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LOG4CXX_QT_MESSAGEHANDLER_H
#define LOG4CXX_QT_MESSAGEHANDLER_H

#include <QString>

#include "log4cxx/log4cxx.h"

namespace log4cxx
{
namespace qt
{

/**
 * The messageHandler function is a log4cxx replacement of the standard
 * Qt message handler.
 *
 * Use this function as follows:
 *   qInstallMessageHandler( log4cxx::qt::messageHandler );
 *
 * Note that similar to Qt, upon receipt of a fatal message this calls
 * std::abort().
 */
LOG4CXX_EXPORT
void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message);

} /* namespace qt */
} /* namespace log4cxx */

#endif /* LOG4CXX_QT_MESSAGEHANDLER_H */
