/**
 * @file
 *
 * Interface for the Package Manager, see HLD 3.3.2
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

#ifndef PACKAGEMANAGER_H_
#define PACKAGEMANAGER_H_

#include <qcc/String.h>
#include "PackageManagerImpl.h"

namespace ajn {
namespace gw {

using namespace qcc;

/**
 * Class to manage installation, updates, and uninstalls of third party
 * applications. See HLDD 3.3.2.1
 */
class PackageManager {

public:

    PackageManager();

    virtual ~PackageManager();

    /**
     * @brief Installs a connector app
     *
     * Results in the download of the connector app package from the URL
     * provided, and installs the package on the Gateway Agent in a new user
     * account.  The package signature is verified before installing the app.
     * Also used for upgrading an existing app to a new version.
     *
     * @param[in] appId                 App id for the connector app that is
     *                                  being installed. This will be the directory name for the connecector app installation
     * @param[in] packageName           Package name for the connector app
     *                                  that is being installed
     * @param[in] appVersion            Version of the connector app that is
     *                                  being installed
     * @param[in] downloadUrl           Web URL from which to download the app
     *                                  package
     * @param[in] appPackageFileSize    File size for the entire app package,
     *                                  including manifest data
     * @param[in] unixUserId            Unix user Id to be used when installing
     *                                  or upgrading the app.  Set to a new
     *                                  unique user Id for new app installs.
     *                                  Set to the current app user id for app
     *                                  upgrade
     * @param[out] responseStatus       Indicates success/failure status for
     *                                  the install/upgrade of the connector
     *                                  app
     */

    void InstallApp(const String& appId,
            const String& packageName,
            const String& appVersion,
            const String& downloadUrl,
            uint64_t appPackageFileSize,
            bool upgradeFlag,
            const String& unixUserId,
            QStatus& responseStatus);

    /**
     * @brief Results in uninstalling the specified connector app from the
     * Gateway Agent.
     *
     * @param[in] appId                 App id for the connector app that is
     *                                  being uninstalled
     * @param[out] responseStatus       Indicates success/failure status for
     *                                  the uninstall of the connector app
     */

    void UninstallApp(const String& appId,  //TODO: if PM is required to delete the application user then the user id is required here
            QStatus& responseStatus);

private:
    PackageManagerImpl *pmImpl;
};

} /* namespace gw */
} /* namespace ajn */

#endif /* PACKAGEMANAGER_H_ */
