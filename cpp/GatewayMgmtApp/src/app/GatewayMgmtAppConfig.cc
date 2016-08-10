/******************************************************************************
 * Copyright (c) Open Connectivity Foundation (OCF) and AllJoyn Open
 *    Source Project (AJOSP) Contributors and others.
 *
 *    SPDX-License-Identifier: Apache-2.0
 *
 *    All rights reserved. This program and the accompanying materials are
 *    made available under the terms of the Apache License, Version 2.0
 *    which accompanies this distribution, and is available at
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Copyright (c) Open Connectivity Foundation and Contributors to AllSeen
 *    Alliance. All rights reserved.
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

#include <qcc/String.h>
#include <qcc/Debug.h>
#include <qcc/platform.h>
#include <libxml/parser.h>
#include "GatewayMgmtAppConfig.h"
#include "../GatewayConstants.h"
#include <fstream>
#include <sstream>

namespace ajn {
namespace gw {

using namespace qcc;

GatewayMgmtAppConfig::GatewayMgmtAppConfig()
{
}

GatewayMgmtAppConfig::~GatewayMgmtAppConfig()
{
}

QStatus GatewayMgmtAppConfig::loadFromFile(qcc::String const& fileName)
{
    m_filePath.assign(fileName.c_str());
    std::ifstream ifs(m_filePath.c_str());
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));

    if (content.empty()) {
        QCC_DbgHLPrintf(("Could not read managment app config file"));
        return ER_READ_ERROR;
    }

    xmlParserCtxtPtr ctxt = xmlNewParserCtxt();
    if (ctxt == NULL) {
        QCC_DbgHLPrintf(("Could not create Parser Context"));
        return ER_OUT_OF_MEMORY;
    }

    xmlDocPtr doc = xmlCtxtReadMemory(ctxt, content.c_str(), content.size(), NULL, NULL, XML_PARSE_NOERROR | XML_PARSE_NOBLANKS);
    if (doc == NULL) {
        QCC_DbgHLPrintf(("Could not parse XML from file"));
        xmlFreeParserCtxt(ctxt);
        return ER_XML_MALFORMED;
    }

    if (ctxt->valid == 0) {
        QCC_DbgHLPrintf(("Invalid XML - validation failed"));
        xmlFreeParserCtxt(ctxt);
        xmlFreeDoc(doc);
        return ER_BUS_BAD_XML;
    }

    xmlNode* root_element = xmlDocGetRootElement(doc);
    for  (xmlNode* currentKey = root_element->children; currentKey != NULL; currentKey = currentKey->next) {

        if (currentKey->type != XML_ELEMENT_NODE || currentKey->children == NULL) {
            continue;
        }

        const xmlChar* keyName = currentKey->name;
        const xmlChar* value = currentKey->children->content;

        if (xmlStrEqual(keyName, (const xmlChar*)"language")) {
            m_Language.assign((const char*)value);
        } else if (xmlStrEqual(keyName, (const xmlChar*)"deviceName")) {
            m_DeviceName.assign((const char*)value);
        } else if (xmlStrEqual(keyName, (const xmlChar*)"appName")) {
            m_AppName.assign((const char*)value);
        } else if (xmlStrEqual(keyName, (const xmlChar*)"supportUrl")) {
            m_SupportUrl.assign((const char*)value);
        } else if (xmlStrEqual(keyName, (const xmlChar*)"manufacturer")) {
            m_Manufacturer.assign((const char*)value);
        } else if (xmlStrEqual(keyName, (const xmlChar*)"modelNumber")) {
            m_ModelNumber.assign((const char*)value);
        } else if (xmlStrEqual(keyName, (const xmlChar*)"softwareVersion")) {
            m_SoftwareVersion.assign((const char*)value);
        } else if (xmlStrEqual(keyName, (const xmlChar*)"description")) {
            m_Description.assign((const char*)value);
        } else if (xmlStrEqual(keyName, (const xmlChar*)"alljoynPasscode")) {
            m_AlljoynPasscode.assign((const char*)value);
        }
    }

    xmlFreeParserCtxt(ctxt);
    xmlFreeDoc(doc);
    return ER_OK;
}

QStatus GatewayMgmtAppConfig::setAlljoynPasscode(const qcc::String& passcode)
{
    QStatus status = ER_FAIL;
    std::ifstream ifs(m_filePath.c_str());
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));

    if (content.empty()) {
        QCC_DbgHLPrintf(("Could not read management app config file"));
        return ER_READ_ERROR;
    }

    xmlParserCtxtPtr ctxt = xmlNewParserCtxt();
    if (ctxt == NULL) {
        QCC_DbgHLPrintf(("Could not create Parser Context"));
        return ER_OUT_OF_MEMORY;
    }

    xmlDocPtr doc = xmlCtxtReadMemory(ctxt, content.c_str(), content.size(), NULL, NULL, XML_PARSE_NOERROR | XML_PARSE_NOBLANKS);
    if (doc == NULL) {
        QCC_DbgHLPrintf(("Could not parse XML from file"));
        xmlFreeParserCtxt(ctxt);
        return ER_XML_MALFORMED;
    }

    if (ctxt->valid == 0) {
        QCC_DbgHLPrintf(("Invalid XML - validation failed"));
        xmlFreeParserCtxt(ctxt);
        xmlFreeDoc(doc);
        return ER_BUS_BAD_XML;
    }

    xmlNode* root_element = xmlDocGetRootElement(doc);
    for  (xmlNode* currentKey = root_element->children; currentKey != NULL; currentKey = currentKey->next) {

        if (currentKey->type != XML_ELEMENT_NODE || currentKey->children == NULL) {
            continue;
        }

        const xmlChar* keyName = currentKey->name;

        if (xmlStrEqual(keyName, (const xmlChar*)"alljoynPasscode")) {
            xmlNodeSetContent(currentKey, (xmlChar*) passcode.c_str());
            break;
        }
    }
    xmlSaveFormatFile(m_filePath.c_str(), doc, 1);
    xmlFreeParserCtxt(ctxt);
    xmlFreeDoc(doc);

    return status;
}

const qcc::String& GatewayMgmtAppConfig::getLanguage() const
{
    return m_Language;
}

const qcc::String& GatewayMgmtAppConfig::getDeviceName() const
{
    return m_DeviceName;
}

const qcc::String& GatewayMgmtAppConfig::getAppName() const
{
    return m_AppName;
}

const qcc::String& GatewayMgmtAppConfig::getSupportUrl() const
{
    return m_SupportUrl;
}

const qcc::String& GatewayMgmtAppConfig::getManufacturer() const
{
    return m_Manufacturer;
}

const qcc::String& GatewayMgmtAppConfig::getModelNumber() const
{
    return m_ModelNumber;
}

const qcc::String& GatewayMgmtAppConfig::getSoftwareVersion() const
{
    return m_SoftwareVersion;
}

const qcc::String& GatewayMgmtAppConfig::getDescription() const
{
    return m_Description;
}

const qcc::String& GatewayMgmtAppConfig::getAlljoynPasscode() const
{
    return m_AlljoynPasscode;
}

}
}