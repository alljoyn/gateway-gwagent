#include "GatewayMgmtAppConfigListener.h"
#include "../GatewayConstants.h"
#include <alljoyn/gateway/GatewayBusListener.h>
#include <vector>

#define DEFAULT_PASSCODE "000000"

namespace ajn {
namespace gw {

using namespace ajn;

GatewayMgmtAppConfigListener::GatewayMgmtAppConfigListener(
    common::SrpKeyXListener* keyListener,
    BusAttachment* bus,
    GatewayBusListener* busListener,
    GatewayMgmtAppConfig* gatewayConfig) :
    m_KeyListener(keyListener),
    m_Bus(bus),
    m_BusListener(busListener),
    m_GatewayConfig(gatewayConfig)
{
}

GatewayMgmtAppConfigListener::~GatewayMgmtAppConfigListener()
{
    m_KeyListener = NULL;
    m_Bus = NULL;
    m_BusListener = NULL;
}

QStatus GatewayMgmtAppConfigListener::FactoryReset()
{
    m_KeyListener->setPassCode(DEFAULT_PASSCODE);
    m_GatewayConfig->setAlljoynPasscode(qcc::String(DEFAULT_PASSCODE));
    return ER_OK;
}

QStatus GatewayMgmtAppConfigListener::Restart()
{
    return ER_OK;
}

QStatus GatewayMgmtAppConfigListener::SetPassphrase(
    const char*daemonRealm,
    size_t passcodeSize,
    const char*passcode,
    SessionId sessionId
    )
{
    QCC_UNUSED(daemonRealm);
    qcc::String passCodeStr(passcode, passcodeSize);
    m_KeyListener->setPassCode(passCodeStr);
    m_GatewayConfig->setAlljoynPasscode(passCodeStr);

    // Clear the key store and close any sessions other than the current one
    // to force them to reauthenticate
    m_Bus->ClearKeyStore();
    m_Bus->EnableConcurrentCallbacks();

    std::vector<SessionId> sessionIds = m_BusListener->getSessionIds();
    for (size_t i = 0; i < sessionIds.size(); i++) {
        if (sessionIds[i] == sessionId) {
            continue;
        }
        m_Bus->LeaveSession(sessionIds[i]);
    }

    return ER_OK;
}

}
}
