
/* This is needed for apple */
#define __APPLE_USE_RFC_3542

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <signal.h>

#include "tinydtls.h"
#include "dtls.h"
#include "debug.h"

#ifdef DTLS_X509
#define DTLS_PRIVATE_KEY_SIZE        (32)
#define DTLS_PUBLIC_KEY_SIZE         (64)
#endif

#define DEFAULT_PORT 20220

/**
 * @struct byte_array
 *
 * General purpose byte array structure.
 *
 * Contains pointer to array of bytes and it's length.
 */

typedef struct
{
    uint8_t *data;    /**< Pointer to the byte array */
    size_t len;      /**< Data size */
} byte_array;


/**@def BYTE_ARRAY_INITIALIZER
 *
 * Initializes of existing byte array pointer to \a NULL.
 */
#undef BYTE_ARRAY_INITIALIZER
#define BYTE_ARRAY_INITIALIZER {NULL, 0}

#ifdef DTLS_X509
#define X509_OPTIONS         "x:r:u:"
#define SERVER_CRT_LEN 295
static const unsigned char g_server_certificate[SERVER_CRT_LEN] = {
        0x00, 0x01, 0x24,
        0x30, 0x82, 0x01, 0x20, 0x30, 0x81, 0xc4, 0xa0,
        0x03, 0x02, 0x01, 0x01, 0x02, 0x02, 0x02, 0x37,
        0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce,
        0x3d, 0x04, 0x03, 0x02, 0x05, 0x00, 0x30, 0x17,
        0x31, 0x15, 0x30, 0x13, 0x06, 0x03, 0x55, 0x04,
        0x03, 0x0c, 0x0c, 0x4c, 0x6f, 0x63, 0x61, 0x6c,
        0x20, 0x49, 0x53, 0x53, 0x55, 0x45, 0x52, 0x30,
        0x1e, 0x17, 0x0d, 0x31, 0x33, 0x30, 0x31, 0x30,
        0x31, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x5a,
        0x17, 0x0d, 0x34, 0x39, 0x30, 0x31, 0x30, 0x31,
        0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x5a, 0x30,
        0x17, 0x31, 0x15, 0x30, 0x13, 0x06, 0x03, 0x55,
        0x04, 0x03, 0x0c, 0x0c, 0x4c, 0x6f, 0x63, 0x61,
        0x6c, 0x20, 0x53, 0x45, 0x52, 0x56, 0x45, 0x52,
        0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86,
        0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a,
        0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03,
        0x42, 0x00, 0x04, 0x07, 0x88, 0x10, 0xdc, 0x62,
        0xd7, 0xe6, 0x9b, 0x7c, 0xad, 0x6e, 0x78, 0xb0,
        0x5f, 0x9a, 0x00, 0x11, 0x74, 0x2c, 0x8b, 0xaf,
        0x09, 0x65, 0x7c, 0x86, 0x8e, 0x55, 0xcb, 0x39,
        0x55, 0x72, 0xc6, 0x65, 0x71, 0xcd, 0x03, 0xdc,
        0x2a, 0x4f, 0x46, 0x5b, 0x14, 0xc8, 0x27, 0x74,
        0xab, 0xf4, 0x1f, 0xc1, 0x35, 0x0d, 0x42, 0xbc,
        0xc2, 0x9f, 0xb5, 0xc1, 0x79, 0xb6, 0x8b, 0xca,
        0xdb, 0xff, 0x82, 0x30, 0x0c, 0x06, 0x08, 0x2a,
        0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x05,
        0x00, 0x03, 0x49, 0x00, 0x30, 0x46, 0x02, 0x21,
        0x00, 0xb1, 0x81, 0x81, 0x92, 0x0e, 0x76, 0x7c,
        0xeb, 0xf5, 0x37, 0xde, 0x27, 0xc4, 0x01, 0xc8,
        0x96, 0xc3, 0xe5, 0x9f, 0x47, 0x7e, 0x25, 0x92,
        0xa4, 0xba, 0x22, 0x25, 0xa3, 0x81, 0x19, 0xcf,
        0x0d, 0x02, 0x21, 0x00, 0xca, 0x92, 0xbe, 0x79,
        0xc7, 0x82, 0x84, 0x64, 0xc4, 0xc4, 0xf4, 0x3d,
        0x69, 0x79, 0x68, 0xc0, 0xf1, 0xba, 0xaf, 0x6c,
        0xbb, 0xdd, 0x54, 0x7d, 0x07, 0xe7, 0x53, 0x3b,
        0xc3, 0x1b, 0x87, 0x04};

//default server's key pair
static const unsigned char x509_priv_key[] = {
        0xaa, 0xa3, 0x46, 0xf1, 0x3c, 0x56, 0x5d, 0x08,
        0x5e, 0x59, 0xba, 0x7f, 0xd2, 0x21, 0x62, 0xc6,
        0xcc, 0x5d, 0xfa, 0x3f, 0xb5, 0x25, 0xa9, 0x89,
        0x4f, 0x32, 0xe8, 0x2a, 0xe0, 0xee, 0x9b, 0x4c};

static const unsigned char x509_pub_key_x[] = {
        0x07, 0x88, 0x10, 0xdc, 0x62, 0xd7, 0xe6, 0x9b,
        0x7c, 0xad, 0x6e, 0x78, 0xb0, 0x5f, 0x9a, 0x00,
        0x11, 0x74, 0x2c, 0x8b, 0xaf, 0x09, 0x65, 0x7c,
        0x86, 0x8e, 0x55, 0xcb, 0x39, 0x55, 0x72, 0xc6};

static const unsigned char x509_pub_key_y[] = {
        0x65, 0x71, 0xcd, 0x03, 0xdc, 0x2a, 0x4f, 0x46,
        0x5b, 0x14, 0xc8, 0x27, 0x74, 0xab, 0xf4, 0x1f,
        0xc1, 0x35, 0x0d, 0x42, 0xbc, 0xc2, 0x9f, 0xb5,
        0xc1, 0x79, 0xb6, 0x8b, 0xca, 0xdb, 0xff, 0x82};

//default CA pub key
static const unsigned char x509_ca_pub_x[] = {
        0x57, 0x94, 0x7f, 0x98, 0x7a, 0x02, 0x67, 0x09,
        0x25, 0xc1, 0xcb, 0x5a, 0xf5, 0x46, 0xfb, 0xad,
        0xf7, 0x68, 0x94, 0x8c, 0xa7, 0xe3, 0xf0, 0x5b,
        0xc3, 0x6b, 0x5c, 0x9b, 0xd3, 0x7d, 0x74, 0x12
};

static const unsigned char x509_ca_pub_y[] = {
        0xce, 0x68, 0xbc, 0x55, 0xf5, 0xf8, 0x1b, 0x3d,
        0xef, 0xed, 0x1f, 0x2b, 0xd2, 0x69, 0x5d, 0xcf,
        0x79, 0x16, 0xa6, 0xbd, 0x97, 0x96, 0x27, 0x60,
        0x5d, 0xd1, 0xb7, 0x93, 0xa2, 0x4a, 0x62, 0x4d
};

static const unsigned char client_pub_key_x[] = {
        0xe3, 0xd1, 0x67, 0x1e, 0xdc, 0x46, 0xf4, 0x19,
        0x50, 0x15, 0x2e, 0x3a, 0x2f, 0xd8, 0x68, 0x6b,
        0x37, 0x32, 0x84, 0x9e, 0x83, 0x81, 0xbf, 0x25,
        0x5d, 0xbb, 0x18, 0x07, 0x3c, 0xbd, 0xf3, 0xab};

static const unsigned char client_pub_key_y[] = {
        0xd3, 0xbf, 0x53, 0x59, 0xc9, 0x1e, 0xce, 0x5b,
        0x39, 0x6a, 0xe5, 0x60, 0xf3, 0x70, 0xdb, 0x66,
        0xb6, 0x80, 0xcb, 0x65, 0x0b, 0x35, 0x2a, 0x62,
        0x44, 0x89, 0x63, 0x64, 0x6f, 0x6f, 0xbd, 0xf0};

static unsigned char x509_server_cert[DTLS_MAX_CERT_SIZE];
static size_t x509_server_cert_len = 0;
static unsigned char x509_server_priv[DTLS_PRIVATE_KEY_SIZE+1];
static size_t x509_server_priv_is_set = 0;
static unsigned char x509_ca_pub[DTLS_PUBLIC_KEY_SIZE+1];
static size_t x509_ca_pub_is_set = 0;

static int x509_info_from_file = 0;
#endif /*DTLS_X509*/
#ifdef DTLS_ECC
static const unsigned char ecdsa_priv_key[] = {
			0xD9, 0xE2, 0x70, 0x7A, 0x72, 0xDA, 0x6A, 0x05,
			0x04, 0x99, 0x5C, 0x86, 0xED, 0xDB, 0xE3, 0xEF,
			0xC7, 0xF1, 0xCD, 0x74, 0x83, 0x8F, 0x75, 0x70,
			0xC8, 0x07, 0x2D, 0x0A, 0x76, 0x26, 0x1B, 0xD4};

static const unsigned char ecdsa_pub_key_x[] = {
			0xD0, 0x55, 0xEE, 0x14, 0x08, 0x4D, 0x6E, 0x06,
			0x15, 0x59, 0x9D, 0xB5, 0x83, 0x91, 0x3E, 0x4A,
			0x3E, 0x45, 0x26, 0xA2, 0x70, 0x4D, 0x61, 0xF2,
			0x7A, 0x4C, 0xCF, 0xBA, 0x97, 0x58, 0xEF, 0x9A};

static const unsigned char ecdsa_pub_key_y[] = {
			0xB4, 0x18, 0xB6, 0x4A, 0xFE, 0x80, 0x30, 0xDA,
			0x1D, 0xDC, 0xF4, 0xF4, 0x2E, 0x2F, 0x26, 0x31,
			0xD0, 0x43, 0xB1, 0xFB, 0x03, 0xE2, 0x2F, 0x4D,
			0x17, 0xDE, 0x43, 0xF9, 0xF9, 0xAD, 0xEE, 0x70};
#endif /*DTLS_ECC*/
#if 0
/* SIGINT handler: set quit to 1 for graceful termination */
void
handle_sigint(int signum) {
  dsrv_stop(dsrv_get_context());
}
#endif

#ifdef DTLS_X509
ssize_t
read_from_file(char *arg, unsigned char *buf, size_t max_buf_len) {
  FILE *f;
  ssize_t result = 0;

  f = fopen(arg, "r");
  if (f == NULL)
    return -1;

  while (!feof(f)) {
    size_t bytes_read;
    bytes_read = fread(buf, 1, max_buf_len, f);
    if (ferror(f)) {
      result = -1;
      break;
    }

    buf += bytes_read;
    result += bytes_read;
    max_buf_len -= bytes_read;
  }

  fclose(f);
  return result;
}
#endif /*DTLS_X509*/

#ifdef DTLS_PSK

#define PSK_SERVER_HINT  "Server_identity"

/* This function is the "key store" for tinyDTLS. It is called to
 * retrieve a key for the given identity within this particular
 * session. */
static int
get_psk_info(struct dtls_context_t *ctx, const session_t *session,
	     dtls_credentials_type_t type,
	     const unsigned char *id, size_t id_len,
	     unsigned char *result, size_t result_length) {

  (void)ctx;
  (void)session;
  struct keymap_t {
    unsigned char *id;
    size_t id_length;
    unsigned char *key;
    size_t key_length;
  } psk[3] = {
    { (unsigned char *)"Client_identity", 15,
      (unsigned char *)"secretPSK", 9 },
    { (unsigned char *)"default identity", 16,
      (unsigned char *)"\x11\x22\x33", 3 },
    { (unsigned char *)"\0", 2,
      (unsigned char *)"", 1 }
  };

  switch (type) {
  case DTLS_PSK_HINT:
    if (result_length < strlen(PSK_SERVER_HINT)) {
      dtls_warn("cannot set psk_hint -- buffer too small\n");
      return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
    }

    memcpy(result, PSK_SERVER_HINT, strlen(PSK_SERVER_HINT));
    return strlen(PSK_SERVER_HINT);

  case DTLS_PSK_KEY:
    if (id) {
      int i;
      for (i = 0; i < (int)(sizeof(psk)/sizeof(struct keymap_t)); i++) {
        if (id_len == psk[i].id_length && memcmp(id, psk[i].id, id_len) == 0) {
	  if (result_length < psk[i].key_length) {
	    dtls_warn("buffer too small for PSK");
	    return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
	  }

	  memcpy(result, psk[i].key, psk[i].key_length);
	  return psk[i].key_length;
        }
      }
    }
    break;

  default:
    dtls_warn("unsupported request type: %d\n", type);
  }

  return dtls_alert_fatal_create(DTLS_ALERT_DECRYPT_ERROR);
}

#endif /* DTLS_PSK */

#ifdef DTLS_ECC
static int
get_ecdsa_key(struct dtls_context_t *ctx,
	      const session_t *session,
	      const dtls_ecc_key_t **result) {
    (void)ctx;
    (void)session;
  static const dtls_ecc_key_t ecdsa_key = {
    .curve = DTLS_ECDH_CURVE_SECP256R1,
    .priv_key = ecdsa_priv_key,
    .pub_key_x = ecdsa_pub_key_x,
    .pub_key_y = ecdsa_pub_key_y
  };

  *result = &ecdsa_key;
  return 0;
}

static int
verify_ecdsa_key(struct dtls_context_t *ctx,
		 const session_t *session,
		 const unsigned char *other_pub_x,
		 const unsigned char *other_pub_y,
		 size_t key_size) {
  (void)ctx;
  (void)session;
  (void)other_pub_x;
  (void)other_pub_y;
  (void)key_size;
  return 0;
}
#endif /* DTLS_ECC */

#ifdef DTLS_X509
static int
get_x509_key(struct dtls_context_t *ctx,
          const session_t *session,
          const dtls_ecc_key_t **result) {
    (void)ctx;
    (void)session;
  static dtls_ecc_key_t ecdsa_key = {
    .curve = DTLS_ECDH_CURVE_SECP256R1,
    .priv_key = x509_priv_key,
    .pub_key_x = x509_pub_key_x,
    .pub_key_y = x509_pub_key_y
  };
  if (x509_info_from_file)
      ecdsa_key.priv_key = x509_server_priv;

  *result = &ecdsa_key;
  return 0;
}

static int
get_x509_cert(struct dtls_context_t *ctx,
		const session_t *session,
		const unsigned char **cert,
		size_t *cert_size)
{
    (void)ctx;
    (void)session;
    if (x509_info_from_file)
    {
        *cert = x509_server_cert;
        *cert_size = x509_server_cert_len;
    }
    else
    {
        *cert = g_server_certificate;
        *cert_size = SERVER_CRT_LEN;
    }

    return 0;
}

static int check_certificate(byte_array cert_der_code, byte_array ca_public_key)
{
    (void)cert_der_code;
    (void)ca_public_key;
    return 0;
}

static int verify_x509_cert(struct dtls_context_t *ctx, const session_t *session,
                                  const unsigned char *cert, size_t cert_size,
                                  unsigned char *x,
                                  size_t x_size,
                                  unsigned char *y,
                                  size_t y_size)
{
    int ret;
    const unsigned char *ca_pub_x;
    const unsigned char *ca_pub_y;
    byte_array cert_der_code = BYTE_ARRAY_INITIALIZER;
    byte_array ca_public_key = BYTE_ARRAY_INITIALIZER;
    unsigned char ca_pub_key[DTLS_PUBLIC_KEY_SIZE];
    (void)ctx;
    (void)session;

    if (x509_info_from_file)
    {
        ca_pub_x = x509_ca_pub;
        ca_pub_y = x509_ca_pub + DTLS_PUBLIC_KEY_SIZE/2;
    }
    else
    {
        ca_pub_x = x509_ca_pub_x;
        ca_pub_y = x509_ca_pub_y;
    }

    cert_der_code.data = (uint8_t *)cert;
    cert_der_code.len = cert_size;

    ca_public_key.len = DTLS_PUBLIC_KEY_SIZE;
    ca_public_key.data = ca_pub_key;
    memcpy(ca_public_key.data, ca_pub_x, DTLS_PUBLIC_KEY_SIZE/2);
    memcpy(ca_public_key.data + DTLS_PUBLIC_KEY_SIZE/2, ca_pub_y, DTLS_PUBLIC_KEY_SIZE/2);

    memcpy(x, client_pub_key_x, x_size);
    memcpy(y, client_pub_key_y, y_size);

    ret = (int) check_certificate(cert_der_code, ca_public_key);

  return -ret;
}

static int is_x509_active(struct dtls_context_t *ctx)
{
    (void)ctx;
    return 0;
}
#endif /* DTLS_X509 */


#define DTLS_SERVER_CMD_CLOSE "server:close"
#define DTLS_SERVER_CMD_RENEGOTIATE "server:renegotiate"

static int
read_from_peer(struct dtls_context_t *ctx,
	       session_t *session, uint8 *data, size_t len) {
  size_t i;
  for (i = 0; i < len; i++)
    printf("%c", data[i]);
  if (len >= strlen(DTLS_SERVER_CMD_CLOSE) &&
      !memcmp(data, DTLS_SERVER_CMD_CLOSE, strlen(DTLS_SERVER_CMD_CLOSE))) {
    printf("server: closing connection\n");
    dtls_close(ctx, session);
    return len;
  } else if (len >= strlen(DTLS_SERVER_CMD_RENEGOTIATE) &&
      !memcmp(data, DTLS_SERVER_CMD_RENEGOTIATE, strlen(DTLS_SERVER_CMD_RENEGOTIATE))) {
    printf("server: renegotiate connection\n");
    dtls_renegotiate(ctx, session);
    return len;
  }

  return dtls_write(ctx, session, data, len);
}

static int
send_to_peer(struct dtls_context_t *ctx, 
	     session_t *session, uint8 *data, size_t len) {

  int fd = *(int *)dtls_get_app_data(ctx);
  return sendto(fd, data, len, MSG_DONTWAIT,
		&session->addr.sa, session->size);
}

static int
dtls_handle_read(struct dtls_context_t *ctx) {
  int *fd;
  session_t session;
  static uint8 buf[DTLS_MAX_BUF];
  int len;

  fd = dtls_get_app_data(ctx);

  assert(fd);

  memset(&session, 0, sizeof(session_t));
  session.size = sizeof(session.addr);
  len = recvfrom(*fd, buf, sizeof(buf), MSG_TRUNC,
		 &session.addr.sa, &session.size);

  if (len < 0) {
    perror("recvfrom");
    return -1;
  } else {
    dtls_debug("got %d bytes from port %d\n", len, 
	     ntohs(session.addr.sin6.sin6_port));
    if ((int)(sizeof(buf)) < len) {
      dtls_warn("packet was truncated (%d bytes lost)\n", len - sizeof(buf));
    }
  }

  return dtls_handle_message(ctx, &session, buf, len);
}    

static int
resolve_address(const char *server, struct sockaddr *dst) {
  
  struct addrinfo *res, *ainfo;
  struct addrinfo hints;
  static char addrstr[256];
  int error;

  memset(addrstr, 0, sizeof(addrstr));
  if (server && strlen(server) > 0)
    memcpy(addrstr, server, strlen(server));
  else
    memcpy(addrstr, "localhost", 9);

  memset ((char *)&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_family = AF_UNSPEC;

  error = getaddrinfo(addrstr, "", &hints, &res);

  if (error != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    return error;
  }

  for (ainfo = res; ainfo != NULL; ainfo = ainfo->ai_next) {

    switch (ainfo->ai_family) {
    case AF_INET6:

      memcpy(dst, ainfo->ai_addr, ainfo->ai_addrlen);
      return ainfo->ai_addrlen;
    default:
      ;
    }
  }

  freeaddrinfo(res);
  return -1;
}

static void
usage(const char *program, const char *version) {
  const char *p;

  p = strrchr( program, '/' );
  if ( p )
    program = ++p;

  fprintf(stderr, "%s v%s -- DTLS server implementation\n"
	  "(c) 2011-2014 Olaf Bergmann <bergmann@tzi.org>\n\n"
	  "usage: %s [-A address] [-p port] [-v num] [-a enable|disable]\n"
#ifdef DTLS_X509
      " [-x file] [-r file] [-u file]"
#endif /* DTLS_X509 */
	  "\t-A address\t\tlisten on specified address (default is ::)\n"
	  "\t-p port\t\tlisten on specified port (default is %d)\n"
	  "\t-v num\t\tverbosity level (default: 3)\n"
	  "\t-a enable|disable\t(default: disable)\n"
  	  "\t\t\t\tenable:enable TLS_ECDH_anon_with_AES_128_CBC_SHA_256\n"
	  "\t\t\t\tdisable:disable TLS_ECDH_anon_with_AES_128_CBC_SHA_256\n"
#ifdef DTLS_X509
      "\t-x file\tread Server certificate from file\n"
      "\t-r file\tread Server private key from file\n"
      "\t-u file\tread CA public key from file\n"
#endif /* DTLS_X509 */
      ,program, version, program, DEFAULT_PORT);
}

static dtls_handler_t cb = {
  .write = send_to_peer,
  .read  = read_from_peer,
  .event = NULL,
#ifdef DTLS_PSK
  .get_psk_info = get_psk_info,
#endif /* DTLS_PSK */
#ifdef DTLS_ECC
  .get_ecdsa_key = get_ecdsa_key,
  .verify_ecdsa_key = verify_ecdsa_key,
#endif /* DTLS_ECC */
#ifdef DTLS_X509
  .get_x509_key = get_x509_key,
  .verify_x509_cert = verify_x509_cert,
  .get_x509_cert = get_x509_cert,
  .is_x509_active = is_x509_active,
#endif

};

int
main(int argc, char **argv) {
  dtls_context_t *the_context = NULL;
  log_t log_level = DTLS_LOG_WARN;
  fd_set rfds, wfds;
  struct timeval timeout;
  int fd, opt, result;
  int on = 1;
  dtls_cipher_enable_t ecdh_anon_enalbe = DTLS_CIPHER_DISABLE;
  struct sockaddr_in6 listen_addr;

  memset(&listen_addr, 0, sizeof(struct sockaddr_in6));

  /* fill extra field for 4.4BSD-based systems (see RFC 3493, section 3.4) */
#if defined(SIN6_LEN) || defined(HAVE_SOCKADDR_IN6_SIN6_LEN)
  listen_addr.sin6_len = sizeof(struct sockaddr_in6);
#endif

  listen_addr.sin6_family = AF_INET6;
  listen_addr.sin6_port = htons(DEFAULT_PORT);
  listen_addr.sin6_addr = in6addr_any;

  while ((opt = getopt(argc, argv, "A:p:v:a:")) != -1) {
    switch (opt) {
    case 'A' :
      if (resolve_address(optarg, (struct sockaddr *)&listen_addr) < 0) {
	fprintf(stderr, "cannot resolve address\n");
	exit(-1);
      }
      break;
    case 'p' :
      listen_addr.sin6_port = htons(atoi(optarg));
      break;
    case 'v' :
      log_level = strtol(optarg, NULL, 10);
      break;
    case 'a':
      if( strcmp(optarg, "enable") == 0)
          ecdh_anon_enalbe = DTLS_CIPHER_ENABLE;
      break;
#ifdef DTLS_X509
    case 'x' :
    {
      ssize_t result = read_from_file(optarg, x509_server_cert, DTLS_MAX_CERT_SIZE);
      if (result < 0)
      {
          dtls_warn("Cannot read Server certificate. Using default\n");
      }
      else
      {
          x509_server_cert_len = result;
      }
      break;
    }
    case 'r' :
    {
      ssize_t result = read_from_file(optarg, x509_server_priv, DTLS_PRIVATE_KEY_SIZE+1);
      if (result < 0)
      {
          dtls_warn("Cannot read Server private key. Using default\n");
      }
      else
      {
          x509_server_priv_is_set = result;
      }
      break;
    }
    case 'u' :
    {
      ssize_t result = read_from_file(optarg, x509_ca_pub, DTLS_PUBLIC_KEY_SIZE+1);
      if (result < 0)
      {
          dtls_warn("Cannot read CA public key. Using default\n");
      }
      else
      {
          x509_ca_pub_is_set = result;
      }
      break;
    }
#endif /* DTLS_X509 */
    default:
      usage(argv[0], dtls_package_version());
      exit(1);
    }
  }

  dtls_set_log_level(log_level);

#ifdef DTLS_X509
  if (x509_server_cert_len && x509_server_priv_is_set && x509_ca_pub_is_set)
  {
      x509_info_from_file = 1;
  }
  else if(!(x509_server_cert_len || x509_server_priv_is_set || x509_ca_pub_is_set))
  {
      x509_info_from_file = 0;
  }
  else
  {
      fprintf(stderr,"please set -x, -r, -u options simultaneously");
      usage(argv[0], dtls_package_version());
      exit(1);
  }
#endif /* DTLS_X509 */

  /* init socket and set it to non-blocking */
  fd = socket(listen_addr.sin6_family, SOCK_DGRAM, 0);

  if (fd < 0) {
    dtls_alert("socket: %s\n", strerror(errno));
    return 0;
  }

  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ) < 0) {
    dtls_alert("setsockopt SO_REUSEADDR: %s\n", strerror(errno));
  }
#if 0
  flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
    dtls_alert("fcntl: %s\n", strerror(errno));
    goto error;
  }
#endif
  on = 1;
#ifdef IPV6_RECVPKTINFO
  if (setsockopt(fd, IPPROTO_IPV6, IPV6_RECVPKTINFO, &on, sizeof(on) ) < 0) {
#else /* IPV6_RECVPKTINFO */
  if (setsockopt(fd, IPPROTO_IPV6, IPV6_PKTINFO, &on, sizeof(on) ) < 0) {
#endif /* IPV6_RECVPKTINFO */
    dtls_alert("setsockopt IPV6_PKTINFO: %s\n", strerror(errno));
  }

  if (bind(fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0) {
    dtls_alert("bind: %s\n", strerror(errno));
    goto error;
  }

  dtls_init();

  the_context = dtls_new_context(&fd);

  /* enable/disable tls_ecdh_anon_with_aes_128_cbc_sha_256 */
  dtls_enables_anon_ecdh(the_context, ecdh_anon_enalbe);

  dtls_set_handler(the_context, &cb);

  while (1) {
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);

    FD_SET(fd, &rfds);
    /* FD_SET(fd, &wfds); */
    
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    
    result = select( fd+1, &rfds, &wfds, 0, &timeout);
    
    if (result < 0) {		/* error */
      if (errno != EINTR)
	perror("select");
    } else if (result == 0) {	/* timeout */
    } else {			/* ok */
      if (FD_ISSET(fd, &wfds))
	;
      else if (FD_ISSET(fd, &rfds)) {
	dtls_handle_read(the_context);
      }
    }
  }
  
 error:
  dtls_free_context(the_context);
  exit(0);
}
