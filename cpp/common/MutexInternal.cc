/**
 * @file
 *
 * Internal functionality of the Mutex class.
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
#include <alljoyn/gateway/common/Mutex.h>
#include <alljoyn/gateway/common/MutexInternal.h>
#include <qcc/Debug.h>

#define QCC_MODULE "MUTEX"

using namespace qcc;
using namespace ajn::gw;

MutexInternal::MutexInternal(Mutex* ownerLock, LockLevel level)
{
#ifdef NDEBUG
    QCC_UNUSED(ownerLock);
    QCC_UNUSED(level);
#else
    m_file = LockOrderChecker::s_unknownFile;
    m_line = LockOrderChecker::s_unknownLineNumber;
    m_ownerThread = 0;
    m_recursionCount = 0;
    m_level = level;
    m_ownerLock = ownerLock;
#endif

    m_initialized = PlatformSpecificInit();
    QCC_ASSERT(m_initialized);
}

MutexInternal::~MutexInternal()
{
    if (m_initialized) {
        PlatformSpecificDestroy();
        m_initialized = false;
    }
}

QStatus MutexInternal::Lock(const char* file, uint32_t line)
{
    QCC_ASSERT(m_initialized);

#ifdef NDEBUG
    QCC_UNUSED(file);
    QCC_UNUSED(line);
    return Lock();
#else
    if (!m_initialized) {
        return ER_INIT_FAILED;
    }

    /*
     * Duplicate the Lock() functionality in Lock(file, line), rather than
     * calling Lock() here because the Lock() path gets executed by Release
     * code too. Adding support for the file & line parameters to that
     * Release code path could have inflicted a small perf overhead.
     */
    AcquiringLock(file, line);
    QStatus status = PlatformSpecificLock();
    if (status == ER_OK) {
        QCC_DbgPrintf(("Lock Acquired %s:%d", file, line));
        m_file = file;
        m_line = line;
        LockAcquired();
    } else {
        QCC_LogError(status, ("Mutex::Lock %s:%u failed", file, line));
    }
    return status;
#endif
}

QStatus MutexInternal::Lock()
{
    QCC_ASSERT(m_initialized);
    if (!m_initialized) {
        return ER_INIT_FAILED;
    }

    AcquiringLock();
    QStatus status = PlatformSpecificLock();
    if (status == ER_OK) {
        LockAcquired();
    }

    return status;
}

QStatus MutexInternal::Unlock(const char* file, uint32_t line)
{
    QCC_ASSERT(m_initialized);

#ifdef NDEBUG
    QCC_UNUSED(file);
    QCC_UNUSED(line);
#else
    QCC_DbgPrintf(("Lock Released: %s:%u (acquired at %s:%u)", file, line, m_file, m_line));
    m_file = file;
    m_line = line;
#endif

    return Unlock();
}

QStatus MutexInternal::Unlock()
{
    QCC_ASSERT(m_initialized);
    if (!m_initialized) {
        return ER_INIT_FAILED;
    }

    ReleasingLock();
    return PlatformSpecificUnlock();
}

bool MutexInternal::TryLock()
{
    QCC_ASSERT(m_initialized);
    if (!m_initialized) {
        return false;
    }

    /*
     * Calling AcquiringLock here means that the SCL internal locks that opted-in
     * for lock order verification do NOT support the following pattern of using Mutex:
     *
     * Thread1:
     *      a.Lock();
     *      b.Lock();
     *
     * Thread2:
     *      b.Lock();
     *      if(!a.TryLock()) {
     *          b.Unlock();
     *          a.Lock();
     *      }
     *
     * The a.TryLock() call above gets flagged as an out-of-order acquire. If the SCL
     * will ever need to support that pattern, AcquiringLock will need to be changed
     * to support it. Currently there are no TryLock calls in the SCL.
     */
    AcquiringLock();
    bool locked = PlatformSpecificTryLock();
    if (locked) {
        LockAcquired();
    }
    return locked;
}

/**
 * Called immediately before current thread tries to acquire this Mutex, on Release
 * builds, or if the caller did not specify the MUTEX_CONTEXT parameter.
 */
void MutexInternal::AcquiringLock()
{
#ifndef NDEBUG
    return AcquiringLock(LockOrderChecker::s_unknownFile, LockOrderChecker::s_unknownLineNumber);
#endif
}

#ifndef NDEBUG
/**
 * Called immediately before current thread tries to acquire this Mutex.
 */
void MutexInternal::AcquiringLock(const char* file, uint32_t line)
{
    /*
     * Perform lock order verification. Test LOCK_LEVEL_CHECKING_DISABLED before calling
     * GetThread, because GetThread uses a LOCK_LEVEL_CHECKING_DISABLED mutex internally.
     */
    if (Thread::initialized && m_level != LOCK_LEVEL_CHECKING_DISABLED) {
        Thread::GetThread()->lockChecker.AcquiringLock(m_ownerLock, file, line);
    }
}
#endif

/**
 * Called immediately after current thread acquired this Mutex.
 */
void MutexInternal::LockAcquired()
{
#ifndef NDEBUG
    /* Use GetCurrentThreadId rather than GetThread, because GetThread acquires a Mutex */
    ThreadId currentThread = Thread::GetCurrentThreadId();
    QCC_ASSERT(currentThread != 0);

    if (m_ownerThread == currentThread) {
        QCC_ASSERT(m_recursionCount != 0);
    } else {
        QCC_ASSERT(m_ownerThread == 0);
        QCC_ASSERT(m_recursionCount == 0);
        m_ownerThread = currentThread;
    }

    m_recursionCount++;

    /*
     * Perform lock order verification. Test LOCK_LEVEL_CHECKING_DISABLED before calling
     * GetThread, because GetThread uses a LOCK_LEVEL_CHECKING_DISABLED mutex internally.
     */
    if (Thread::initialized && m_level != LOCK_LEVEL_CHECKING_DISABLED) {
        Thread::GetThread()->lockChecker.LockAcquired(m_ownerLock);
    }
#endif
}

void MutexInternal::LockAcquired(Mutex& lock)
{
    lock.m_mutexInternal->LockAcquired();
}

/**
 * Called immediately before current thread releases this Mutex.
 */
void MutexInternal::ReleasingLock()
{
#ifndef NDEBUG
    /* Use GetCurrentThreadId rather than GetThread, because GetThread acquires a Mutex */
    ThreadId currentThread = Thread::GetCurrentThreadId();
    QCC_ASSERT(currentThread != 0);
    QCC_ASSERT(m_ownerThread == currentThread);
    QCC_ASSERT(m_recursionCount != 0);

    m_recursionCount--;

    if (m_recursionCount == 0) {
        m_ownerThread = 0;
    }

    /*
     * Perform lock order verification. Test LOCK_LEVEL_CHECKING_DISABLED before calling
     * GetThread, because GetThread uses a LOCK_LEVEL_CHECKING_DISABLED mutex internally.
     */
    if (Thread::initialized && m_level != LOCK_LEVEL_CHECKING_DISABLED) {
        Thread::GetThread()->lockChecker.ReleasingLock(m_ownerLock);
    }
#endif
}

void MutexInternal::ReleasingLock(Mutex& lock)
{
    lock.m_mutexInternal->ReleasingLock();
}

/**
 * Assert that current thread owns this Mutex.
 */
void MutexInternal::AssertOwnedByCurrentThread() const
{
#ifndef NDEBUG
    /* Use GetCurrentThreadId rather than GetThread, because GetThread acquires a Mutex */
    ThreadId currentThread = Thread::GetCurrentThreadId();
    QCC_ASSERT(currentThread != 0);
    QCC_ASSERT(m_ownerThread == currentThread);
    QCC_ASSERT(m_recursionCount != 0);
#endif
}

/**
 * Set the level value for locks that cannot get a proper level from their constructor -
 * because an entire array of locks has been allocated (e.g. locks = new Mutex[numLocks];).
 */
#ifndef NDEBUG
void MutexInternal::SetLevel(Mutex& lock, LockLevel level)
{
    QCC_ASSERT(lock.m_mutexInternal->m_level == LOCK_LEVEL_NOT_SPECIFIED);
    QCC_ASSERT(level != LOCK_LEVEL_NOT_SPECIFIED);
    lock.m_mutexInternal->m_level = level;
}
#endif
