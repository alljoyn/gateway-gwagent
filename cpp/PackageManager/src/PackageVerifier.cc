/**
 * @file PackageVerifier.cc
 *
 * Implemention of a class to perform verification of a signed file
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

#include <alljoyn/gateway/PackageVerifier.h>
#include <alljoyn/gateway/ShaHash.h>
#include <fstream>
#include <alljoyn/gateway/PmLogModule.h>

using namespace qcc;
using namespace std;

namespace ajn {
namespace gw {

PackageVerifier::PackageVerifier(const String& dataFilePath, const String& PackageSignaturePath, const String& publicKeyfilePath) :
datafile(dataFilePath), signatureFile(PackageSignaturePath), publicKeyfile(publicKeyfilePath)
{
}

QStatus PackageVerifier::VerifyPackage()
{
    QStatus result = ReadDataFile();
    if(result != ER_OK) {
        QCC_LogError(result, ("PackageVerifier::ReadDataFile failed"));
        return result;
    }

    result = ReadSignatureFile();
     if(result != ER_OK) {
        QCC_LogError(result, ("PackageVerifier::ReadSignatureFile failed"));
        return result;
    }

    result = hash.ImportPublicPem(publicKeyfile);
     if(result != ER_OK) {
         QCC_LogError(result, ("PackageVerifier could not import public key"));
        return result;
    }

    result = hash.RsaVerify(&sig_buffer[0], sig_buffer.size());
    if(result != ER_OK) {
         QCC_LogError(result, ("PackageVerifier RsaVerify failed"));
    }

    return result;
}

PackageVerifier::~PackageVerifier() {}

QStatus PackageVerifier::ReadSignatureFile()
{
    // open the corresponding signature file that was downloaded with the target object
    ifstream sigfile;
    sigfile.open(signatureFile.c_str(), ios::in | ios::binary | ios::ate);
    if (sigfile.bad())
    {
        QCC_LogError(ER_OPEN_FAILED, ("unable to open signature file: %s", signatureFile.c_str()));
        return ER_OPEN_FAILED;
    }

    unsigned int sig_file_size = sigfile.tellg();
    sigfile.seekg(0, ios::beg);
    sig_buffer.resize(sig_file_size);
    sigfile.read((char*)(&sig_buffer[0]), sig_file_size);
    sigfile.close();
    return ER_OK;
}

QStatus PackageVerifier::ReadDataFile()
{
    // read target file in chunks and run them through our hash object
	QCC_DbgPrintf( ("reading data file: %s", datafile.c_str()) );
    QStatus result = ER_FAIL;
    ifstream data;
    data.open(datafile.c_str(), ios::in | ios::binary | ios::ate);
    if (data.bad())
    {
        QCC_LogError(ER_OPEN_FAILED, ("unable to open data file: %s", datafile.c_str()));
        return ER_OPEN_FAILED;
    }
    unsigned char* databuffer = 0;
    unsigned int datafile_size = data.tellg();
    data.seekg(0, ios::beg);
    unsigned int bytes_remaining = datafile_size;
    const unsigned int chunk_size = 1024;
    databuffer = new uint8_t[chunk_size];
    unsigned int block_size(0); // because the data file could be arbitrarily large we read it a block at a time

    result = hash.Initialized();
    while (bytes_remaining > 0 && result == ER_OK)
    {
        block_size = (bytes_remaining >= chunk_size) ? chunk_size : bytes_remaining;
        data.read((char*) databuffer, block_size);
        result = hash.Update(databuffer, block_size);
        bytes_remaining -= block_size;
    }
    data.close();
    result = hash.ComputeDigest();
    return result;
}
} /* namespace gw */
} /* namespace ajn */
