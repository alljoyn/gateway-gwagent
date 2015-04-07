/**
 * @file
 *
 * Various constants required by the Package Manager
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

#ifndef PACKAGE_MANAGER_CONSTANTS_H_
#define PACKAGE_MANAGER_CONSTANTS_H_
#include <qcc/String.h>



namespace pm_constants {
//todo:: The Gateway Management application also defines some of these constants, if
//       those defaults are not used then perhaps they should be set as environmental variables
//       before PM is called.  In that case PM should check the environment before using these as defaults
static const qcc::String GATEWAY_APPS_DIRECTORY = "/opt/alljoyn/apps";
static const qcc::String APP_GROUP_ID = "1003";
#ifndef PMUTILS_H_
static const char* VERIFICATION_CERTIFICATE_LOCATION = "/usr/local/share/ca-certificates/0E88.public.pem";
#endif

#ifdef PMUTILS_H_
static const char* TEMP_DIR_PATH = "/tmp/aj";
#endif

#ifdef SSLDOWNLOADER_H_
static const char* AJPM_USER_AGENT = "alljoyn_package_manager/1.0";
static const char* AJPM_CERT_STORE_LOCATION = "/etc/ssl/certs/cacert.org.pem";
#endif

}
#endif
