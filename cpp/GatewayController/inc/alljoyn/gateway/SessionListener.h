/******************************************************************************
 * Copyright (c) 2016 Open Connectivity Foundation (OCF) and AllJoyn Open
 *    Source Project (AJOSP) Contributors and others.
 *
 *    SPDX-License-Identifier: Apache-2.0
 *
 *    All rights reserved. This program and the accompanying materials are
 *    made available under the terms of the Apache License, Version 2.0
 *    which accompanies this distribution, and is available at
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Copyright 2016 Open Connectivity Foundation and Contributors to
 *    AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for
 *    any purpose with or without fee is hereby granted, provided that the
 *    above copyright notice and this permission notice appear in all
 *    copies.
 *
 *     THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 *     WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 *     WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 *     AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 *     DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 *     PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 *     TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 *     PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

#ifndef SessionListener_H
#define SessionListener_H

#include <alljoyn/SessionListener.h>
#include <alljoyn/SessionPortListener.h>

namespace ajn {
namespace gwc {

class GatewayMgmtApp;

/**
 *  This class is responsible for handling session related events from the AllJoyn system.
 *  Extend this class to receive the events of:
 *      - sessionEstablished
 *      - sessionLost
 *
 *  The events are called on the AllJoyn thread, so avoid blocking them with
 *  long running tasks.
 */
class SessionListener {
  public:

    /**
     * Constructor for SessionListener
     */
    SessionListener() { };

    /**
     * Destructor for SessionListener
     */
    virtual ~SessionListener() { };

    /**
     * sessionEstablished - callback when a session is established with a device
     * @param gatewayMgmtApp - the gateway that the session was established with
     */
    virtual void sessionEstablished(GatewayMgmtApp* gatewayMgmtApp) = 0;

    /**
     * sessionLost - callback when a session is lost with a device
     * @param gatewayMgmtApp - the gateway that the session was lost with
     */
    virtual void sessionLost(GatewayMgmtApp* gatewayMgmtApp) = 0;
};
}
}
#endif /* defined(SessionListener_H) */