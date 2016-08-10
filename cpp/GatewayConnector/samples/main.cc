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

#include <alljoyn/gateway/common/AJInitializer.h>
#include <alljoyn/gateway/common/SrpKeyXListener.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/Init.h>
#include <alljoyn/AboutProxy.h>
#include <alljoyn/AboutIcon.h>
#include <alljoyn/AboutIconProxy.h>
#include <alljoyn/notification/NotificationReceiver.h>
#include <alljoyn/notification/NotificationService.h>
#include "alljoyn/gateway/GatewayConnector.h"
#include <alljoyn/PasswordManager.h>
#include <alljoyn/config/ConfigClient.h>
#include <alljoyn/services_common/GuidUtil.h>
#include <alljoyn/AboutObj.h>

#include <iomanip>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <set>
#include <algorithm>

using namespace ajn;
using namespace ajn::services;
using namespace ajn::gw;
using namespace ajn::gw::common;
using namespace std;

#define CHECK_RETURN(x) if ((status = x) != ER_OK) { return status; }

class ExitManager {
  public:
    ExitManager() : exiting(false), signum(0) { }
    ~ExitManager() { }

    bool isExiting() const
    {
        return exiting;
    }

    void setExiting(int32_t signum)
    {
        exiting = true;
        this->signum = signum;
    }

    int32_t getSignum() const
    {
        return signum;
    }

  private:
    bool exiting;
    int32_t signum;
};

ExitManager exitManager;


class CommonBusListener : public ajn::BusListener, public ajn::SessionPortListener, public ajn::SessionListener {
  public:
    CommonBusListener(ajn::BusAttachment* bus = NULL, void(*daemonDisconnectCB)() = NULL) : BusListener(), SessionPortListener(), m_SessionPort(0), m_Bus(bus), m_DaemonDisconnectCB(daemonDisconnectCB)
    {
    }

    ~CommonBusListener()
    {
    }

    bool AcceptSessionJoiner(ajn::SessionPort sessionPort, const char* joiner, const ajn::SessionOpts& opts)
    {
        QCC_UNUSED(joiner);
        QCC_UNUSED(opts);
        if (sessionPort != m_SessionPort) {
            return false;
        }

        std::cout << "Accepting JoinSessionRequest" << std::endl;
        return true;
    }
    void setSessionPort(ajn::SessionPort sessionPort)
    {
        m_SessionPort = sessionPort;
    }
    void SessionJoined(ajn::SessionPort sessionPort, ajn::SessionId id, const char* joiner)
    {
        QCC_UNUSED(sessionPort);
        QCC_UNUSED(joiner);
        std::cout << "Session has been joined successfully" << std::endl;
        if (m_Bus) {
            m_Bus->SetSessionListener(id, this);
        }
        m_SessionIds.push_back(id);
    }
    void SessionLost(ajn::SessionId sessionId, SessionLostReason reason)
    {
        QCC_UNUSED(reason);
        std::cout << "Session has been lost" << std::endl;
        std::vector<SessionId>::iterator it = std::find(m_SessionIds.begin(), m_SessionIds.end(), sessionId);
        if (it != m_SessionIds.end()) {
            m_SessionIds.erase(it);
        }
    }
    ajn::SessionPort getSessionPort()
    {
        return m_SessionPort;
    }
    const std::vector<ajn::SessionId>& getSessionIds() const
    {
        return m_SessionIds;
    }
    void BusDisconnected()
    {
        std::cout << "Bus has been disconnected" << std::endl;
        if (m_DaemonDisconnectCB) {
            m_DaemonDisconnectCB();
        }
    }

  private:
    ajn::SessionPort m_SessionPort;
    ajn::BusAttachment* m_Bus;
    std::vector<ajn::SessionId> m_SessionIds;
    void (*m_DaemonDisconnectCB)();

};


class ConfigSession : public BusAttachment::JoinSessionAsyncCB, public SessionListener {
  private:
    BusAttachment* bus;
    ConfigSession()
    {
        // Private to force use of the ctor with BusAttachment* parameter
    }
    void PrintAboutData(AboutData aboutData)
    {
        size_t count = aboutData.GetFields();

        const char** fields = new const char*[count];
        aboutData.GetFields(fields, count);

        for (size_t i = 0; i < count; ++i) {
            printf("Key: %s", fields[i]);
            cout << "Key name= " << fields[i];

            MsgArg* tmp;
            aboutData.GetField(fields[i], tmp, NULL);
            if (tmp->Signature() == "s") {
                const char* tmp_s;
                tmp->Get("s", &tmp_s);
                cout << "value= " << tmp_s << endl;
            } else if (tmp->Signature() == "as") {
                size_t las;
                MsgArg* as_arg;
                tmp->Get("as", &las, &as_arg);
                cout << " values: ";
                for (size_t j = 0; j < las; ++j) {
                    const char* tmp_s;
                    as_arg[j].Get("s", &tmp_s);
                    cout << tmp_s << " ";
                }
                cout << endl;
            } else if (tmp->Signature() == "ay") {
                size_t lay;
                uint8_t* pay;
                tmp->Get("ay", &lay, &pay);
                for (size_t j = 0; j < lay; ++j) {
                    cout << setw(2) << (unsigned int)pay[j];
                }
            }
            cout << nouppercase << dec << endl;
        }
        delete [] fields;
    }

  public:
    ConfigSession(BusAttachment* busAttachment) : bus(busAttachment) { }

    virtual void JoinSessionCB(QStatus status, SessionId sessionId, const SessionOpts& opts, void* context) {
        QCC_UNUSED(opts);

        static bool firstJoin = true;
        QStatus myStat;

        if (status != ER_OK) {
            cout << "Error joining session " <<  QCC_StatusText(status) << endl;
            free(context);
        } else {
            bus->EnableConcurrentCallbacks();
            if (firstJoin) {
                firstJoin = false;

                bool isIconInterface = false;
                bool isConfigInterface = false;
                AboutProxy aboutClient(*bus, (char*)context, sessionId);
                int ver = 0;

                MsgArg objArg;
                myStat = aboutClient.GetObjectDescription(objArg);
                AboutObjectDescription ObjectDescriptionsRefill(objArg);

                if (myStat != ER_OK) {
                    cout << "getObjectDescriptions: status=" << QCC_StatusText(myStat) << endl;
                } else {
                    if (ObjectDescriptionsRefill.HasInterface("/About/DeviceIcon",
                                                              "org.alljoyn.Icon") == true) {
                        isIconInterface = true;
                    }
                    if (ObjectDescriptionsRefill.HasInterface("/Config",
                                                              "org.alljoyn.Config") == true) {
                        isConfigInterface = true;
                    }
                }

                MsgArg aboutArg;
                myStat = aboutClient.GetAboutData("en", aboutArg);

                std::vector<qcc::String> supportedLanguages;
                if (myStat != ER_OK) {
                    cout << "getAboutData: status="  << QCC_StatusText(myStat) << endl;
                } else {
                    AboutData aboutDataRefill(aboutArg);
                    size_t lngsNum = aboutDataRefill.GetSupportedLanguages();
                    char** languages = new char*[lngsNum];
                    for (size_t i = 0; i < lngsNum; i++) {
                        supportedLanguages.push_back(languages[i]);
                    }
                }

                for (std::vector<qcc::String>::iterator it = supportedLanguages.begin(); it != supportedLanguages.end();
                     ++it) {
                    cout << endl << (char*)context << " AboutClient AboutData using language=" << it->c_str() << endl;
                    cout << "-----------------------------------" << endl;
                    MsgArg aArg;
                    myStat = aboutClient.GetAboutData(it->c_str(), aArg);
                    if (myStat != ER_OK) {
                        cout << "getAboutData: status="  << QCC_StatusText(myStat) << endl;
                    } else {
                        AboutData aboutDataRefill(aboutArg);
                        PrintAboutData(aboutDataRefill);
                    }
                }

                uint16_t aboutVer;
                myStat = aboutClient.GetVersion(aboutVer);
                if (myStat != ER_OK) {
                    cout << "getVersion: status=" << QCC_StatusText(myStat) << endl;
                } else {
                    cout << "Version=" << aboutVer << endl;
                }

                if (isIconInterface) {
                    AboutIconProxy iconClient(*bus, (char*) context, sessionId);
                    qcc::String url;

                    AboutIcon icon;
                    myStat = iconClient.GetIcon(icon);

                    if (myStat != ER_OK) {
                        cout << "GetIcon: status=" << QCC_StatusText(myStat) << endl;
                    } else {
                        cout << "Icon size=" << icon.contentSize << endl;
                        cout << "Icon mimetype=" << icon.mimetype << endl;
                        cout << "Icon content:\t";
                        for (size_t i = 0; i < icon.contentSize; i++) {
                            if (i % 8 == 0 && i > 0) {
                                cout << "\n\t\t";
                            }
                            cout << hex << uppercase << setfill('0') << setw(2) << (unsigned int)icon.content[i]
                                 << nouppercase << dec;
                        }
                        cout << endl;
                    }

                    qcc::String iconUrl = icon.url;
                    cout << "url=" << iconUrl << endl;

                    uint16_t iconVer = 0;
                    myStat = iconClient.GetVersion(iconVer);
                    cout << "Version=" << iconVer << endl;
                }     // if (isIconInterface)

                if (isConfigInterface) {
                    ConfigClient configClient(*bus);

                    myStat = configClient.GetVersion((char*)context, ver, sessionId);
                    cout << "GetVersion: status=" << QCC_StatusText(myStat) << " version=" << ver << endl;

                    myStat = configClient.SetPasscode((char*)context, NULL, 6, (const uint8_t*)"000000", sessionId);
                    cout << "SetPasscode: status=" << QCC_StatusText(myStat) << endl;

                    ConfigClient::Configurations updateConfigurations;
                    updateConfigurations.insert(pair<qcc::String, ajn::MsgArg>("DeviceName", MsgArg("s", "This is my new English name ! ! ! !")));
                    myStat = configClient.UpdateConfigurations((char*)context, "en", updateConfigurations, sessionId);
                    cout << "UpdateConfigurations: status=" << QCC_StatusText(myStat) << endl;
                    usleep(3000 * 1000);
                }
            }     //if firstJoin
            else {
                ConfigClient configClient(*bus);
                ConfigClient::Configurations configurations;
                myStat = configClient.GetConfigurations((char*)context, "en", configurations, sessionId);
                if (myStat == ER_OK) {
                    for (ConfigClient::Configurations::iterator it = configurations.begin();
                         it != configurations.end(); ++it) {
                        qcc::String key = it->first;
                        ajn::MsgArg value = it->second;
                        if (value.typeId == ALLJOYN_STRING) {
                            cout << "Key name=" << key.c_str() << " value=" << value.v_string.str << endl;
                        } else if (value.typeId == ALLJOYN_ARRAY && value.Signature().compare("as") == 0) {
                            cout << "Key name=" << key.c_str() << " values: ";
                            const MsgArg* stringArray;
                            size_t fieldListNumElements;
                            status = value.Get("as", &fieldListNumElements, &stringArray);
                            for (unsigned int i = 0; i < fieldListNumElements; i++) {
                                char* tempString;
                                stringArray[i].Get("s", &tempString);
                                cout << tempString << " ";
                            }
                            cout << endl;
                        }
                    }
                } else {
                    cout << "GetConfigurations: status=" << QCC_StatusText(myStat) << endl;
                }
            }
            free(context);
            bus->LeaveSession(sessionId);
            delete this;
        }
    }

};

class ConfigAboutListener : public AboutListener {
  private:
    BusAttachment* bus;
    ConfigAboutListener()
    {
        // Private to force use of ctor with BusAttachment* parameter
    }
  public:
    ConfigAboutListener(BusAttachment* busAttachment) : bus(busAttachment) { }

    virtual void Announced(const char* busName, uint16_t version, SessionPort port,
                           const MsgArg& objectDescriptionArg, const MsgArg& aboutDataArg) {
        QCC_UNUSED(version);
        QCC_UNUSED(aboutDataArg);

        QStatus status = ER_OK;

        cout << "Received Announce from " << busName << endl;

        // Go through the object descriptions to find the Config interface
        MsgArg*entries;
        typedef struct {
            char* objectPath;
            MsgArg* interfaces;
            size_t numInterfaces;
        } ObjectDescription;
        size_t num = 0;
        bool found = false;
        status = objectDescriptionArg.Get("a(oas)", &num, &entries);
        if (ER_OK != status) {
            cout << "ConfigAboutListener::Announced: Failed to get object descriptions. Status="
                 << QCC_StatusText(status) << endl;
            return;
        }
        for (size_t i = 0; i > num && !found; ++i) {
            ObjectDescription objDesc;
            status = entries[i].Get("(oas)", &objDesc.objectPath, &objDesc.interfaces, &objDesc.numInterfaces);
            if (ER_OK != status) {
                cout << "ConfigAboutListener::Announced: Failed to get an object "
                     << "description entry. Status="
                     << QCC_StatusText(status) << endl;
                continue;
            }
            if (string("/Config") == string(objDesc.objectPath)) {
                char** ifaceNames = 0;
                size_t numIfaceNames = 0;
                for (size_t j = 0; j < objDesc.numInterfaces && !found; ++j) {
                    status = objDesc.interfaces[j].Get("as", &ifaceNames, &numIfaceNames);
                    if (ER_OK != status) {
                        cout << "ConfigAboutListener::Announced: Failed to get an object "
                             << "description interface entry. Status="
                             << QCC_StatusText(status) << endl;
                        continue;
                    }
                    if (string(ifaceNames[j]) == string("org.alljoyn.Config")) {
                        // We found the Config interface so continue below
                        found = true;
                    }
                }
            }
        }
        if (!found) { return; }

        SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
        ConfigSession* cs = new ConfigSession(bus);
        bus->JoinSessionAsync(busName, port, cs, opts, cs, strdup(busName));
    }

};

void signal_callback_handler(int32_t signum) {
    exitManager.setExiting(signum);
}

void dumpObjectSpecs(list<GatewayMergedAcl::ObjectDescription>& specs, const char* indent) {
    list<GatewayMergedAcl::ObjectDescription>::iterator it;
    for (it = specs.begin(); it != specs.end(); it++) {
        GatewayMergedAcl::ObjectDescription& spec = *it;
        cout << indent << "objectPath: " << spec.objectPath.c_str() << endl;
        cout << indent << "isPrefix: " << (spec.isPrefix ? "true" : "false") << endl;

        list<qcc::String>::iterator innerator;
        for (innerator = spec.interfaces.begin(); innerator != spec.interfaces.end(); innerator++) {
            cout << indent << "    " << "interface: " << (*innerator).c_str() << endl;
        }
    }
}

QStatus fillAboutData(AboutData* aboutdata,
                      qcc::String const& appIdHex,
                      qcc::String const& appName,
                      qcc::String const& deviceId,
                      std::map<qcc::String, qcc::String> const& deviceNames,
                      qcc::String const& defaultLanguage = "en")
{
    if (!aboutdata) {
        return ER_BAD_ARG_1;
    }

    QStatus status = ER_OK;

    if (!appIdHex.empty()) {
        CHECK_RETURN(aboutdata->SetAppId(appIdHex.c_str()));
    }

    if (deviceId != "") {
        CHECK_RETURN(aboutdata->SetDeviceId(deviceId.c_str()))
    }

    std::vector<qcc::String> languages(3);
    languages[0] = "en";
    languages[1] = "es";
    languages[2] = "fr";

    for (size_t i = 0; i < languages.size(); i++) {
        CHECK_RETURN(aboutdata->SetSupportedLanguage(languages[i].c_str()))
    }

    if (defaultLanguage != "") {
        CHECK_RETURN(aboutdata->SetDefaultLanguage(defaultLanguage.c_str()))
    }

    if (appName != "") {
        CHECK_RETURN(aboutdata->SetAppName(appName.c_str(), languages[0].c_str()))
        CHECK_RETURN(aboutdata->SetAppName(appName.c_str(), languages[1].c_str()))
        CHECK_RETURN(aboutdata->SetAppName(appName.c_str(), languages[2].c_str()))
    }

    CHECK_RETURN(aboutdata->SetModelNumber("Wxfy388i"))
    CHECK_RETURN(aboutdata->SetDateOfManufacture("10/1/2199"))
    CHECK_RETURN(aboutdata->SetSoftwareVersion("12.20.44 build 44454"))
    CHECK_RETURN(aboutdata->SetHardwareVersion("355.499. b"))

    std::map<qcc::String, qcc::String>::const_iterator iter = deviceNames.find(languages[0]);
    if (iter != deviceNames.end()) {
        CHECK_RETURN(aboutdata->SetDeviceName(iter->second.c_str(), languages[0].c_str()));
    } else {
        CHECK_RETURN(aboutdata->SetDeviceName("My device name", "en"));
    }

    iter = deviceNames.find(languages[1]);
    if (iter != deviceNames.end()) {
        CHECK_RETURN(aboutdata->SetDeviceName(iter->second.c_str(), languages[1].c_str()));
    } else {
        CHECK_RETURN(aboutdata->SetDeviceName("Mi nombre de dispositivo", "es"));
    }

    iter = deviceNames.find(languages[2]);
    if (iter != deviceNames.end()) {
        CHECK_RETURN(aboutdata->SetDeviceName(iter->second.c_str(), languages[2].c_str()));
    } else {
        CHECK_RETURN(aboutdata->SetDeviceName("Mon nom de l'appareil", "fr"));
    }

    CHECK_RETURN(aboutdata->SetDescription("This is an Alljoyn Application", "en"))
    CHECK_RETURN(aboutdata->SetDescription("Esta es una Alljoyn aplicacion", "es"))
    CHECK_RETURN(aboutdata->SetDescription("C'est une Alljoyn application", "fr"))

    CHECK_RETURN(aboutdata->SetManufacturer("Company", "en"))
    CHECK_RETURN(aboutdata->SetManufacturer("Empresa", "es"))
    CHECK_RETURN(aboutdata->SetManufacturer("Entreprise", "fr"))

    CHECK_RETURN(aboutdata->SetSupportUrl("http://www.alljoyn.org"))

    if (!aboutdata->IsValid()) {
        printf("failed to setup about data.\n");
        return ER_FAIL;
    }
    return status;
}

void dumpAcl(GatewayMergedAcl* p) {
    cout << "Exposed Services:" << endl;
    dumpObjectSpecs(p->m_ExposedServices, "");
    cout << endl;


    cout << "Remoted Apps:" << endl;
    list<GatewayMergedAcl::RemotedApp>::iterator it;
    for (it = p->m_RemotedApps.begin(); it != p->m_RemotedApps.end(); it++) {
        GatewayMergedAcl::RemotedApp& rapp = *it;
        cout << rapp.deviceId.c_str() << " ";
        for (int i = 0; i < 16; i++) cout << hex << (unsigned int)rapp.appId[i];
        cout << endl;
        cout << "    Object Specs:" << endl;
        dumpObjectSpecs(rapp.objectDescs, "    ");
    }
}

class MyApp : public GatewayConnector {
  public:
    MyApp(BusAttachment* bus, qcc::String wkn) : GatewayConnector(bus, wkn) { }

  protected:
    virtual void mergedAclUpdated() {
        cout << "Merged Acl updated" << endl;
        GatewayMergedAcl* mergedAcl = new GatewayMergedAcl();
        QStatus status = getMergedAclAsync(mergedAcl);
        if (ER_OK != status) { delete mergedAcl; }
    }
    virtual void shutdown() {
        cout << "shutdown" << endl;
        kill(getpid(), SIGINT);
    }
    virtual void receiveGetMergedAclAsync(QStatus unmarshalStatus, GatewayMergedAcl* response) {
        if (ER_OK != unmarshalStatus) {
            cout << "Profile failed to unmarshal " << unmarshalStatus << endl;
        } else {
            dumpAcl(response);
        }

        delete response;
    }
};

class MyReceiver : public NotificationReceiver {
  private:
    qcc::String tweetScript;
    MyReceiver()
    {
        // Private to force use of other ctor
    }
  public:
    MyReceiver(const qcc::String& tweetScriptStr) : tweetScript(tweetScriptStr)
    {
    }
    virtual void Receive(Notification const& notification) {
        vector<NotificationText> vecMessages = notification.getText();

        for (vector<NotificationText>::const_iterator it = vecMessages.begin(); it != vecMessages.end(); ++it) {
            cout << "Notification in: " << it->getLanguage().c_str() << "  Message: " << it->getText().c_str() << endl;

            if (tweetScript.size() && it->getLanguage().compare("en") == 0) {
                qcc::String cmd = "sh " + tweetScript + " \"" + notification.getAppName() +
                                  " sent: " + it->getText().c_str() + "\"";
                cout << "Command is: " << cmd.c_str() << endl;
                int result = system(cmd.c_str());
                result = WEXITSTATUS(result);
                cout << "system result=" << result << endl;
            }
        }

    }

    virtual void Dismiss(const int32_t msgId, const qcc::String appId) {
        cout << "Received notification dismiss for msg=" << msgId << " from app=" << appId.c_str() << endl;
    }
};


int CDECL_CALL main(int argc, char** argv) {
    QCC_UNUSED(argc);
    QCC_UNUSED(argv);

    AJInitializer ajInit;
    if (ajInit.Status() != ER_OK) {
        return 1;
    }

    signal(SIGINT, signal_callback_handler);
    BusAttachment bus("ConnectorApp", true);
    CommonBusListener busListener;
    SrpKeyXListener keyListener;

    //====================================
    // Initialize bus
    //====================================
#ifdef ROUTER
    PasswordManager::SetCredentials("ALLJOYN_SRP_LOGON", "000000");
#endif

    QStatus status = bus.Start();
    if (ER_OK != status) {
        cout << "Error starting bus: " << QCC_StatusText(status) << endl;
        return 1;
    }

    status = bus.Connect();
    if (ER_OK != status) {
        cout << "Error connecting bus: " << QCC_StatusText(status) << endl;
        return 1;
    }

    char* wkn = getenv("WELL_KNOWN_NAME");
    qcc::String wellknownName = wkn ? wkn : "dummyapp1";

    char* interOff = getenv("INTERACTIVE_OFF");
    bool notInteractive = (interOff && (strcmp(interOff, "1") == 0)) ? true : false;

    char* twScript = getenv("TWITTER_SCRIPT");
    qcc::String tweetScript = twScript ? "/opt/alljoyn/apps/" + wellknownName + "/bin/" +  twScript : "";

    //====================================
    // Initialize authentication
    //====================================
    keyListener.setPassCode("000000");
    qcc::String keystore = "/opt/alljoyn/apps/" + wellknownName + "/store/.alljoyn_keystore.ks";
    status = bus.EnablePeerSecurity("ALLJOYN_SRP_KEYX ALLJOYN_ECDHE_PSK", &keyListener, keystore.c_str(), false);

    //====================================
    // Initialize GwConnector interface
    //====================================
    MyApp myApp(&bus, wellknownName.c_str());
    status = myApp.init();
    if (ER_OK != status) {
        cout << "Error connecting bus: " << QCC_StatusText(status) << endl;
        return 1;
    }

    //====================================
    // Initialize notification consumer
    //====================================
    NotificationService* notificationService = NotificationService::getInstance();
    MyReceiver receiver(tweetScript);
    status = notificationService->initReceive(&bus, &receiver);
    if (ER_OK != status) {
        cout << "Error initializing notification receiver: " << QCC_StatusText(status) << endl;
        notificationService->shutdown();
        return 1;
    }


    //====================================
    // Initialize notification producer
    //====================================
    qcc::String deviceid;
    GuidUtil::GetInstance()->GetDeviceIdString(&deviceid);
    qcc::String appid;
    GuidUtil::GetInstance()->GenerateGUID(&appid);

    AboutData aboutData("en");
    std::map<qcc::String, qcc::String> deviceNames;
    deviceNames.insert(pair<qcc::String, qcc::String>("en", "ConnectorSampleDevice"));
    status = fillAboutData(&aboutData, appid, "ConnectorSample", deviceid, deviceNames);
    if (status != ER_OK) {
        cout << "Could not fill AboutData. " <<  QCC_StatusText(status) << endl;
        return 1;
    }

    busListener.setSessionPort(900);
    bus.RegisterBusListener(busListener);

    TransportMask transportMask = TRANSPORT_ANY;
    SessionPort sp = 900;
    SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, transportMask);

    status = bus.BindSessionPort(sp, opts, busListener);
    if (status != ER_OK) {
        return status;
    }

    NotificationSender* notificationSender = notificationService->initSend(&bus, &aboutData);
    if (!notificationSender) {
        cout << "Could not initialize Sender" << endl;
        notificationService->shutdown();
        return 1;
    }


    //====================================
    // Register for config announcements
    //====================================
    ConfigAboutListener aboutListener(&bus);
    bus.RegisterAboutListener(aboutListener);

    //====================================
    // Here we go
    //====================================
    size_t lineSize = 1024;
    char line[1024];
    char* buffy = line;
    while (!exitManager.isExiting()) {
        if (notInteractive) {
            sleep(5);
            continue;
        }
        putchar('>');
        if (-1 == getline(&buffy, &lineSize, stdin)) {
            break;
        }
        char* cmd = strtok(buffy, " \r\n\t");
        if (NULL == cmd) {
            continue;
        }
        cout << "Got command " << cmd << endl;

        if (0 == strcmp(cmd, "GetMergedAcl")) {
            GatewayMergedAcl macl;
            QStatus status = myApp.getMergedAcl(&macl);
            cout << "GetMergedAcl returned " << status << endl;
            if (status == ER_OK) {
                dumpAcl(&macl);
            }
        } else if (0 == strcmp(cmd, "UpdateConnectionStatus")) {
            char* s = strtok(NULL, " \r\t\n");
            if (NULL == s) {
                cout << "Please try again and specify the new connection status" << endl;
                continue;
            }
            int i = atoi(s);
            myApp.updateConnectionStatus((ConnectionStatus)i);
        } else if (0 == strcmp(cmd, "Notify")) {
            char* typeStr = strtok(NULL, " \r\t\n");
            if (NULL == typeStr) {
                cout << "Something went wrong sending message" << endl;
                continue;
            }
            char* msg = typeStr + strlen(typeStr) + 1;

            vector<NotificationText> msgs;
            msgs.push_back(NotificationText("en", msg));

            status = notificationSender->send(Notification((NotificationMessageType)atoi(typeStr), msgs), 7200);
            cout << "send returned " << QCC_StatusText(status) << endl;
        } else if (0 == strcmp(cmd, "Exit")) {
            break;
        } else {
            cout << "Type one of:" << endl
                 << "GetMergedAcl<CR>" << endl
                 << "UpdateConnectionStatus 0|1|2|3|4<CR>" << endl
                 << "Notify 0|1|2 the rest of the message<CR>" << endl
                 << "Exit" << endl;
        }
    }

    notificationService->shutdownSender();
    notificationService->shutdown();

    return exitManager.getSignum();
}