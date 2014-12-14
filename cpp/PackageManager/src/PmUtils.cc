/**
 * @file
 *
 * This file implements various utility functions used by the Package Manager
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

#include <alljoyn/gateway/PmUtils.h>
#include <alljoyn/gateway/PackageManagerConstants.h>
#include <alljoyn/gateway/PmLogModule.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <qcc/Util.h>

namespace ajn {
namespace gw {

using namespace std;
using namespace qcc;
using namespace pm_constants;

PmUtils::PmUtils()
{
}

PmUtils::~PmUtils()
{
    if(!tempDirPath.empty()) {
        DeleteTempDir();
    }
}

QStatus PmUtils::InitTempDir(const String& prefix)
{
    String pre = TEMP_DIR_PATH + prefix;
    String nametemplate = pre + "XXXXXXXX";
    char tempnamebuffer[MAX_TMP_SIZE];

    // create a temporary directory into which the packages contents will be extracted
    memset(tempnamebuffer, 0, MAX_TMP_SIZE);
    strncpy(tempnamebuffer, nametemplate.c_str(), MAX_TMP_SIZE);

    if(NULL == mkdtemp(tempnamebuffer)) {
        QCC_LogError(ER_OPEN_FAILED, ("Unable to create temp director: %s", tempnamebuffer));
        return ER_OPEN_FAILED;
    }

    tempDirPath = tempnamebuffer;
    return ER_OK;
}

QStatus PmUtils::ExpandOuterTarBall(const String& FileName)
{
    //extract the outer tar ball
    outerTarFile = FileName;
    String args = "tar xf " + tempDirPath + "/" + outerTarFile + " -C " + tempDirPath;
    QStatus result = ExecuteCmdLine(args);

    RemoveFileFromTempDir(FileName);

    innerTarFile.clear();
    hashStringFile.clear();

    result = EnumerateTempDir();  // dir should contain only the inner tar and the hash files
    if(result != ER_OK) {
            QCC_LogError(result, ("could not ennumerate outer tar ball"));
            return result;
        }

    for(unsigned int i=0; i < tempdir_files.size(); i++) {
        if(tempdir_files[i].find(".tar") > 0 &&  tempdir_files[i].find(".sha") == String::npos  ) {
            innerTarFile = tempdir_files[i];
            continue;
        }

        if(tempdir_files[i].find(".sha") > 0) {
            hashStringFile = tempdir_files[i];
        }
    }

    if(hashStringFile.empty() || innerTarFile.empty()) {
        result = ER_INVALID_DATA;
        QCC_LogError(result, ("inner tar file malformed"));
    }

    return result;
}

QStatus PmUtils::RemoveFileFromTempDir(const String& filename)
{
    String arg =  tempDirPath + "/" + filename;
    QStatus result = unlink(arg.c_str()) == 0 ?  ER_OK : ER_FAIL;
    if(result == ER_FAIL) {
        QCC_LogError(result, ("failed to unlink %s", arg.c_str()));
    }
    return result;
}

QStatus PmUtils::EnumerateTempDir()
{
    DIR* dp = opendir(tempDirPath.c_str());
    if(dp == NULL) {
        return ER_OPEN_FAILED;
    }
    tempdir_files.clear();
    struct dirent *dptr = NULL;
    while(NULL != (dptr = readdir(dp) ))
    {
        if(dptr->d_type == DT_REG) {
            tempdir_files.push_back(String(dptr->d_name));
        }
    }
    closedir(dp);
    return tempdir_files.size() > 0 ? ER_OK : ER_FAIL;
}

void PmUtils::DeleteTempDir()
{
    EnumerateTempDir();
    for(unsigned int i=0; i < tempdir_files.size(); i++) {
        RemoveFileFromTempDir(tempdir_files[i]);
    }
    if( 0 != rmdir(tempDirPath.c_str())) {
        QCC_DbgPrintf(("failed to remove %s", tempDirPath.c_str()));
    }

}

QStatus PmUtils::ExecuteCmdLine(const String& cmd)
{
    QStatus result = ER_OPEN_FAILED;
    FILE* fp = popen(cmd.c_str(), "re");
    if (fp) {
        while (!feof(fp)) {fgetc(fp);}
        result = pclose(fp) == 0 ? ER_OK : ER_INVALID_DATA;
    }
    if(result != ER_OK) {
       QCC_LogError(result, ("ExecuteCmdLine error: %s", cmd.c_str()));
    }

    return result;
}

QStatus PmUtils::CreateDir(const String& path)
{
    String cmd = "mkdir -p " +     path;
    return ExecuteCmdLine(cmd);
}

bool PmUtils::DirectoryExists(const String& path)
{
    DIR * dir;
    bool result = false;
    dir = opendir(path.c_str());
    if (dir)
    {
        result = true;
        closedir(dir);
    }
    return result;
}

QStatus PmUtils::ExtractInnerTarToDir(const String& packageName, const String& installPath)
{
    String args = "tar xf " + tempDirPath + "/" + innerTarFile + " -C " + tempDirPath;
    QStatus result = ExecuteCmdLine(args);
    if(result != ER_OK) {
        QCC_LogError(result, ("unable to extract: %s", innerTarFile.c_str()));
    }
    args = "cp -r " + tempDirPath + "/" + packageName + "/* " + installPath;
    ExecuteCmdLine(args);
    args = "rm " + tempDirPath + "/" +innerTarFile;
    ExecuteCmdLine(args);
    args = "rm -rf " + tempDirPath + "/" + packageName;
    ExecuteCmdLine(args);
    return result;
}

} /* namespace gw */
} /* namespace ajn */
