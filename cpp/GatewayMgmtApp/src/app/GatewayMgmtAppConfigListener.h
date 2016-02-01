#include <alljoyn/config/ConfigService.h>
#include <alljoyn/config/AboutDataStoreInterface.h>
#include <alljoyn/BusAttachment.h>
#include "GatewayMgmtAppConfig.h"

#ifndef GATEWAYMGMTAPPCONFIGLISTENER_H_
#define GATEWAYMGMTAPPCONFIGLISTENER_H_

namespace ajn {
namespace gw {

class SrpKeyXListener;
class GatewayBusListener;

class GatewayMgmtAppConfigListener : public ajn::services::ConfigService::Listener {
    public:
        GatewayMgmtAppConfigListener(
                SrpKeyXListener* keyListener,
                ajn::BusAttachment* bus,
                GatewayBusListener* busListener,
                GatewayMgmtAppConfig* gatewayConfig
                );
        ~GatewayMgmtAppConfigListener();
        virtual QStatus Restart();
        virtual QStatus FactoryReset();
        virtual QStatus SetPassphrase(
                const char *daemonRealm,
                size_t passcodeSize,
                const char *passcode,
                ajn::SessionId sessionId
                );
    private:
        GatewayMgmtAppConfigListener();
        SrpKeyXListener* m_KeyListener;
        ajn::BusAttachment* m_Bus;
        GatewayBusListener* m_BusListener;
        GatewayMgmtAppConfig* m_GatewayConfig;
};

class GatewayMgmtAppDataStore : public AboutDataStoreInterface {
    public:
        GatewayMgmtAppDataStore (const char *factoryConfigFile, const char *configFile) : AboutDataStoreInterface(factoryConfigFile, configFile)
        {
            QCC_UNUSED(factoryConfigFile);
            QCC_UNUSED(configFile);
        }
        virtual ~GatewayMgmtAppDataStore() {}
        virtual void FactoryReset() { }
        virtual QStatus ReadAll(const char *languageTag, DataPermission::Filter filter, ajn::MsgArg &all)
        {
            QCC_UNUSED(languageTag);
            QCC_UNUSED(filter);
            QCC_UNUSED(all);
            return ER_OK;
        }
        virtual QStatus Update(const char *name, const char *languageTag, const ajn::MsgArg *value)
        {
            QCC_UNUSED(name);
            QCC_UNUSED(languageTag);
            QCC_UNUSED(value);
            return ER_OK;
        }
        virtual QStatus Delete(const char *name, const char *languageTag)
        {
            QCC_UNUSED(name);
            QCC_UNUSED(languageTag);
            return ER_OK;
        }
};


} // namespace gw
} // namespace ajn

#endif // GATEWAYMGMTAPPCONFIG_H_
