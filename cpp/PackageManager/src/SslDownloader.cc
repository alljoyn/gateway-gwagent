/**
 * @file
 *
 * This file implements wrappers around Openssl.
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

#include <alljoyn/gateway/SslDownloader.h>
#include <alljoyn/gateway/PackageManagerConstants.h>
#include <alljoyn/gateway/PmLogModule.h>
#include <curl/curl.h>
#include <pthread.h>
#include <ostream>
#include <qcc/ScopedMutexLock.h>

namespace ajn {
namespace gw {

using namespace std;
using namespace pm_constants;

class CurlInitHelper {
public:
    CurlInitHelper(CURL*& curl) :
            pcurl(curl) {
        pcurl = curl_easy_init();
        curl = pcurl;
    }

    ~CurlInitHelper() {
        curl_easy_cleanup(pcurl);
    }

private:
    CURL* pcurl;
};

static pthread_once_t curl_inited = PTHREAD_ONCE_INIT;
extern void init_curl();
static pthread_once_t curl_released = PTHREAD_ONCE_INIT;
extern void release_curl();

void release_curl() {
    curl_global_cleanup();
}

void init_curl() {
    curl_global_init(CURL_GLOBAL_SSL);
}

SslDownloader::SslDownloader() {
    pthread_once(&curl_inited, init_curl);
}

SslDownloader::~SslDownloader() {
    pthread_once(&curl_released, release_curl);
}

QStatus SslDownloader::DownloadFile(qcc::String url, qcc::String fileLocation, bool sslVerify) {

    using namespace qcc;
    CURL* curl = NULL;
    static qcc::Mutex mutex;
    ScopedMutexLock lock(mutex);

#ifdef NDEBUG
    sslVerify = true;  //do not skip verification in release builds
#endif

    CurlInitHelper helper(curl);
    if (!curl) {
        return ER_SSL_INIT;
    }
    FILE* fp = fopen(fileLocation.c_str(), "wb");
    if(NULL == fp) {
        QCC_LogError(ER_OPEN_FAILED, ("SslDownloader could not open %s", fileLocation.c_str()));
        return ER_OPEN_FAILED;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, AJPM_USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    if (sslVerify) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_CAINFO, AJPM_CERT_STORE_LOCATION );
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    } else {
         curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
         curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    CURLcode res = curl_easy_perform(curl);

    if (0 != fclose(fp)) {

        QCC_LogError(ER_WRITE_ERROR, ("SslDownloader failed to close file %s", fileLocation.c_str()));
        return ER_WRITE_ERROR;
    }

    QStatus result = ER_OK;
    if (res == CURLE_OK) {
        QCC_DbgPrintf(("SSL downloaded: %s", fileLocation.c_str()));
    } else {
        result = res == CURLE_PEER_FAILED_VERIFICATION ? ER_SSL_VERIFY : ER_SSL_CONNECT;
        QCC_LogError(result, ("Ssl download failed: %s ", fileLocation.c_str()));
   }

    return result;
}

size_t SslDownloader::WriteCallback(void *ptr, size_t size, size_t nmemb,
        FILE *stream) {
    size_t result(0);
    result = fwrite(ptr, size, nmemb, stream);
    return result;
}

} /* namespace gw */
} /* namespace ajn */
