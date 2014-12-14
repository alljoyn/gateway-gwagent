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
#include <pwd.h>
#include <stdio.h>
#include <string.h>

namespace ajn {
namespace gw {

using namespace qcc;
using namespace std;
using namespace pm_constants;

PackageManagerImpl::PackageManagerImpl()
{
    //ensure that the basic alljoyn directories exist
    if (!PmUtils::DirectoryExists(GATEWAY_APPS_DIRECTORY))
    {
        PmUtils::CreateDir(GATEWAY_APPS_DIRECTORY);
    }
}

PackageManagerImpl::~PackageManagerImpl()
{
}

void PackageManagerImpl::InstallApp(const String& appId,
                                    const String& package_name,
                                    const String& appVersion,
                                    const String& downloadUrl,
                                    uint64_t appPackageFileSize,
                                    bool upgradeFlag,
                                    const String& unixUserId,
                                    const String& groupId,
                                    QStatus& responseStatus)
{
    packageName = package_name;
    responseStatus = pmUtils.InitTempDir(appId);
    if (ER_OK != responseStatus)
    {
        QCC_LogError(responseStatus, ("Unable create temporary directory for : %s ", package_name.c_str()) );
        return;
    }
    
    String downloadLocation = pmUtils.getTempDirName() + "/" + packageName;
    cout << "Downloading " << packageName.c_str() << " from " << downloadLocation.c_str() << endl;
    responseStatus = DownloadFile(downloadUrl, downloadLocation);

    if (ER_OK != responseStatus)
    {
         QCC_LogError(responseStatus, ("Unable to download package: %s  ", downloadLocation.c_str()) );
        return;
    }

    if (upgradeFlag)
    {
        String appDir = GATEWAY_APPS_DIRECTORY + "/" + appId;
        PmUtils pdtemp;
        PreservePersistantData(appDir, pdtemp);
        String cmd = "rm -rf " + appDir + "/";
        responseStatus = pmUtils.ExecuteCmdLine(cmd.c_str());
        if (ER_OK != responseStatus)
        {
            QCC_LogError(responseStatus, ("Unable to remove package at: %s prior to update", appDir.c_str()) );
            return;
        }
        if(!RestorePersistantData(appDir, pdtemp)) {
            QCC_DbgPrintf(("Unable to restore peristant data for %s", appDir.c_str()));
        }
    }
    else
    {
        stringstream usercmdss;
        usercmdss <<  "useradd -u " <<  unixUserId << " -g " << groupId << " " << appId.c_str();  //fails if user already exists
        responseStatus = pmUtils.ExecuteCmdLine(usercmdss.str().c_str());
    }

    if (ER_OK != responseStatus)
    {
        QCC_LogError(responseStatus, ("Unable to add package for : %s - Verify groupId exists and user ID does not exist", appId.c_str()) );
        return;
    }

    responseStatus = VerifyAppPackage(packageName, VERIFICATION_CERTIFICATE_LOCATION);
    if (ER_OK != responseStatus)
    {
        QCC_LogError(responseStatus, ("Unable to verify package download: %s  ", packageName.c_str()) );
        return;
    }


    vector<String> filenames;
    pmUtils.GetTempDirFiles(filenames);

    //create directories for this app
    String mkdir;
    mkdir = GATEWAY_APPS_DIRECTORY + "/" + appId;
    responseStatus = pmUtils.CreateDir(mkdir);
    if (ER_OK != responseStatus)
    {
        QCC_LogError(responseStatus, ("Unable to create AJ package directory: %s", mkdir.c_str()) );
        return;
    }

    responseStatus = pmUtils.ExtractInnerTarToDir(packageName, GATEWAY_APPS_DIRECTORY + "/" + appId);

    if (ER_OK != responseStatus)
    {
        QCC_LogError(responseStatus, ("Unable to extract AJ package"));
        return;
    }

    // set ownership of this app
    String cmd = "chown -R " + appId + " " + GATEWAY_APPS_DIRECTORY + "/" + appId;
    responseStatus = pmUtils.ExecuteCmdLine(cmd);
    if (ER_OK != responseStatus)
    {
        QCC_LogError(responseStatus, ("unable to set ownership of application"));;
    }

    // set write permissions on bin and lib directories
    cmd = "chmod -R a+rx " +  GATEWAY_APPS_DIRECTORY + "/" + appId + "/lib";
    responseStatus = pmUtils.ExecuteCmdLine(cmd); // not every app will have a lib directory
    
    cmd = "chmod -R a+rx " +  GATEWAY_APPS_DIRECTORY + "/" + appId + "/bin"; //every app should have a bin directory
    responseStatus = pmUtils.ExecuteCmdLine(cmd);
    
    if (ER_OK == responseStatus)
    {
        QCC_DbgHLPrintf(("Install of %s succeeded", appId.c_str()));
    } else
    {
         QCC_LogError(responseStatus, ("Install attempt of %s failed",appId.c_str()));
    }
    
    return;
}

void PackageManagerImpl::UninstallApp(const String& appId,   
                                      const String& packageName,
                                      const String& fileUrl,
                                      const String& unixUserId,
                                      QStatus& responseStatus)  
{
    String appDir = GATEWAY_APPS_DIRECTORY + "/" + appId;
    uid_t nuid = atoi(unixUserId.c_str());
    passwd* pwd = getpwuid(nuid);
    String username;
    String cmd;
    if(pwd) {
    username = pwd->pw_name;
    cmd = "userdel -r " + username;
    responseStatus = PmUtils::ExecuteCmdLine(cmd.c_str());
    } else
    {
        QCC_DbgPrintf( ("could not delete uid %s", unixUserId.c_str()) );
    }
   
    if (ER_OK != responseStatus)
    {
        QCC_LogError(responseStatus, ("Could not delete user %s", username.c_str()));
    }
    
    cmd = "rm -rf " + fileUrl;
    responseStatus = pmUtils.ExecuteCmdLine(cmd.c_str());
     if (ER_OK == responseStatus)
    {
        QCC_DbgHLPrintf(("Uninstall of %s succeeded", appId.c_str()));
    } else
    {
         QCC_LogError(responseStatus, ("Uninstall attempt of %s failed",appId.c_str()));
    }
    
}

QStatus PackageManagerImpl::DownloadFile(const String& url,
                                         const String& fileLocation)
{
    static SslDownloader& s_downloader = SslDownloader::getInstance();
    QStatus result = s_downloader.DownloadFile(url, fileLocation);
    if(result != ER_OK) {
        QCC_LogError(result, ("download of %s failed", url.c_str()));
    }
    return result;
}

QStatus PackageManagerImpl::VerifyAppPackage(const String& fileName,
                                             const String& localSigningPublicKeyPath)
{

    QStatus result = pmUtils.ExpandOuterTarBall(fileName);  
    if(result != ER_OK)
    {
    	cout << "app package filename:" << fileName.c_str() << endl;
        cout << "public key path:" << localSigningPublicKeyPath.c_str() << endl;
        QCC_LogError(result, ("unable to expand out tarball: %s", fileName.c_str()));
        return result;
    }

  PackageVerifier verifier(pmUtils.getInnerTarPath(), 
                           pmUtils.getHashFilePath(), 
                           localSigningPublicKeyPath);

  result = verifier.VerifyPackage();

    return result;
}

QStatus PackageManagerImpl::ReadBinaryFile(const String filelocation,
                                           uint8_t* buffer,
                                           size_t& buffer_size)
{
    ifstream input(filelocation.c_str(), ios::in | ios::binary | ios::ate);
    if (input.bad())
    {
        return ER_READ_ERROR;
    }
    size_t input_size = (size_t) input.tellg();
    if (input_size > buffer_size)
    {
        buffer_size = input_size; // tell the caller the required size
        return ER_BUFFER_TOO_SMALL;
    }
    input.seekg(0, ios::beg);
    if (input.bad())
    {
        return ER_OS_ERROR;
    }
    input.read((char*) buffer, buffer_size);
    if (input.bad())
    {
        return ER_READ_ERROR;
    }
    buffer_size = input_size;
    return ER_OK;    
}


//if PERSISTANT_DATA_DIR is present in this directory, copy it to a temporary folder
bool PackageManagerImpl::PreservePersistantData(const String& dir, 
                                                PmUtils& tempDir) 
{
    tempDir.InitTempDir("ajPdata");
    cout << "moving " << dir.c_str() << " to " << tempDir.getTempDirName().c_str() << endl;
    int result = rename(dir.c_str(), tempDir.getTempDirName().c_str());
    cout << "result: " << strerror(result) << endl;
    if(result != 0) {
        QCC_DbgPrintf(("PreservePersistantData failed, could not move %s to %s", dir.c_str(), tempDir.getTempDirName().c_str()) );
        QCC_DbgPrintf(("PreservePersistantData error code %i : %s", result, strerror(result)) );
    }

    return result == 0;
}

bool PackageManagerImpl::RestorePersistantData(const String& dir, 
                                               PmUtils& tempDir) 
{
    cout << "restoring " << tempDir.getTempDirName().c_str() << " to " << dir.c_str() << endl;
    int result = rename(tempDir.getTempDirName().c_str(), dir.c_str());
    if(result != 0) {
        QCC_DbgPrintf(("RestorePersistantData failed, could not move %s to %s", tempDir.getTempDirName().c_str(), dir.c_str()) );
        QCC_DbgPrintf(("RestorePersistantData error code %i : %s", result, strerror(result)) );
    }

    return result == 0;
}

} /* namespace gw */
} /* namespace ajn */
