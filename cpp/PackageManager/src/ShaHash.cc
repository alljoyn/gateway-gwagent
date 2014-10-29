 
/**
 * @file ShaHash.cc
 *
 * Implemention of a class to perform openssl hashing
 */

/******************************************************************************
 * Copyright (c) 2009-2013,2014 AllSeen Alliance. All rights reserved.
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

#include <alljoyn/gateway/ShaHash.h>
#include <alljoyn/gateway/PmLogModule.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace qcc;
using namespace std;

namespace ajn {
namespace gw {

ShaHash::ShaHash(const char* algorithm) : initialized(ER_CRYPTO_ERROR), rsaPublicKey(0), alg(algorithm)

{
    static Mutex mutex;
    ScopedMutexLock lock(mutex);
    OpenSSL_add_all_digests();
    const EVP_MD* md = EVP_get_digestbyname(alg.c_str());
    if(!md) {
        QCC_LogError(initialized, ("EVP_get_digestbyname failed"));
        return; 
    } 
    ctx = EVP_MD_CTX_create();

    if(1 != EVP_DigestInit_ex(ctx, md, NULL)) {
        QCC_LogError(initialized, ("EVP_DigestInit_ex failed"));
        return;
    }

    initialized = ER_OK;
}

ShaHash::~ShaHash() 
{
	EVP_cleanup();
}

QStatus ShaHash::Update(void* input, unsigned long length)
{
    QStatus result = initialized;
    if(result != ER_OK) {
        return result;
    }

    if(!input) {
        return ER_BAD_ARG_1;
    }

    if(1 != EVP_DigestUpdate(ctx, input, length)) 
    {
        result = ER_CRYPTO_ERROR;
    } 

    return result;
}

QStatus ShaHash::ComputeDigest()
{
    QStatus result = ER_OK;
    md_len = 0;
    if( 1 == EVP_DigestFinal_ex(ctx, md_value, &md_len))
    {
        EVP_MD_CTX_destroy(ctx);
    } else {
        result = ER_CRYPTO_ERROR;
    }
    return result;
}

//pemfilepath should be the public key of same certificate used to by the sender to create the signature file of the downloaded object
QStatus ShaHash::ImportPublicPem(const String& pemfilepath)
{
QStatus result = ER_OK;

// read the public key in .pem format
QCC_DbgPrintf(("public key location: %s", pemfilepath.c_str()));

ifstream input(pemfilepath.c_str(), ios::in);
if (input.bad())
{
    result = ER_OPEN_FAILED;
    QCC_LogError(result, ("unable to open file: %s", pemfilepath.c_str()));
    return result;
}

stringstream sstr;
sstr << input.rdbuf();
String pemText = sstr.str().c_str(); 
input.close();
sstr.flush();

BIO* bio = BIO_new(BIO_s_mem());
BIO_write(bio, pemText.c_str(), pemText.size());
rsaPublicKey = PEM_read_bio_RSA_PUBKEY( bio, NULL, NULL, NULL );
BIO_free(bio);
result = rsaPublicKey != NULL ? ER_OK : ER_CRYPTO_ERROR;
if(result == ER_CRYPTO_ERROR) {
    QCC_LogError(result, ("unable to verify signature using %s", pemfilepath.c_str()));
}
return result;
}

QStatus ShaHash::RsaVerify(uint8_t* sigbuffer, const unsigned int siglen)
{
    if (1 != RSA_verify(NID_sha1, md_value, SHA1_SIZE, sigbuffer, siglen, rsaPublicKey))  
    {
        QCC_LogError(ER_AUTH_FAIL, ("RSA authorization failed"));
        return ER_AUTH_FAIL;
    }

    return ER_OK;
}


} /* namespace gw */
} /* namespace ajn */
