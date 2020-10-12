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

#include "apr_general.h"
#include "apr_lib.h"
#include "apr_strings.h"
#include "apr_portable.h"
#include "apr_arch_thread_mutex.h"
#include "apr_arch_thread_cond.h"
#include "apr_arch_file_io.h"
#include <string.h>

#ifndef DCE_POSTONE
#define DCE_POSTONE   0x0800 // Post one flag
#endif

static apr_status_t thread_cond_cleanup(void *data)
{
    apr_thread_cond_t *cv = data;

    if (cv->semaphore) {
        DosCloseEventSem(cv->semaphore);
    }

    if (cv->mutex) {
        DosCloseMutexSem(cv->mutex);
    }

    return APR_SUCCESS;
}



APR_DECLARE(apr_status_t) apr_thread_cond_create(apr_thread_cond_t **cond,
                                                 apr_pool_t *pool)
{
    int rc;
    apr_thread_cond_t *cv;

    cv = apr_pcalloc(pool, sizeof(**cond));
    rc = DosCreateEventSem(NULL, &cv->semaphore, DCE_POSTONE, FALSE);

    if (rc == 0) {
        rc = DosCreateMutexSem(NULL, &cv->mutex, 0, FALSE);
    }

    *cond = cv;
    cv->pool = pool;
    apr_pool_cleanup_register(cv->pool, cv, thread_cond_cleanup,
                              apr_pool_cleanup_null);

    return APR_FROM_OS_ERROR(rc);
}



static apr_status_t thread_cond_timedwait(apr_thread_cond_t *cond,
                                          apr_thread_mutex_t *mutex,
                                          ULONG timeout_ms )
{
    ULONG rc;
    apr_status_t rv = APR_SUCCESS;
    int wake = FALSE;
    unsigned long generation;

    DosRequestMutexSem(cond->mutex, SEM_INDEFINITE_WAIT);
    cond->num_waiting++;
    generation = cond->generation;
    DosReleaseMutexSem(cond->mutex);

    apr_thread_mutex_unlock(mutex);

    do {
        rc = DosWaitEventSem(cond->semaphore, timeout_ms);

        DosRequestMutexSem(cond->mutex, SEM_INDEFINITE_WAIT);

        if (cond->num_wake) {
            if (cond->generation != generation) {
                cond->num_wake--;
                cond->num_waiting--;
                rv = APR_SUCCESS;
                break;
            } else {
                wake = TRUE;
            }
        }
        else if (rc != 0) {
            cond->num_waiting--;
            rv = APR_TIMEUP;
            break;
        }

        DosReleaseMutexSem(cond->mutex);

        if (wake) {
            wake = FALSE;
            DosPostEventSem(cond->semaphore);
        }
    } while (1);

    DosReleaseMutexSem(cond->mutex);
    apr_thread_mutex_lock(mutex);
    return rv;
}



APR_DECLARE(apr_status_t) apr_thread_cond_wait(apr_thread_cond_t *cond,
                                               apr_thread_mutex_t *mutex)
{
    return thread_cond_timedwait(cond, mutex, SEM_INDEFINITE_WAIT);
}



APR_DECLARE(apr_status_t) apr_thread_cond_timedwait(apr_thread_cond_t *cond,
                                                    apr_thread_mutex_t *mutex,
                                                    apr_interval_time_t timeout)
{
    ULONG timeout_ms = (timeout >= 0) ? apr_time_as_msec(timeout)
                                      : SEM_INDEFINITE_WAIT;
    return thread_cond_timedwait(cond, mutex, timeout_ms);
}



APR_DECLARE(apr_status_t) apr_thread_cond_signal(apr_thread_cond_t *cond)
{
    int wake = FALSE;

    DosRequestMutexSem(cond->mutex, SEM_INDEFINITE_WAIT);

    if (cond->num_waiting > cond->num_wake) {
        wake = TRUE;
        cond->num_wake++;
        cond->generation++;
    }

    DosReleaseMutexSem(cond->mutex);

    if (wake) {
        DosPostEventSem(cond->semaphore);
    }

    return APR_SUCCESS;
}



APR_DECLARE(apr_status_t) apr_thread_cond_broadcast(apr_thread_cond_t *cond)
{
    unsigned long num_wake = 0;

    DosRequestMutexSem(cond->mutex, SEM_INDEFINITE_WAIT);

    if (cond->num_waiting > cond->num_wake) {
        num_wake = cond->num_waiting - cond->num_wake;
        cond->num_wake = cond->num_waiting;
        cond->generation++;
    }

    DosReleaseMutexSem(cond->mutex);

    for (; num_wake; num_wake--) {
        DosPostEventSem(cond->semaphore);
    }

    return APR_SUCCESS;
}



APR_DECLARE(apr_status_t) apr_thread_cond_destroy(apr_thread_cond_t *cond)
{
    return apr_pool_cleanup_run(cond->pool, cond, thread_cond_cleanup);
}



APR_POOL_IMPLEMENT_ACCESSOR(thread_cond)
