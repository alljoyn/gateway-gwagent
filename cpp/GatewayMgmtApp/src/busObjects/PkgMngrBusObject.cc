/******************************************************************************
 * Copyright (c) AllSeen Alliance. All rights reserved.
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


#include "../GatewayConstants.h"

namespace ajn {
namespace gw {

using namespace services;
using namespace qcc;
using namespace gwConsts;

PkgManagerBusObject::PkgManagerBusObject(BusAttachment* bus, QStatus status) :
   m_PackageManager(NULL)
{
   status = createPkgManagerInterface(bus);
    if (*status != ER_OK) {
        QCC_LogError(*status, ("Could not create the PkgManagerInterface"));
        return;
    }

    m_PackageManager = new PackageManager();

}

PkgManagerBusObject::~PkgManagerBusObject()
{
   if (m_PackageManager) {
      delete m_PackageManager;
   }
}

QStatus PkgManagerBusObject::createPkgManagerInterface(BusAttachment* bus)
{
    QStatus status = ER_OK;

    InterfaceDescription* interfaceDescription = (InterfaceDescription*) bus->GetInterface(AJ_PKG_MNGR_INTERFACE.c_str());
    if (!interfaceDescription) {
        status = bus->CreateInterface(AJ_GW_APP_INTERFACE.c_str(), interfaceDescription, true);
        if (status != ER_OK) {
            goto postCreate;
        }
        status = interfaceDescription->AddMethod(AJ_METHOD_INSTALL_APP.c_str(),
                                                 AJ_INSTALL_APP_PARAMS_IN.c_str(),
                                                 AJ_INSTALL_APP_PARAMS_OUT.c_str(),
                                                 AJ_INSTALL_APP_PARAMS_NAMES.c_str());
        if (status != ER_OK) {
            goto postCreate;
        }
        status = interfaceDescription->AddMethod(AJ_METHOD_UNINSTALL_APP.c_str(),
                                                 AJ_UNINSTALL_APP_PARAMS_IN.c_str(),
                                                 AJ_UNINSTALL_APP_PARAMS_OUT.c_str(),
                                                 AJ_UNINSTALL_APP_PARAMS_NAMES.c_str());
        if (status != ER_OK) {
            goto postCreate;
        }
    }

postCreate:
    if (status != ER_OK) {
        QCC_LogError(status, ("Could not create interface"));
        return status;
    }

    status = AddInterface(*interfaceDescription, ANNOUNCED);
    if (status != ER_OK) {
        QCC_LogError(status, ("Could not add interface"));
        return status;
    }

    const ajn::InterfaceDescription::Member* methodMember = interfaceDescription->GetMember(AJ_METHOD_GET_APP_STATUS.c_str());
    status = AddMethodHandler(methodMember, static_cast<MessageReceiver::MethodHandler>(&AppBusObject::GetManifestFile));
    if (status != ER_OK) {
        QCC_LogError(status, ("Could not register the GetManifestFile MethodHandler"));
        return status;
    }

    methodMember = interfaceDescription->GetMember(AJ_METHOD_UNINSTALL_APP.c_str());
    status = AddMethodHandler(methodMember, static_cast<MessageReceiver::MethodHandler>(&AppBusObject::GetManifestInterfaces));
    if (status != ER_OK) {
        QCC_LogError(status, ("Could not register the GetManifestInterfaces MethodHandler"));
        return status;
    }

    QCC_DbgTrace(("Created PkgManager BusObject successfully"));

    return status;
    
}

void PkgManagerBusObject::InstallApp(const InterfaceDescription::Member* member, Message& msg)
{
   qcc::String appId;
   qcc::String packageName;
   qcc::String appVersion;
   qcc::String downloadUrl,
   bool upgradeFlag;
   qcc::string appUserId;

   
}
void PkgManagerBusObject::UninstallApp(const InterfaceDescription::Member* member, Message& msg)
{
   qcc::String appName;
   qcc::String userId;

   
}

} // namespace gw
} // namespace ajn

