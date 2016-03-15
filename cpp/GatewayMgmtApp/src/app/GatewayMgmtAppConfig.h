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

#ifndef GATEWAYMGMTAPPCONFIG_H_
#define GATEWAYMGMTAPPCONFIG_H_

#include <alljoyn/Status.h>
#include <libxml/tree.h>
#include <libxml/xmlwriter.h>

namespace ajn {
namespace gw {

/**
 * Class to prase Management App Configurations
 */
class GatewayMgmtAppConfig {

  public:

    /**
     * Constructor for GatewayMgmtAppConfig
     */
    GatewayMgmtAppConfig();

    /**
     * Destructor for GatewayMgmtAppConfig
     */
    virtual ~GatewayMgmtAppConfig();

    /**
     * Load the values of App Config from a file
     * @param fileName - file used to parse
     * @return status - success/failure
     */
    QStatus loadFromFile(qcc::String const& fileName);

    /**
     * Set the AllJoyn Passcode fo the Gateway Mangement App
     */
    QStatus setAlljoynPasscode(const qcc::String& passcode);

    /**
     * Get the Language for the Gateway Management App
     * @return the language
     */
    const qcc::String& getLanguage() const;

    /**
     * Get the Device Name for the Gateway Management App
     * @return the deviceName
     */
    const qcc::String& getDeviceName() const;

    /**
     * Get the AppName for the Gateway Management App
     * @return the appName
     */
    const qcc::String& getAppName() const;

    /**
     * Get the SupportUrl for the Gateway Management App
     * @return the supportUrl
     */
    const qcc::String& getSupportUrl() const;

    /**
     * Get the Manufacturer for the Gateway Management App
     * @return the manufacturer
     */
    const qcc::String& getManufacturer() const;

    /**
     * Get the ModelNumber for the Gateway Management App
     * @return the modelNumber
     */
    const qcc::String& getModelNumber() const;

    /**
     * Get the SoftwareVersion for the Gateway Management App
     * @return the softwareVersion
     */
    const qcc::String& getSoftwareVersion() const;

    /**
     * Get the Description for the Gateway Management App
     * @return the description
     */
    const qcc::String& getDescription() const;

    /**
     * Get the AllJoynPasscode for the Gateway Management App
     * @return the AllJoynPasscode
     */
    const qcc::String& getAlljoynPasscode() const;

  private:

    /**
     * The configuration file path
     */
    qcc::String m_filePath;

    /**
     * The language of the Gateway Management App
     */
    qcc::String m_Language;

    /**
     * The DeviceName of the Gateway Management App
     */
    qcc::String m_DeviceName;

    /**
     * The AppName of the Gateway Management App
     */
    qcc::String m_AppName;

    /**
     * The SupportUrl of the Gateway Management App
     */
    qcc::String m_SupportUrl;

    /**
     * The Manufacturer of the Gateway Management App
     */
    qcc::String m_Manufacturer;

    /**
     * The ModelNumber of the Gateway Management App
     */
    qcc::String m_ModelNumber;

    /**
     * The SoftwareVersion of the Gateway Management App
     */
    qcc::String m_SoftwareVersion;

    /**
     * The Description of the Gateway Management App
     */
    qcc::String m_Description;

    /**
     * The AllJoynPasscode used to authenticate
     * with the Gateway Management App
     */
    qcc::String m_AlljoynPasscode;

};

} /* namespace gw */
} /* namespace ajn */

#endif /* GATEWAYMGMTAPPCONFIG_H_ */
