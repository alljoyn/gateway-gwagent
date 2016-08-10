/******************************************************************************
 *    Copyright (c) Open Connectivity Foundation (OCF) and AllJoyn Open
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
#include <alljoyn/PasswordManager.h>
#include <alljoyn/AboutData.h>
#include <alljoyn/AboutObj.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/Init.h>
#include <alljoyn/gateway/GatewayMgmt.h>
#include <alljoyn/gateway/GatewayBusListener.h>
#include <alljoyn/gateway/common/AJInitializer.h>
#include <alljoyn/gateway/common/SrpKeyXListener.h>
#include <alljoyn/services_common/GuidUtil.h>
#include <string.h>
#include <signal.h>
#include <fstream>

#include "../GatewayConstants.h"
#include "GatewayMgmtAppConfigListener.h"

using namespace ajn;
using namespace gw;
using namespace qcc;

#define SERVICE_PORT 900

GatewayMgmt* gatewayMgmt = NULL;
BusAttachment* bus = NULL;
AboutData* aboutData = NULL;
GatewayBusListener*  busListener = NULL;
common::SrpKeyXListener* keyListener = NULL;
GatewayMgmtAppConfig* appConfig = NULL;
static volatile sig_atomic_t s_interrupt = false;
static volatile sig_atomic_t s_restart = false;

static void DaemonDisconnectHandler()
{
    s_restart = true;
}

void WaitForSigInt(void) {
    while (s_interrupt == false && s_restart == false) {
        usleep(100 * 1000);
    }
}

QStatus prepareBusAttachment()
{
    bus = new BusAttachment("GatewayMgmtApp", true);

    QStatus status = bus->Start();
    if (status != ER_OK) {
        return status;
    }

    uint16_t retry = 0;
    do {
        status = bus->Connect();
        if (status != ER_OK) {
            std::cout << "Could not connect BusAttachment. Retrying" << std::endl;
            sleep(1);
            retry++;
        }
    } while (status != ER_OK && retry != 180 && !s_interrupt);

    return status;
}

QStatus prepareBusListener()
{
    if (!bus || !busListener) {
        return ER_FAIL;
    }

    busListener->setSessionPort(SERVICE_PORT);
    bus->RegisterBusListener(*busListener);

    TransportMask transportMask = TRANSPORT_ANY;
    SessionPort sp = SERVICE_PORT;
    SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, transportMask);

    QStatus status = bus->BindSessionPort(sp, opts, *busListener);
    if (status != ER_OK) {
        return status;
    }

    return status;
}

QStatus fillAboutData()
{
    using namespace gwConsts;

    if (!aboutData) {
        return ER_BAD_ARG_1;
    }

    QStatus status = ER_OK;

    qcc::String deviceId;
    ajn::services::GuidUtil::GetInstance()->GetDeviceIdString(&deviceId);
    qcc::String appId;
    std::fstream ifs(GATEWAY_APPID_FILE_PATH.c_str(), std::fstream::in);
    if (ifs) {
        std::string tmpStr;
        if (ifs.peek() != std::fstream::traits_type::eof()) {
            std::getline(ifs, tmpStr);
            appId = tmpStr.c_str();
        }
    } else {
        QCC_DbgPrintf(("AppId file does not exists. A new AppId will be created.\n"));
        ifs.open(GATEWAY_APPID_FILE_PATH.c_str(), std::fstream::out | std::fstream::app);

        ajn::services::GuidUtil::GetInstance()->GenerateGUID(&appId);

        ifs << appId.c_str();
        ifs.flush();
    }

    ifs.close();

    const char* language = appConfig->getLanguage().c_str();

    status = aboutData->SetDeviceId(deviceId.c_str());
    if (status != ER_OK) {
        return status;
    }
    status = aboutData->SetDeviceName(appConfig->getDeviceName().c_str(), language);
    if (status != ER_OK) {
        return status;
    }
    status = aboutData->SetAppId(appId.c_str());
    if (status != ER_OK) {
        return status;
    }
    status = aboutData->SetAppName(appConfig->getAppName().c_str(), "en");
    if (status != ER_OK) {
        return status;
    }
    status = aboutData->SetDefaultLanguage(appConfig->getLanguage().c_str());
    if (status != ER_OK) {
        return status;
    }
    status = aboutData->SetSupportUrl(language);
    if (status != ER_OK) {
        return status;
    }
    status = aboutData->SetManufacturer(appConfig->getManufacturer().c_str(), language);
    if (status != ER_OK) {
        return status;
    }
    status = aboutData->SetModelNumber(appConfig->getModelNumber().c_str());
    if (status != ER_OK) {
        return status;
    }
    status = aboutData->SetSoftwareVersion(appConfig->getSoftwareVersion().c_str());
    if (status != ER_OK) {
        return status;
    }
    status = aboutData->SetDescription(appConfig->getDescription().c_str(), language);
    if (status != ER_OK) {
        return status;
    }
    return status;
}

void cleanup()
{
    if (gatewayMgmt) {
        gatewayMgmt->shutdownGatewayMgmt();
        gatewayMgmt = NULL;
    }
    if (bus) {
        bus->CancelAdvertiseName(GW_WELLKNOWN_NAME, TRANSPORT_ANY);
        bus->ReleaseName(GW_WELLKNOWN_NAME);

        if (busListener) {
            bus->UnregisterBusListener(*busListener);
            bus->UnbindSessionPort(busListener->getSessionPort());
        }
    }
    if (busListener) {
        delete busListener;
        busListener = NULL;
    }
    if (aboutData) {
        delete aboutData;
        aboutData = NULL;
    }
    if (keyListener) {
        delete keyListener;
        keyListener = NULL;
    }
    if (bus) {
        delete bus;
        bus = NULL;
    }
    if (appConfig) {
        delete appConfig;
        appConfig = NULL;
    }
}

void signal_callback_handler(int32_t signum)
{
    if (signum == SIGCHLD) {
        signal(SIGCHLD, signal_callback_handler); //reset signal handler
        GatewayMgmt::sigChildCallback(signum);
    } else {
        s_interrupt = true;
    }
}
qcc::String policyFileOption = "--gwagent-policy-file=";
qcc::String appsPolicyDirOption = "--apps-policy-dir=";
qcc::String routingNodeConfigFileOption = "--config-file=";
qcc::String gwMgmtAppConfigPathOption = "--gwagent-config-file=";

int main(int argc, char** argv)
{
    qcc::String configPath;

    common::AJInitializer ajInit;
    if (ajInit.Status() != ER_OK) {
        return 1;
    }

    // Allow CTRL+C to end application
    signal(SIGINT, signal_callback_handler);
    signal(SIGTERM, signal_callback_handler);
    signal(SIGCHLD, signal_callback_handler);

start:

    // Initialize GatewayMgmt object
    gatewayMgmt = GatewayMgmt::getInstance();
    // Initialize GatewayMgmtAppConfig object
    appConfig = new GatewayMgmtAppConfig;
    qcc::String gwMgmtAppConfig = gwConsts::GATEWAY_DEFAULT_MGMT_APP_CONF_PATH;
    for (int i = 1; i < argc; i++) {
        qcc::String arg(argv[i]);
        if (arg.compare(0, policyFileOption.size(), policyFileOption) == 0) {
            qcc::String policyFile = arg.substr(policyFileOption.size());
            QCC_DbgPrintf(("Setting gatewayPolicyFile to: %s", policyFile.c_str()));
            gatewayMgmt->setGatewayPolicyFile(policyFile.c_str());
        }
        if (arg.compare(0, appsPolicyDirOption.size(), appsPolicyDirOption) == 0) {
            qcc::String policyDir = arg.substr(appsPolicyDirOption.size());
            QCC_DbgPrintf(("Setting appsPolicyDir to: %s", policyDir.c_str()));
            gatewayMgmt->setAppPolicyDir(policyDir.c_str());
        }
        if (arg.compare(0, gwMgmtAppConfigPathOption.size(), gwMgmtAppConfigPathOption) == 0) {
            gwMgmtAppConfig = arg.substr(gwMgmtAppConfigPathOption.size());
            QCC_DbgPrintf(("Setting gwMgmtAppConfig to: %s", gwMgmtAppConfig.c_str()));
        }
    }

    appConfig->loadFromFile(gwMgmtAppConfig);

    QStatus status = prepareBusAttachment();
    if (status != ER_OK) {
        QCC_LogError(status, ("Could not initialize BusAttachment."));
        cleanup();
        return 1;
    }

    keyListener = new common::SrpKeyXListener();
    keyListener->setPassCode(appConfig->getAlljoynPasscode());
    status = bus->EnablePeerSecurity("ALLJOYN_SRP_KEYX ALLJOYN_ECDHE_PSK", keyListener);
    if (status != ER_OK) {
        QCC_LogError(status, ("Could not enable PeerSecurity"));
        cleanup();
        return 1;
    }

    aboutData = new AboutData("en");
    status = fillAboutData();
    if (status != ER_OK) {
        QCC_LogError(status, ("Could not fill AboutData."));
        cleanup();
        return 1;
    }

    busListener = new GatewayBusListener(bus, DaemonDisconnectHandler);
    status = prepareBusListener();
    if (status != ER_OK) {
        QCC_LogError(status, ("Could not set up the BusListener."));
        cleanup();
        return 1;
    }

    GatewayMgmtAppConfigListener configServiceListener(
        keyListener,
        bus,
        busListener,
        appConfig
        );

    GatewayMgmtAppDataStore configDataStore(NULL, NULL);

    ajn::services::ConfigService configService(*bus, configDataStore, configServiceListener);

    status = configService.Register();
    if (status != ER_OK) {
        QCC_LogError(status, ("Could not register the ConfigService"));
        cleanup();
        return 1;
    }

    status = bus->RegisterBusObject(configService);
    if (status != ER_OK) {
        QCC_LogError(status, ("Could not register the ConfigService BusObject"));
        cleanup();
        return 1;
    }

    const uint32_t flags = DBUS_NAME_FLAG_REPLACE_EXISTING | DBUS_NAME_FLAG_DO_NOT_QUEUE;
    status = bus->RequestName(GW_WELLKNOWN_NAME, flags);
    if (status != ER_OK) {
        QCC_LogError(status, ("Could not request Wellknown name"));
        cleanup();
        return 1;
    }

    status = bus->AdvertiseName(GW_WELLKNOWN_NAME, TRANSPORT_ANY);
    if (status != ER_OK) {
        QCC_LogError(status, ("Could not advertise Wellknown name"));
        cleanup();
        return 1;
    }

    status = bus->AdvertiseName(bus->GetUniqueName().c_str(), TRANSPORT_ANY);
    if (status != ER_OK) {
        QCC_LogError(status, ("Could not advertise Unique name"));
        cleanup();
        return 1;
    }

    status = gatewayMgmt->initGatewayMgmt(bus);
    if (status != ER_OK) {
        QCC_LogError(status, ("Could not initialize Gateway App - exiting application"));
        cleanup();
        return 1;
    }

    AboutObj* aboutObj = new AboutObj(*bus);
    status = aboutObj->Announce(SERVICE_PORT, *aboutData);
    if (status != ER_OK) {
        QCC_LogError(status, ("Could not announce."));
        cleanup();
        return 1;
    }

    QCC_DbgPrintf(("Finished initializing Gateway App"));

    WaitForSigInt();

    delete aboutObj;

    cleanup();
    if (s_restart) {
        s_restart = false;
        goto start;
    }

    return 0;
}