/**
 * @file
 *
 * This file performs various GTests on Package Manager components
 */

/******************************************************************************
 * Copyright (c) 2009-2013, AllSeen Alliance. All rights reserved.
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
#include <alljoyn/gateway/PackageManagerImpl.h>
#include <gtest/gtest.h>
#include <ostream>
#include <Status.h>
#include <qcc/String.h>
#include <alljoyn/gateway/SslDownloader.h>
#include <qcc/FileStream.h>
#include <qcc/Util.h>



namespace {

using namespace std;
using namespace ajn::gw;
using namespace qcc;

class pm_test : public ::testing::Test {

protected:
    pm_test() : downloadUrl("url") {};
    String downloadUrl;
    String ssl_unverified_url;
    String ssl_verified_url;
    String url_404;
    String url_DoesNotExist;
    String file_to_verify;
    String locally_stored_signing_cert;
    String sig_file_location;
    String private_key_location;
    String altered_file_to_verify;
    String package_location;
    String test_server;
    String appId;
    String packageName;
    String userId;

    virtual void SetUp() {
        ssl_unverified_url = "https://localhost:8001";
        ssl_verified_url = "https://www.digicert.com/digicert-root-certificates.htm";
        url_DoesNotExist = "https://zzzzzzzzzz.io";
        url_404 = "http://localhost:8001/not_there.html";
        altered_file_to_verify = "/testObjects/badimage.png";  //badimage.png is image.png with one bit changed
        file_to_verify = "/testObjects/image.png";
        locally_stored_signing_cert = "/testObjects/0E7B.pem";  
        sig_file_location = "/testObjects/image.png.sha1";
        private_key_location ="/testObjects/0E7B.pem";
        test_server = "https://10.1.1.222/examplepackage.tar";
        appId = "exampleId";
        packageName = "examplepackage";
        userId = "1234";
    };

}; /*pm_test*/

TEST_F(pm_test, InstallTest) {
    QStatus status = ER_OK;
    PackageManager mgr;
    mgr.InstallApp(appId,
            packageName,
            "appVersion 1.0",
            test_server,
            0,
            false,
            userId,
            status);

    if (status != ER_OK) {
        std::cout << "InstallApp status result: " << QCC_StatusText(status)	<< std::endl;
        ADD_FAILURE_AT(__FILE__, __LINE__);
    } else {
        SUCCEED();
    }

}

TEST_F(pm_test, UnInstallTest) {
    QStatus status = ER_OK;
    PackageManager mgr;
    mgr.UninstallApp(appId,
            status);

    if (status != ER_OK) {
        std::cout << "InstallApp status result: " << QCC_StatusText(status)	<< std::endl;
        ADD_FAILURE_AT(__FILE__, __LINE__);
    } else {
        SUCCEED();
    }
}

TEST_F(pm_test, UpdateTest) {
    QStatus status = ER_OK;
    PackageManager mgr;
    mgr.InstallApp(appId,
            packageName,
            "appVersion 1.0",
            test_server,
            0,
            true,
            userId,
            status);

    if (status != ER_OK) {
        std::cout << "Update app status result: " << QCC_StatusText(status)	<< std::endl;
        ADD_FAILURE_AT(__FILE__, __LINE__);
    } else {
        SUCCEED();
    }

}

TEST_F(pm_test,PackageMangerImplDownloadVerifiedSsslTest) {
    PackageManagerImpl mgrImpl;
    QStatus result = mgrImpl.DownloadFile(ssl_verified_url, "verified.txt");
    EXPECT_EQ(ER_OK, result);
}

TEST_F(pm_test,PackageMangerImplDownloadUnVerifiedSsslTest) {
    PackageManagerImpl mgrImpl;
    QStatus result = mgrImpl.DownloadFile(ssl_unverified_url, "unverified.txt", false);
    EXPECT_NE(ER_OK, result);
    EXPECT_EQ(ER_OK, DeleteFile("unverified.txt"));
}

TEST_F(pm_test,PackageMangerImplDownload404lTest) {
    PackageManagerImpl mgrImpl;
    QStatus result = mgrImpl.DownloadFile(url_404, "404.txt");
    EXPECT_NE(ER_OK, result);
    EXPECT_EQ(ER_OK, DeleteFile("404.txt"));
}

TEST_F(pm_test,PackageMangerImplDownloadNonexistantTest) {
    PackageManagerImpl mgrImpl;
    QStatus result = mgrImpl.DownloadFile(url_DoesNotExist, "NonExistant.txt");
    EXPECT_NE(ER_OK, result);
}
} /*namespace*/

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}








