/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#include "DeviceAttestationVerifierExample.h"

#include <credentials/CHIPCert.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <crypto/CHIPCryptoPAL.h>

#include <lib/core/CHIPError.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>

using namespace chip::Crypto;

namespace chip {
namespace Credentials {
namespace Examples {

namespace {

CHIP_ERROR GetProductAttestationAuthorityCert(const ByteSpan & skid, MutableByteSpan & outDacBuffer)
{
    struct PAALookupTable
    {
        const uint8_t mPAACertificate[kMax_x509_Certificate_Length];
        const uint8_t mSKID[kKeyIdentifierLength];
    };

    static PAALookupTable
        sPAALookupTable[] = {
            { /*
              credentials/test/attestation/Chip-Test-PAA-FFF1-Cert.pem
              -----BEGIN CERTIFICATE-----
              MIIBmTCCAT+gAwIBAgIIaDhPq7kZ/N8wCgYIKoZIzj0EAwIwHzEdMBsGA1UEAwwU
              TWF0dGVyIFRlc3QgUEFBIEZGRjEwIBcNMjEwNjI4MTQyMzQzWhgPOTk5OTEyMzEy
              MzU5NTlaMB8xHTAbBgNVBAMMFE1hdHRlciBUZXN0IFBBQSBGRkYxMFkwEwYHKoZI
              zj0CAQYIKoZIzj0DAQcDQgAEG5isW7wR3GoXVaBbCsXha6AsRu5vwrvnb/fPbKeq
              Tp/R15jcvvtP6uIl03c8kTSMwm1JMTHjCWMtXp7zHRLek6NjMGEwDwYDVR0TAQH/
              BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYEFO8Y4OzUZgQ03w28kR7U
              UhaZZoOfMB8GA1UdIwQYMBaAFO8Y4OzUZgQ03w28kR7UUhaZZoOfMAoGCCqGSM49
              BAMCA0gAMEUCIQCn+l+nZv/3tf0VjNNPYl1IkSAOBYUO8SX23udWVPmXNgIgI7Ub
              bkJTKCjbCZIDNwUNcPC2tyzNPLeB5nGsIl31Rys=
              -----END CERTIFICATE-----
              */
              { 0x30, 0x82, 0x01, 0x99, 0x30, 0x82, 0x01, 0x3F, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x08, 0x68, 0x38, 0x4F, 0xAB,
                0xB9, 0x19, 0xFC, 0xDF, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x30, 0x1F, 0x31,
                0x1D, 0x30, 0x1B, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x14, 0x4D, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x54, 0x65,
                0x73, 0x74, 0x20, 0x50, 0x41, 0x41, 0x20, 0x46, 0x46, 0x46, 0x31, 0x30, 0x20, 0x17, 0x0D, 0x32, 0x31, 0x30, 0x36,
                0x32, 0x38, 0x31, 0x34, 0x32, 0x33, 0x34, 0x33, 0x5A, 0x18, 0x0F, 0x39, 0x39, 0x39, 0x39, 0x31, 0x32, 0x33, 0x31,
                0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5A, 0x30, 0x1F, 0x31, 0x1D, 0x30, 0x1B, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C,
                0x14, 0x4D, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x50, 0x41, 0x41, 0x20, 0x46, 0x46,
                0x46, 0x31, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x86,
                0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x1B, 0x98, 0xAC, 0x5B, 0xBC, 0x11, 0xDC, 0x6A, 0x17,
                0x55, 0xA0, 0x5B, 0x0A, 0xC5, 0xE1, 0x6B, 0xA0, 0x2C, 0x46, 0xEE, 0x6F, 0xC2, 0xBB, 0xE7, 0x6F, 0xF7, 0xCF, 0x6C,
                0xA7, 0xAA, 0x4E, 0x9F, 0xD1, 0xD7, 0x98, 0xDC, 0xBE, 0xFB, 0x4F, 0xEA, 0xE2, 0x25, 0xD3, 0x77, 0x3C, 0x91, 0x34,
                0x8C, 0xC2, 0x6D, 0x49, 0x31, 0x31, 0xE3, 0x09, 0x63, 0x2D, 0x5E, 0x9E, 0xF3, 0x1D, 0x12, 0xDE, 0x93, 0xA3, 0x63,
                0x30, 0x61, 0x30, 0x0F, 0x06, 0x03, 0x55, 0x1D, 0x13, 0x01, 0x01, 0xFF, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xFF,
                0x30, 0x0E, 0x06, 0x03, 0x55, 0x1D, 0x0F, 0x01, 0x01, 0xFF, 0x04, 0x04, 0x03, 0x02, 0x01, 0x06, 0x30, 0x1D, 0x06,
                0x03, 0x55, 0x1D, 0x0E, 0x04, 0x16, 0x04, 0x14, 0xEF, 0x18, 0xE0, 0xEC, 0xD4, 0x66, 0x04, 0x34, 0xDF, 0x0D, 0xBC,
                0x91, 0x1E, 0xD4, 0x52, 0x16, 0x99, 0x66, 0x83, 0x9F, 0x30, 0x1F, 0x06, 0x03, 0x55, 0x1D, 0x23, 0x04, 0x18, 0x30,
                0x16, 0x80, 0x14, 0xEF, 0x18, 0xE0, 0xEC, 0xD4, 0x66, 0x04, 0x34, 0xDF, 0x0D, 0xBC, 0x91, 0x1E, 0xD4, 0x52, 0x16,
                0x99, 0x66, 0x83, 0x9F, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x03, 0x48, 0x00,
                0x30, 0x45, 0x02, 0x21, 0x00, 0xA7, 0xFA, 0x5F, 0xA7, 0x66, 0xFF, 0xF7, 0xB5, 0xFD, 0x15, 0x8C, 0xD3, 0x4F, 0x62,
                0x5D, 0x48, 0x91, 0x20, 0x0E, 0x05, 0x85, 0x0E, 0xF1, 0x25, 0xF6, 0xDE, 0xE7, 0x56, 0x54, 0xF9, 0x97, 0x36, 0x02,
                0x20, 0x23, 0xB5, 0x1B, 0x6E, 0x42, 0x53, 0x28, 0x28, 0xDB, 0x09, 0x92, 0x03, 0x37, 0x05, 0x0D, 0x70, 0xF0, 0xB6,
                0xB7, 0x2C, 0xCD, 0x3C, 0xB7, 0x81, 0xE6, 0x71, 0xAC, 0x22, 0x5D, 0xF5, 0x47, 0x2B },
              { 0xEF, 0x18, 0xE0, 0xEC, 0xD4, 0x66, 0x04, 0x34, 0xDF, 0x0D,
                0xBC, 0x91, 0x1E, 0xD4, 0x52, 0x16, 0x99, 0x66, 0x83, 0x9F } },
            { /*
              credentials/test/attestation/Chip-Test-PAA-FFF2-Cert.pem
              -----BEGIN CERTIFICATE-----
              MIIBnTCCAUKgAwIBAgIIA5KnZVo+bHcwCgYIKoZIzj0EAwIwHzEdMBsGA1UEAwwU
              TWF0dGVyIFRlc3QgUEFBIEZGRjIwIBcNMjEwNjI4MTQyMzQzWhgPOTk5OTEyMzEy
              MzU5NTlaMB8xHTAbBgNVBAMMFE1hdHRlciBUZXN0IFBBQSBGRkYyMFkwEwYHKoZI
              zj0CAQYIKoZIzj0DAQcDQgAEdW4YkvnpULAOlQqilfM1sEhLh20i4m+WZZLKweUQ
              1f6Zsx1cmIgWeorWUDd+dRD7dYI8fluYuMAG7F8Gz66FSqNmMGQwEgYDVR0TAQH/
              BAgwBgEB/wIBATAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYEFOfv6sMzXF/Qw+Y0
              Up8WcEbEvKVcMB8GA1UdIwQYMBaAFOfv6sMzXF/Qw+Y0Up8WcEbEvKVcMAoGCCqG
              SM49BAMCA0kAMEYCIQCSUQ0dYCFfARvaLqeV/ssklO+QppeHrQr8IGxhjAnMUgIh
              AKA2sK+D40VcCTi5S/9HdRlyuNy+cZyfYbVW7LTqF8xX
              -----END CERTIFICATE-----
              */
              { 0x30, 0x82, 0x01, 0x9D, 0x30, 0x82, 0x01, 0x42, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x08, 0x03, 0x92, 0xA7, 0x65,
                0x5A, 0x3E, 0x6C, 0x77, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x30, 0x1F, 0x31,
                0x1D, 0x30, 0x1B, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x14, 0x4D, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x54, 0x65,
                0x73, 0x74, 0x20, 0x50, 0x41, 0x41, 0x20, 0x46, 0x46, 0x46, 0x32, 0x30, 0x20, 0x17, 0x0D, 0x32, 0x31, 0x30, 0x36,
                0x32, 0x38, 0x31, 0x34, 0x32, 0x33, 0x34, 0x33, 0x5A, 0x18, 0x0F, 0x39, 0x39, 0x39, 0x39, 0x31, 0x32, 0x33, 0x31,
                0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5A, 0x30, 0x1F, 0x31, 0x1D, 0x30, 0x1B, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C,
                0x14, 0x4D, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x50, 0x41, 0x41, 0x20, 0x46, 0x46,
                0x46, 0x32, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x86,
                0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x75, 0x6E, 0x18, 0x92, 0xF9, 0xE9, 0x50, 0xB0, 0x0E,
                0x95, 0x0A, 0xA2, 0x95, 0xF3, 0x35, 0xB0, 0x48, 0x4B, 0x87, 0x6D, 0x22, 0xE2, 0x6F, 0x96, 0x65, 0x92, 0xCA, 0xC1,
                0xE5, 0x10, 0xD5, 0xFE, 0x99, 0xB3, 0x1D, 0x5C, 0x98, 0x88, 0x16, 0x7A, 0x8A, 0xD6, 0x50, 0x37, 0x7E, 0x75, 0x10,
                0xFB, 0x75, 0x82, 0x3C, 0x7E, 0x5B, 0x98, 0xB8, 0xC0, 0x06, 0xEC, 0x5F, 0x06, 0xCF, 0xAE, 0x85, 0x4A, 0xA3, 0x66,
                0x30, 0x64, 0x30, 0x12, 0x06, 0x03, 0x55, 0x1D, 0x13, 0x01, 0x01, 0xFF, 0x04, 0x08, 0x30, 0x06, 0x01, 0x01, 0xFF,
                0x02, 0x01, 0x01, 0x30, 0x0E, 0x06, 0x03, 0x55, 0x1D, 0x0F, 0x01, 0x01, 0xFF, 0x04, 0x04, 0x03, 0x02, 0x01, 0x06,
                0x30, 0x1D, 0x06, 0x03, 0x55, 0x1D, 0x0E, 0x04, 0x16, 0x04, 0x14, 0xE7, 0xEF, 0xEA, 0xC3, 0x33, 0x5C, 0x5F, 0xD0,
                0xC3, 0xE6, 0x34, 0x52, 0x9F, 0x16, 0x70, 0x46, 0xC4, 0xBC, 0xA5, 0x5C, 0x30, 0x1F, 0x06, 0x03, 0x55, 0x1D, 0x23,
                0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0xE7, 0xEF, 0xEA, 0xC3, 0x33, 0x5C, 0x5F, 0xD0, 0xC3, 0xE6, 0x34, 0x52, 0x9F,
                0x16, 0x70, 0x46, 0xC4, 0xBC, 0xA5, 0x5C, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02,
                0x03, 0x49, 0x00, 0x30, 0x46, 0x02, 0x21, 0x00, 0x92, 0x51, 0x0D, 0x1D, 0x60, 0x21, 0x5F, 0x01, 0x1B, 0xDA, 0x2E,
                0xA7, 0x95, 0xFE, 0xCB, 0x24, 0x94, 0xEF, 0x90, 0xA6, 0x97, 0x87, 0xAD, 0x0A, 0xFC, 0x20, 0x6C, 0x61, 0x8C, 0x09,
                0xCC, 0x52, 0x02, 0x21, 0x00, 0xA0, 0x36, 0xB0, 0xAF, 0x83, 0xE3, 0x45, 0x5C, 0x09, 0x38, 0xB9, 0x4B, 0xFF, 0x47,
                0x75, 0x19, 0x72, 0xB8, 0xDC, 0xBE, 0x71, 0x9C, 0x9F, 0x61, 0xB5, 0x56, 0xEC, 0xB4, 0xEA, 0x17, 0xCC, 0x57 },
              { 0xE7, 0xEF, 0xEA, 0xC3, 0x33, 0x5C, 0x5F, 0xD0, 0xC3, 0xE6,
                0x34, 0x52, 0x9F, 0x16, 0x70, 0x46, 0xC4, 0xBC, 0xA5, 0x5C } },
        };

    size_t paaLookupTableIdx;
    for (paaLookupTableIdx = 0; paaLookupTableIdx < ArraySize(sPAALookupTable); ++paaLookupTableIdx)
    {
        if (skid.data_equal(ByteSpan(sPAALookupTable[paaLookupTableIdx].mSKID)))
        {
            break;
        }
    }

    VerifyOrReturnError(paaLookupTableIdx < ArraySize(sPAALookupTable), CHIP_ERROR_INVALID_ARGUMENT);

    return CopySpanToMutableSpan(ByteSpan{ sPAALookupTable[paaLookupTableIdx].mPAACertificate }, outDacBuffer);
}

class ExampleDACVerifier : public DeviceAttestationVerifier
{
public:
    AttestationVerificationResult VerifyAttestationInformation(const ByteSpan & attestationInfoBuffer,
                                                               const ByteSpan & attestationChallengeBuffer,
                                                               const ByteSpan & attestationSignatureBuffer,
                                                               const ByteSpan & paiCertDerBuffer, const ByteSpan & dacCertDerBuffer,
                                                               const ByteSpan & attestationNonce) override;
};

AttestationVerificationResult ExampleDACVerifier::VerifyAttestationInformation(const ByteSpan & attestationInfoBuffer,
                                                                               const ByteSpan & attestationChallengeBuffer,
                                                                               const ByteSpan & attestationSignatureBuffer,
                                                                               const ByteSpan & paiCertDerBuffer,
                                                                               const ByteSpan & dacCertDerBuffer,
                                                                               const ByteSpan & attestationNonce)
{
    // match DAC and PAI VIDs
    if (!paiCertDerBuffer.empty())
    {
        VendorId paiVid;
        VendorId dacVid;

        CHIP_ERROR error     = ExtractVIDFromX509Cert(paiCertDerBuffer, paiVid);
        const bool paiHasVid = error != CHIP_ERROR_KEY_NOT_FOUND;
        VerifyOrReturnError(error == CHIP_NO_ERROR || paiHasVid == false, AttestationVerificationResult::kPaiFormatInvalid);

        if (paiHasVid)
        {
            VerifyOrReturnError(ExtractVIDFromX509Cert(dacCertDerBuffer, dacVid) == CHIP_NO_ERROR,
                                AttestationVerificationResult::kDacFormatInvalid);

            VerifyOrReturnError(paiVid == dacVid, AttestationVerificationResult::kDacVendorIdMismatch);
        }
    }

    P256PublicKey remoteManufacturerPubkey;
    VerifyOrReturnError(ExtractPubkeyFromX509Cert(dacCertDerBuffer, remoteManufacturerPubkey) == CHIP_NO_ERROR,
                        AttestationVerificationResult::kDacFormatInvalid);

    // Validate overall attestation signature on attestation information
    P256ECDSASignature deviceSignature;
    // SetLength will fail if signature doesn't fit
    VerifyOrReturnError(deviceSignature.SetLength(attestationSignatureBuffer.size()) == CHIP_NO_ERROR,
                        AttestationVerificationResult::kAttestationSignatureInvalidFormat);
    memcpy(deviceSignature.Bytes(), attestationSignatureBuffer.data(), attestationSignatureBuffer.size());
    VerifyOrReturnError(ValidateAttestationSignature(remoteManufacturerPubkey, attestationInfoBuffer, attestationChallengeBuffer,
                                                     deviceSignature) == CHIP_NO_ERROR,
                        AttestationVerificationResult::kAttestationSignatureInvalid);

    uint8_t akidBuf[Credentials::kKeyIdentifierLength];
    MutableByteSpan akid(akidBuf);
    ExtractAKIDFromX509Cert(paiCertDerBuffer.empty() ? dacCertDerBuffer : paiCertDerBuffer, akid);

    constexpr size_t paaCertAllocatedLen = kMaxDERCertLength;
    chip::Platform::ScopedMemoryBuffer<uint8_t> paaCert;
    VerifyOrReturnError(paaCert.Alloc(paaCertAllocatedLen), AttestationVerificationResult::kNoMemory);
    MutableByteSpan paa(paaCert.Get(), paaCertAllocatedLen);
    VerifyOrReturnError(GetProductAttestationAuthorityCert(akid, paa) == CHIP_NO_ERROR,
                        AttestationVerificationResult::kPaaNotFound);

    VerifyOrReturnError(ValidateCertificateChain(paa.data(), paa.size(), paiCertDerBuffer.data(), paiCertDerBuffer.size(),
                                                 dacCertDerBuffer.data(), dacCertDerBuffer.size()) == CHIP_NO_ERROR,
                        AttestationVerificationResult::kDacSignatureInvalid);

    ByteSpan certificationDeclarationSpan;
    ByteSpan attestationNonceSpan;
    uint32_t timestampDeconstructed;
    ByteSpan firmwareInfoSpan;
    // TODO: refactor once final vendor-specific data tags is handled.
    ByteSpan vendorReservedDeconstructed[2];
    size_t vendorReservedDeconstructedSize = ArraySize(vendorReservedDeconstructed);
    uint16_t vendorIdDeconstructed;
    uint16_t profileNumDeconstructed;
    VerifyOrReturnError(DeconstructAttestationElements(attestationInfoBuffer, certificationDeclarationSpan, attestationNonceSpan,
                                                       timestampDeconstructed, firmwareInfoSpan, vendorReservedDeconstructed,
                                                       vendorReservedDeconstructedSize, vendorIdDeconstructed,
                                                       profileNumDeconstructed) == CHIP_NO_ERROR,
                        AttestationVerificationResult::kAttestationElementsMalformed);

    // Verify that Nonce matches with what we sent
    VerifyOrReturnError(attestationNonceSpan.data_equal(attestationNonce),
                        AttestationVerificationResult::kAttestationNonceMismatch);

    return AttestationVerificationResult::kSuccess;
}

} // namespace

DeviceAttestationVerifier * GetExampleDACVerifier()
{
    static ExampleDACVerifier exampleDacVerifier;

    return &exampleDacVerifier;
}

} // namespace Examples
} // namespace Credentials
} // namespace chip