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

#include "AJInitializer.h"
#include <alljoyn/Init.h>

AJInitializer::AJInitializer()
{
    m_Status = AllJoynInit();

#ifdef ROUTER
    if (m_Status == ER_OK) {
        m_Status = AllJoynRouterInit();
    }
#endif
}

/*AJInitializer::AJInitializer(qcc::String configPath) {

    m_Status = AllJoynInit();


#ifdef ROUTER
    if (m_Status == ER_OK) {
        if (!configPath.empty()) {
            m_Status = AllJoynRouterInitConfig(configPath);
        } else {
            m_Status = AllJoynRouterInit();
        }
    } 
#endif
}*/

AJInitializer::~AJInitializer()
{
#ifdef ROUTER
    AllJoynRouterShutdown();
#endif
    AllJoynShutdown();
}

QStatus AJInitializer::Status() const
{
    return m_Status;
}
