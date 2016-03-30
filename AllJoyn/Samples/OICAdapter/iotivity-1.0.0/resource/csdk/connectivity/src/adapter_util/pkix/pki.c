/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      LICENSE-2.0" target="_blank">http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *

 ******************************************************************/
#include "pki.h"
#include "cert.h"
#include "sn_store.h"
#include "der_dec.h"
#include "crypto_adapter.h"

#ifndef WITH_ARDUINO
#include <time.h>
#endif


/**
 * Check: notBefore <= now <= notAfter.
 */
PKIError CheckValidity(ByteArray dateFrom, ByteArray dateTo)
{
    FUNCTION_INIT(
        struct tm t = {0};
        struct tm lnow = {0};
        time_t now;
        int i;
        ByteArray date;
        CHECK_EQUAL(dateFrom.len, UTC_TIME_LEN, PKI_INVALID_DATE_FORMAT);
        CHECK_EQUAL(dateTo.len, UTC_TIME_LEN, PKI_INVALID_DATE_FORMAT);
    );
    /* Get the current time */
    now = time(NULL);
    gmtime_r( &now, &lnow);
    for (i = 0; i < 2; i ++)
    {
        date = (i == 0 ? dateFrom : dateTo);
        t.tm_year = (date.data[0] - '0') * 10 + date.data[1] - '0';
        /* It is considered date from 1950 to 2050 */
        if (t.tm_year < 50)
        {
            t.tm_year += 100;
        }
        t.tm_mon = (date.data[2] - '0') * 10 + date.data[3] - '0' - 1;
        t.tm_mday = (date.data[4] - '0') * 10 + date.data[5] - '0';
        t.tm_hour = (date.data[6] - '0') * 10 + date.data[7] - '0';
        t.tm_min = (date.data[8] - '0') * 10 + date.data[9] - '0';
        t.tm_sec = (date.data[10] - '0') * 10 + date.data[11] - '0';
        if (i == 0)
        {
            CHECK_LESS_EQUAL(t.tm_year, lnow.tm_year, PKI_CERT_DATE_INVALID);
            if (t.tm_year == lnow.tm_year)
            CHECK_LESS_EQUAL(t.tm_mon, lnow.tm_mon, PKI_CERT_DATE_INVALID);
            if (t.tm_year == lnow.tm_year && t.tm_mon == lnow.tm_mon)
                CHECK_LESS_EQUAL(t.tm_mday, lnow.tm_mday, PKI_CERT_DATE_INVALID);
            if (t.tm_year == lnow.tm_year && t.tm_mon == lnow.tm_mon &&  t.tm_mday == lnow.tm_mday)
                CHECK_LESS_EQUAL(t.tm_hour, lnow.tm_hour, PKI_CERT_DATE_INVALID);
            if (t.tm_year == lnow.tm_year && t.tm_mon == lnow.tm_mon &&  t.tm_mday == lnow.tm_mday
                && t.tm_hour == lnow.tm_hour)
                CHECK_LESS_EQUAL(t.tm_min, lnow.tm_min, PKI_CERT_DATE_INVALID);
            if (t.tm_year == lnow.tm_year && t.tm_mon == lnow.tm_mon &&  t.tm_mday == lnow.tm_mday
                && t.tm_hour == lnow.tm_hour && t.tm_min == lnow.tm_min)
                CHECK_LESS_EQUAL(t.tm_sec, lnow.tm_sec, PKI_CERT_DATE_INVALID);
        }
        else
        {
            CHECK_LESS_EQUAL(lnow.tm_year, t.tm_year,  PKI_CERT_DATE_INVALID);
            if (t.tm_year == lnow.tm_year)
            CHECK_LESS_EQUAL(lnow.tm_mon, t.tm_mon,  PKI_CERT_DATE_INVALID);
            if (t.tm_year == lnow.tm_year && t.tm_mon == lnow.tm_mon)
                CHECK_LESS_EQUAL(lnow.tm_mday, t.tm_mday,  PKI_CERT_DATE_INVALID);
            if (t.tm_year == lnow.tm_year && t.tm_mon == lnow.tm_mon &&  t.tm_mday == lnow.tm_mday)
                CHECK_LESS_EQUAL(lnow.tm_hour, t.tm_hour,  PKI_CERT_DATE_INVALID);
            if (t.tm_year == lnow.tm_year && t.tm_mon == lnow.tm_mon &&  t.tm_mday == lnow.tm_mday
                && t.tm_hour == lnow.tm_hour)
                CHECK_LESS_EQUAL(lnow.tm_min, t.tm_min,  PKI_CERT_DATE_INVALID);
            if (t.tm_year == lnow.tm_year && t.tm_mon == lnow.tm_mon &&  t.tm_mday == lnow.tm_mday
                && t.tm_hour == lnow.tm_hour && t.tm_min == lnow.tm_min)
                CHECK_LESS_EQUAL(lnow.tm_sec, t.tm_sec, PKI_CERT_DATE_INVALID);
        }
    }
    FUNCTION_CLEAR();
}

/**
 * Decode certDerCode certificate and performs verification.
 *
 * @param[in] certDerCode  Byte array with DER encoded certificate
 * @param[in] caPublicKey  Byte array with CA public key
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError CheckCertificate(ByteArray certDerCode, ByteArray caPublicKey)
{
    FUNCTION_INIT(
        CertificateX509 crt;
        INIT_BYTE_ARRAY(crt.tbs);
        INIT_BYTE_ARRAY(crt.signR);
        INIT_BYTE_ARRAY(crt.signS);
        INIT_BYTE_ARRAY(crt.pubKey);
        INIT_BYTE_ARRAY(crt.issuer);
        INIT_BYTE_ARRAY(crt.subject);
    );

    CHECK_CALL(DecodeCertificate, certDerCode, &crt);
    CHECK_CALL(CheckValidity, crt.validFrom, crt.validTo);
    CHECK_CALL(ParsePublicKey, &caPublicKey);
    CHECK_SIGN(crt, caPublicKey);
    CHECK_CALL(CheckSerialNumber, crt.serNum);

    FUNCTION_CLEAR();
}

/*
 * https://tools.ietf.org/html/rfc5246
 * This is a sequence (chain) of certificates.  The sender's  certificate MUST come first
 * in the list.  Each following certificate MUST directly certify the one preceding it.
 */


/*
 * Handshake Message: certificate consist of the list of certificates.
 * Certificate length (3 bytes)
 * DER encoded certificate
 * The first is server’s certificate
 * Other certificates are optional
 * Usually intermediate CA certificates
 */

// Parses each certificate from list.
PKIError ParseCertificateChain (ByteArray *chainDerCode, CertificateX509 *chainCrt,
                                uint8_t chainLen)
{
    FUNCTION_INIT(
        int i;
        CHECK_NULL(chainDerCode, PKI_NULL_PASSED);
        CHECK_NULL(chainCrt, PKI_NULL_PASSED);
        CHECK_LESS_EQUAL(chainLen, MAX_CHAIN_LEN, PKI_WRONG_ARRAY_LEN);
    );
    for (i = 0; i < chainLen; i++)
    {
        CHECK_CALL(DecodeCertificate, (*chainDerCode), chainCrt);
#ifdef X509_DEBUG
        PrintCertificate(chainCrt);
#endif
        chainDerCode++;
        chainCrt++;
    }
    FUNCTION_CLEAR();
}

// Loads certificates from TLS message
PKIError LoadCertificateChain (ByteArray msg, ByteArray *chain, uint8_t *chainLength)
{
    FUNCTION_INIT(
        CHECK_NULL(msg.data, PKI_NULL_PASSED);
        CHECK_LESS_EQUAL(3, msg.len, PKI_WRONG_ARRAY_LEN);
        CHECK_NULL(chain, PKI_NULL_PASSED);
        CHECK_NULL(chainLength, PKI_NULL_PASSED);
        uint32_t tmpLengthChain = 0;
        *chainLength = 0;
    );

    CHECK_COND(msg.data[0] != 0 || msg.data[1] != 0 || msg.data[2] != 3, PKI_SUCCESS);
#ifdef X509_DEBUG
    printf("start chain parsing\n");
#endif
    while (msg.len > 0)
    {
#ifdef X509_DEBUG
        printf("chain parsing: %d\n", msg.len);
#endif
        CHECK_LESS_EQUAL(3, msg.len, PKI_WRONG_ARRAY_LEN);
        tmpLengthChain = (((uint32_t) msg.data[0]) << 16) | (((uint32_t) msg.data[1]) << 8) | msg.data[2];
        CHECK_INC_BYTE_ARRAY(msg, 3);
        (*chain).data = msg.data;
        (*chain).len = tmpLengthChain;
        chain ++;
        (*chainLength) ++;
        CHECK_LESS_EQUAL((*chainLength), MAX_CHAIN_LEN, PKI_WRONG_ARRAY_LEN);
        CHECK_INC_BYTE_ARRAY(msg, tmpLengthChain); // Check this
    }
    FUNCTION_CLEAR();
}

/*
 * Certificate validation requires that root keys be distributed independently, 
 * the self-signed certificate that specifies the root certificate authority MAY be omitted 
 * from the chain, under the assumption that the remote end must already possess it in order to
 * validate it in any case.
 */

// Verifies each certificate from list using next public key from list
PKIError CheckCertificateChain (CertificateX509 *chainCrt, uint8_t chainLen, ByteArray caPubKey)
{
    FUNCTION_INIT(
        int i;
        CHECK_NULL(chainCrt, PKI_NULL_PASSED);
        CHECK_LESS_EQUAL(chainLen, MAX_CHAIN_LEN, PKI_WRONG_ARRAY_LEN);
    );
    for (i = 0; i < chainLen - 1; i++)
    {
       ParsePublicKey(&(chainCrt + 1)->pubKey);
       CHECK_SIGN(*chainCrt, (chainCrt + 1)->pubKey);
       CHECK_CALL(CheckSerialNumber, chainCrt->serNum);
       chainCrt++;
    }
    CHECK_SIGN(*chainCrt, caPubKey);
    CHECK_CALL(CheckSerialNumber, chainCrt->serNum);
    FUNCTION_CLEAR();
}

