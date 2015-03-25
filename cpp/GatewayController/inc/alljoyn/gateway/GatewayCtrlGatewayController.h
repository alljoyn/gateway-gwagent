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

#ifndef GatewayCtrlGatewayController_H
#define GatewayCtrlGatewayController_H

#include <vector>
#include <qcc/String.h>
#include <alljoyn/Status.h>
#include <alljoyn/gateway/GatewayCtrlGateway.h>
#include <alljoyn/gateway/GatewayCtrlGateway.h>
#include <alljoyn/about/AnnounceHandler.h>



namespace ajn {
namespace services {
/**
 * This class includes the main functionality for the Gateway Controller Application
 */
class GatewayCtrlGatewayController {
  public:

    /**
     * GetInstance
     * @return The {@link GatewayController} object
     */
    static GatewayCtrlGatewayController*getInstance();

    /**
     * Initialize the gateway controller
     * @param bus {@link BusAttachment} to use
     */
    static void Init(BusAttachment*bus);

    /**
     * Shutdown the gateway controller
     */
    void Shutdown();

    /**
     * @return {@link BusAttachment} that is used by the {@link GatewayController}
     */
    BusAttachment* GetBusAttachment();


    /**
     * create a Gateway by parsing announce descriptions.
     * @param gatewayBusName - BusName of device received in announce
     * @param objectDescs - ObjectDescriptions received in announce
     * @return a GatewayCtrlGateway
     */
    GatewayCtrlGateway* CreateGateway(qcc::String const& gatewayBusName, const AnnounceHandler::ObjectDescriptions& objectDescs, const AnnounceHandler::AboutData& aboutData);

    /**
     * getGateway - get a Gateway using the busName
     * @param deviceBusName - gatewayBusName to get
     * @return GatewayCtrlGateway* - returns the Gateway or NULL if not found
     */
    GatewayCtrlGateway* getGateway(qcc::String const& gatewayBusName);

    /**
     * deleteGateway - delete a GateWay from the map
     * @param gatewayBusName - deviceName to delete
     * @return status - success-failure
     */
    QStatus deleteGateway(qcc::String const& gatewayBusName);

    /**
     * deleteAllGateways - shutdown and delete all GateWays from the controller
     * @return status - success-failure
     */
    QStatus deleteAllGateways();

    /**
     * Get map of All Gateways
     * @return controllable Devices map
     */
    const std::map<qcc::String, GatewayCtrlGateway*>& getGateways() const;

    /**
     * @return Status of release
     */
    QStatus Release();

  private:

    /**
     * Constructor
     * @param bus
     * @param store
     */
    GatewayCtrlGatewayController(BusAttachment* bus);
    /**
     * Desctructor
     */
    virtual ~GatewayCtrlGatewayController() { }
    /**
     *  pointer to AboutServiceApi
     */
    static GatewayCtrlGatewayController* m_instance;

    /**
     * The BusAttachment to be used
     */
    static BusAttachment* m_Bus;


    std::map<qcc::String, GatewayCtrlGateway*> m_Gateways;

    void EmptyMap();
};
}
}
#endif /* defined(GatewayCtrlGatewayController_H) */