#ifndef _PM_SHAHASH_H
#define _PM_SHAHASH_H
/**
 * @file ShaHash.h
 *
 * Interface of a class to perform openssl hashing
 */

/******************************************************************************
 * Copyright (c) 2009-2014, AllSeen Alliance. All rights reserved.
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

#include <qcc/String.h>
#include <Status.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <qcc/ScopedMutexLock.h>
#include <openssl/sha.h>

namespace ajn {
namespace gw {
using namespace qcc;

class ShaHash {
  public:

    /**
     * ShaHash constructor initalizes the hash tools.
     * Note: currently only SHA1 is supported.
     */
    ShaHash(const char* algorithm = "SHA256");  // TODO:  support SHA256
    virtual ~ShaHash();

    /**
     * Update is called to update the hash object with chunks of the compressed AJ package file
     * @param [in] -  input  pointer to a block of data
     * @param [in] -  lenght lenght of the data block
     * @return ER_OK if successful, otherwise an error code
     */
    QStatus Update(void* input, unsigned long length);

    /**
     * Computes the cryptological digest of the data provided in calls to Update
     * @return ER_OK if successful, otherwise an error code
     */
    QStatus ComputeDigest();

    /**
     * accessor for intialization state of the Openssl crypto components
     * @return ER_OK if initialization was successful, otherwise an error code
     */
    QStatus Initialized() const { return initialized; };

    /**
     * Reads a .pem encoded public key
     * @param [in] -  file system path to the signing certificate public key file
     * @return ER_OK if read was successful, otherwise an error code
     */
    QStatus ImportPublicPem(const String& pemfilepath);

    /**
     * verifies that the hash pointed to by sigbuffer was signed by a trusted signature authority
     * @param [in] -  pointer to a buffer containing the hash to be verified
     * @param [in] -  lenght of the signature hash buffer
     * @return ER_OK if verified, otherwise an error code
     */
    QStatus RsaVerify(uint8_t* sigbuffer, const unsigned int siglen);

    static const size_t SHA1_SIZE = 20;   // SHA1 digest size - 20 bytes == 160 bits
    static const size_t SHA256_SIZE = 32; // SHA256 digest size - 32 bytes == 256 bits

  private:
    EVP_MD_CTX* ctx;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    QStatus initialized;
    RSA* rsaPublicKey;
    String alg;

};

} /* namespace gw */
} /* namespace ajn */

#endif //_PM_SHAHASH_H
