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

#include <alljoyn/gateway/Acl.h>
#include <alljoyn/gateway/GatewayController.h>
#include <alljoyn/gateway/LogModule.h>
#include <qcc/StringUtil.h>
#include <algorithm>

#include "Constants.h"
#include "PayloadAdapter.h"

namespace ajn {
namespace gwc {

using namespace gwcConsts;

Acl::Acl() : m_GwBusName(""), m_AclWriteResponse(NULL), m_AclRules(NULL)
{

}

QStatus Acl::init(const qcc::String& gwBusName, const ajn::MsgArg*aclInfoAJ)
{
    char* AclID;
    char* AclName;
    short RetAclStatus;
    char* ObjectPath;

    m_GwBusName = gwBusName;

    m_AclStatus = GW_AS_INACTIVE;

    QStatus status = aclInfoAJ->Get("(ssqo)", &AclID, &AclName, &RetAclStatus, &ObjectPath);

    if (status == ER_OK) {
        m_AclId = AclID;

        m_AclName = AclName;

        m_AclStatus = (AclStatus)RetAclStatus;

        m_ObjectPath = ObjectPath;
    }

    BusAttachment* busAttachment = GatewayController::getInstance()->getBusAttachment();

    {
        qcc::String interfaceName = AJ_GATEWAYCONTROLLERACL_INTERFACE;
        InterfaceDescription* interfaceDescription = (InterfaceDescription*) busAttachment->GetInterface(interfaceName.c_str());
        if (!interfaceDescription) {

            status = busAttachment->CreateInterface(interfaceName.c_str(), interfaceDescription);
            if (status != ER_OK) {
                QCC_LogError(status, ("Could not create interface"));
                return status;
            }

            status = interfaceDescription->AddMethod(AJ_METHOD_ACTIVATEACL.c_str(), NULL, "q", "AclResponseCode");
            if (status != ER_OK) {
                QCC_LogError(status, ("Could not AddMethod"));
                return status;
            }

            status = interfaceDescription->AddMethod(AJ_METHOD_DEACTIVATEACL.c_str(), NULL, "q", "AclResponseCode");
            if (status != ER_OK) {
                QCC_LogError(status, ("Could not AddMethod"));
                return status;
            }

            status = interfaceDescription->AddMethod(AJ_METHOD_GETACL.c_str(), NULL, "sa(obas)a(saya(obas))a{ss}a{ss}", "aclName,exposedServices,remotedApps,metadata");
            if (status != ER_OK) {
                QCC_LogError(status, ("Could not AddMethod"));
                return status;
            }

            status = interfaceDescription->AddMethod(AJ_METHOD_GETACLSTATUS.c_str(), NULL, "q", "aclStatus");
            if (status != ER_OK) {
                QCC_LogError(status, ("Could not AddMethod"));
                return status;
            }

            status = interfaceDescription->AddMethod(AJ_METHOD_UPDATEACL.c_str(), "sa(obas)a(saya(obas))a{ss}a{ss}", "q", "aclName,exposedServices,remotedApps,metadata,restartResponseCode");
            if (status != ER_OK) {
                QCC_LogError(status, ("Could not AddMethod"));
                return status;
            }
            status = interfaceDescription->AddMethod(AJ_METHOD_UPDATEACLMETADATA.c_str(), "a{ss}", "q", "metadata,aclResponseCode");
            if (status != ER_OK) {
                QCC_LogError(status, ("Could not AddMethod"));
                return status;
            }

            status = interfaceDescription->AddMethod(AJ_METHOD_UPDATECUSTOMMETADATA.c_str(), "a{ss}", "q", "metadata,aclResponseCode");
            if (status != ER_OK) {
                QCC_LogError(status, ("Could not AddMethod"));
                return status;
            }

            status = interfaceDescription->AddProperty(AJ_PROPERTY_VERSION.c_str(), AJPARAM_UINT16.c_str(), PROP_ACCESS_READ);
            if (status != ER_OK) {
                QCC_LogError(status, ("Could not AddProperty"));
                return status;
            }

            interfaceDescription->Activate();
        }
    }

    return ER_OK;
}

Acl::~Acl() {
}

const qcc::String& Acl::getName()
{
    return m_AclName;
}

void Acl::setName(const qcc::String& name)
{
    m_AclName = name;
}


const qcc::String& Acl::getId()
{
    return m_AclId;
}


const qcc::String& Acl::getObjectPath()
{
    return m_ObjectPath;
}

const qcc::String& Acl::getGwBusName()
{
    return m_GwBusName;
}

QStatus Acl::activate(SessionId sessionId, AclResponseCode& aclResp)
{
    QStatus status = ER_OK;

    {

        BusAttachment* busAttachment = GatewayController::getInstance()->getBusAttachment();

        // create proxy bus object
        ProxyBusObject proxy(*busAttachment, m_GwBusName.c_str(), m_ObjectPath.c_str(), sessionId, true);

        qcc::String interfaceName = AJ_GATEWAYCONTROLLERACL_INTERFACE;
        InterfaceDescription* interfaceDescription = (InterfaceDescription*) busAttachment->GetInterface(interfaceName.c_str());
        if (!interfaceDescription) {
            status = ER_FAIL;
            goto end;
        }

        status = proxy.AddInterface(*interfaceDescription);
        if (status != ER_OK) {
            QCC_LogError(status, ("AddInterface failed"));
            goto end;
        }

        Message replyMsg(*busAttachment);
        status = proxy.MethodCall(interfaceName.c_str(), AJ_METHOD_ACTIVATEACL.c_str(), NULL, 0, replyMsg);
        if (status != ER_OK) {
            QCC_LogError(status, ("Call to ActivateAcl failed"));
            goto end;
        }

        const ajn::MsgArg* returnArgs;
        size_t numArgs = 0;
        replyMsg->GetArgs(numArgs, returnArgs);
        if (numArgs != 1) {
            QCC_DbgHLPrintf(("Received unexpected amount of returnArgs"));
            status = ER_BUS_UNEXPECTED_SIGNATURE;
            goto end;
        }

        short aclResponse;

        status = returnArgs[0].Get("q", &aclResponse);

        if (status != ER_OK) {
            QCC_LogError(status, ("Failed getting restartStatus"));
            goto end;
        }

        if (aclResponse == GW_ACL_RC_SUCCESS) {
            m_AclStatus = GW_AS_ACTIVE;
        }

        aclResp = (AclResponseCode)aclResponse;
        return status;
    }
end:
    {
        aclResp = GW_ACL_RC_INVALID;
        return status;
    }
}

QStatus Acl::deactivate(SessionId sessionId, AclResponseCode& aclResp)
{
    QStatus status = ER_OK;

    {
        BusAttachment* busAttachment = GatewayController::getInstance()->getBusAttachment();

        // create proxy bus object
        ProxyBusObject proxy(*busAttachment, m_GwBusName.c_str(), m_ObjectPath.c_str(), sessionId, true);

        qcc::String interfaceName = AJ_GATEWAYCONTROLLERACL_INTERFACE;
        InterfaceDescription* interfaceDescription = (InterfaceDescription*) busAttachment->GetInterface(interfaceName.c_str());
        if (!interfaceDescription) {
            status = ER_FAIL;
            goto end;
        }

        status = proxy.AddInterface(*interfaceDescription);
        if (status != ER_OK) {
            QCC_LogError(status, ("AddInterface failed"));
            goto end;
        }

        Message replyMsg(*busAttachment);
        status = proxy.MethodCall(interfaceName.c_str(), AJ_METHOD_DEACTIVATEACL.c_str(), NULL, 0, replyMsg);
        if (status != ER_OK) {
            QCC_LogError(status, ("Call to DeactivateAcl failed"));
            goto end;
        }

        const ajn::MsgArg* returnArgs;
        size_t numArgs = 0;
        replyMsg->GetArgs(numArgs, returnArgs);
        if (numArgs != 1) {
            QCC_DbgHLPrintf(("Received unexpected amount of returnArgs"));
            status = ER_BUS_UNEXPECTED_SIGNATURE;
            goto end;
        }

        short aclResponse;

        status = returnArgs[0].Get("q", &aclResponse);

        if (status != ER_OK) {
            QCC_LogError(status, ("Failed getting restartStatus"));
            goto end;
        }

        if (aclResponse == GW_ACL_RC_SUCCESS) {
            m_AclStatus = GW_AS_INACTIVE;
        }

        aclResp = (AclResponseCode)aclResponse;
        return status;
    }
end:

    {
        aclResp = GW_ACL_RC_INVALID;
        return status;
    }
}

QStatus Acl::update(SessionId sessionId, AclRules* aclRules, ConnectorCapabilities* connectorCapabilties, AclWriteResponse** aclWriteResponse)
{

    if (m_AclWriteResponse) {
        delete m_AclWriteResponse;
        m_AclWriteResponse = NULL;
    }

    QStatus status = ER_OK;

    {

        BusAttachment* busAttachment = GatewayController::getInstance()->getBusAttachment();

        // create proxy bus object
        ProxyBusObject proxy(*busAttachment, m_GwBusName.c_str(), m_ObjectPath.c_str(), sessionId, true);

        qcc::String interfaceName = AJ_GATEWAYCONTROLLERACL_INTERFACE;
        InterfaceDescription* interfaceDescription = (InterfaceDescription*) busAttachment->GetInterface(interfaceName.c_str());
        if (!interfaceDescription) {
            status = ER_FAIL;
            goto end;
        }

        status = proxy.AddInterface(*interfaceDescription);
        if (status != ER_OK) {
            QCC_LogError(status, ("AddInterface failed"));
            goto end;
        }

        // Validate the rules in the ACL

        std::vector<RuleObjectDescription*> expServicesTargetOut;

        std::vector<RuleObjectDescription*> invalidExpServices = validateManifObjDescs(aclRules->getExposedServices(),
                                                                                       expServicesTargetOut,
                                                                                       connectorCapabilties->getExposedServices());

        const std::vector<RemotedApp*> remotedAppsIn =  aclRules->getRemotedApps();

        std::vector<RemotedApp*> invalidRemotedApps;

        std::vector<RemotedApp*> remotedAppsOut;

        for (std::vector<RemotedApp*>::const_iterator itr = remotedAppsIn.begin(); itr != remotedAppsIn.end(); itr++) {
            std::vector<RuleObjectDescription*> remotedAppsTarget;

            std::vector<RuleObjectDescription*> invalidRemAppRules =
                validateManifObjDescs((*itr)->getRuleObjDesciptions(),
                                      remotedAppsTarget,
                                      connectorCapabilties->getRemotedServices());

            if (invalidRemAppRules.size() > 0) {
                RemotedApp*remotedApp = new RemotedApp();

                status = remotedApp->init((*itr), invalidRemAppRules);

                if (status != ER_OK) {
                    QCC_LogError(status, ("PayloadAdapter::MarshalAclRules failed"));
                    delete remotedApp;
                    remotedApp = NULL;

                    for (std::vector<RemotedApp*>::iterator itr = invalidRemotedApps.begin(); itr != invalidRemotedApps.end(); itr++) {
                        delete (*itr);
                    }

                    goto end;
                }

                invalidRemotedApps.push_back(remotedApp);
            }

            //If there is no any marshaled rule, no valid rule was found -> continue
            if (remotedAppsTarget.size() == 0) {
                continue;
            }

            //Populate the RemotedApp
            RemotedApp*remotedApp = new RemotedApp();

            status = remotedApp->init((*itr), remotedAppsTarget);

            if (status != ER_OK) {
                QCC_LogError(status, ("RemotedApp::init failed"));

                delete remotedApp;
                remotedApp = NULL;

                for (std::vector<RemotedApp*>::iterator itr = invalidRemotedApps.begin(); itr != invalidRemotedApps.end(); itr++) {
                    delete (*itr);
                }


                for (std::vector<RemotedApp*>::iterator itr = remotedAppsOut.begin(); itr != remotedAppsOut.end(); itr++) {
                    delete (*itr);
                }


                goto end;
            }

            remotedAppsOut.push_back(remotedApp);

            //Store this application data in the internal metadata
            qcc::String AppId;
            const uint8_t*binary_appid = (*itr)->getAppId();

            AppId = qcc::BytesToHexString(binary_appid, UUID_LENGTH);

            qcc::String keyPrefix = (*itr)->getDeviceId() + "_" + AppId;
            m_InternalMetadata.insert(std::pair<qcc::String, qcc::String>(keyPrefix + AJSUFFIX_APP_NAME, remotedApp->getAppName()));
            m_InternalMetadata.insert(std::pair<qcc::String, qcc::String>(keyPrefix + AJSUFFIX_DEVICE_NAME, remotedApp->getDeviceName()));
        }

        AclRules*transmittedAcessRules = new AclRules();
        status = transmittedAcessRules->init(expServicesTargetOut, remotedAppsOut);

        if (status != ER_OK) {
            QCC_LogError(status, ("AclRules init failed"));

            delete transmittedAcessRules;
            transmittedAcessRules = NULL;

            goto end;

        }

        transmittedAcessRules->setMetadata(m_InternalMetadata);

        std::vector<MsgArg*> inputArgsVector;

        status = PayloadAdapter::MarshalAclRules(*transmittedAcessRules, inputArgsVector);

        if (status != ER_OK) {
            QCC_LogError(status, ("PayloadAdapter::MarshalAclRules failed"));
            transmittedAcessRules->release();
            delete transmittedAcessRules;
            transmittedAcessRules = NULL;

            goto end;
        }

        MsgArg*aclNameArg = new MsgArg("s", m_AclName.c_str());

        inputArgsVector.insert(inputArgsVector.begin(), aclNameArg);

        MsgArg*internalMetadataKeyValueMapArg = new MsgArg("a{ss}", 0, NULL);

        inputArgsVector.push_back(internalMetadataKeyValueMapArg);

        for (std::vector<MsgArg*>::const_iterator itr = inputArgsVector.begin(); itr != inputArgsVector.end(); itr++) {
            (*itr)->SetOwnershipFlags(MsgArg::OwnsArgs, true);
        }

        if (inputArgsVector.size() != AJ_METHOD_UPDATEACL_INPUT_PARAM_COUNT) {
            QCC_LogError(ER_FAIL, ("UpdateAcl failed - wrong number of arguments gathered to be sent"));
            status = ER_FAIL;

            transmittedAcessRules->release();
            delete transmittedAcessRules;
            transmittedAcessRules = NULL;

            for (std::vector<RemotedApp*>::iterator itr = invalidRemotedApps.begin(); itr != invalidRemotedApps.end(); itr++) {
                delete (*itr);
            }


            for (std::vector<RemotedApp*>::iterator itr = remotedAppsOut.begin(); itr != remotedAppsOut.end(); itr++) {
                delete (*itr);
            }

            goto end;
        }

        MsgArg args[AJ_METHOD_UPDATEACL_INPUT_PARAM_COUNT];

        for (unsigned int x = 0; x != AJ_METHOD_UPDATEACL_INPUT_PARAM_COUNT; x++) {
            args[x] = *inputArgsVector[x];
        }


        Message replyMsg(*busAttachment);
        status = proxy.MethodCall(interfaceName.c_str(), AJ_METHOD_UPDATEACL.c_str(), args, AJ_METHOD_UPDATEACL_INPUT_PARAM_COUNT, replyMsg);

        transmittedAcessRules->release();
        delete transmittedAcessRules;
        transmittedAcessRules = NULL;

        if (status != ER_OK) {
            QCC_LogError(status, ("Call to UpdateAcl failed"));

            for (std::vector<RemotedApp*>::iterator itr = invalidRemotedApps.begin(); itr != invalidRemotedApps.end(); itr++) {
                delete (*itr);
            }


            for (std::vector<RemotedApp*>::iterator itr = remotedAppsOut.begin(); itr != remotedAppsOut.end(); itr++) {
                delete (*itr);
            }
            goto end;
        }

        const ajn::MsgArg* returnArgs;
        size_t numArgs = 0;
        replyMsg->GetArgs(numArgs, returnArgs);
        if (numArgs != 1) {
            QCC_DbgHLPrintf(("Received unexpected amount of returnArgs"));
            status = ER_BUS_UNEXPECTED_SIGNATURE;

            for (std::vector<RemotedApp*>::iterator itr = invalidRemotedApps.begin(); itr != invalidRemotedApps.end(); itr++) {
                delete (*itr);
            }


            for (std::vector<RemotedApp*>::iterator itr = remotedAppsOut.begin(); itr != remotedAppsOut.end(); itr++) {
                delete (*itr);
            }

            goto end;
        }

        short aclResponse;

        status = returnArgs[0].Get("q", &aclResponse);

        if (status != ER_OK) {
            QCC_LogError(status, ("Failed getting restartStatus"));

            for (std::vector<RemotedApp*>::iterator itr = invalidRemotedApps.begin(); itr != invalidRemotedApps.end(); itr++) {
                delete (*itr);
            }


            for (std::vector<RemotedApp*>::iterator itr = remotedAppsOut.begin(); itr != remotedAppsOut.end(); itr++) {
                delete (*itr);
            }

            goto end;
        }

        AclRules*invalidRules = new AclRules();
        status = invalidRules->init(invalidExpServices, invalidRemotedApps);

        if (status != ER_OK) {
            QCC_LogError(status, ("Failed AclRules init"));
            delete invalidRules;
            invalidRules = NULL;

            for (std::vector<RemotedApp*>::iterator itr = invalidRemotedApps.begin(); itr != invalidRemotedApps.end(); itr++) {
                delete (*itr);
            }


            for (std::vector<RemotedApp*>::iterator itr = remotedAppsOut.begin(); itr != remotedAppsOut.end(); itr++) {
                delete (*itr);
            }

            goto end;
        }


        m_AclWriteResponse = new AclWriteResponse(m_AclId, (AclResponseCode)aclResponse, invalidRules, m_ObjectPath);

        *aclWriteResponse = m_AclWriteResponse;
        return status;
    }
end:

    return ER_FAIL;
}

QStatus Acl::updateCustomMetadata(SessionId sessionId, const std::map<qcc::String, qcc::String>& metadata, AclResponseCode& aclResponseCode)
{
    QStatus status = ER_OK;

    {

        BusAttachment* busAttachment = GatewayController::getInstance()->getBusAttachment();

        // create proxy bus object
        ProxyBusObject proxy(*busAttachment, m_GwBusName.c_str(), m_ObjectPath.c_str(), sessionId, true);

        qcc::String interfaceName = AJ_GATEWAYCONTROLLERACL_INTERFACE;
        InterfaceDescription* interfaceDescription = (InterfaceDescription*) busAttachment->GetInterface(interfaceName.c_str());
        if (!interfaceDescription) {
            status = ER_FAIL;
            goto end;
        }

        status = proxy.AddInterface(*interfaceDescription);
        if (status != ER_OK) {
            QCC_LogError(status, ("AddInterface failed"));
            goto end;
        }

        ajn::MsgArg*metadataArg = PayloadAdapter::MarshalMetadata(metadata, status);
        if (status != ER_OK) {
            QCC_LogError(status, ("AddInterface failed"));
            delete [] metadataArg;
            goto end;
        }

        Message replyMsg(*busAttachment);
        status = proxy.MethodCall(interfaceName.c_str(), AJ_METHOD_UPDATECUSTOMMETADATA.c_str(), metadataArg, 1, replyMsg);

        delete [] metadataArg;
        metadataArg = NULL;

        if (status != ER_OK) {
            QCC_LogError(status, ("Call to updateMetadata failed"));
            goto end;
        }

        const ajn::MsgArg* returnArgs;
        size_t numArgs = 0;
        replyMsg->GetArgs(numArgs, returnArgs);
        if (numArgs != 1) {
            QCC_DbgHLPrintf(("Received unexpected amount of returnArgs"));
            status = ER_BUS_UNEXPECTED_SIGNATURE;
            goto end;
        }

        short aclResponse;

        status = returnArgs[0].Get("q", &aclResponse);

        if (status != ER_OK) {
            QCC_LogError(status, ("Failed getting restartStatus"));
            goto end;
        }

        aclResponseCode = (AclResponseCode)aclResponse;
        return status;
    }
end:
    {
        aclResponseCode = GW_ACL_RC_INVALID;
        return status;

    }
}

QStatus Acl::updateAclMetadata(SessionId sessionId, const std::map<qcc::String, qcc::String>& metadata, AclResponseCode& aclResponseCode)
{
    QStatus status = ER_OK;

    {

        BusAttachment* busAttachment = GatewayController::getInstance()->getBusAttachment();

        // create proxy bus object
        ProxyBusObject proxy(*busAttachment, m_GwBusName.c_str(), m_ObjectPath.c_str(), sessionId, true);

        qcc::String interfaceName = AJ_GATEWAYCONTROLLERACL_INTERFACE;
        InterfaceDescription* interfaceDescription = (InterfaceDescription*) busAttachment->GetInterface(interfaceName.c_str());
        if (!interfaceDescription) {
            status = ER_FAIL;
            goto end;
        }

        status = proxy.AddInterface(*interfaceDescription);
        if (status != ER_OK) {
            QCC_LogError(status, ("AddInterface failed"));
            goto end;
        }

        MsgArg*metadataArg = PayloadAdapter::MarshalMetadata(metadata, status);
        if (status != ER_OK) {
            QCC_LogError(status, ("AddInterface failed"));
            delete [] metadataArg;
            goto end;
        }

        MsgArg internalMetadataKeyValueMapArg("a{ss}", metadata.size(), metadataArg);

        Message replyMsg(*busAttachment);
        status = proxy.MethodCall(interfaceName.c_str(), AJ_METHOD_UPDATEACLMETADATA.c_str(), &internalMetadataKeyValueMapArg, 1, replyMsg);
        if (status != ER_OK) {
            QCC_LogError(status, ("Call to updateMetadata failed"));
            goto end;
        }

        const ajn::MsgArg* returnArgs;
        size_t numArgs = 0;
        replyMsg->GetArgs(numArgs, returnArgs);
        if (numArgs != 1) {
            QCC_DbgHLPrintf(("Received unexpected amount of returnArgs"));
            status = ER_BUS_UNEXPECTED_SIGNATURE;
            goto end;
        }

        short aclResponse;

        status = returnArgs[0].Get("q", &aclResponse);

        if (status != ER_OK) {
            QCC_LogError(status, ("Failed getting restartStatus"));
            goto end;
        }
        aclResponseCode = (AclResponseCode)aclResponse;
        return status;
    }
end:
    {
        aclResponseCode = GW_ACL_RC_INVALID;
        return status;
    }
}

AclStatus Acl::getStatus()
{
    return m_AclStatus;
}

QStatus Acl::retrieveStatus(SessionId sessionId, AclStatus& aclStatus)
{
    QStatus status = ER_OK;

    {

        BusAttachment* busAttachment = GatewayController::getInstance()->getBusAttachment();

        // create proxy bus object
        ProxyBusObject proxy(*busAttachment, m_GwBusName.c_str(), m_ObjectPath.c_str(), sessionId, true);

        qcc::String interfaceName = AJ_GATEWAYCONTROLLERACL_INTERFACE;
        InterfaceDescription* interfaceDescription = (InterfaceDescription*) busAttachment->GetInterface(interfaceName.c_str());
        if (!interfaceDescription) {
            status = ER_FAIL;
            goto end;
        }

        status = proxy.AddInterface(*interfaceDescription);
        if (status != ER_OK) {
            QCC_LogError(status, ("AddInterface failed"));
            goto end;
        }

        Message replyMsg(*busAttachment);
        status = proxy.MethodCall(interfaceName.c_str(), AJ_METHOD_GETACLSTATUS.c_str(), NULL, 0, replyMsg);
        if (status != ER_OK) {
            QCC_LogError(status, ("Call to getAclStatus failed"));
            goto end;
        }

        const ajn::MsgArg* returnArgs;
        size_t numArgs = 0;
        replyMsg->GetArgs(numArgs, returnArgs);
        if (numArgs != 1) {
            QCC_DbgHLPrintf(("Received unexpected amount of returnArgs"));
            status = ER_BUS_UNEXPECTED_SIGNATURE;
            goto end;
        }

        short aclResponse;

        status = returnArgs[0].Get("q", &aclResponse);

        if (status != ER_OK) {
            QCC_LogError(status, ("Failed calling getAclStatus"));
            goto end;
        }

        m_AclStatus = (AclStatus)aclResponse;

        aclStatus = (AclStatus)aclResponse;
        return status;
    }
end:

    aclStatus = GW_AS_INACTIVE;
    return status;
}

/**
 * Compare {@link RuleObjectDescription} according to their {@link RuleObjectPath}.
 * The algorithm performs lexicographical comparison of the object paths
 * with the condition that for equal object paths the object path that is not defined
 * as prefix is less than the object path that is prefix.
 */

int RuleObjectDescriptionComparator(const RuleObjectDescription*lhs, const RuleObjectDescription*rhs) {

    qcc::String lhsOP = lhs->getObjectPath()->getPath();
    qcc::String rhsOP = rhs->getObjectPath()->getPath();

    int compRes  = rhsOP.compare(lhsOP);
    if (compRes == 0) {

        if (lhs->getObjectPath()->isPrefix()) {
            return 1;
        } else {
            return -1;
        }
    }

    return compRes;
}

QStatus Acl::retrieve(SessionId sessionId,
                      const ConnectorCapabilities& connectorCapabilities,
                      std::vector<AnnouncementData*> const& announcements,
                      AclRules** aclRules)
{
    if (m_AclRules) {
        m_AclRules->release();
        delete m_AclRules;
        m_AclRules = NULL;
    }

    AclStatus aclStatus;
    QStatus status = retrieveStatus(sessionId, aclStatus);

    if (status != ER_OK) {
        QCC_LogError(status, ("RetrieveStatus failed"));
        goto end;
    }

    {

        BusAttachment* busAttachment = GatewayController::getInstance()->getBusAttachment();

        // create proxy bus object
        ProxyBusObject proxy(*busAttachment, m_GwBusName.c_str(), m_ObjectPath.c_str(), sessionId, true);

        qcc::String interfaceName = AJ_GATEWAYCONTROLLERACL_INTERFACE;
        InterfaceDescription* interfaceDescription = (InterfaceDescription*) busAttachment->GetInterface(interfaceName.c_str());
        if (!interfaceDescription) {
            status = ER_FAIL;
            goto end;
        }

        status = proxy.AddInterface(*interfaceDescription);
        if (status != ER_OK) {
            QCC_LogError(status, ("AddInterface failed"));
            goto end;
        }


        Message replyMsg(*busAttachment);
        status = proxy.MethodCall(interfaceName.c_str(), AJ_METHOD_GETACL.c_str(), NULL, 0, replyMsg);
        if (status != ER_OK) {
            QCC_LogError(status, ("Call to GetAcl failed"));
            goto end;
        }

        const ajn::MsgArg* returnArgs;
        size_t numArgs = 0;
        replyMsg->GetArgs(numArgs, returnArgs);
        if (numArgs != 5) {
            QCC_DbgHLPrintf(("Received unexpected amount of returnArgs"));
            status = ER_BUS_UNEXPECTED_SIGNATURE;
            goto end;
        }

        char*name;

        status = returnArgs[0].Get("s", &name);

        if (status != ER_OK) {
            QCC_LogError(status, ("Failed Get"));
            goto end;
        }

        m_AclName = name;

        status = PayloadAdapter::unmarshalMetadata(&returnArgs[3], &m_InternalMetadata);

        if (status != ER_OK) {
            QCC_LogError(status, ("Failed unmarshaling InternalMetadata"));
            goto end;
        }

        std::map<qcc::String, qcc::String> customMetadata;

        status = PayloadAdapter::unmarshalMetadata(&returnArgs[4], &customMetadata);

        if (status != ER_OK) {
            QCC_LogError(status, ("Failed unmarshaling custom Metadata"));
            goto end;
        }


        // prepare the returned aclRules by intersecting them with the manifest rules
        std::vector<RuleObjectDescription*> expServices     = connectorCapabilities.getExposedServices();
        std::vector<RuleObjectDescription*> remServices     = connectorCapabilities.getRemotedServices();
        std::sort(expServices.begin(), expServices.end(), RuleObjectDescriptionComparator);
        std::sort(remServices.begin(), remServices.end(), RuleObjectDescriptionComparator);

        AclRules*tmpAclRules = new AclRules();

        status = tmpAclRules->init(&returnArgs[1], &returnArgs[2], connectorCapabilities, m_InternalMetadata);

        if (status != ER_OK) {
            QCC_LogError(status, ("Failed tmpAclRules->init"));
            delete tmpAclRules;
            tmpAclRules = NULL;
            goto end;

        }

        // The information retreived from the gateway manager will always include configured rules and configured rules only.
        for (std::vector<RemotedApp*>::const_iterator remotedAppIter = tmpAclRules->getRemotedApps().begin(); remotedAppIter != tmpAclRules->getRemotedApps().end(); remotedAppIter++) {
            const std::vector<RuleObjectDescription*> objDescs = ((RemotedApp*)*remotedAppIter)->getRuleObjDesciptions();

            for (std::vector<RuleObjectDescription*>::const_iterator objDescsIter = objDescs.begin(); objDescsIter != objDescs.end(); objDescsIter++) {
                RuleObjectDescription*objDesc = *objDescsIter;
                objDesc->setConfigured(true);
            }


        }

        for (std::vector<RuleObjectDescription*>::const_iterator objDescsIter = tmpAclRules->getExposedServices().begin(); objDescsIter != tmpAclRules->getExposedServices().end(); objDescsIter++) {
            RuleObjectDescription*objDesc = *objDescsIter;
            objDesc->setConfigured(true);
        }

        std::vector<RuleObjectDescription*> exposedServices = convertExposedServices(tmpAclRules->getExposedServices(), expServices);

        std::vector<RemotedApp*> remotedApps;
        bool updateMetadata       =  convertRemotedApps(tmpAclRules->getRemotedApps(),
                                                        remotedApps,
                                                        remServices,
                                                        announcements, status);

        if (status != ER_OK) {
            QCC_LogError(status, ("ConvertRemotedApps failed"));
            delete tmpAclRules;
            tmpAclRules = NULL;

            goto end;
        }



        delete tmpAclRules;
        tmpAclRules = NULL;

        if (updateMetadata) {
            if (m_InternalMetadata.size() > 0) {
                AclResponseCode aclResponseCode;
                status = updateAclMetadata(sessionId, m_InternalMetadata, aclResponseCode);

                if (status != ER_OK) {
                    QCC_LogError(status, ("Failed UpdateAclMetadata"));
                }
            }
        }

        m_AclRules = new AclRules();

        status = m_AclRules->init(exposedServices, remotedApps);

        if (status != ER_OK) {
            QCC_LogError(status, ("Failed m_AclRules->init"));
            delete m_AclRules;
            m_AclRules = NULL;

            goto end;
        }

        m_AclRules->setMetadata(customMetadata);

        *aclRules = m_AclRules;
        return status;
    }
end:

    return ER_FAIL;
}


QStatus Acl::release()
{
    if (m_AclWriteResponse) {
        m_AclWriteResponse->release();
        delete m_AclWriteResponse;
        m_AclWriteResponse = NULL;
    }

    if (m_AclRules) {
        m_AclRules->release();
        delete m_AclRules;
        m_AclRules = NULL;
    }

    return ER_OK;
}

std::vector<RuleObjectDescription*>
Acl::validateManifObjDescs(const std::vector<RuleObjectDescription*>& toValidate,
                           std::vector<RuleObjectDescription*>& target,
                           const std::vector<RuleObjectDescription*>& connectorCapabilities) {

    std::vector<RuleObjectDescription*> invalidRules;


    for (std::vector<RuleObjectDescription*>::const_iterator itr = toValidate.begin(); itr != toValidate.end(); itr++) {

        std::set<RuleInterface> invInterfaces;
        bool isValid                = isValidRule(*itr, invInterfaces, connectorCapabilities);

        //If current RuleObjectDescription is NOT valid it need to be added to the invalid rules
        //OR the RuleObjectDescription could be valid but some of its interfaces are not
        if (!isValid || invInterfaces.size() > 0) {
            invalidRules.push_back(new RuleObjectDescription(*(*itr)->getObjectPath(), invInterfaces));
        }

        if (!isValid) {
            continue;
        }

        //Marshal and add the valid rules to the target
        target.push_back(*itr);
    }

    return invalidRules;
}

bool Acl::isValidRule(RuleObjectDescription*toValidate, std::set<RuleInterface>& notValid, const std::vector<RuleObjectDescription*>& connectorCapabilities)
{
    bool validRuleFound = false;
    std::set<RuleInterface>*validIfaces = (std::set<RuleInterface>*)toValidate->getInterfaces();           //this pointer to interfaces in "toValidate" is important - we change the set inside of the "toValidate" in/out variable.

    notValid.insert(validIfaces->begin(), validIfaces->end());

    validIfaces->empty();

    //If toValidate is not configured it considered as an invalid rule and won't be sent to the gateway
    if (!toValidate->isConfigured()) {
        return false;
    }

    std::vector<RuleObjectDescription*>::const_iterator itr;

    for (itr = connectorCapabilities.begin(); itr != connectorCapabilities.end(); itr++) {

        RuleObjectDescription* mRule = (*itr);

        RuleObjectPath*manop          = mRule->getObjectPath();
        const std::set<RuleInterface>*manifs = mRule->getInterfaces();

        //Check object path validity
        if (isValidObjPath(manop, toValidate->getObjectPath()->getPath(), toValidate->getObjectPath()->isPrefix())) {

            //If the the list of the manifest interfaces is empty, it means that all the interfaces
            //of the toValidate object path are supported, so toValidate object is fully valid ==> return true
            if (manifs->size() == 0) {

                validIfaces->insert(notValid.begin(), notValid.end());
                notValid.clear();
                return true;
            }

            //Validate interfaces
            std::set<RuleInterface>::const_iterator notValidIter = notValid.begin();
            while (notValidIter != notValid.end()) {

                RuleInterface ifaceToTest = *notValidIter;

                if (manifs->find(ifaceToTest) != manifs->end()) {                      // Check if the interface is valid
                    validRuleFound = true;
                    notValid.erase(notValidIter++);                       // Remove the interface from notValid group
                    validIfaces->insert(ifaceToTest);                   // Add the interface to the valid group
                    continue;
                }

                if ((notValid.size() == 0) || (notValidIter == notValid.end())) {
                    break;
                }

                notValidIter++;
            }

            //All the interfaces toValidate are valid
            if (validRuleFound && notValid.size() == 0) {
                return true;
            }

        }        //if :: objPath

    }        //for :: connectorCapabilities

    return validRuleFound;
}

bool Acl::isValidObjPath(const RuleObjectPath*manifOp, const qcc::String& toValidOP, bool isPrefix)
{
    if ((manifOp->isPrefix() && ConnectorApp::stringStartWith(manifOp->getPath(), toValidOP)) ||
        (!manifOp->isPrefix() && !isPrefix && (manifOp->getPath() == toValidOP))) {

        return true;
    }

    return false;
}

std::vector<RuleObjectDescription*>
Acl::convertExposedServices(const std::vector<RuleObjectDescription*>& aclExpServicesAJ,
                            const std::vector<RuleObjectDescription*>& manifExpServices)
{

    std::map<RuleObjectPath, std::set<RuleInterface> > usedManRules;
    std::vector<RuleObjectDescription*> aclExpServices   = convertObjectDescription(aclExpServicesAJ,
                                                                                    manifExpServices,
                                                                                    usedManRules);

    //Find out the manifest exposed services rules that weren't used
    for (std::vector<RuleObjectDescription*>::const_iterator manifExpSrvc = manifExpServices.begin(); manifExpSrvc != manifExpServices.end(); manifExpSrvc++) {

        RuleObjectPath*manop       = (*manifExpSrvc)->getObjectPath();
        std::set<RuleInterface>*manifs      = (std::set<RuleInterface>*)(*manifExpSrvc)->getInterfaces();                 // we manipulate this array


        std::set<RuleInterface>*usedIfaces = NULL;


        if (usedManRules.find(*manop) != usedManRules.end()) {
            usedIfaces = &usedManRules.find(*manop)->second;
        }
        RuleObjectPath storeOp(manop->getPath(), manop->getFriendlyName(), false, manop->isPrefixAllowed());


        //Check if this rule was NOT used then add it to the resExpServices
        if (usedIfaces == NULL) {
            aclExpServices.push_back(new RuleObjectDescription(storeOp, *manifs, false));
            continue;
        }

        //Remove from the manifest interfaces the interfaces that have been used
        for (std::set<RuleInterface>::const_iterator itr = usedIfaces->begin(); itr != usedIfaces->end(); itr++) {
            QCC_SyncPrintf("erasing %s", itr->getName().c_str());
            manifs->erase(manifs->find(*itr));
        }


        //Add to the resExpServices the object path and the interfaces that weren't used
        if (manifs->size() > 0) {
            aclExpServices.push_back(new RuleObjectDescription(storeOp, *manifs, false));
        }
    }

    return aclExpServices;
}        //convertExposedServices



std::vector<RuleObjectDescription*>
Acl::convertObjectDescription(const std::vector<RuleObjectDescription*>& objDescs,
                              const std::vector<RuleObjectDescription*>& manifest,
                              std::map<RuleObjectPath, std::set<RuleInterface> >& usedManRules)

{

    std::map<RuleObjectPath, std::set<RuleInterface> >  resRules;

    std::set<qcc::String> objDescInterfaceNames;

    // create a unique list of interface strings
    for (std::vector<RuleObjectDescription*>::const_iterator objDesc = objDescs.begin(); objDesc != objDescs.end(); objDesc++) {
        for (std::set<RuleInterface>::const_iterator itr = (*objDesc)->getInterfaces()->begin(); itr != (*objDesc)->getInterfaces()->end(); itr++) {
            objDescInterfaceNames.insert(itr->getName());
        }
    }

    for (std::vector<RuleObjectDescription*>::const_iterator objDesc = objDescs.begin(); objDesc != objDescs.end(); objDesc++) {

        std::set<qcc::String> ifacesToConvert = objDescInterfaceNames;

        for (std::vector<RuleObjectDescription*>::const_iterator manifRule = manifest.begin(); manifRule != manifest.end(); manifRule++) {
            RuleObjectPath*manop               = (*manifRule)->getObjectPath();
            const std::set<RuleInterface>*manifs              = (*manifRule)->getInterfaces();
            int manifsSize          = (int)manifs->size();

            if (!isValidObjPath(manop, (*objDesc)->getObjectPath()->getPath(), (*objDesc)->getObjectPath()->isPrefix())) {
                continue;
            }

            RuleObjectPath*resObjPath = NULL;

            if (manop->getPath().compare((*objDesc)->getObjectPath()->getPath()) == 0) {
                resObjPath = new RuleObjectPath((*objDesc)->getObjectPath()->getPath(), manop->getFriendlyName(), (*objDesc)->getObjectPath()->isPrefix(), manop->isPrefixAllowed());
            } else {
                resObjPath = new RuleObjectPath((*objDesc)->getObjectPath()->getPath(), "", (*objDesc)->getObjectPath()->isPrefix(), manop->isPrefixAllowed());
            }

            //Add used manifest rules for the empty manifest interfaces array
            if (manifsSize == 0) {
                std::map<RuleObjectPath, std::set<RuleInterface> >::const_iterator usedManOP = usedManRules.find(*manop);
                if (usedManOP != usedManRules.end()) {
                    std::set<RuleInterface> usedIfaces = usedManOP->second;
                    if (usedIfaces.size() == 0) {
                        std::set<RuleInterface> empty;
                        usedManRules.insert(std::pair<RuleObjectPath, std::set<RuleInterface> >(*manop, empty));
                    }
                }
            }

            std::set<qcc::String>::const_iterator ifacesToConvertIter = ifacesToConvert.begin();

            std::set<RuleInterface> resInterfaces;         //result interfaces

            while (ifacesToConvertIter != ifacesToConvert.end()) {

                qcc::String ajIface = (*ifacesToConvertIter);

                //If there are not interfaces in the manifest, it means that all the interfaces are supported
                //add them without display names
                if (manifsSize == 0) {
                    resInterfaces.insert(RuleInterface(ajIface, "", false));
                    ifacesToConvert.erase(ifacesToConvertIter++);
                }

                if (ifacesToConvertIter == ifacesToConvert.end()) {
                    break;
                }

                for (std::set<RuleInterface>::const_iterator manIface = manifs->begin(); manIface != manifs->end(); manIface++) {

                    //aclInterface is found in manifest
                    if (ajIface.compare(manIface->getName()) == 0) {


                        resInterfaces.insert(RuleInterface(ajIface, (*manIface).getFriendlyName(), manIface->isSecured()));
                        ifacesToConvert.erase(ifacesToConvertIter++);
                        continue;
                    }
                }        //for :: manifest interfaces

                if ((ifacesToConvert.size() == 0) || (ifacesToConvertIter == ifacesToConvert.end())) {
                    break;
                }

                ifacesToConvertIter++;
            }


            //Not found any matched interfaces, continue to the next manifest rule
            if (resInterfaces.size() == 0) {
                delete resObjPath;
                resObjPath = NULL;
                continue;
            }

            //Add the interfaces to the resObjPath
            std::set<RuleInterface> ifaces;

            if (resRules.find(*resObjPath) != resRules.end()) {
                ifaces = resRules.find(*resObjPath)->second;

                //Merge interfaces
                ifaces.insert(resInterfaces.begin(), resInterfaces.end());

            } else {

                resRules.insert(std::pair<RuleObjectPath, std::set<RuleInterface> >(*resObjPath, resInterfaces));
            }


            //Add used manifest rules
            if (manifsSize > 0) {

                std::set<RuleInterface> usedIfaces;
                if (usedManRules.find(*manop) != usedManRules.end()) {
                    usedIfaces.insert(resInterfaces.begin(), resInterfaces.end());
                } else {
                    usedManRules.insert(std::pair<RuleObjectPath, std::set<RuleInterface> >(*manop, resInterfaces));
                }
            }

            //If all the objDescAJ interfaces have been handled, no need to continue iterating
            //over the manifest rules
            if (ifacesToConvert.size() == 0) {
                delete resObjPath;
                resObjPath = NULL;
                break;
            }
            delete resObjPath;
            resObjPath = NULL;
        }        //for :: manifest

    }        //for :: objDescsAJ

    //Create final list of the configured rules

    std::map<RuleObjectPath, std::set<RuleInterface> >::const_iterator itr;
    std::vector<RuleObjectDescription*> rules;
    for (itr = resRules.begin(); itr != resRules.end(); itr++) {
        const RuleObjectPath*path = &itr->first;
        std::set<RuleInterface> connAppInterfacesSet = itr->second;

        QCC_SyncPrintf("size:%d", resRules.size());


        RuleObjectDescription*newOD = new RuleObjectDescription(*path, connAppInterfacesSet, true);
        rules.push_back(newOD);
    }

    return rules;
}        //convertObjectDescription


bool Acl::convertRemotedApps(const std::vector<RemotedApp*>& inputRemotedApps,
                             std::vector<RemotedApp*>& outputRemotedApps,
                             std::vector<RuleObjectDescription*>& remotedServices,
                             std::vector<AnnouncementData*> const& announcements, QStatus& status)
{
    //Gets TRUE if the metadata needs to be updated
    bool updatedMeta               = false;
    std::vector<RemotedApp*> configurableApps;
    status = ConnectorApp::extractRemotedApps(remotedServices, announcements, configurableApps);

    if (status != ER_OK) {
        QCC_LogError(status, ("ExtractRemotedApps failed"));
        return updatedMeta;
    }

    std::vector<RemotedApp*>::const_iterator remAppIter;

    //Iterate over the remoted apps
    for (remAppIter = inputRemotedApps.begin(); remAppIter != inputRemotedApps.end(); remAppIter++) {

        RemotedApp*remApp = (*remAppIter);

        //Retrieve announcement data to check whether the aclRemApps should be completed
        if (remApp->getAppId() == NULL) {
            QCC_SyncPrintf("retrieveRemotedApps - remotedApp with a bad appId has been received, objPath: '%s'", m_ObjectPath.c_str());
            continue;
        }


        std::map<RuleObjectPath, std::set<RuleInterface> > usedManRules;
        //Convert the acl remoted app object descriptions to the list of RuleObjectDescriptions
        //by intersecting with the manifest data.
        std::vector<RuleObjectDescription*> configuredRules = convertObjectDescription(
            remApp->getRuleObjDesciptions(), remotedServices, usedManRules);

        //Construct the standard deviceId_appId key
        qcc::String AppId;
        const uint8_t*binary_appid = remApp->getAppId();


        AppId = qcc::BytesToHexString(binary_appid, UUID_LENGTH);

        qcc::String keyPrefix = remApp->getDeviceId() + "_" + AppId;

        int confRulesSize = (int)configuredRules.size();
        QCC_SyncPrintf("retrieveRemotedApps - Created ObjDesc rules of the remoted app: '%s' rules size: '%d', objPath: '%s'", keyPrefix.c_str(), confRulesSize, m_ObjectPath.c_str());

        //Retrieve appName and deviceName from the metadata
        bool findMeta      = true;

        qcc::String key =  keyPrefix + AJSUFFIX_APP_NAME;

        std::map<qcc::String, qcc::String>::const_iterator itr = m_InternalMetadata.find(key);

        qcc::String appNameMeta;

        if (itr != m_InternalMetadata.end()) {
            appNameMeta = itr->second;
        }


        key = keyPrefix + AJSUFFIX_DEVICE_NAME;

        itr = m_InternalMetadata.find(key);

        qcc::String deviceNameMeta;

        if (itr != m_InternalMetadata.end()) {
            deviceNameMeta = itr->second;
        }

        if (deviceNameMeta.empty() || deviceNameMeta.length() == 0 ||
            appNameMeta.empty()  || appNameMeta.length() == 0) {

            QCC_SyncPrintf("retrieveRemotedApps - metadata is corrupted!!!. deviceName or appName weren't found, objPath: '%s'", m_ObjectPath.c_str());

            findMeta = false;
        }

        //Look for the configurable RemotedApp from intersection of the manifest
        //with announcement data
        RemotedApp*configurableApp = getRemotedApp(&configurableApps, remApp->getDeviceId(), binary_appid);

        //If there is no configurableApp, but aclMetadata has appName and deviceName to construct the RemotedApp object
        //and the acl configuredRules were created successfully, then create the RemotedApp object
        if (configurableApp == NULL) {

            QCC_SyncPrintf("retrieveRemotedApps - not found any ConfigurableApp for the remoted app: '%s', objPath: '%s'", key.c_str(), m_ObjectPath.c_str());

            if  (findMeta && confRulesSize > 0) {

                QCC_SyncPrintf("retrieveRemotedApps - metadata has the required values, creating the remoted app");
                //Create RemotedApp

                RemotedApp*remotedApp = new RemotedApp();

                status = remotedApp->init(qcc::String(""), appNameMeta, (uint8_t*)binary_appid, deviceNameMeta, remApp->getDeviceId(), configuredRules);

                if (status != ER_OK) {
                    QCC_LogError(status, ("remotedApp->init failed"));

                    delete remotedApp;
                    remotedApp = NULL;

                    return false;
                }

                outputRemotedApps.push_back(remotedApp);
            }
        } else {         //There is a configurableApp

            QCC_SyncPrintf("retrieveRemotedApps - found announcement for the remoted app: '%s', objPath: '%s'", key.c_str(), m_ObjectPath.c_str());


            if (metadataUpdated(deviceNameMeta, appNameMeta, *configurableApp, keyPrefix)) {
                updatedMeta = true;
            }

            //Completes already configured rules with rules that haven't configured yet
            addUnconfiguredRemotedAppRules(configurableApp->getRuleObjDesciptions(), configuredRules);

            if (configuredRules.size() > 0) {
                RemotedApp*remotedApp = new RemotedApp();

                status = remotedApp->init(configurableApp, configuredRules);

                if (status != ER_OK) {
                    QCC_LogError(status, ("Call to GetAcl failed"));
                    delete remotedApp;
                    remotedApp = NULL;
                    return false;
                }

                outputRemotedApps.push_back(remotedApp);
            }

        }        //if :: annData != null

    }        //for :: remotedApp

    //Add to the configured remotedApps the unconfigured remoted apps.
    //These apps remained in the configurableApps after working the algorithm above
    for (std::vector<RemotedApp*>::const_iterator itr = configurableApps.begin(); itr != configurableApps.end(); itr++) {
        outputRemotedApps.push_back(*itr);

        qcc::String AppId;

        AppId = qcc::BytesToHexString((*itr)->getAppId(), UUID_LENGTH);

        QCC_SyncPrintf("ConvertRemotedApps: '%s'", AppId.c_str());
    }


    return updatedMeta;
}


/**
 * Compares configured rules of the remoted apps with the unconfigured rules.
 * Completes the configured rules with the rules that haven't configured yet.
 * @param unconfRules
 * @param confRules
 */
void Acl::addUnconfiguredRemotedAppRules(const std::vector<RuleObjectDescription*>& unconfRules,
                                         std::vector<RuleObjectDescription*>& confRules) {

    std::vector<RuleObjectDescription*>::const_iterator iter;


    for (iter = unconfRules.begin(); iter != unconfRules.end(); iter++) {

        RuleObjectDescription*unconfRule = (*iter);


        RuleObjectPath*unconfOP          = unconfRule->getObjectPath();
        std::set<RuleInterface>* unconfIfaces  = (std::set<RuleInterface>*)unconfRule->getInterfaces();

        //Gets TRUE if unconfOP was found among the confRules
        bool unconfOpInConf = false;

        std::vector<RuleObjectDescription*>::const_iterator confRulesIter;

        for (confRulesIter = confRules.begin(); confRulesIter != confRules.end(); confRulesIter++) {

            RuleObjectPath* confOP         = (*confRulesIter)->getObjectPath();
            const std::set<RuleInterface>* confIfaces = (*confRulesIter)->getInterfaces();

            //Check if the unconfOP not equals confOP
            if (unconfOP->getPath().compare(confOP->getPath())) {
                continue;
            }

            unconfOpInConf = true;

            for (std::set<RuleInterface>::const_iterator itr = confIfaces->begin(); itr != confIfaces->end(); itr++) {
                QCC_SyncPrintf("erasing %s", itr->getName().c_str());


                std::set<RuleInterface>::iterator ruleInterfaceItr = unconfIfaces->find(*itr);

                if (ruleInterfaceItr != unconfIfaces->end()) {
                    unconfIfaces->erase(ruleInterfaceItr);
                }
            }
            //unconfIfaces->erase(confIfaces->begin(), confIfaces->end());
            break;
        }

        if (!unconfOpInConf || unconfIfaces->size() > 0) {
            confRules.push_back(new RuleObjectDescription(*unconfOP, *unconfIfaces, false));
        }

    }        //for::unconfRule
}

bool Acl::metadataUpdated(const qcc::String& deviceNameMeta, const qcc::String& appNameMeta, const RemotedApp& annApp, const qcc::String& keyPrefix) {

    bool updatedMeta      = false;

    qcc::String annAppName        = annApp.getAppName();
    qcc::String annDeviceName     = annApp.getDeviceName();

    qcc::String appNameMetaKey    = keyPrefix + AJSUFFIX_APP_NAME;
    qcc::String deviceNameMetaKey = keyPrefix + AJSUFFIX_DEVICE_NAME;

    //Check appName, deviceName correctness vs. announcements
    if (annAppName.compare(appNameMeta)) {

        QCC_SyncPrintf("retrieveRemotedApps - metaAppName differs from the announcement app name, update the metadata with the app name: '%s', objPath: '%s'", annAppName.c_str(), m_ObjectPath.c_str());

        m_InternalMetadata.insert(std::pair<qcc::String, qcc::String>(appNameMetaKey, annAppName));
        updatedMeta = true;
    }

    if (annDeviceName.compare(deviceNameMeta)) {

        QCC_SyncPrintf("retrieveRemotedApps - metaDeviceName differs from the announcement device name, update the metadata with the device name: '%s', objPath: '%s'", annDeviceName.c_str(), m_ObjectPath.c_str());

        m_InternalMetadata.insert(std::pair<qcc::String, qcc::String>(deviceNameMetaKey, annDeviceName));

        updatedMeta = true;
    }

    return updatedMeta;
}
}
}






















