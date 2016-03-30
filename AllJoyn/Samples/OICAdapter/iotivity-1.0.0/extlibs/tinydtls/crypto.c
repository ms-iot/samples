/* dtls -- a very basic DTLS implementation
 *
 * Copyright (C) 2011--2012 Olaf Bergmann <bergmann@tzi.org>
 * Copyright (C) 2013 Hauke Mehrtens <hauke@hauke-m.de>
 *
 *
 * Modified source code for micro-ecc porting,
 *
 * Following functions are removed:
 *   - dtls_ec_key_to_uint32
 *   - dtls_ec_key_from_uint32
 * Following functions are modified:
 *   - dtls_ecdh_pre_master_secret
 *   - dtls_ecdsa_generate_key
 *   - dtls_ecdsa_create_sig_hash
 *   - dtls_ecdsa_verify_sig_hash
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>

#include "tinydtls.h"
#include "dtls_config.h"

#ifdef HAVE_ASSERT_H
#include <assert.h>
#else
#define assert(x)
#endif

#include "global.h"
#include "debug.h"
#include "numeric.h"
#include "dtls.h"
#include "crypto.h"
#include "ccm.h"
#include "ecc/ecc.h"
#include "aes/rijndael.h"
#include "sha2/sha2.h"
#include "prng.h"
#include "netq.h"
#include "hmac.h"

#ifndef WITH_CONTIKI
#include <pthread.h>
#endif

#define HMAC_UPDATE_SEED(Context,Seed,Length)		\
  if (Seed) dtls_hmac_update(Context, (Seed), (Length))

static struct dtls_cipher_context_t cipher_context;
#ifndef WITH_CONTIKI
static pthread_mutex_t cipher_context_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

static struct dtls_cipher_context_t *dtls_cipher_context_get(void)
{
#ifndef WITH_CONTIKI
  pthread_mutex_lock(&cipher_context_mutex);
#endif
  return &cipher_context;
}

static void dtls_cipher_context_release(void)
{
#ifndef WITH_CONTIKI
  pthread_mutex_unlock(&cipher_context_mutex);
#endif
}

#ifndef WITH_CONTIKI
void crypto_init()
{
}

static dtls_handshake_parameters_t *dtls_handshake_malloc() {
  return malloc(sizeof(dtls_handshake_parameters_t));
}

static void dtls_handshake_dealloc(dtls_handshake_parameters_t *handshake) {
  free(handshake);
}

static dtls_security_parameters_t *dtls_security_malloc() {
  return malloc(sizeof(dtls_security_parameters_t));
}

static void dtls_security_dealloc(dtls_security_parameters_t *security) {
  free(security);
}
#else /* WITH_CONTIKI */

#include "memb.h"
MEMB(handshake_storage, dtls_handshake_parameters_t, DTLS_HANDSHAKE_MAX);
MEMB(security_storage, dtls_security_parameters_t, DTLS_SECURITY_MAX);

void crypto_init() {
  memb_init(&handshake_storage);
  memb_init(&security_storage);
}

static dtls_handshake_parameters_t *dtls_handshake_malloc() {
  return memb_alloc(&handshake_storage);
}

static void dtls_handshake_dealloc(dtls_handshake_parameters_t *handshake) {
  memb_free(&handshake_storage, handshake);
}

static dtls_security_parameters_t *dtls_security_malloc() {
  return memb_alloc(&security_storage);
}

static void dtls_security_dealloc(dtls_security_parameters_t *security) {
  memb_free(&security_storage, security);
}
#endif /* WITH_CONTIKI */

dtls_handshake_parameters_t *dtls_handshake_new()
{
  dtls_handshake_parameters_t *handshake;

  handshake = dtls_handshake_malloc();
  if (!handshake) {
    dtls_crit("can not allocate a handshake struct\n");
    return NULL;
  }

  memset(handshake, 0, sizeof(*handshake));

  if (handshake) {
    /* initialize the handshake hash wrt. the hard-coded DTLS version */
    dtls_debug("DTLSv12: initialize HASH_SHA256\n");
    /* TLS 1.2:  PRF(secret, label, seed) = P_<hash>(secret, label + seed) */
    /* FIXME: we use the default SHA256 here, might need to support other
              hash functions as well */
    dtls_hash_init(&handshake->hs_state.hs_hash);
  }
  return handshake;
}

void dtls_handshake_free(dtls_handshake_parameters_t *handshake)
{
  if (!handshake)
    return;

  netq_delete_all(handshake->reorder_queue);
  dtls_handshake_dealloc(handshake);
}

dtls_security_parameters_t *dtls_security_new()
{
  dtls_security_parameters_t *security;

  security = dtls_security_malloc();
  if (!security) {
    dtls_crit("can not allocate a security struct\n");
    return NULL;
  }

  memset(security, 0, sizeof(*security));

  if (security) {
    security->cipher = TLS_NULL_WITH_NULL_NULL;
    security->compression = TLS_COMPRESSION_NULL;
  }
  return security;
}

void dtls_security_free(dtls_security_parameters_t *security)
{
  if (!security)
    return;

  dtls_security_dealloc(security);
}

size_t
dtls_p_hash(dtls_hashfunc_t h,
	    const unsigned char *key, size_t keylen,
	    const unsigned char *label, size_t labellen,
	    const unsigned char *random1, size_t random1len,
	    const unsigned char *random2, size_t random2len,
	    unsigned char *buf, size_t buflen) {
  dtls_hmac_context_t *hmac_a, *hmac_p;

  unsigned char A[DTLS_HMAC_DIGEST_SIZE];
  unsigned char tmp[DTLS_HMAC_DIGEST_SIZE];
  size_t dlen;			/* digest length */
  size_t len = 0;			/* result length */

  hmac_a = dtls_hmac_new(key, keylen);
  if (!hmac_a)
    return 0;

  /* calculate A(1) from A(0) == seed */
  HMAC_UPDATE_SEED(hmac_a, label, labellen);
  HMAC_UPDATE_SEED(hmac_a, random1, random1len);
  HMAC_UPDATE_SEED(hmac_a, random2, random2len);

  dlen = dtls_hmac_finalize(hmac_a, A);

  hmac_p = dtls_hmac_new(key, keylen);
  if (!hmac_p)
    goto error;

  while (len + dlen < buflen) {

    /* FIXME: rewrite loop to avoid superflous call to dtls_hmac_init() */
    dtls_hmac_init(hmac_p, key, keylen);
    dtls_hmac_update(hmac_p, A, dlen);

    HMAC_UPDATE_SEED(hmac_p, label, labellen);
    HMAC_UPDATE_SEED(hmac_p, random1, random1len);
    HMAC_UPDATE_SEED(hmac_p, random2, random2len);

    len += dtls_hmac_finalize(hmac_p, tmp);
    memcpy(buf, tmp, dlen);
    buf += dlen;

    /* calculate A(i+1) */
    dtls_hmac_init(hmac_a, key, keylen);
    dtls_hmac_update(hmac_a, A, dlen);
    dtls_hmac_finalize(hmac_a, A);
  }

  dtls_hmac_init(hmac_p, key, keylen);
  dtls_hmac_update(hmac_p, A, dlen);

  HMAC_UPDATE_SEED(hmac_p, label, labellen);
  HMAC_UPDATE_SEED(hmac_p, random1, random1len);
  HMAC_UPDATE_SEED(hmac_p, random2, random2len);

  dtls_hmac_finalize(hmac_p, tmp);
  memcpy(buf, tmp, buflen - len);

 error:
  dtls_hmac_free(hmac_a);
  dtls_hmac_free(hmac_p);

  return buflen;
}

size_t
dtls_prf(const unsigned char *key, size_t keylen,
	 const unsigned char *label, size_t labellen,
	 const unsigned char *random1, size_t random1len,
	 const unsigned char *random2, size_t random2len,
	 unsigned char *buf, size_t buflen) {

  /* Clear the result buffer */
  memset(buf, 0, buflen);
  return dtls_p_hash(HASH_SHA256,
		     key, keylen,
		     label, labellen,
		     random1, random1len,
		     random2, random2len,
		     buf, buflen);
}

void
dtls_mac(dtls_hmac_context_t *hmac_ctx,
	 const unsigned char *record,
	 const unsigned char *packet, size_t length,
	 unsigned char *buf) {
  uint16 L;
  dtls_int_to_uint16(L, length);

  assert(hmac_ctx);
  dtls_hmac_update(hmac_ctx, record +3, sizeof(uint16) + sizeof(uint48));
  dtls_hmac_update(hmac_ctx, record, sizeof(uint8) + sizeof(uint16));
  dtls_hmac_update(hmac_ctx, L, sizeof(uint16));
  dtls_hmac_update(hmac_ctx, packet, length);

  dtls_hmac_finalize(hmac_ctx, buf);
}

static size_t
dtls_ccm_encrypt(aes128_t *ccm_ctx, const unsigned char *src, size_t srclen,
		 unsigned char *buf,
		 unsigned char *nounce,
		 const unsigned char *aad, size_t la) {
  long int len;

  assert(ccm_ctx);

  len = dtls_ccm_encrypt_message(&ccm_ctx->ctx, 8 /* M */,
				 max(2, 15 - DTLS_CCM_NONCE_SIZE),
				 nounce,
				 buf, srclen,
				 aad, la);
  return len;
}

static size_t
dtls_ccm_decrypt(aes128_t *ccm_ctx, const unsigned char *src,
		 size_t srclen, unsigned char *buf,
		 unsigned char *nounce,
		 const unsigned char *aad, size_t la) {
  long int len;

  assert(ccm_ctx);

  len = dtls_ccm_decrypt_message(&ccm_ctx->ctx, 8 /* M */,
				 max(2, 15 - DTLS_CCM_NONCE_SIZE),
				 nounce,
				 buf, srclen,
				 aad, la);
  return len;
}

static size_t
dtls_cbc_encrypt(aes128_t *aes_ctx,
                 unsigned char *key, size_t keylen,
                 const unsigned char *iv,
                 const unsigned char *src, size_t srclen,
                 unsigned char *buf) {

    unsigned char cbc[DTLS_BLK_LENGTH];
    unsigned char tmp[DTLS_BLK_LENGTH];
    unsigned char *pos;
    const unsigned char *dtls_hdr = NULL;
    int i, j;
    int blocks;
    dtls_hmac_context_t* hmac_ctx = NULL;
    int paddinglen = 0;

    pos = buf;

    dtls_hdr = src - DTLS_CBC_IV_LENGTH - sizeof(dtls_record_header_t);

    //Calculate MAC : Append the MAC code to end of content
    hmac_ctx = dtls_hmac_new(key, keylen);
    dtls_mac(hmac_ctx,
             dtls_hdr,
             src, srclen,
             buf + srclen);
    dtls_hmac_free(hmac_ctx);
    
    dtls_debug_dump("[MAC]",
                    buf + srclen,
                    DTLS_HMAC_DIGEST_SIZE);

    paddinglen = DTLS_BLK_LENGTH - ((srclen + DTLS_HMAC_DIGEST_SIZE) % DTLS_BLK_LENGTH);
    
    //TLS padding
    memset(buf + (srclen + DTLS_HMAC_DIGEST_SIZE), paddinglen - 1, paddinglen);

    memcpy(cbc, iv, DTLS_BLK_LENGTH);
    blocks = (srclen + DTLS_HMAC_DIGEST_SIZE + paddinglen) / DTLS_BLK_LENGTH;

    for (i = 0; i < blocks; i++) {
        for (j = 0; j < DTLS_BLK_LENGTH; j++) {
            cbc[j] ^= pos[j];
        }

        rijndael_encrypt(&aes_ctx->ctx, cbc, tmp);
        memcpy(cbc, tmp, DTLS_BLK_LENGTH);
        memcpy(pos, cbc, DTLS_BLK_LENGTH);
        pos += DTLS_BLK_LENGTH;
    }

    dtls_debug_dump("[Encrypted Data]",
                    buf,
                    srclen + DTLS_HMAC_DIGEST_SIZE + paddinglen);
    
    return srclen + DTLS_HMAC_DIGEST_SIZE + paddinglen;
}


static size_t
dtls_cbc_decrypt(aes128_t *aes_ctx,
                 unsigned char *key, size_t keylen,
                 const unsigned char *iv,
                 const unsigned char *src, size_t srclen,
                 unsigned char *buf) {

    unsigned char cbc[DTLS_BLK_LENGTH];
    unsigned char tmp[DTLS_BLK_LENGTH];
    unsigned char tmp2[DTLS_BLK_LENGTH];
    unsigned char mac_buf[DTLS_HMAC_DIGEST_SIZE] = {0,};
    const unsigned char *dtls_hdr = NULL;
    unsigned char *pos;
    int i, j;
    int blocks;
    int depaddinglen = 0;
    dtls_hmac_context_t* hmac_ctx = NULL;

    pos = buf;

    dtls_hdr = src - DTLS_CBC_IV_LENGTH - sizeof(dtls_record_header_t);

    memcpy(cbc, iv, DTLS_BLK_LENGTH);
    blocks = srclen / DTLS_BLK_LENGTH;

    for (i = 0; i < blocks; i++)
    {
        memcpy(tmp, pos, DTLS_BLK_LENGTH);
        rijndael_decrypt(&aes_ctx->ctx, pos, tmp2);
        memcpy(pos, tmp2, DTLS_BLK_LENGTH);

        for (j = 0; j < DTLS_BLK_LENGTH; j++) {
            pos[j] ^= cbc[j];
        }

        memcpy(cbc, tmp, DTLS_BLK_LENGTH);
        pos += DTLS_BLK_LENGTH;
    }

    //de-padding
    depaddinglen = buf[srclen -1];

    //Calculate MAC
    hmac_ctx = dtls_hmac_new(key, keylen);
    if(!hmac_ctx) {
        return -1;
    }
    dtls_mac(hmac_ctx, dtls_hdr, buf,
             srclen - DTLS_HMAC_DIGEST_SIZE - depaddinglen - 1,
             mac_buf);
    dtls_hmac_free(hmac_ctx);

    dtls_debug_dump("[MAC]",
                    mac_buf,
                    DTLS_HMAC_DIGEST_SIZE);
    dtls_debug_dump("[Decrypted data]",
                    buf,
                    srclen - DTLS_HMAC_DIGEST_SIZE - depaddinglen - 1);

    //verify the MAC
    if(memcmp(mac_buf,
              buf + (srclen - DTLS_HMAC_DIGEST_SIZE - depaddinglen - 1),
              DTLS_HMAC_DIGEST_SIZE) != 0)
    {
        dtls_crit("Failed to verification of MAC\n");
        return -1;
    }

    //verify the padding bytes
    for (i =0; i < depaddinglen; i++)
    {
        if (buf[srclen - depaddinglen - 1 + i] != depaddinglen)
        {
            dtls_crit("Failed to verify padding bytes\n");
            return -1;
        }
    }

    return srclen - DTLS_HMAC_DIGEST_SIZE - depaddinglen - 1;
}

#ifdef DTLS_PSK
int
dtls_psk_pre_master_secret(unsigned char *key, size_t keylen,
			   unsigned char *result, size_t result_len) {
  unsigned char *p = result;

  if (result_len < (2 * (sizeof(uint16) + keylen))) {
    return -1;
  }

  dtls_int_to_uint16(p, keylen);
  p += sizeof(uint16);

  memset(p, 0, keylen);
  p += keylen;

  memcpy(p, result, sizeof(uint16));
  p += sizeof(uint16);

  memcpy(p, key, keylen);

  return 2 * (sizeof(uint16) + keylen);
}
#endif /* DTLS_PSK */

#if defined(DTLS_ECC) || defined(DTLS_X509)

int dtls_ec_key_from_uint32_asn1(const uint32_t *key, size_t key_size,
				 unsigned char *buf) {
  int i;
  unsigned char *buf_orig = buf;
  int first = 1;

  for (i = (key_size / sizeof(uint32_t)) - 1; i >= 0 ; i--) {
    if (key[i] == 0)
      continue;
    /* the first bit has to be set to zero, to indicate a poritive integer */
    if (first && key[i] & 0x80000000) {
      *buf = 0;
      buf++;
      dtls_int_to_uint32(buf, key[i]);
      buf += 4;
    } else if (first && !(key[i] & 0xFF800000)) {
      buf[0] = (key[i] >> 16) & 0xff;
      buf[1] = (key[i] >> 8) & 0xff;
      buf[2] = key[i] & 0xff;
      buf += 3;
    } else if (first && !(key[i] & 0xFFFF8000)) {
      buf[0] = (key[i] >> 8) & 0xff;
      buf[1] = key[i] & 0xff;
      buf += 2;
    } else if (first && !(key[i] & 0xFFFFFF80)) {
      buf[0] = key[i] & 0xff;
      buf += 1;
    } else {
      dtls_int_to_uint32(buf, key[i]);
      buf += 4;
    }
    first = 0;
  }
  return buf - buf_orig;
}

int dtls_ecdh_pre_master_secret(unsigned char *priv_key,
				   unsigned char *pub_key_x,
                                   unsigned char *pub_key_y,
                                   size_t key_size,
                                   unsigned char *result,
                                   size_t result_len) {

  uint8_t publicKey[64];
  uint8_t privateKey[32];

  if (result_len < key_size) {
    return -1;
  }


  memcpy(publicKey, pub_key_x, 32);
  memcpy(publicKey + 32, pub_key_y, 32);
  memcpy(privateKey, priv_key, 32);
  uECC_shared_secret(publicKey, privateKey, result);

  return key_size;
}

void
dtls_ecdsa_generate_key(unsigned char *priv_key,
			unsigned char *pub_key_x,
			unsigned char *pub_key_y,
			size_t key_size) {

  uint8_t publicKey[64];
  uint8_t privateKey[32];

  uECC_make_key(publicKey, privateKey);
  memcpy(pub_key_x, publicKey, 32);
  memcpy(pub_key_y, publicKey + 32, 32);
  memcpy(priv_key, privateKey, 32);

}

/* rfc4492#section-5.4 */
void
dtls_ecdsa_create_sig_hash(const unsigned char *priv_key, size_t key_size,
                           const unsigned char *sign_hash, size_t sign_hash_size,
                           uint32_t point_r[9], uint32_t point_s[9])
{
    uint8_t sign[64];

    // Check the buffers
    if (priv_key == NULL || key_size < 32)
        return 0;
    if (sign_hash == NULL || sign_hash_size < 32)
        return 0;

    uECC_sign(priv_key, sign_hash, sign);

    int i;
    for (i = 0; i < 32; i++)
    {
        ((uint8_t *) point_r)[i] = sign[31 - i];
        ((uint8_t *) point_s)[i] = sign[63 - i];
    }
}

void
dtls_ecdsa_create_sig(const unsigned char *priv_key, size_t key_size,
		      const unsigned char *client_random, size_t client_random_size,
		      const unsigned char *server_random, size_t server_random_size,
		      const unsigned char *keyx_params, size_t keyx_params_size,
		      uint32_t point_r[9], uint32_t point_s[9]) {
  dtls_hash_ctx data;
  unsigned char sha256hash[DTLS_HMAC_DIGEST_SIZE];

  dtls_hash_init(&data);
  dtls_hash_update(&data, client_random, client_random_size);
  dtls_hash_update(&data, server_random, server_random_size);
  dtls_hash_update(&data, keyx_params, keyx_params_size);
  dtls_hash_finalize(sha256hash, &data);

  dtls_ecdsa_create_sig_hash(priv_key, key_size, sha256hash,
			     sizeof(sha256hash), point_r, point_s);
}

/* rfc4492#section-5.4 */
int
dtls_ecdsa_verify_sig_hash(const unsigned char *pub_key_x,
                           const unsigned char *pub_key_y, size_t key_size,
                           const unsigned char *sign_hash, size_t sign_hash_size,
                           unsigned char *result_r, unsigned char *result_s)
{
    uint8_t publicKey[64];
    uint8_t sign[64];

    // Check the buffers
    if (pub_key_x == NULL || pub_key_y == NULL || key_size < 32)
        return 0;
    if (sign_hash == NULL || sign_hash_size < 32)
        return 0;
    if (result_r == NULL || result_s == NULL)
        return 0;

    // Copy the public key into a single buffer
    memcpy(publicKey, pub_key_x, 32);
    memcpy(publicKey + 32, pub_key_y, 32);

    // Copy the signature into a single buffer
    memcpy(sign, result_r, 32);
    memcpy(sign + 32, result_s, 32);

    return uECC_verify(publicKey, sign_hash, sign);
}

int
dtls_ecdsa_verify_sig(const unsigned char *pub_key_x,
		      const unsigned char *pub_key_y, size_t key_size,
		      const unsigned char *client_random, size_t client_random_size,
		      const unsigned char *server_random, size_t server_random_size,
		      const unsigned char *keyx_params, size_t keyx_params_size,
		      unsigned char *result_r, unsigned char *result_s) {
  dtls_hash_ctx data;
  unsigned char sha256hash[DTLS_HMAC_DIGEST_SIZE];

  dtls_hash_init(&data);
  dtls_hash_update(&data, client_random, client_random_size);
  dtls_hash_update(&data, server_random, server_random_size);
  dtls_hash_update(&data, keyx_params, keyx_params_size);
  dtls_hash_finalize(sha256hash, &data);

  return dtls_ecdsa_verify_sig_hash(pub_key_x, pub_key_y, key_size, sha256hash,
				    sizeof(sha256hash), result_r, result_s);
}
#endif /* DTLS_ECC */

#if defined(DTLS_PSK) && defined(DTLS_ECC)
int dtls_ecdhe_psk_pre_master_secret(unsigned char *psk, size_t psklen,
                                     unsigned char *ecc_priv_key,
                                     unsigned char *ecc_pub_key_x,
                                     unsigned char *ecc_pub_key_y,
                                     size_t ecc_key_size,
                                     unsigned char *result,
                                     size_t result_len)
{
  uint8_t eccPublicKey[64];
  uint8_t eccPrivateKey[32];
  unsigned char *p = result;

  if (result_len < uECC_BYTES + psklen + (sizeof(uint16) * 2)) {
    return -1;
  }

  dtls_int_to_uint16(p, uECC_BYTES);
  p += sizeof(uint16);

  memcpy(eccPublicKey, ecc_pub_key_x, 32);
  memcpy(eccPublicKey + 32, ecc_pub_key_y, 32);
  memcpy(eccPrivateKey, ecc_priv_key, 32);
  uECC_shared_secret(eccPublicKey, eccPrivateKey, p);
  p += uECC_BYTES;

  dtls_int_to_uint16(p, psklen);
  p += sizeof(uint16);

  memcpy(p, psk, psklen);

  return uECC_BYTES + psklen + (sizeof(uint16) * 2);
}
#endif /* defined(DTLS_PSK) && defined(DTLS_ECC) */

int
dtls_encrypt(const unsigned char *src, size_t length,
	     unsigned char *buf,
	     unsigned char *nounce,
	     unsigned char *key, size_t keylen,
	     const unsigned char *aad, size_t la,
	     const dtls_cipher_t cipher)
{
  int ret = 0;
  struct dtls_cipher_context_t *ctx = dtls_cipher_context_get();

  if(cipher == TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8 ||
     cipher == TLS_PSK_WITH_AES_128_CCM_8) {
      ret = rijndael_set_key_enc_only(&ctx->data.ctx, key, 8 * keylen);
      if (ret < 0) {
        /* cleanup everything in case the key has the wrong size */
        dtls_warn("cannot set rijndael key\n");
        goto error;
      }

      if (src != buf)
        memmove(buf, src, length);
      ret = dtls_ccm_encrypt(&ctx->data, src, length, buf, nounce, aad, la);
  }
  if(cipher == TLS_ECDH_anon_WITH_AES_128_CBC_SHA_256 ||
     cipher == TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA_256) {
      ret = rijndael_set_key(&ctx->data.ctx, key, 8 * keylen);
      if (ret < 0) {
        /* cleanup everything in case the key has the wrong size */
        dtls_warn("cannot set rijndael key\n");
        goto error;
      }

      if (src != buf)
        memmove(buf, src, length);
      ret = dtls_cbc_encrypt(&ctx->data, key, keylen, nounce, src, length, buf);
  }

error:
  dtls_cipher_context_release();
  return ret;
}

int
dtls_decrypt(const unsigned char *src, size_t length,
	     unsigned char *buf,
	     unsigned char *nounce,
	     unsigned char *key, size_t keylen,
	     const unsigned char *aad, size_t la,
	     const dtls_cipher_t cipher)
{
  int ret = 0;
  struct dtls_cipher_context_t *ctx = dtls_cipher_context_get();

  if(cipher == TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8 ||
     cipher == TLS_PSK_WITH_AES_128_CCM_8) {
      ret = rijndael_set_key_enc_only(&ctx->data.ctx, key, 8 * keylen);
      if (ret < 0) {
        /* cleanup everything in case the key has the wrong size */
        dtls_warn("cannot set rijndael key\n");
        goto error;
      }

      if (src != buf)
        memmove(buf, src, length);
      ret = dtls_ccm_decrypt(&ctx->data, src, length, buf, nounce, aad, la);
  }

  if(cipher == TLS_ECDH_anon_WITH_AES_128_CBC_SHA_256 ||
     cipher == TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA_256) {
      ret = rijndael_set_key(&ctx->data.ctx, key, 8 * keylen);
      if (ret < 0) {
        /* cleanup everything in case the key has the wrong size */
        dtls_warn("cannot set rijndael key\n");
        goto error;
      }

      if (src != buf)
        memmove(buf, src, length);
      ret = dtls_cbc_decrypt(&ctx->data, key, keylen, nounce, src, length, buf);
    }

error:
  dtls_cipher_context_release();
  return ret;
}

