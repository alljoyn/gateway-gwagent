/******************************************************************************
 *  * Copyright (c) Open Connectivity Foundation (OCF) and AllJoyn Open
 *    Source Project (AJOSP) Contributors and others.
 *
 *    SPDX-License-Identifier: Apache-2.0
 *
 *    All rights reserved. This program and the accompanying materials are
 *    made available under the terms of the Apache License, Version 2.0
 *    which accompanies this distribution, and is available at
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Copyright (c) Open Connectivity Foundation and Contributors to AllSeen
 *    Alliance. All rights reserved.
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

#include <alljoyn/gateway/GatewayController.h>
#include "Constants.h"
#include <qcc/Debug.h>

namespace ajn {
namespace gwc {

using namespace gwcConsts;

GatewayController* GatewayController::m_instance = NULL;
BusAttachment* GatewayController::m_Bus = NULL;

GatewayController::GatewayController()
{

}

GatewayController*GatewayController::getInstance()
{
    if (!m_instance) {
        m_instance = new GatewayController();
    }
    return m_instance;
}

void GatewayController::init(BusAttachment*bus)
{
    m_Bus = bus;
}

void GatewayController::shutdown()
{
    release();

    if (m_instance) {
        delete m_instance;
        m_instance = NULL;
    }
}


BusAttachment* GatewayController::getBusAttachment()
{
    return m_Bus;
}


QStatus GatewayController::createGateway(const qcc::String& gatewayBusName, const ajn::AboutObjectDescription& objectDescs, const ajn::AboutData& aboutData, GatewayMgmtApp** gatewayMgmtApp)
{
    if (objectDescs.HasInterface(AJ_OBJECTPATH_PREFIX.c_str(), AJ_GATEWAYCONTROLLER_APPMGMT_INTERFACE.c_str()) == true) {
        *gatewayMgmtApp = new GatewayMgmtApp();
        QStatus status = (*gatewayMgmtApp)->init(gatewayBusName, aboutData);

        if (status != ER_OK) {
            QCC_LogError(status, ("GatewayMgmtApp init failed"));
            delete gatewayMgmtApp;
            return status;
        }
    }

    if (gatewayMgmtApp) {
        m_Gateways[gatewayBusName] = *gatewayMgmtApp;
    } else {
        return ER_FAIL;
    }

    return ER_OK;
}

GatewayMgmtApp* GatewayController::getGateway(const qcc::String& gatewayBusName)
{
    std::map<qcc::String, GatewayMgmtApp*>::const_iterator gateway = m_Gateways.find(gatewayBusName);

    if (gateway != m_Gateways.end()) {
        return gateway->second;
    }
    return NULL;
}

const std::map<qcc::String, GatewayMgmtApp*>& GatewayController::getGateways() const
{
    return m_Gateways;
}

void GatewayController::emptyMap()
{
    while (!m_Gateways.empty()) {
        std::map<qcc::String, GatewayMgmtApp*>::iterator itr = m_Gateways.begin();
        GatewayMgmtApp*gateway = (*itr).second;
        m_Gateways.erase(itr);
        gateway->release();
        delete gateway;
    }
}


QStatus GatewayController::release()
{
    emptyMap();

    // static member variables are being taken care of in ShutDown

    return ER_OK;

}
}
}