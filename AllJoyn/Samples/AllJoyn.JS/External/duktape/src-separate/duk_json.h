/*
 *  Defines for JSON, especially duk_bi_json.c.
 */

#ifndef DUK_JSON_H_INCLUDED
#define DUK_JSON_H_INCLUDED

/* Object/array recursion limit (to protect C stack) */
#if defined(DUK_USE_DEEP_C_STACK)
#define DUK_JSON_ENC_RECURSION_LIMIT          1000
#define DUK_JSON_DEC_RECURSION_LIMIT          1000
#else
#define DUK_JSON_ENC_RECURSION_LIMIT          100
#define DUK_JSON_DEC_RECURSION_LIMIT          100
#endif

/* Encoding/decoding flags */
#define DUK_JSON_FLAG_ASCII_ONLY          (1 << 0)  /* escape any non-ASCII characters */
#define DUK_JSON_FLAG_AVOID_KEY_QUOTES    (1 << 1)  /* avoid key quotes when key is an ASCII Identifier */
#define DUK_JSON_FLAG_EXT_CUSTOM          (1 << 2)  /* extended types: custom encoding */
#define DUK_JSON_FLAG_EXT_COMPATIBLE      (1 << 3)  /* extended types: compatible encoding */

/* How much stack to require on entry to object/array encode */
#define DUK_JSON_ENC_REQSTACK                 32

/* How much stack to require on entry to object/array decode */
#define DUK_JSON_DEC_REQSTACK                 32

/* Encoding state.  Heap object references are all borrowed. */
typedef struct {
	duk_hthread *thr;
	duk_hbuffer_dynamic *h_buf;
	duk_hobject *h_replacer;     /* replacer function */
	duk_hstring *h_gap;          /* gap (if empty string, NULL) */
	duk_hstring *h_indent;       /* current indent (if gap is NULL, this is NULL) */
	duk_idx_t idx_proplist;      /* explicit PropertyList */
	duk_idx_t idx_loop;          /* valstack index of loop detection object */
	duk_small_uint_t flags;
	duk_small_uint_t flag_ascii_only;
	duk_small_uint_t flag_avoid_key_quotes;
#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
	duk_small_uint_t flag_ext_custom;
	duk_small_uint_t flag_ext_compatible;
#endif
	duk_int_t recursion_depth;
	duk_int_t recursion_limit;
	duk_uint_t mask_for_undefined;      /* type bit mask: types which certainly produce 'undefined' */
#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
	duk_small_uint_t stridx_custom_undefined;
	duk_small_uint_t stridx_custom_nan;
	duk_small_uint_t stridx_custom_neginf;
	duk_small_uint_t stridx_custom_posinf;
	duk_small_uint_t stridx_custom_function;
#endif
} duk_json_enc_ctx;

typedef struct {
	duk_hthread *thr;
	const duk_uint8_t *p;
	const duk_uint8_t *p_start;
	const duk_uint8_t *p_end;
	duk_idx_t idx_reviver;
	duk_small_uint_t flags;
#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
	duk_small_uint_t flag_ext_custom;
	duk_small_uint_t flag_ext_compatible;
#endif
	duk_int_t recursion_depth;
	duk_int_t recursion_limit;
} duk_json_dec_ctx;

#endif  /* DUK_JSON_H_INCLUDED */
