/**
 * @file
 *
 * This file defines various utility functions used by the Package Manager
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

#ifndef PMUTILS_H_
#define PMUTILS_H_

#include <Status.h>
#include <qcc/String.h>
#include <linux/limits.h>
#include <vector>

namespace ajn {
namespace gw {

using namespace qcc;
using namespace std;

class PmUtils
{
public:
    /**
    * Constructor for PmUtils
    */
    PmUtils();

    /**
    * Destructor for PmUtils
    */
    virtual ~PmUtils();

    //File Utilities
    
    /**
     * Set temporary file and dir prefix string
     * 
     * @param [in] prefix - Sets the prefix to be used for temporary files and directories
     * @return   ER_OK if successful, otherwise ER_OPEN_FAILED
     */
    QStatus InitTempDir(const String& prefix);

    /**
     * Returns the full path to the current temporary directory
     * 
     * @returns full path to the current temporary directory
     */
    const String getTempDirName() { return tempDirPath; };
    
    /**
     * Get a list of the files in the temporary directory
     * 
     * @param [out] filenames - vector of Strings to recieve the filesnames
     */
    void GetTempDirFiles(vector<String>& filenames){
        EnumerateTempDir();
        filenames = tempdir_files;};
        
    /**
     * Verify that a directory exists
     * 
     * @param [in] - dir - directory to check
     * @return true if the directory exists
     */        
    static bool DirectoryExists(const String& dir);
    
     /**
     * Create a directory
     * 
     * @param [in] - path - directory path to create
     * @return ER_OK on success, otherwise ER_OPEN_FAILED or ER_INVAILD_DATA
     */
    static QStatus CreateDir(const String& path);
  
    // Package related utilities
    
    /**
    * Expand the outer tarball of an Alljoyn application package
    * @param [in] FileName of the outer tarball
    * @return ER_OK on success, otherwise ER_OPEN_FAILED or ER_INVAILD_DATA
    */
    QStatus ExpandOuterTarBall(const String& FileName);
    
    /**
    * Get the path to the outer tarball
    * @return String containing the desired path
    */
    const String getOuterTarFilePath() { return tempDirPath + "/" + outerTarFile; };
    
    /**
    * Get the path to the Alljoyn package hash file
    * @return String containing the desired path
    */
    const String getHashFilePath() { return tempDirPath + "/" + hashStringFile; };
    
    /**
    * Get the path to the Alljoyn package inner tarball
    * @return String containing the desired path
    */
    const String getInnerTarPath() { return tempDirPath + "/" + innerTarFile; };
    
    /**
    * Remove the temporary hash file
    * @return  ER_OK on success, otherwise ER_OPEN_FAILED or ER_INVAILD_DATA
    */
    QStatus RemoveHashFile() { return RemoveFileFromTempDir(hashStringFile); };
    
    /**
    * Remove the inner tar file
    * @return  ER_OK on success, otherwise ER_OPEN_FAILED or ER_INVAILD_DATA
    */
    QStatus RemoveInnerTarFile() { return RemoveFileFromTempDir(innerTarFile); };
    
    /**
    * Executes an operating system command
    * @param [in] cmd - the command to execute
    * @return ER_OK on success, otherwise ER_OPEN_FAILED or ER_INVAILD_DATA
    */
    static QStatus ExecuteCmdLine(const String& cmd);
    
    /**
    * Extract the inner tar to the desired application directory
    * @param [in] packageName - name of the package tar ball. This object is deleted during this function.
    * @param [in] installPath - path to the desired location of the application directory
    * @return ER_OK on success, otherwise ER_OPEN_FAILED or ER_INVAILD_DATA
    */
    QStatus ExtractInnerTarToDir(const String& packageName, const String& installPath);
    
    static const size_t MAX_TMP_SIZE = PATH_MAX;
    
private:
    QStatus EnumerateTempDir();
    QStatus RemoveFileFromTempDir(const String& filename);
    void DeleteTempDir();
    vector<String> tempdir_files;
    String tempDirPath;
    String hashStringFile;
    String innerTarFile;
    String outerTarFile;
};

} /* namespace gw */
} /* namespace ajn */

#endif /* PMUTILS_H_ */
