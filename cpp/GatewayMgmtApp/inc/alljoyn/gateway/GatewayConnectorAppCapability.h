/******************************************************************************
 * Copyright (c) 2016 Open Connectivity Foundation (OCF) and AllJoyn Open
 *    Source Project (AJOSP) Contributors and others.
 *
 *    SPDX-License-Identifier: Apache-2.0
 *
 *    All rights reserved. This program and the accompanying materials are
 *    made available under the terms of the Apache License, Version 2.0
 *    which accompanies this distribution, and is available at
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Copyright 2016 Open Connectivity Foundation and Contributors to
 *    AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for
 *    any purpose with or without fee is hereby granted, provided that the
 *    above copyright notice and this permission notice appear in all
 *    copies.
 *
 *     THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 *     WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 *     WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 *     AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 *     DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 *     PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 *     TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 *     PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

#ifndef GatewayConnectorAppCapability_H_
#define GatewayConnectorAppCapability_H_

#include <qcc/String.h>
#include <vector>

namespace ajn {
namespace gw {

/**
 * Class used to define an ObjectDescription
 */
class GatewayConnectorAppCapability {

  public:

    /**
     * struct to define an Interface, its secure flag and its friendly Name
     */
    typedef struct {
        qcc::String interfaceName;           ///< The name of the interface
        qcc::String interfaceFriendlyName;   ///< The friendly name of the interface
        bool isSecured;                      ///< The secured flag of the interface
    } InterfaceDesc;

    /**
     * Constructor for the GatewayConnectorAppCapability class
     */
    GatewayConnectorAppCapability();

    /**
     * Constructor for the GatewayConnectorAppCapability class
     * @param objectPath - objectPath of the ObjectDescription
     * @param objectPathFriendly - friendly name of the objectPath of the ObjectDescription
     * @param isPrefix - is the objectPath a Prefix
     * @param interfaces - interfaces of the ObjectDescription
     */
    GatewayConnectorAppCapability(qcc::String const& objectPath, qcc::String const& objectPathFriendly, bool isPrefix,
                                  std::vector<InterfaceDesc> const& interfaces);

    /**
     * Destructor of the GatewayConnectorAppCapability class
     */
    virtual ~GatewayConnectorAppCapability();

    /**
     * Get the ObjectPath of the ObjectDescription
     * @return objectPath
     */
    const qcc::String& getObjectPath() const;

    /**
     * Set the ObjectPath of the ObjectDescription
     * @param objectPath
     */
    void setObjectPath(const qcc::String& objectPath);

    /**
     * Get the IsObjectPathPrefix flag of the ObjectDescription
     * @return true/false
     */
    bool getIsObjectPathPrefix() const;

    /**
     * Set the IsObjectPathPrefix flag of the ObjectDescription
     * @param isObjectPathPrefix - set whether the objectPath is a prefix
     */
    void setIsObjectPathPrefix(bool isObjectPathPrefix);

    /**
     * Get the interfaces of the ObjectDescription
     * @return interfaces vector
     */
    const std::vector<InterfaceDesc>& getInterfaces() const;

    /**
     * Set the interfaces of the ObjectDescription
     * @param interfaces
     */
    void setInterfaces(const std::vector<InterfaceDesc>& interfaces);

    /**
     * Get the ObjectPath FriendlyName of the ObjectDescription
     * @return objectPath
     */
    const qcc::String& getObjectPathFriendlyName() const;

    /**
     * Set the ObjectPath Friendly Name of the ObjectDescription
     * @param objectPathFriendlyName - set the objectPathFriendlyName
     */
    void setObjectPathFriendlyName(const qcc::String& objectPathFriendlyName);

  private:

    /**
     * The ObjectPath of the ObjectDescription
     */
    qcc::String m_ObjectPath;

    /**
     * The ObjectPath FriendlyName of the ObjectDescription
     */
    qcc::String m_ObjectPathFriendlyName;

    /**
     * Is the ObjectPath a Prefix
     */
    bool m_IsObjectPathPrefix;

    /**
     * The Interfaces of the ObjectDescription
     */
    std::vector<InterfaceDesc> m_Interfaces;
};

} /* namespace gw */
} /* namespace ajn */

#endif /* GatewayConnectorAppCapability_H_ */