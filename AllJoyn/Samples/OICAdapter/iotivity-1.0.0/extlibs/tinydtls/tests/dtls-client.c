#include "tinydtls.h" 

/* This is needed for apple */
#define __APPLE_USE_RFC_3542

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>

#include "global.h"
#include "debug.h"
#include "dtls.h"

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

#define DTLS_PRIVATE_KEY_SIZE        (32)
#define DTLS_PUBLIC_KEY_SIZE         (64)

#define DEFAULT_PORT 20220

#define PSK_CLIENT_IDENTITY  "Client_identity"
#define PSK_SERVER_IDENTITY  "Server_identity"
#define PSK_DEFAULT_KEY      "secretPSK"
#define PSK_OPTIONS          "i:s:k:"
#define X509_OPTIONS         "x:r:u:"

#ifdef __GNUC__
#define UNUSED_PARAM __attribute__((unused))
#else
#define UNUSED_PARAM
#endif /* __GNUC__ */

static char buf[200];
static size_t len = 0;

typedef struct {
  size_t length;               /* length of string */
  unsigned char *s;            /* string data */
} dtls_str;

static dtls_str output_file = { 0, NULL }; /* output file name */

static dtls_context_t *dtls_context = NULL;
static dtls_context_t *orig_dtls_context = NULL;

#ifdef DTLS_X509
#define CLIENT_CRT_LEN 293
static const unsigned char g_client_certificate[CLIENT_CRT_LEN] = {
        0x00, 0x01, 0x22,
        0x30, 0x82, 0x01, 0x1e, 0x30, 0x81, 0xc4, 0xa0,
        0x03, 0x02, 0x01, 0x01, 0x02, 0x02, 0x02, 0x38,
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
        0x6c, 0x20, 0x43, 0x4c, 0x49, 0x45, 0x4e, 0x54,
        0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86,
        0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a,
        0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03,
        0x42, 0x00, 0x04, 0xe3, 0xd1, 0x67, 0x1e, 0xdc,
        0x46, 0xf4, 0x19, 0x50, 0x15, 0x2e, 0x3a, 0x2f,
        0xd8, 0x68, 0x6b, 0x37, 0x32, 0x84, 0x9e, 0x83,
        0x81, 0xbf, 0x25, 0x5d, 0xbb, 0x18, 0x07, 0x3c,
        0xbd, 0xf3, 0xab, 0xd3, 0xbf, 0x53, 0x59, 0xc9,
        0x1e, 0xce, 0x5b, 0x39, 0x6a, 0xe5, 0x60, 0xf3,
        0x70, 0xdb, 0x66, 0xb6, 0x80, 0xcb, 0x65, 0x0b,
        0x35, 0x2a, 0x62, 0x44, 0x89, 0x63, 0x64, 0x6f,
        0x6f, 0xbd, 0xf0, 0x30, 0x0c, 0x06, 0x08, 0x2a,
        0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x05,
        0x00, 0x03, 0x47, 0x00, 0x30, 0x44, 0x02, 0x20,
        0x60, 0xdc, 0x45, 0x77, 0x7d, 0xcb, 0xc3, 0xb4,
        0xba, 0x60, 0x5a, 0x2e, 0xe5, 0x4e, 0x19, 0x8b,
        0x48, 0x8a, 0x87, 0xd4, 0x66, 0xb4, 0x1a, 0x86,
        0x23, 0x67, 0xb8, 0xb6, 0x50, 0xfe, 0x4d, 0xde,
        0x02, 0x20, 0x60, 0x68, 0x46, 0xff, 0x74, 0x11,
        0xfb, 0x36, 0x13, 0xf4, 0xa7, 0x3d, 0xb7, 0x35,
        0x79, 0x23, 0x29, 0x14, 0x6a, 0x28, 0x09, 0xff,
        0x8c, 0x19, 0x26, 0xe3, 0x41, 0xc8, 0xe4, 0x13,
        0xbc, 0x8e};
//default client's key pair
static const unsigned char x509_priv_key[] = {
        0xf9, 0x42, 0xb4, 0x16, 0x89, 0x10, 0xf4, 0x07,
        0x99, 0xb2, 0xe2, 0x9a, 0xed, 0xd4, 0x39, 0xb8,
        0xca, 0xd4, 0x9d, 0x76, 0x11, 0x43, 0x3a, 0xac,
        0x14, 0xba, 0x17, 0x9d, 0x3e, 0xbb, 0xbf, 0xbc};

static const unsigned char x509_pub_key_x[] = {
        0xe3, 0xd1, 0x67, 0x1e, 0xdc, 0x46, 0xf4, 0x19,
        0x50, 0x15, 0x2e, 0x3a, 0x2f, 0xd8, 0x68, 0x6b,
        0x37, 0x32, 0x84, 0x9e, 0x83, 0x81, 0xbf, 0x25,
        0x5d, 0xbb, 0x18, 0x07, 0x3c, 0xbd, 0xf3, 0xab};

static const unsigned char x509_pub_key_y[] = {
        0xd3, 0xbf, 0x53, 0x59, 0xc9, 0x1e, 0xce, 0x5b,
        0x39, 0x6a, 0xe5, 0x60, 0xf3, 0x70, 0xdb, 0x66,
        0xb6, 0x80, 0xcb, 0x65, 0x0b, 0x35, 0x2a, 0x62,
        0x44, 0x89, 0x63, 0x64, 0x6f, 0x6f, 0xbd, 0xf0};

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

//default server's key pair
static const unsigned char serv_pub_key_x[] = {
        0x07, 0x88, 0x10, 0xdc, 0x62, 0xd7, 0xe6, 0x9b,
        0x7c, 0xad, 0x6e, 0x78, 0xb0, 0x5f, 0x9a, 0x00,
        0x11, 0x74, 0x2c, 0x8b, 0xaf, 0x09, 0x65, 0x7c,
        0x86, 0x8e, 0x55, 0xcb, 0x39, 0x55, 0x72, 0xc6};

static const unsigned char serv_pub_key_y[] = {
        0x65, 0x71, 0xcd, 0x03, 0xdc, 0x2a, 0x4f, 0x46,
        0x5b, 0x14, 0xc8, 0x27, 0x74, 0xab, 0xf4, 0x1f,
        0xc1, 0x35, 0x0d, 0x42, 0xbc, 0xc2, 0x9f, 0xb5,
        0xc1, 0x79, 0xb6, 0x8b, 0xca, 0xdb, 0xff, 0x82};


static unsigned char x509_client_cert[DTLS_MAX_CERT_SIZE];
static size_t x509_client_cert_len = 0;
static unsigned char x509_client_priv[DTLS_PRIVATE_KEY_SIZE+1];
static size_t x509_client_priv_is_set = 0;
static unsigned char x509_ca_pub[DTLS_PUBLIC_KEY_SIZE+1];
static size_t x509_ca_pub_is_set = 0;

static int x509_info_from_file = 0;
#endif /*DTLS_X509*/
#ifdef DTLS_ECC
static const unsigned char ecdsa_priv_key[] = {
			0x41, 0xC1, 0xCB, 0x6B, 0x51, 0x24, 0x7A, 0x14,
			0x43, 0x21, 0x43, 0x5B, 0x7A, 0x80, 0xE7, 0x14,
			0x89, 0x6A, 0x33, 0xBB, 0xAD, 0x72, 0x94, 0xCA,
			0x40, 0x14, 0x55, 0xA1, 0x94, 0xA9, 0x49, 0xFA};

static const unsigned char ecdsa_pub_key_x[] = {
			0x36, 0xDF, 0xE2, 0xC6, 0xF9, 0xF2, 0xED, 0x29,
			0xDA, 0x0A, 0x9A, 0x8F, 0x62, 0x68, 0x4E, 0x91,
			0x63, 0x75, 0xBA, 0x10, 0x30, 0x0C, 0x28, 0xC5,
			0xE4, 0x7C, 0xFB, 0xF2, 0x5F, 0xA5, 0x8F, 0x52};

static const unsigned char ecdsa_pub_key_y[] = {
			0x71, 0xA0, 0xD4, 0xFC, 0xDE, 0x1A, 0xB8, 0x78,
			0x5A, 0x3C, 0x78, 0x69, 0x35, 0xA7, 0xCF, 0xAB,
			0xE9, 0x3F, 0x98, 0x72, 0x09, 0xDA, 0xED, 0x0B,
			0x4F, 0xAB, 0xC3, 0x6F, 0xC7, 0x72, 0xF8, 0x29};

#endif /*DTLS_ECC*/
#if defined(DTLS_PSK) || defined(DTLS_X509)
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
#endif /*DTLS_PSK||DTLS_X509*/
#ifdef DTLS_PSK
/* The PSK information for DTLS */
#define PSK_ID_MAXLEN 256
#define PSK_MAXLEN 256
static unsigned char psk_client_id[PSK_ID_MAXLEN];
static size_t psk_client_id_length = 0;
static unsigned char psk_server_id[PSK_ID_MAXLEN];
static size_t psk_server_id_length = 0;
static unsigned char psk_key[PSK_MAXLEN];
static size_t psk_key_length = 0;

/* This function is the "key store" for tinyDTLS. It is called to
 * retrieve a key for the given identity within this particular
 * session. */
static int
get_psk_info(struct dtls_context_t *ctx UNUSED_PARAM,
	    const session_t *session UNUSED_PARAM,
	    dtls_credentials_type_t type,
	    const unsigned char *id, size_t id_len,
	    unsigned char *result, size_t result_length) {

  switch (type) {
  case DTLS_PSK_IDENTITY:
    if (id_len) {
      dtls_debug("got psk_identity_hint: '%.*s'\n", id_len, id);
    }

    if (result_length < psk_client_id_length) {
      dtls_warn("cannot set psk_identity -- buffer too small\n");
      return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
    }

    memcpy(result, psk_client_id, psk_client_id_length);
    return psk_client_id_length;
  case DTLS_PSK_KEY:
    if (id_len != psk_server_id_length || memcmp(psk_server_id, id, id_len) != 0) {
      dtls_warn("PSK for unknown id requested, exiting\n");
      return dtls_alert_fatal_create(DTLS_ALERT_ILLEGAL_PARAMETER);
    } else if (result_length < psk_key_length) {
      dtls_warn("cannot set psk -- buffer too small\n");
      return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
    }

    memcpy(result, psk_key, psk_key_length);
    return psk_key_length;
  default:
    dtls_warn("unsupported request type: %d\n", type);
  }

  return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
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
      ecdsa_key.priv_key = x509_client_priv;
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
        *cert = x509_client_cert;
        *cert_size = x509_client_cert_len;
    }
    else
    {
        *cert = g_client_certificate;
        *cert_size = CLIENT_CRT_LEN;
    }

    return 0;
}

int check_certificate(byte_array cert_der_code, byte_array ca_public_key)
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

    memcpy(x, serv_pub_key_x, x_size);
    memcpy(y, serv_pub_key_y, y_size);

    ret = (int) check_certificate(cert_der_code, ca_public_key);

    return -ret;
}

static int is_x509_active(struct dtls_context_t *ctx)
{
    (void)ctx;
    return 0;
}

#endif /* DTLS_X509 */

static void
try_send(struct dtls_context_t *ctx, session_t *dst) {
  int res;
  res = dtls_write(ctx, dst, (uint8 *)buf, len);
  if (res >= 0) {
    memmove(buf, buf + res, len - res);
    len -= res;
  }
}

static void
handle_stdin() {
  if (fgets(buf + len, sizeof(buf) - len, stdin))
    len += strlen(buf + len);
}

static int
read_from_peer(struct dtls_context_t *ctx, 
	       session_t *session, uint8 *data, size_t len) {
  size_t i;
  (void)ctx;
  (void)session;
  for (i = 0; i < len; i++)
    printf("%c", data[i]);
  return 0;
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
  int fd;
  session_t session;
#define MAX_READ_BUF 2000
  static uint8 buf[MAX_READ_BUF];
  int len;

  fd = *(int *)dtls_get_app_data(ctx);
  
  if (!fd)
    return -1;

  memset(&session, 0, sizeof(session_t));
  session.size = sizeof(session.addr);
  len = recvfrom(fd, buf, MAX_READ_BUF, 0, 
		 &session.addr.sa, &session.size);
  
  if (len < 0) {
    perror("recvfrom");
    return -1;
  } else {
    dtls_dsrv_log_addr(DTLS_LOG_DEBUG, "peer", &session);
    dtls_debug_dump("bytes from peer", buf, len);
  }

  return dtls_handle_message(ctx, &session, buf, len);
}    

static void dtls_handle_signal(int sig)
{
  dtls_free_context(dtls_context);
  dtls_free_context(orig_dtls_context);
  signal(sig, SIG_DFL);
  kill(getpid(), sig);
}

/* stolen from libcoap: */
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
    case AF_INET:

      memcpy(dst, ainfo->ai_addr, ainfo->ai_addrlen);
      return ainfo->ai_addrlen;
    default:
      ;
    }
  }

  freeaddrinfo(res);
  return -1;
}

/*---------------------------------------------------------------------------*/
static void
usage( const char *program, const char *version) {
  const char *p;

  p = strrchr( program, '/' );
  if ( p )
    program = ++p;

  fprintf(stderr, "%s v%s -- DTLS client implementation\n"
	  "(c) 2011-2014 Olaf Bergmann <bergmann@tzi.org>\n\n"
	  "usage: %s"
#ifdef DTLS_PSK
          " [-i file] [-s file] [-k file]"
#endif /* DTLS_PSK */
#ifdef DTLS_X509
          " [-x file] [-r file] [-u file]"
#endif /* DTLS_X509 */
          " [-o file] [-p port] [-v num] [-c num] addr [port]\n"
#ifdef DTLS_PSK
	  "\t-i file\t\tread PSK Client identity from file\n"
	  "\t-s file\t\tread PSK Server identity from file\n"
	  "\t-k file\t\tread pre-shared key from file\n"
#endif /* DTLS_PSK */
#ifdef DTLS_X509
          "\t-x file\tread Client certificate from file\n"
          "\t-r file\tread Client private key from file\n"
          "\t-u file\tread CA public key from file\n"
#endif /* DTLS_X509 */
	  "\t-o file\t\toutput received data to this file (use '-' for STDOUT)\n"
	  "\t-p port\t\tlisten on specified port (default is %d)\n"
	  "\t-v num\t\tverbosity level (default: 3)\n"
          "\t-c num\t\tcipher suite (default: 1)\n"
          "\t\t\t1: TLS_ECDH_anon_WITH_AES_128_CBC_SHA_256 \n"
          "\t\t\t2: TLS_PSK_WITH_AES_128_CCM_8\n"
          "\t\t\t3: TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8\n"
          "\t\t\t4: TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA_256\n",
	   program, version, program, DEFAULT_PORT);
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
#endif /* DTLS_X509 */

};

#define DTLS_CLIENT_CMD_CLOSE "client:close"
#define DTLS_CLIENT_CMD_RENEGOTIATE "client:renegotiate"

/* As per RFC 6347 section 4.2.8, DTLS Server should support requests
 * from clients who have silently abandoned the existing association
 * and initiated a new handshake request by sending a ClientHello.
 * Below command tests this feature.
 */
#define DTLS_CLIENT_CMD_REHANDSHAKE "client:rehandshake"

int 
main(int argc, char **argv) {
  fd_set rfds, wfds;
  struct timeval timeout;
  unsigned short port = DEFAULT_PORT;
  char port_str[NI_MAXSERV] = "0";
  log_t log_level = DTLS_LOG_WARN;
  int fd, result;
  int on = 1;
  dtls_cipher_t selected_cipher = TLS_NULL_WITH_NULL_NULL;
  dtls_cipher_enable_t ecdh_anon_enalbe = DTLS_CIPHER_ENABLE;
  int opt, res;
  session_t dst;

  dtls_init();
  snprintf(port_str, sizeof(port_str), "%d", port);

#ifdef DTLS_PSK
  psk_client_id_length = strlen(PSK_CLIENT_IDENTITY);
  psk_server_id_length = strlen(PSK_SERVER_IDENTITY);
  psk_key_length = strlen(PSK_DEFAULT_KEY);
  memcpy(psk_client_id, PSK_CLIENT_IDENTITY, psk_client_id_length);
  memcpy(psk_server_id, PSK_SERVER_IDENTITY, psk_server_id_length);
  memcpy(psk_key, PSK_DEFAULT_KEY, psk_key_length);
#endif /* DTLS_PSK */

  while ((opt = getopt(argc, argv, "p:o:v:c:" PSK_OPTIONS X509_OPTIONS)) != -1) {
    switch (opt) {
#ifdef DTLS_PSK
    case 'i' : {
      ssize_t result = read_from_file(optarg, psk_client_id, PSK_ID_MAXLEN);
      if (result < 0) {
	dtls_warn("cannot read Client PSK identity\n");
      } else {
	psk_client_id_length = result;
      }
      break;
    }
    case 's' : {
      ssize_t result = read_from_file(optarg, psk_server_id, PSK_ID_MAXLEN);
      if (result < 0) {
	dtls_warn("cannot read Server PSK identity\n");
      } else {
	psk_server_id_length = result;
      }
      break;
    }
    case 'k' : {
      ssize_t result = read_from_file(optarg, psk_key, PSK_MAXLEN);
      if (result < 0) {
	dtls_warn("cannot read PSK\n");
      } else {
	psk_key_length = result;
      }
      break;
    }
#endif /* DTLS_PSK */
#ifdef DTLS_X509
    case 'x' :
    {
      ssize_t result = read_from_file(optarg, x509_client_cert, DTLS_MAX_CERT_SIZE);
      if (result < 0)
      {
          dtls_warn("Cannot read Client certificate. Using default\n");
      }
      else
      {
          x509_client_cert_len = result;
      }
      break;
    }
    case 'r' :
    {
      ssize_t result = read_from_file(optarg, x509_client_priv, DTLS_PRIVATE_KEY_SIZE+1);
      if (result < 0)
      {
          dtls_warn("Cannot read Client private key. Using default\n");
      }
      else
      {
          x509_client_priv_is_set = result;
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
    case 'p' :
      strncpy(port_str, optarg, NI_MAXSERV-1);
      port_str[NI_MAXSERV - 1] = '\0';
      break;
    case 'o' :
      output_file.length = strlen(optarg);
      output_file.s = (unsigned char *)malloc(output_file.length + 1);

      if (!output_file.s) {
	dtls_crit("cannot set output file: insufficient memory\n");
	exit(-1);
      } else {
	/* copy filename including trailing zero */
	memcpy(output_file.s, optarg, output_file.length + 1);
      }
      break;
    case 'v' :
      log_level = strtol(optarg, NULL, 10);
      break;
    case 'c':
      if( strcmp(optarg, "1") == 0)
      {
          selected_cipher = TLS_ECDH_anon_WITH_AES_128_CBC_SHA_256;
          ecdh_anon_enalbe = DTLS_CIPHER_ENABLE;
      }
      else if( strcmp(optarg, "2") == 0)
      {
          selected_cipher = TLS_PSK_WITH_AES_128_CCM_8 ;
          ecdh_anon_enalbe = DTLS_CIPHER_DISABLE;
      }
      else if( strcmp(optarg, "3") == 0)
      {
          selected_cipher = TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8 ;
          ecdh_anon_enalbe = DTLS_CIPHER_DISABLE;
      }
      else if( strcmp(optarg, "4") == 0)
      {
          selected_cipher = TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA_256;
          ecdh_anon_enalbe = DTLS_CIPHER_DISABLE;
      }
      break;
    default:
      usage(argv[0], dtls_package_version());
      exit(1);
    }
  }

  dtls_set_log_level(log_level);

  if (argc <= optind) {
    usage(argv[0], dtls_package_version());
    exit(1);
  }

#ifdef DTLS_X509
  if (x509_client_cert_len && x509_client_priv_is_set && x509_ca_pub_is_set)
  {
      x509_info_from_file = 1;
  }
  else if(!(x509_client_cert_len || x509_client_priv_is_set || x509_ca_pub_is_set))
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

  memset(&dst, 0, sizeof(session_t));
  /* resolve destination address where server should be sent */
  res = resolve_address(argv[optind++], &dst.addr.sa);
  if (res < 0) {
    dtls_emerg("failed to resolve address\n");
    exit(-1);
  }
  dst.size = res;

  /* use port number from command line when specified or the listen
     port, otherwise */
  dst.addr.sin.sin_port = htons(atoi(optind < argc ? argv[optind++] : port_str));

  
  /* init socket and set it to non-blocking */
  fd = socket(dst.addr.sa.sa_family, SOCK_DGRAM, 0);

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

  if (signal(SIGINT, dtls_handle_signal) == SIG_ERR) {
    dtls_alert("An error occurred while setting a signal handler.\n");
    return EXIT_FAILURE;
  }

  dtls_context = dtls_new_context(&fd);
  if (!dtls_context) {
    dtls_emerg("cannot create context\n");
    exit(-1);
  }


  /* select cipher suite */
  dtls_select_cipher(dtls_context, selected_cipher);

  /* enable/disable tls_ecdh_anon_with_aes_128_cbc_sha_256 */
  dtls_enables_anon_ecdh(dtls_context, ecdh_anon_enalbe);

  dtls_set_handler(dtls_context, &cb);

  dtls_connect(dtls_context, &dst);

  while (1) {
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);

    FD_SET(fileno(stdin), &rfds);
    FD_SET(fd, &rfds);
    /* FD_SET(fd, &wfds); */
    
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    
    result = select(fd+1, &rfds, &wfds, 0, &timeout);
    
    if (result < 0) {		/* error */
      if (errno != EINTR)
	perror("select");
    } else if (result == 0) {	/* timeout */
    } else {			/* ok */
      if (FD_ISSET(fd, &wfds))
	/* FIXME */;
      else if (FD_ISSET(fd, &rfds))
	dtls_handle_read(dtls_context);
      else if (FD_ISSET(fileno(stdin), &rfds))
	handle_stdin();
    }

    if (len) {
      if (len >= strlen(DTLS_CLIENT_CMD_CLOSE) &&
	  !memcmp(buf, DTLS_CLIENT_CMD_CLOSE, strlen(DTLS_CLIENT_CMD_CLOSE))) {
	printf("client: closing connection\n");
	dtls_close(dtls_context, &dst);
	len = 0;
      } else if (len >= strlen(DTLS_CLIENT_CMD_RENEGOTIATE) &&
	         !memcmp(buf, DTLS_CLIENT_CMD_RENEGOTIATE, strlen(DTLS_CLIENT_CMD_RENEGOTIATE))) {
	printf("client: renegotiate connection\n");
	dtls_renegotiate(dtls_context, &dst);
	len = 0;
      } else if (len >= strlen(DTLS_CLIENT_CMD_REHANDSHAKE) &&
	         !memcmp(buf, DTLS_CLIENT_CMD_REHANDSHAKE, strlen(DTLS_CLIENT_CMD_REHANDSHAKE))) {
	printf("client: rehandshake connection\n");
	if (orig_dtls_context == NULL) {
	  /* Cache the current context. We cannot free the current context as it will notify 
	   * the Server to close the connection (which we do not want).
	   */
	  orig_dtls_context = dtls_context;
	  /* Now, Create a new context and attempt to initiate a handshake. */
	  dtls_context = dtls_new_context(&fd);
	  if (!dtls_context) {
	    dtls_emerg("cannot create context\n");
	    exit(-1);
          }
	  dtls_set_handler(dtls_context, &cb);
	  dtls_connect(dtls_context, &dst);
	}
	len = 0;
      } else {
	try_send(dtls_context, &dst);
      }
    }
  }
  
  dtls_free_context(dtls_context);
  dtls_free_context(orig_dtls_context);
  exit(0);
}

