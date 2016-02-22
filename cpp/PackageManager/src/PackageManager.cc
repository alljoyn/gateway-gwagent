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

#include <alljoyn/gateway/PackageManager.h>

namespace ajn {
namespace gw {

PackageManager::PackageManager() {
    pmImpl = new PackageManagerImpl;
}

PackageManager::~PackageManager() {
    if (pmImpl) {
        delete pmImpl;
    }
}

void PackageManager::InstallApp(
    const String& downloadUrl,
    bool upgradeFlag,
    QStatus& responseStatus) {

    pmImpl->InstallApp(downloadUrl,
                       upgradeFlag,
                       responseStatus);
}

void PackageManager::UninstallApp(
    const String& appId,
    QStatus& responseStatus) {

    pmImpl->UninstallApp(appId,
                         responseStatus);
}

} /* namespace gw */
} /* namespace ajn */
