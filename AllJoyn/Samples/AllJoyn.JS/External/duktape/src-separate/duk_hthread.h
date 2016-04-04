/*
 *  Heap thread object representation.
 *
 *  duk_hthread is also the 'context' (duk_context) for exposed APIs
 *  which mostly operate on the topmost frame of the value stack.
 */

#ifndef DUK_HTHREAD_H_INCLUDED
#define DUK_HTHREAD_H_INCLUDED

/*
 *  Stack constants
 */

#define DUK_VALSTACK_GROW_STEP          128     /* roughly 1 kiB */
#define DUK_VALSTACK_SHRINK_THRESHOLD   256     /* roughly 2 kiB */
#define DUK_VALSTACK_SHRINK_SPARE       64      /* roughly 0.5 kiB */
#define DUK_VALSTACK_INITIAL_SIZE       128     /* roughly 1.0 kiB -> but rounds up to DUK_VALSTACK_GROW_STEP in practice */
#define DUK_VALSTACK_INTERNAL_EXTRA     64      /* internal extra elements assumed on function entry,
                                                 * always added to user-defined 'extra' for e.g. the
                                                 * duk_check_stack() call.
                                                 */
#define DUK_VALSTACK_API_ENTRY_MINIMUM  DUK_API_ENTRY_STACK
                                                /* number of elements guaranteed to be user accessible
                                                 * (in addition to call arguments) on Duktape/C function entry.
                                                 */

/* Note: DUK_VALSTACK_INITIAL_SIZE must be >= DUK_VALSTACK_API_ENTRY_MINIMUM
 * + DUK_VALSTACK_INTERNAL_EXTRA so that the initial stack conforms to spare
 * requirements.
 */

#define DUK_VALSTACK_DEFAULT_MAX        1000000L

#define DUK_CALLSTACK_GROW_STEP         8       /* roughly 256 bytes */
#define DUK_CALLSTACK_SHRINK_THRESHOLD  16      /* roughly 512 bytes */
#define DUK_CALLSTACK_SHRINK_SPARE      8       /* roughly 256 bytes */
#define DUK_CALLSTACK_INITIAL_SIZE      8
#define DUK_CALLSTACK_DEFAULT_MAX       10000L

#define DUK_CATCHSTACK_GROW_STEP         4      /* roughly 64 bytes */
#define DUK_CATCHSTACK_SHRINK_THRESHOLD  8      /* roughly 128 bytes */
#define DUK_CATCHSTACK_SHRINK_SPARE      4      /* roughly 64 bytes */
#define DUK_CATCHSTACK_INITIAL_SIZE      4
#define DUK_CATCHSTACK_DEFAULT_MAX       10000L

/*
 *  Activation defines
 */

#define DUK_ACT_FLAG_STRICT             (1 << 0)  /* function executes in strict mode */
#define DUK_ACT_FLAG_TAILCALLED         (1 << 1)  /* activation has tailcalled one or more times */
#define DUK_ACT_FLAG_CONSTRUCT          (1 << 2)  /* function executes as a constructor (called via "new") */
#define DUK_ACT_FLAG_PREVENT_YIELD      (1 << 3)  /* activation prevents yield (native call or "new") */
#define DUK_ACT_FLAG_DIRECT_EVAL        (1 << 4)  /* activation is a direct eval call */
#define DUK_ACT_FLAG_BREAKPOINT_ACTIVE  (1 << 5)  /* activation has active breakpoint(s) */

#define DUK_ACT_GET_FUNC(act)        ((act)->func)

/*
 *  Flags for __FILE__ / __LINE__ registered into tracedata
 */

#define DUK_TB_FLAG_NOBLAME_FILELINE   (1 << 0)  /* don't report __FILE__ / __LINE__ as fileName/lineNumber */

/*
 *  Catcher defines
 */

/* flags field: LLLLLLFT, L = label (24 bits), F = flags (4 bits), T = type (4 bits) */
#define DUK_CAT_TYPE_MASK            0x0000000fUL
#define DUK_CAT_TYPE_BITS            4
#define DUK_CAT_LABEL_MASK           0xffffff00UL
#define DUK_CAT_LABEL_BITS           24
#define DUK_CAT_LABEL_SHIFT          8

#define DUK_CAT_FLAG_CATCH_ENABLED          (1 << 4)   /* catch part will catch */
#define DUK_CAT_FLAG_FINALLY_ENABLED        (1 << 5)   /* finally part will catch */
#define DUK_CAT_FLAG_CATCH_BINDING_ENABLED  (1 << 6)   /* request to create catch binding */
#define DUK_CAT_FLAG_LEXENV_ACTIVE          (1 << 7)   /* catch or with binding is currently active */

#define DUK_CAT_TYPE_UNKNOWN         0
#define DUK_CAT_TYPE_TCF             1
#define DUK_CAT_TYPE_LABEL           2

#define DUK_CAT_GET_TYPE(c)          ((c)->flags & DUK_CAT_TYPE_MASK)
#define DUK_CAT_GET_LABEL(c)         (((c)->flags & DUK_CAT_LABEL_MASK) >> DUK_CAT_LABEL_SHIFT)

#define DUK_CAT_HAS_CATCH_ENABLED(c)           ((c)->flags & DUK_CAT_FLAG_CATCH_ENABLED)
#define DUK_CAT_HAS_FINALLY_ENABLED(c)         ((c)->flags & DUK_CAT_FLAG_FINALLY_ENABLED)
#define DUK_CAT_HAS_CATCH_BINDING_ENABLED(c)   ((c)->flags & DUK_CAT_FLAG_CATCH_BINDING_ENABLED)
#define DUK_CAT_HAS_LEXENV_ACTIVE(c)           ((c)->flags & DUK_CAT_FLAG_LEXENV_ACTIVE)

#define DUK_CAT_SET_CATCH_ENABLED(c)    do { \
		(c)->flags |= DUK_CAT_FLAG_CATCH_ENABLED; \
	} while (0)
#define DUK_CAT_SET_FINALLY_ENABLED(c)  do { \
		(c)->flags |= DUK_CAT_FLAG_FINALLY_ENABLED; \
	} while (0)
#define DUK_CAT_SET_CATCH_BINDING_ENABLED(c)    do { \
		(c)->flags |= DUK_CAT_FLAG_CATCH_BINDING_ENABLED; \
	} while (0)
#define DUK_CAT_SET_LEXENV_ACTIVE(c)    do { \
		(c)->flags |= DUK_CAT_FLAG_LEXENV_ACTIVE; \
	} while (0)

#define DUK_CAT_CLEAR_CATCH_ENABLED(c)    do { \
		(c)->flags &= ~DUK_CAT_FLAG_CATCH_ENABLED; \
	} while (0)
#define DUK_CAT_CLEAR_FINALLY_ENABLED(c)  do { \
		(c)->flags &= ~DUK_CAT_FLAG_FINALLY_ENABLED; \
	} while (0)
#define DUK_CAT_CLEAR_CATCH_BINDING_ENABLED(c)    do { \
		(c)->flags &= ~DUK_CAT_FLAG_CATCH_BINDING_ENABLED; \
	} while (0)
#define DUK_CAT_CLEAR_LEXENV_ACTIVE(c)    do { \
		(c)->flags &= ~DUK_CAT_FLAG_LEXENV_ACTIVE; \
	} while (0)

/*
 *  Thread defines
 */

#if defined(DUK_USE_HEAPPTR16)
#define DUK_HTHREAD_GET_STRING(thr,idx) \
	((duk_hstring *) DUK_USE_HEAPPTR_DEC16((thr)->heap->heap_udata, (thr)->strs16[(idx)]))
#else
#define DUK_HTHREAD_GET_STRING(thr,idx) \
	((thr)->strs[(idx)])
#endif

#define DUK_HTHREAD_GET_CURRENT_ACTIVATION(thr)  (&(thr)->callstack[(thr)->callstack_top - 1])

/* values for the state field */
#define DUK_HTHREAD_STATE_INACTIVE     1   /* thread not currently running */
#define DUK_HTHREAD_STATE_RUNNING      2   /* thread currently running (only one at a time) */
#define DUK_HTHREAD_STATE_RESUMED      3   /* thread resumed another thread (active but not running) */
#define DUK_HTHREAD_STATE_YIELDED      4   /* thread has yielded */
#define DUK_HTHREAD_STATE_TERMINATED   5   /* thread has terminated */

/*
 *  Assert context is valid: non-NULL pointer, fields look sane.
 *
 *  This is used by public API call entrypoints to catch invalid 'ctx' pointers
 *  as early as possible; invalid 'ctx' pointers cause very odd and difficult to
 *  diagnose behavior so it's worth checking even when the check is not 100%.
 */

#define DUK_ASSERT_CTX_VALID(ctx) do { \
		DUK_ASSERT((ctx) != NULL); \
		DUK_ASSERT(DUK_HEAPHDR_GET_TYPE((duk_heaphdr *) (ctx)) == DUK_HTYPE_OBJECT); \
		DUK_ASSERT(DUK_HOBJECT_IS_THREAD((duk_hobject *) (ctx))); \
		DUK_ASSERT(((duk_hthread *) (ctx))->unused1 == 0); \
		DUK_ASSERT(((duk_hthread *) (ctx))->unused2 == 0); \
		DUK_ASSERT(((duk_hthread *) (ctx))->valstack != NULL); \
		DUK_ASSERT(((duk_hthread *) (ctx))->valstack_end >= ((duk_hthread *) (ctx))->valstack); \
		DUK_ASSERT(((duk_hthread *) (ctx))->valstack_top >= ((duk_hthread *) (ctx))->valstack); \
		DUK_ASSERT(((duk_hthread *) (ctx))->valstack_top >= ((duk_hthread *) (ctx))->valstack_bottom); \
	} while (0)

/*
 *  Struct defines
 */

/* XXX: for a memory-code tradeoff, remove 'func' and make it's access either a function
 * or a macro.  This would make the activation 32 bytes long on 32-bit platforms again.
 */

/* Note: it's nice if size is 2^N (at least for 32-bit platforms). */
struct duk_activation {
	duk_tval tv_func;       /* borrowed: full duk_tval for function being executed; for lightfuncs */
	duk_hobject *func;      /* borrowed: function being executed; for bound function calls, this is the final, real function, NULL for lightfuncs */
	duk_hobject *var_env;   /* current variable environment (may be NULL if delayed) */
	duk_hobject *lex_env;   /* current lexical environment (may be NULL if delayed) */
#ifdef DUK_USE_NONSTD_FUNC_CALLER_PROPERTY
	/* Previous value of 'func' caller, restored when unwound.  Only in use
	 * when 'func' is non-strict.
	 */
	duk_hobject *prev_caller;
#endif

	duk_small_uint_t flags;
	duk_uint32_t pc;        /* next instruction to execute */
#if defined(DUK_USE_DEBUGGER_SUPPORT)
	duk_uint32_t prev_line; /* needed for stepping */
#endif

	/* idx_bottom and idx_retval are only used for book-keeping of
	 * Ecmascript-initiated calls, to allow returning to an Ecmascript
	 * function properly.  They are duk_size_t to match the convention
	 * that value stack sizes are duk_size_t and local frame indices
	 * are duk_idx_t.
	 */

	/* Bottom of valstack for this activation, used to reset
	 * valstack_bottom on return; index is absolute.  Note:
	 * idx_top not needed because top is set to 'nregs' always
	 * when returning to an Ecmascript activation.
	 */
	duk_size_t idx_bottom;

	/* Return value when returning to this activation (points to caller
	 * reg, not callee reg); index is absolute (only set if activation is
	 * not topmost).
	 *
	 * Note: idx_bottom is always set, while idx_retval is only applicable
	 * for activations below the topmost one.  Currently idx_retval for
	 * the topmost activation is considered garbage (and it not initialized
	 * on entry or cleared on return; may contain previous or garbage
	 * values).
	 */
	duk_size_t idx_retval;

	/* Current 'this' binding is the value just below idx_bottom.
	 * Previously, 'this' binding was handled with an index to the
	 * (calling) valstack.  This works for everything except tail
	 * calls, which must not "cumulate" valstack temps.
	 */
};

/* Note: it's nice if size is 2^N (not 4x4 = 16 bytes on 32 bit) */
struct duk_catcher {
	duk_hstring *h_varname;         /* borrowed reference to catch variable name (or NULL if none) */
	                                /* (reference is valid as long activation exists) */
	duk_size_t callstack_index;     /* callstack index of related activation */
	duk_size_t idx_base;            /* idx_base and idx_base+1 get completion value and type */
	duk_uint32_t pc_base;           /* resume execution from pc_base or pc_base+1 */
	duk_uint32_t flags;             /* type and control flags, label number */
};

struct duk_hthread {
	/* shared object part */
	duk_hobject obj;

	/* backpointers */
	duk_heap *heap;

	/* current strictness flag: affects API calls */
	duk_uint8_t strict;
	duk_uint8_t state;
	duk_uint8_t unused1;
	duk_uint8_t unused2;

	/* sanity limits */
	duk_size_t valstack_max;
	duk_size_t callstack_max;
	duk_size_t catchstack_max;

	/* XXX: valstack, callstack, and catchstack are currently assumed
	 * to have non-NULL pointers.  Relaxing this would not lead to big
	 * benefits (except perhaps for terminated threads).
	 */

	/* value stack: these are expressed as pointers for faster stack manipulation */
	duk_tval *valstack;                     /* start of valstack allocation */
	duk_tval *valstack_end;                 /* end of valstack allocation (exclusive) */
	duk_tval *valstack_bottom;              /* bottom of current frame */
	duk_tval *valstack_top;                 /* top of current frame (exclusive) */

	/* call stack */
	duk_activation *callstack;
	duk_size_t callstack_size;              /* allocation size */
	duk_size_t callstack_top;               /* next to use, highest used is top - 1 */
	duk_size_t callstack_preventcount;      /* number of activation records in callstack preventing a yield */

	/* catch stack */
	duk_catcher *catchstack;
	duk_size_t catchstack_size;             /* allocation size */
	duk_size_t catchstack_top;              /* next to use, highest used is top - 1 */

	/* yield/resume book-keeping */
	duk_hthread *resumer;                   /* who resumed us (if any) */

	/* current compiler state (if any), used for augmenting SyntaxErrors */
	duk_compiler_ctx *compile_ctx;

#ifdef DUK_USE_INTERRUPT_COUNTER
	/* Interrupt counter for triggering a slow path check for execution
	 * timeout, debugger interaction such as breakpoints, etc.  This is
	 * actually a value copied from the heap structure into the current
	 * thread to be more convenient for the bytecode executor inner loop.
	 * The final value is copied back to the heap structure on a thread
	 * switch by DUK_HEAP_SWITCH_THREAD().
	 */
	duk_int_t interrupt_counter;
#endif

	/* Builtin-objects; may or may not be shared with other threads,
	 * threads existing in different "compartments" will have different
	 * built-ins.  Must be stored on a per-thread basis because there
	 * is no intermediate structure for a thread group / compartment.
	 * This takes quite a lot of space, currently 43x4 = 172 bytes on
	 * 32-bit platforms.
	 */
	duk_hobject *builtins[DUK_NUM_BUILTINS];

	/* convenience copies from heap/vm for faster access */
#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t *strs16;
#else
	duk_hstring **strs;
#endif
};

/*
 *  Prototypes
 */

DUK_INTERNAL_DECL void duk_hthread_copy_builtin_objects(duk_hthread *thr_from, duk_hthread *thr_to);
DUK_INTERNAL_DECL void duk_hthread_create_builtin_objects(duk_hthread *thr);
DUK_INTERNAL_DECL duk_bool_t duk_hthread_init_stacks(duk_heap *heap, duk_hthread *thr);
DUK_INTERNAL_DECL void duk_hthread_terminate(duk_hthread *thr);

DUK_INTERNAL_DECL void duk_hthread_callstack_grow(duk_hthread *thr);
DUK_INTERNAL_DECL void duk_hthread_callstack_shrink_check(duk_hthread *thr);
DUK_INTERNAL_DECL void duk_hthread_callstack_unwind(duk_hthread *thr, duk_size_t new_top);
DUK_INTERNAL_DECL void duk_hthread_catchstack_grow(duk_hthread *thr);
DUK_INTERNAL_DECL void duk_hthread_catchstack_shrink_check(duk_hthread *thr);
DUK_INTERNAL_DECL void duk_hthread_catchstack_unwind(duk_hthread *thr, duk_size_t new_top);

DUK_INTERNAL_DECL duk_activation *duk_hthread_get_current_activation(duk_hthread *thr);
DUK_INTERNAL_DECL void *duk_hthread_get_valstack_ptr(duk_heap *heap, void *ud);  /* indirect allocs */
DUK_INTERNAL_DECL void *duk_hthread_get_callstack_ptr(duk_heap *heap, void *ud);  /* indirect allocs */
DUK_INTERNAL_DECL void *duk_hthread_get_catchstack_ptr(duk_heap *heap, void *ud);  /* indirect allocs */

#endif  /* DUK_HTHREAD_H_INCLUDED */
