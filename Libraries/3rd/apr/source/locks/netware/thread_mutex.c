/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "apr.h"
#include "apr_private.h"
#include "apr_general.h"
#include "apr_strings.h"
#include "apr_arch_thread_mutex.h"
#include "apr_thread_cond.h"
#include "apr_portable.h"

static apr_status_t thread_mutex_cleanup(void *data)
{
    apr_thread_mutex_t *mutex = (apr_thread_mutex_t *)data;

    NXMutexFree(mutex->mutex);        
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_mutex_create(apr_thread_mutex_t **mutex,
                                                  unsigned int flags,
                                                  apr_pool_t *pool)
{
    apr_thread_mutex_t *new_mutex = NULL;

    /* XXX: Implement _UNNESTED flavor and favor _DEFAULT for performance
     */
    if (flags & APR_THREAD_MUTEX_UNNESTED) {
        return APR_ENOTIMPL;
    }
    new_mutex = (apr_thread_mutex_t *)apr_pcalloc(pool, sizeof(apr_thread_mutex_t));

    if (new_mutex == NULL) {
        return APR_ENOMEM;
    }     
    new_mutex->pool = pool;

    new_mutex->mutex = NXMutexAlloc(NX_MUTEX_RECURSIVE, 0, NULL);
    
    if(new_mutex->mutex == NULL)
        return APR_ENOMEM;

    if (flags & APR_THREAD_MUTEX_TIMED) {
        apr_status_t rv = apr_thread_cond_create(&new_mutex->cond, pool);
        if (rv != APR_SUCCESS) {
            NXMutexFree(new_mutex->mutex);        
            return rv;
        }
    }

    apr_pool_cleanup_register(new_mutex->pool, new_mutex, 
                                (void*)thread_mutex_cleanup,
                                apr_pool_cleanup_null);
   *mutex = new_mutex;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_mutex_lock(apr_thread_mutex_t *mutex)
{
    if (mutex->cond) {
        apr_status_t rv;
        NXLock(mutex->mutex);
        if (mutex->locked) {
            mutex->num_waiters++;
            rv = apr_thread_cond_wait(mutex->cond, mutex);
            mutex->num_waiters--;
        }
        else {
            mutex->locked = 1;
            rv = APR_SUCCESS;
        }
        NXUnlock(mutex->mutex);
        return rv;
    }

    NXLock(mutex->mutex);
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_mutex_trylock(apr_thread_mutex_t *mutex)
{
    if (mutex->cond) {
        apr_status_t rv;
        NXLock(mutex->mutex);
        if (mutex->locked) {
            rv = APR_EBUSY;
        }
        else {
            mutex->locked = 1;
            rv = APR_SUCCESS;
        }
        NXUnlock(mutex->mutex);
        return rv;
    }

    if (!NXTryLock(mutex->mutex))
        return APR_EBUSY;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_mutex_timedlock(apr_thread_mutex_t *mutex,
                                                 apr_interval_time_t timeout)
{
    if (mutex->cond) {
        apr_status_t rv = APR_SUCCESS;

        NXLock(mutex->mutex);
        if (mutex->locked) {
            if (timeout <= 0) {
                rv = APR_TIMEUP;
            }
            else {
                mutex->num_waiters++;
                do {
                    rv = apr_thread_cond_timedwait(mutex->cond, mutex,
                                                   timeout);
                } while (rv == APR_SUCCESS && mutex->locked);
                mutex->num_waiters--;
            }
        }
        if (rv == APR_SUCCESS) {
            mutex->locked = 1;
        }
        NXUnlock(mutex->mutex);
        return rv;
    }

    return APR_ENOTIMPL;
}

APR_DECLARE(apr_status_t) apr_thread_mutex_unlock(apr_thread_mutex_t *mutex)
{
    apr_status_t rv = APR_SUCCESS;

    if (mutex->cond) {
        NXLock(mutex->mutex);

        if (!mutex->locked) {
            rv = APR_EINVAL;
        }
        else if (mutex->num_waiters) {
            rv = apr_thread_cond_signal(mutex->cond);
        }
        if (rv == APR_SUCCESS) {
            mutex->locked = 0;
        }
    }

    NXUnlock(mutex->mutex);
    return rv;
}

APR_DECLARE(apr_status_t) apr_thread_mutex_destroy(apr_thread_mutex_t *mutex)
{
    apr_status_t stat, rv = APR_SUCCESS;
    if (mutex->cond) {
        rv = apr_thread_cond_destroy(mutex->cond);
        mutex->cond = NULL;
    }
    stat = apr_pool_cleanup_run(mutex->pool, mutex, thread_mutex_cleanup);
    if (stat == APR_SUCCESS && rv) {
        stat = rv;
    }
    return stat;
}

APR_POOL_IMPLEMENT_ACCESSOR(thread_mutex)

