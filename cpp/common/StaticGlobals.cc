/**
 * @file
 * File for holding global variables.
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
#include <alljoyn/gateway/common/StaticGlobals.h>

#ifdef CRYPTO_CNG
#include <qcc/CngCache.h>
#endif
#include <alljoyn/gateway/common/Logger.h>
#include <alljoyn/gateway/common/Thread.h>
#include <alljoyn/gateway/common/Util.h>
#include <alljoyn/gateway/common/PerfCounters.h>
#ifdef QCC_OS_GROUP_WINDOWS
#include <alljoyn/gateway/common/windows/utility.h>
#include <alljoyn/gateway/common/windows/NamedPipeWrapper.h>
#endif
#include <alljoyn/gateway/common/Crypto.h>
#include <alljoyn/gateway/common/DebugControl.h>

#define QCC_MODULE "STATICGLOBALS"

namespace ajn {
namespace gw {

/* Counters easily found from a debugger, incremented for frequent SCL actions */
volatile uint32_t s_PerfCounters[PERF_COUNTER_COUNT];

};
};
