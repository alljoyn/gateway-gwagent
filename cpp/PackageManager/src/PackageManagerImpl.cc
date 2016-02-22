/**
 * @file
 *
 * This file is the main implementaion of the Package Manager interface
 * defined in HLD 3.3.2
 */

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

#include <alljoyn/gateway/PackageManagerConstants.h>
#include <alljoyn/gateway/PackageManagerImpl.h>
#include <alljoyn/gateway/SslDownloader.h>
#include <alljoyn/gateway/PackageVerifier.h>
#include <alljoyn/gateway/PmLogModule.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <libxml/tree.h>
#include <map>
#include <vector>
#include <qcc/KeyBlob.h>
#include <qcc/Util.h>
#include <sstream>
#include <sys/types.h>
#include <dirent.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlwriter.h>

namespace ajn {
namespace gw {

using namespace qcc;
using namespace std;
using namespace pm_constants;

PackageManagerImpl::PackageManagerImpl()
{
    //ensure that the basic alljoyn directories exist
    if (!PmUtils::DirectoryExists(GATEWAY_APPS_DIRECTORY)) {
        PmUtils::CreateDir(GATEWAY_APPS_DIRECTORY);
    }
}

PackageManagerImpl::~PackageManagerImpl()
{
}

qcc::String PackageManagerImpl::InstallApp(
    const String& downloadUrl,
    const String& fileHash,
    bool upgradeFlag,
    QStatus& responseStatus)
{
    responseStatus = pmUtils.InitTempDir("alljoyn.pkgmanager.");
    if (ER_OK != responseStatus) {
        QCC_LogError(responseStatus,
                     ("Unable create temporary directory for package installation"));
        return "";
    }

    String downloadLocation = pmUtils.getTempDirName() + "/pkg.tar";
    responseStatus = DownloadFile(downloadUrl, downloadLocation, false);  //Last argument is ignored in non-debug builds

    if (ER_OK != responseStatus) {
        QCC_LogError(responseStatus,
                     ("Unable to download package: %s  ", downloadLocation.c_str()));
        return "";
    }

    responseStatus = VerifyAppPackage("pkg.tar", fileHash, VERIFICATION_CERTIFICATE_LOCATION);
    if (ER_OK != responseStatus) {
        QCC_LogError(responseStatus, ("Unable to verify package download: %s  ", packageName.c_str()));
        return "";
    }

    responseStatus = pmUtils.ExtractInnerTarToDir("pkg.tar", pmUtils.getTempDirName() + "/pkg");
    if (ER_OK != responseStatus) {
        QCC_LogError(responseStatus, ("Unable to extract AJ package"));
        return "";
    }
    qcc::String manifestFilePath = pmUtils.getTempDirName() + "/pkg/Manifest.xml";
    std::ifstream ifs(manifestFilePath.c_str());
    if (!ifs) {
        QCC_LogError(ER_READ_ERROR, ("Unable to find package Manifest file."));
        return "";

    }
    std::string content((std::istreambuf_iterator<char>(ifs)),
            (std::istreambuf_iterator<char>()));
    ifs.close();

    if (content.empty()) {
        QCC_LogError(ER_READ_ERROR, ("Unable to read package Manifest file."));
        return "";
    }
    
    xmlDocPtr doc = xmlParseMemory(content.c_str(), content.size());
    if (doc == NULL) {
        QCC_LogError(ER_OS_ERROR, ("Could not parse XML from memory"));
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return "";
    }

    xmlNode* root_element = xmlDocGetRootElement(doc);

    qcc::String appId;

    for(xmlNode* currentKey = root_element->children; currentKey != NULL; currentKey = currentKey->next) {

        if (currentKey->type != XML_ELEMENT_NODE || currentKey->children == NULL) {
            continue;
        }

        const xmlChar* keyName = currentKey->name;
        const xmlChar* value = currentKey->children->content;

        if (xmlStrEqual(keyName, (const xmlChar*)"connectorId")) {
            appId.assign((const char*) value);
        }
    }

    if (upgradeFlag) {
        String appDir = GATEWAY_APPS_DIRECTORY + "/" + appId;
        String cmd = "rm -rf " + appDir + "/";
        responseStatus = pmUtils.ExecuteCmdLine(cmd.c_str());
        if (ER_OK != responseStatus) {
            QCC_LogError(responseStatus,
                         ("Unable to remove package at: %s prior to update", appDir.c_str()));
            return "";
        }
    } else {   //TODO: if PM is  not required create the application's user then this code should be deleted
               //create user appId
        stringstream usercmdss;
        usercmdss <<  "useradd -u " <<  appId << " -g " << APP_GROUP_ID << " " << appId.c_str();  //fails if user already exists
        responseStatus = pmUtils.ExecuteCmdLine(usercmdss.str().c_str());
    }

    if (ER_OK != responseStatus) {
        QCC_LogError(responseStatus, ("Unable to add package for : %s ", appId.c_str()));
        return "";
    }

    //todo: the temp mgr now contains the inner tar, so complete the installation by expanding that into the target folder
    vector<String> filenames;
    pmUtils.GetTempDirFiles(filenames);

    //create directories for this app
    String mkdir;
    mkdir = GATEWAY_APPS_DIRECTORY + "/" + appId;
    responseStatus = pmUtils.CreateDir(mkdir);
    if (ER_OK != responseStatus) {
        QCC_LogError(responseStatus, ("Unable to create AJ package directory: %s", mkdir.c_str()));
        return "";
    }

    // set ownership of this app
    String cmd = "chown -R " + appId + " " + GATEWAY_APPS_DIRECTORY + "/" + appId;
    responseStatus = pmUtils.ExecuteCmdLine(cmd);
    if (ER_OK != responseStatus) {
        QCC_LogError(responseStatus, ("unable to set ownership of application"));;
    }

    // set write permissions on bin and lib directories
    cmd = "chmod -R a+rx " +  GATEWAY_APPS_DIRECTORY + "/" + appId + "/lib";
    pmUtils.ExecuteCmdLine(cmd); // not every app will have a lib directory
    //TODO:  will every app have a bin directory???
    cmd = "chmod -R a+rx " +  GATEWAY_APPS_DIRECTORY + "/" + appId + "/bin"; //every app should have a bin directory
    responseStatus = pmUtils.ExecuteCmdLine(cmd);

    if (ER_OK != responseStatus) {
        QCC_LogError(responseStatus, ("Install attempt failed"));
    }

    return appId;
}

void PackageManagerImpl::UninstallApp(
    const String& appId,   //TODO: if PM is required to delete the application user then the user id is required here
    QStatus& responseStatus)
{
    String userId = "1234"; // place holder pending clarification
    String appDir = GATEWAY_APPS_DIRECTORY + "/" + appId;
    String cmd = "userdel -r " + userId;
    responseStatus = PmUtils::ExecuteCmdLine(cmd.c_str());
    if (ER_OK != responseStatus) {
        return;
    }

    cmd = "rm -rf " + appDir + "/";
    responseStatus = pmUtils.ExecuteCmdLine(cmd.c_str());
}

QStatus PackageManagerImpl::DownloadFile(
    const String& url,
    const String& fileLocation,
    bool sslVerify /*= true */)
{
    static SslDownloader& s_downloader = SslDownloader::getInstance();

    QStatus result = s_downloader.DownloadFile(url, fileLocation, sslVerify);
    if (result != ER_OK) {
        QCC_LogError(result, ("download of %s failed", url.c_str()));
    }
    return result;
}

QStatus PackageManagerImpl::VerifyAppPackage(
    const String& fileName,
    const String& fileHash,
    const String& localSigningPublicKeyPath)
{

    QStatus result = pmUtils.ExpandOuterTarBall(fileName);
    if (result != ER_OK) {
        QCC_LogError(result, ("unable to expand out tarball: %s", fileName.c_str()));
        return result;
    }

    PackageVerifier verifier(pmUtils.getInnerTarPath(), pmUtils.getHashFilePath(), localSigningPublicKeyPath, fileHash);

    result = verifier.VerifyPackage();

    return result;
}

QStatus PackageManagerImpl::ReadBinaryFile(
    const String filelocation,
    uint8_t* buffer,
    size_t& buffer_size)
{
    ifstream input(filelocation.c_str(), ios::in | ios::binary | ios::ate);
    if (input.bad()) {
        return ER_READ_ERROR;
    }
    size_t input_size = (size_t) input.tellg();
    if (input_size > buffer_size) {
        buffer_size = input_size; // tell the caller the required size
        return ER_BUFFER_TOO_SMALL;
    }
    input.seekg(0, ios::beg);
    if (input.bad()) {
        return ER_OS_ERROR;
    }
    input.read((char*) buffer, buffer_size);
    if (input.bad()) {
        return ER_READ_ERROR;
    }
    buffer_size = input_size;
    return ER_OK;
}

} /* namespace gw */
} /* namespace ajn */
