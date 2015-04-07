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

void PackageManagerImpl::InstallApp(
    const String& appId,
    const String& package_name,
    const String& appVersion,
    const String& downloadUrl,
    uint64_t appPackageFileSize,
    bool upgradeFlag,
    const String& unixUserId,
    QStatus& responseStatus)
{
    packageName = package_name;
    responseStatus = pmUtils.InitTempDir(appId);
    if (ER_OK != responseStatus) {
        QCC_LogError(responseStatus,
                     ("Unable create temporary directory for : %s ", package_name.c_str()));
        return;
    }

    String downloadLocation = pmUtils.getTempDirName() + "/" + packageName;
    responseStatus = DownloadFile(downloadUrl, downloadLocation, false);  //Last argument is ignored in non-debug builds

    if (ER_OK != responseStatus) {
        QCC_LogError(responseStatus,
                     ("Unable to download package: %s  ", downloadLocation.c_str()));
        return;
    }

    if (upgradeFlag) {
        String appDir = GATEWAY_APPS_DIRECTORY + "/" + appId;
        String cmd = "rm -rf " + appDir + "/";
        responseStatus = pmUtils.ExecuteCmdLine(cmd.c_str());
        if (ER_OK != responseStatus) {
            QCC_LogError(responseStatus,
                         ("Unable to remove package at: %s prior to update", appDir.c_str()));
            return;
        }
    } else {   //TODO: if PM is  not required create the application's user then this code should be deleted
               //create user appId
        stringstream usercmdss;
        usercmdss <<  "useradd -u " <<  unixUserId << " -g " << APP_GROUP_ID << " " << appId.c_str();  //fails if user already exists
        responseStatus = pmUtils.ExecuteCmdLine(usercmdss.str().c_str());
    }

    if (ER_OK != responseStatus) {
        QCC_LogError(responseStatus, ("Unable to add package for : %s ", appId.c_str()));
        return;
    }

    responseStatus = VerifyAppPackage(packageName, VERIFICATION_CERTIFICATE_LOCATION);
    if (ER_OK != responseStatus) {
        QCC_LogError(responseStatus, ("Unable to verify package download: %s  ", packageName.c_str()));
        return;
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
        return;
    }

    responseStatus = pmUtils.ExtractInnerTarToDir(packageName, GATEWAY_APPS_DIRECTORY + "/" + appId);

    if (ER_OK != responseStatus) {
        QCC_LogError(responseStatus, ("Unable to extract AJ package"));
        return;
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

    return;
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
    const String& localSigningPublicKeyPath)
{

    QStatus result = pmUtils.ExpandOuterTarBall(fileName);
    if (result != ER_OK) {
        QCC_LogError(result, ("unable to expand out tarball: %s", fileName.c_str()));
        return result;
    }

    PackageVerifier verifier(pmUtils.getInnerTarPath(), pmUtils.getHashFilePath(), localSigningPublicKeyPath);

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
