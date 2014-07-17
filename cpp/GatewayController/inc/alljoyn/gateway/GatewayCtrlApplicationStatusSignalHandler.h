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

#ifndef GatewayCtrlApplicationStatusSignalHandler_H
#define GatewayCtrlApplicationStatusSignalHandler_H

#include <alljoyn/gateway/GatewayCtrlConnectorApplicationStatus.h>

namespace ajn {
namespace services {
/**
 * Implement this interface to be notified about changes in the Third Party Application status
 */
class GatewayCtrlApplicationStatusSignalHandler {
  public:
    GatewayCtrlApplicationStatusSignalHandler() { }
    virtual ~GatewayCtrlApplicationStatusSignalHandler() { }
    /**
     * The event is emitted when the status of the Third Party Application
     * changes. Avoid blocking the thread on which the method is called.
     * @param appId The application id
     * @param ConnectorApplicationStatus {@link GatewayCtrlConnectorApplicationStatus}
     */
    virtual void onStatusChanged(const qcc::String& appId, const GatewayCtrlConnectorApplicationStatus*ConnectorApplicationStatus) = 0;

    /**
     * An event could not be emitted because of an error creating its data
     * @param appId The application id
     * @param status {@link ConnectorApplicationStatus}
     */
    virtual void onError(const qcc::String& appId, const QStatus& status) = 0;
};
}
}

#endif /* defined(GatewayCtrlApplicationStatusSignalHandler_H) */
