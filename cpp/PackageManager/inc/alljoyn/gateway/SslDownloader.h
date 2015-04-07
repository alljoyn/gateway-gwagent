/**
 * @file
 *
 * This file defines wrappers around Curl ssl downloading tools.
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

#ifndef SSLDOWNLOADER_H_
#define SSLDOWNLOADER_H_

#include <Status.h>
#include <qcc/String.h>
#include <qcc/FileStream.h>
#include <qcc/Ptr.h>

namespace ajn {
namespace gw {

//todo:  does this class really need to be a singleton?
class SslDownloader {
  public:

    ~SslDownloader();

    /**
     * static function to get an Instance of the SslDownloader class
     * @return pointer to SslDownloader instance
     */
    static SslDownloader& getInstance() {
        static SslDownloader instance;
        return instance;
    }

    /**
     * Downloads a file from an https web server
     * @param [in] url - url from which to download the file
     * @param [in] fileLocation - local path to which to save the downloaded file
     * @param [in] sslVerify - if true verify the https server certificate
     * @return ER_OK if download succeeded,  otherwise an error code
     */
    QStatus DownloadFile(qcc::String url, qcc::String fileLocation, bool sslVerify = true);

  private:
    SslDownloader();
    SslDownloader(SslDownloader const&);   //prohibit copy construction
    void operator=(SslDownloader const&);  //prohibit assignment
    static size_t WriteCallback(void*ptr, size_t size, size_t nmemb, FILE*stream);
};

} /* namespace gw */
} /* namespace ajn */

#endif /* SSLDOWNLOADER_H_ */
