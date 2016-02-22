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
#include "PkgManagerBusObject.h"

namespace ajn {
namespace gw {

using namespace qcc;
using namespace gwConsts;

PkgManagerBusObject::PkgManagerBusObject(BusAttachment* bus, QStatus* status) :
   BusObject("/gw/PackageManager"), m_PackageManager(NULL)
{
    *status = createPkgManagerInterface(bus);
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
        status = bus->CreateInterface(AJ_PKG_MNGR_INTERFACE.c_str(), interfaceDescription, true);
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

    const ajn::InterfaceDescription::Member* methodMember = interfaceDescription->GetMember(AJ_METHOD_INSTALL_APP.c_str());
    status = AddMethodHandler(methodMember, static_cast<MessageReceiver::MethodHandler>(&PkgManagerBusObject::InstallApp));
    if (status != ER_OK) {
        QCC_LogError(status, ("Could not register the GetManifestFile MethodHandler"));
        return status;
    }

    methodMember = interfaceDescription->GetMember(AJ_METHOD_UNINSTALL_APP.c_str());
    status = AddMethodHandler(methodMember, static_cast<MessageReceiver::MethodHandler>(&PkgManagerBusObject::UninstallApp));
    if (status != ER_OK) {
        QCC_LogError(status, ("Could not register the GetManifestInterfaces MethodHandler"));
        return status;
    }

    QCC_DbgTrace(("Created PkgManager BusObject successfully"));

    return status;
    
}

void PkgManagerBusObject::InstallApp(const InterfaceDescription::Member* member, Message& msg)
{
   QCC_UNUSED(member);
   qcc::String downloadUrl;
   bool upgradeFlag;

   const MsgArg* currentArg = NULL;
   qcc::String appUserId;
   MsgArg outArg("b", false);
   QStatus status = ER_OK;

   currentArg = msg->GetArg(0);
   if (currentArg) {
      downloadUrl = currentArg->v_string.str;
   } else {
      status = ER_BAD_ARG_2;
      goto connAppInstallFailed;
   }

   currentArg = msg->GetArg(1);
   if (currentArg) {
      upgradeFlag = currentArg->v_string.v_bool;
   } else {
      status = ER_BAD_ARG_2;
      goto connAppInstallFailed;
   }

   m_PackageManager->InstallApp(downloadUrl, upgradeFlag, status);
   if (status == ER_OK) {
      outArg.Set("b", true);
      status = MethodReply(msg, &outArg, 1);
      if (status != ER_OK) {
         QCC_LogError(status, (("Failed to send reply")));
      }
   }

connAppInstallFailed:
      QCC_LogError(status, ("Invalid argument passed to UninstallApp"));

      status = MethodReply(msg, &outArg, 1);
      if (status != ER_OK) {
         QCC_LogError(status, (("Failed to send reply")));
      }
      return;
}
void PkgManagerBusObject::UninstallApp(const InterfaceDescription::Member* member, Message& msg)
{
   QCC_UNUSED(member);
   qcc::String appId;
   qcc::String userId;
   qcc::String groupId;
   const MsgArg* currentArg = NULL;
   QStatus status = ER_OK;
   MsgArg outArg("b", false);

   currentArg = msg->GetArg(0);
   if (currentArg) {
      appId = currentArg->v_string.str;
   } else {
      status = ER_BAD_ARG_1;
      goto connAppUninstallFailed;
   }

   currentArg = msg->GetArg(1);
   if (currentArg) {
      userId = currentArg->v_string.str;
   } else {
      status = ER_BAD_ARG_2;
      goto connAppUninstallFailed;
   }

   currentArg = msg->GetArg(2);
   if (currentArg) {
      groupId = currentArg->v_string.str;
   } else {
      status = ER_BAD_ARG_2;
      goto connAppUninstallFailed;
   }

   m_PackageManager->UninstallApp(appId, status);
   if (status == ER_OK) {
      outArg.Set("b", true);
      status = MethodReply(msg, &outArg, 1);
      if (status != ER_OK) {
         QCC_LogError(status, ("Failed to send reply"));
      }
      return;
   }

connAppUninstallFailed:
      QCC_LogError(status, ("Invalid argument passed to UninstallApp"));

      status = MethodReply(msg, &outArg, 1);
      if (status != ER_OK) {
         QCC_LogError(status, (("Failed to send reply")));
      }
      return;
}

} // namespace gw
} // namespace ajn

