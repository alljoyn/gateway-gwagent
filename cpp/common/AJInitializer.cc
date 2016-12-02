/******************************************************************************
 *  * 
 *    Copyright (c) 2016 Open Connectivity Foundation and AllJoyn Open
 *    Source Project Contributors and others.
 *    
 *    All rights reserved. This program and the accompanying materials are
 *    made available under the terms of the Apache License, Version 2.0
 *    which accompanies this distribution, and is available at
 *    http://www.apache.org/licenses/LICENSE-2.0

 ******************************************************************************/

#include "AJInitializer.h"
#include <alljoyn/Init.h>

namespace ajn {
namespace gw {
namespace common {

AJInitializer::AJInitializer()
{
    m_Status = AllJoynInit();
#ifdef ROUTER
    if (m_Status == ER_OK) {
        m_Status = AllJoynRouterInit();
        if (m_Status != ER_OK) {
            AllJoynShutdown();
        }
    }
#endif
}

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

}
}
}