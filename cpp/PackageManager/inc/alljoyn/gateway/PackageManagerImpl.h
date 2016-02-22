/**
 * @file
 *
 * This file defines main implementation of the Package Manager interface
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

#ifndef PACKAGEMANAGERIMPL_H_
#define PACKAGEMANAGERIMPL_H_

#include <qcc/String.h>
#include <Status.h>
#include "PmUtils.h"

namespace ajn {
namespace gw {

using namespace qcc;

//TODO: add doxygen comments

class PackageManagerImpl {
  public:

    PackageManagerImpl();

    virtual ~PackageManagerImpl();

    void InstallApp(const String& downloadUrl,
                    bool upgradeFlag,
                    QStatus& responseStatus);

    void UninstallApp(const String& appId,  //TODO: if PM is required to delete the application user then the user id is required here
                      QStatus& responseStatus);

    QStatus DownloadFile(const String& url,
                         const String& fileLocation,
                         bool sslVerify = true);

    QStatus VerifyAppPackage(const String& fileName,
                             const String& publicKeyLocation);

  private:
    PmUtils pmUtils;
    QStatus ReadBinaryFile(const String filelocation,
                           uint8_t* buffer,
                           size_t& buffer_size);

    String userId;
    String packageName;
};

} /* namespace gw */
} /* namespace ajn */

#endif /* PACKAGEMANAGERIMPL_H_ */
