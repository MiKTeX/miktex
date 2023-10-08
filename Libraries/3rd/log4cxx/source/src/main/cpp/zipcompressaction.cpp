/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <log4cxx/rolling/zipcompressaction.h>
#include <apr_thread_proc.h>
#include <apr_strings.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/private/action_priv.h>
#include <log4cxx/helpers/loglog.h>

using namespace log4cxx;
using namespace log4cxx::rolling;
using namespace log4cxx::helpers;

#define priv static_cast<ZipCompressActionPrivate*>(m_priv.get())

struct ZipCompressAction::ZipCompressActionPrivate : public ActionPrivate
{
	ZipCompressActionPrivate( const File& toRename,
		const File& renameTo,
		bool deleteSource):
		source(toRename), destination(renameTo), deleteSource(deleteSource) {}

	const File source;
	const File destination;
	bool deleteSource;
	bool throwIOExceptionOnForkFailure = true;
};

IMPLEMENT_LOG4CXX_OBJECT(ZipCompressAction)

ZipCompressAction::ZipCompressAction(const File& src,
	const File& dest,
	bool del)
	: Action(std::make_unique<ZipCompressActionPrivate>(
			  src, dest, del))
{
}

bool ZipCompressAction::execute(log4cxx::helpers::Pool& p) const
{
	if (!priv->source.exists(p))
	{
		return false;
	}

	apr_pool_t* aprpool = p.getAPRPool();
	apr_procattr_t* attr;
	apr_status_t stat = apr_procattr_create(&attr, aprpool);

	if (stat != APR_SUCCESS)
	{
		throw IOException(stat);
	}

	stat = apr_procattr_io_set(attr, APR_NO_PIPE, APR_NO_PIPE, APR_FULL_BLOCK);

	if (stat != APR_SUCCESS)
	{
		throw IOException(stat);
	}

	stat = apr_procattr_cmdtype_set(attr, APR_PROGRAM_PATH);

	if (stat != APR_SUCCESS)
	{
		throw IOException(stat);
	}

	//
	// redirect the child's error stream to this processes' error stream
	//
	apr_file_t* child_err;
	stat = apr_file_open_stderr(&child_err, aprpool);

	if (stat == APR_SUCCESS)
	{
		stat =  apr_procattr_child_err_set(attr, child_err, NULL);

		if (stat != APR_SUCCESS)
		{
			throw IOException(stat);
		}
	}

	const char** args = (const char**)
		apr_palloc(aprpool, 5 * sizeof(*args));
	int i = 0;

	args[i++] = "zip";
	args[i++] = "-q";
	args[i++] = Transcoder::encode(priv->destination.getPath(), p);
	args[i++] = Transcoder::encode(priv->source.getPath(), p);
	args[i++] = NULL;

	if (priv->destination.exists(p))
	{
		priv->destination.deleteFile(p);
	}

	apr_proc_t pid;
	stat = apr_proc_create(&pid, "zip", args, NULL, attr, aprpool);

	if (stat != APR_SUCCESS && priv->throwIOExceptionOnForkFailure)
	{
		throw IOException(stat);
	}else if(stat != APR_SUCCESS && !priv->throwIOExceptionOnForkFailure)
	{
		/* If we fail here (to create the zip child process),
		 * skip the compression and consider the rotation to be
		 * otherwise successful. The caller has already rotated
		 * the log file (`source` here refers to the
		 * uncompressed, rotated path, and `destination` the
		 * same path with `.zip` appended). Remove the empty
		 * destination file and leave source as-is.
		 */
		LogLog::warn(LOG4CXX_STR("Failed to fork zip during log rotation; leaving log file uncompressed"));
		return true;
	}

	int exitCode;
	apr_proc_wait(&pid, &exitCode, NULL, APR_WAIT);

	if (exitCode != APR_SUCCESS)
	{
		throw IOException(exitCode);
	}

	if (priv->deleteSource)
	{
		priv->source.deleteFile(p);
	}

	return true;
}

void ZipCompressAction::setThrowIOExceptionOnForkFailure(bool throwIO){
	priv->throwIOExceptionOnForkFailure = throwIO;
}
