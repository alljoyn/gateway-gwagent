/******************************************************************************
 * Copyright (c) 2014, AllSeen Alliance. All rights reserved.
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

#include <alljoyn/gateway/GatewayCtrlTPApplicationStatus.h>
#include <alljoyn/gateway/LogModule.h>

namespace ajn {
namespace services {

GatewayCtrlTPApplicationStatus::GatewayCtrlTPApplicationStatus(InstallStatus installStatus, qcc::String installDescription, ConnectionStatus connectionStatus, OperationalStatus operationalStatus)
    : m_InstallStatus((InstallStatus)installStatus), m_InstallDescription(installDescription), m_ConnectionStatus((ConnectionStatus)connectionStatus), m_OperationalStatus((OperationalStatus)operationalStatus)
{
}

GatewayCtrlTPApplicationStatus::GatewayCtrlTPApplicationStatus(const ajn::MsgArg* returnArgs)
{

    QStatus status = ER_OK;

    short installStatus;
    char*installDescription;
    short connectionStatus;
    short operationalStatus;

    status = returnArgs[0].Get("q", &installStatus);

    if (status != ER_OK) {
        QCC_LogError(status, ("Failed getting installStatus"));
        return;
    }

    status = returnArgs[1].Get("s", &installDescription);

    if (status != ER_OK) {
        QCC_LogError(status, ("Failed getting installDescription"));
        return;
    }
    status = returnArgs[2].Get("q", &connectionStatus);

    if (status != ER_OK) {
        QCC_LogError(status, ("Failed getting connectionStatus"));
        return;
    }
    status = returnArgs[3].Get("q", &operationalStatus);

    if (status != ER_OK) {
        QCC_LogError(status, ("Failed getting operationalStatus"));
        return;
    }

    m_InstallStatus = (InstallStatus)installStatus;
    m_InstallDescription = installDescription;
    m_ConnectionStatus = (ConnectionStatus)connectionStatus;
    m_OperationalStatus = (OperationalStatus)operationalStatus;
}

GatewayCtrlTPApplicationStatus::~GatewayCtrlTPApplicationStatus() {
}

InstallStatus GatewayCtrlTPApplicationStatus::GetInstallStatus()
{
    return m_InstallStatus;
}

qcc::String GatewayCtrlTPApplicationStatus::GetInstallDescriptions()
{
    return m_InstallDescription;
}

ConnectionStatus GatewayCtrlTPApplicationStatus::GetConnectionStatus()
{
    return m_ConnectionStatus;
}

OperationalStatus GatewayCtrlTPApplicationStatus::GetOperationalStatus()
{
    return m_OperationalStatus;
}
}
}