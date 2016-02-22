#ifndef _PM_VERIFIER_H
#define _PM_VERIFIER_H
/**
 * @file PackageVerifier.h
 *
 * Interface of a class to perform ssl verification of a signed file
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
#include <alljoyn/gateway/ShaHash.h>
#include <vector>

namespace ajn {
namespace gw {
using namespace qcc;

/**
 * This class verifies that an AJ package was signed by a trusted signature authority
 */

class PackageVerifier {
  public:
    /**
     * The PackageVerifier constructor provides the class with the parameters needed
     * to perform verification of a downloaded AJ package
     * @param [in] - dataFilePath - the compressed AJ application itself
     * @param [in] - PackageSignaturePath - The cryptologic signature hash downloaded as part of the AJ package
     * @param [in] - publicKeyfilePath - Path to the public key of the signing certificate that was used to generate the AJ package signature
     */
    PackageVerifier(const String& dataFilePath,
                    const String& PackageSignaturePath,
                    const String& publicKeyfilePath,
                    const String& packageFileHash);

    /**
     * Using the OpenSsl api, computes a crypo has of the data file and verifies that this hash with the preinstalled public signing key.
     * @return ER_OK if successful, otherwise an error code
     */
    QStatus VerifyPackage();
    virtual ~PackageVerifier();

  private:
    QStatus ReadDataFile();
    QStatus ReadSignatureFile();
    String m_dataFile;
    String m_signatureFile;
    String m_publicKeyFile;
    String m_packageFileHash;
    ShaHash hash;
    uint8_t digest[ShaHash::SHA256_SIZE];
    std::vector<unsigned char> sig_buffer;
};


} /* namespace gw */
} /* namespace ajn */

#endif // _PM_VERIFIER_H
