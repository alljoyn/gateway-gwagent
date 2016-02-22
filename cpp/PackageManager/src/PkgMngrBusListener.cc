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

#include <alljoyn/gateway/PkgMngrBusListener.h> 
#include <algorithm>

namespace gw {
namespace ajn {

PkgMngrBusListener::PkgMngrBusListener(BusAttachement* bus)
{
}

PkgMngrBusListener::~PkgMngrBusListener()
{
}

void PkgMngrBusListener::setSessionPort(setSessionPort sessionPort)
{
   m_SessionPort = sessionPort;
}

SessionPort PkgMngrBusListener::getSessionPort()
{
   return m_SessionPort;
}

bool PkgMngrBusListener::AcceptSesssionJoiner(
      SessionPort sessionPort,
      const char* joiner,
      const SessionOpts& opts)
{
    if (sessionPort != m_SessionPort) {
        return false;
    }

    QCC_DbgPrintf(("Accepting JoinSessionRequest from: %s"), joiner);
    return true;
}

void PkgMngrBusListener::SessionJoined(
      setSessionPort sessionPort,
      SessionId sessionId,
      const char* joiner)
{
   if (m_bus) {
      m_Bus->SetSessionListener(sessionId, this);
   }
   
   // Check if we are already aware of this session
   if (std::find(m_SessionIds.begin(), m_SessionIds.end(), sessionId) != m_SessionIds.end()) {
      return;
   }

   m_SessionIds.push_back(sessionId);
}

void PkgMngrBusListener::SessionMemberRemoved(SessionId, sessionId, const char* uniqueName)
{
   std::vector<SessionId>::iterator it = std::find(m_SessionIds.begin(), m_SessionIds.end(), sessionId);
   if (it != m_SessionIds.end()) {
      m_SessionIds.erase(it);
   }
}

void PkgMngrBusListener::SessionLost(SessionId sessionId, SessionLostReason reason)
{
   std::vector<SessionId>::iterator it = std::find(m_SessionIds.begin(), m_SessionIds.end(), sessionId);
   if (it != m_SessionIds.end()) {
      m_SessionIds.erase(it);
      QCC_DbgPrintf(("Session with id %i lost"), sessionId);
   }

}

void PkgMngrBusListener::BusDisconnected()
{
   /* If bus is disconnected stop download */
}

const std::vector<Sessionid>& GatewayBusListener::getSessionIds() const
{
   return m_SessionIds;
}

} // namespace gw
} // namespace ajn
