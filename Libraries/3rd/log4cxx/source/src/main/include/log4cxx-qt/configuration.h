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
#ifndef LOG4CXX_QT_CONFIGURATION_H
#define LOG4CXX_QT_CONFIGURATION_H

#include <QString>
#include <QVector>
#include <log4cxx/log4cxx.h>
#include <log4cxx/defaultconfigurator.h>

namespace log4cxx {
namespace qt {

class LOG4CXX_EXPORT Configuration {
private:
	Configuration();

	static log4cxx::spi::ConfigurationStatus tryLoadFile(const QString& filename);

public:
	/**
	 * Configure Log4cxx and watch the file for changes.  See also DefaultConfigurator::configureFromFile.
	 *
	 * @param directories
	 * @param filenames
	 * @return
	 */
	static std::tuple<log4cxx::spi::ConfigurationStatus,QString> configureFromFileAndWatch(const QVector<QString>& directories,
																						   const QVector<QString>& filenames);
};

} /* namespace qt */
} /* namespace log4cxx */

#endif /* LOG4CXX_QT_CONFIGURATION_H */
