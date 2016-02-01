#include "GatewayMgmtAppConfigListener.h"
#include "../GatewayConstants.h"
#include <alljoyn/gateway/GatewayBusListener.h>
#include <alljoyn/gateway/common/SrpKeyXListener.h>
#include <vector>

#define DEFAULT_PASSCODE "000000"

namespace ajn {
namespace gw {

using namespace ajn;

GatewayMgmtAppConfigListener::GatewayMgmtAppConfigListener(
        SrpKeyXListener* keyListener,
        BusAttachment* bus,
        GatewayBusListener* busListener) :
    m_KeyListener(keyListener),
    m_Bus(bus),
    m_BusListener(busListener)
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
    return ER_OK;
}

QStatus GatewayMgmtAppConfigListener::Restart()
{
    return ER_OK;    
}

QStatus GatewayMgmtAppConfigListener::SetPassphrase(
                const char *daemonRealm,
                size_t passcodeSize,
                const char *passcode,
                SessionId sessionId
                )
{
    QCC_UNUSED(daemonRealm);
    QCC_UNUSED(passcodeSize);
    qcc::String passCodeStr(passcode);
    m_KeyListener->setPassCode(passCodeStr);

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
