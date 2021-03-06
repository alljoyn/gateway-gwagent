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

#ifndef AclRules_H
#define AclRules_H

#include <map>
#include <vector>
#include <qcc/String.h>
#include <alljoyn/gateway/RemotedApp.h>
#include <alljoyn/gateway/RuleObjectDescription.h>
#include <alljoyn/gateway/ConnectorCapabilities.h>


namespace ajn {
namespace gwc {
class AclRules {
  public:
    /**
     * Constructor - must call appropriate init
     */
    AclRules();


    /**
     * init
     * @param exposedServicesArrayArg MsgArg containing the exposed services
     * @param remotedAppsArrayArg MsgArg containing the remoted apps
     * @param connectorCapabilities map of manifest rules for this connector app
     * @param internalMetadata internal metadata information from the server
     * @return {@link QStatus}
     */
    QStatus init(const MsgArg*exposedServicesArrayArg, const MsgArg*remotedAppsArrayArg, const ConnectorCapabilities& connectorCapabilities, const std::map<qcc::String, qcc::String>& internalMetadata);


    /**
     * init
     * @param exposedServices The interfaces that Connector App exposes to its clients
     * @param remotedApps The applications that may be reached by the Connector App
     * via the configured interfaces and object paths
     */
    QStatus init(std::vector<RuleObjectDescription*> const& exposedServices, std::vector<RemotedApp*> const& remotedApps);

    /**
     * Destructor
     */
    virtual ~AclRules();

    /**
     * The applications that may be reached by the Connector App
     * via the configured interfaces and object paths
     * @return List of the remoted applications
     */
    const std::vector<RemotedApp*>& getRemotedApps();

    /**
     * The interfaces that Connector App exposes to its clients
     * @return List of exposed services
     */
    const std::vector<RuleObjectDescription*>& getExposedServices();


    /**
     * Set the given metadata to the given one
     * @param metadata
     */
    void setMetadata(std::map<qcc::String, qcc::String> const& metadata);

    /**
     * Returns metadata value for the given key
     * @param key The metadata key
     * @return Metadata value or NULL if not found
     */
    qcc::String*getMetadata(const qcc::String& key);

    /**
     * Returns current metadata object
     * @return metadata
     */
    const std::map<qcc::String, qcc::String>& getMetadata();


    /**
     * release allocations and empty object. must be called before deletion of object.
     * @return {@link QStatus}
     */
    QStatus release();

  private:
    /**
     * This {@link AclRules} metadata
     */
    std::map<qcc::String, qcc::String> m_Metadata;


    /**
     * The interfaces that the Connector App exposes to its clients
     */
    std::vector<RuleObjectDescription*> m_ExposedServices;

    /**
     * The applications that may be reached by the Connector App
     * via the configured interfaces and object paths
     */
    std::vector<RemotedApp*> m_RemotedApps;

    void emptyVectors();
};
}
}
#endif /* defined(AclRules_H) */
