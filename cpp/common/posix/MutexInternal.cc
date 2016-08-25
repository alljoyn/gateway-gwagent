/**
 * @file
 *
 * Define a class that abstracts Linux mutex's.
 */

/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/
#include <qcc/platform.h>
#include <alljoyn/gateway/common/MutexInternal.h>
#include <qcc/Debug.h>

#define QCC_MODULE "MUTEX"

using namespace ajn::gw;

bool MutexInternal::PlatformSpecificInit()
{
    bool success;
    pthread_mutexattr_t attr;
    // Can't use QCC_LogError() since it uses mutexes under the hood.
    QCC_VERIFY(success = (pthread_mutexattr_init(&attr) == 0));

    if (success) {
        // We want entities to be able to lock a mutex multiple times without deadlocking or reporting an error.
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        QCC_VERIFY(success = (pthread_mutex_init(&m_mutex, &attr) == 0));

        // Don't need the attribute once it has been assigned to a mutex.
        pthread_mutexattr_destroy(&attr);
    }

    return success;
}

void MutexInternal::PlatformSpecificDestroy()
{
    // Can't use QCC_LogError() since it uses mutexes under the hood.
    QCC_VERIFY(pthread_mutex_destroy(&m_mutex) == 0);
}

QStatus MutexInternal::PlatformSpecificLock()
{
    int ret = pthread_mutex_lock(&m_mutex);
    // Can't use QCC_LogError() since it uses mutexes under the hood.
    QCC_ASSERT(ret == 0);
    if (ret != 0) {
        return ER_OS_ERROR;
    }

    return ER_OK;
}

QStatus MutexInternal::PlatformSpecificUnlock()
{
    int ret = pthread_mutex_unlock(&m_mutex);
    // Can't use QCC_LogError() since it uses mutexes under the hood.
    QCC_ASSERT(ret == 0);
    if (ret != 0) {
        return ER_OS_ERROR;
    }
    return ER_OK;
}

bool MutexInternal::PlatformSpecificTryLock()
{
    return (pthread_mutex_trylock(&m_mutex) == 0);
}
