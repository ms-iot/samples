/*
 *  Date built-ins
 *
 *  Unlike most built-ins, Date has a lot of platform dependencies for
 *  getting UTC time, converting between UTC and local time, and parsing
 *  and formatting time values.
 *
 *  See doc/datetime.txt.
 *
 *  Platform specific links:
 *
 *    - http://msdn.microsoft.com/en-us/library/windows/desktop/ms725473(v=vs.85).aspx
 */

#include "duk_internal.h"

/*
 *  Platform specific includes and defines
 *
 *  Note that necessary system headers (like <sys/time.h>) are included
 *  by duk_internal.h (or duk_features.h, which is included by duk_internal.h)
 *  because the header locations vary between systems and we don't want
 *  that clutter here.
 */

#define DUK__GET_NOW_TIMEVAL      duk_bi_date_get_now
#define DUK__GET_LOCAL_TZOFFSET   duk__get_local_tzoffset

/* Buffer sizes for some UNIX calls.  Larger than strictly necessary
 * to avoid Valgrind errors.
 */
#define DUK__STRPTIME_BUF_SIZE  64
#define DUK__STRFTIME_BUF_SIZE  64

/*
 *  Other file level defines
 */

/* Forward declarations. */
DUK_LOCAL_DECL duk_double_t duk__push_this_get_timeval_tzoffset(duk_context *ctx, duk_small_uint_t flags, duk_int_t *out_tzoffset);
DUK_LOCAL_DECL duk_double_t duk__push_this_get_timeval(duk_context *ctx, duk_small_uint_t flags);
DUK_LOCAL_DECL void duk__timeval_to_parts(duk_double_t d, duk_int_t *parts, duk_double_t *dparts, duk_small_uint_t flags);
DUK_LOCAL_DECL duk_double_t duk__get_timeval_from_dparts(duk_double_t *dparts, duk_small_uint_t flags);
DUK_LOCAL_DECL void duk__twodigit_year_fixup(duk_context *ctx, duk_idx_t idx_val);
DUK_LOCAL_DECL duk_bool_t duk__is_leap_year(duk_int_t year);
DUK_LOCAL_DECL duk_bool_t duk__timeval_in_valid_range(duk_double_t x);
DUK_LOCAL_DECL duk_bool_t duk__timeval_in_leeway_range(duk_double_t x);
DUK_LOCAL_DECL duk_bool_t duk__year_in_valid_range(duk_double_t year);

/* Millisecond count constants. */
#define DUK__MS_SECOND          1000L
#define DUK__MS_MINUTE          (60L * 1000L)
#define DUK__MS_HOUR            (60L * 60L * 1000L)
#define DUK__MS_DAY             (24L * 60L * 60L * 1000L)

/* Ecmascript date range is 100 million days from Epoch:
 * > 100e6 * 24 * 60 * 60 * 1000  // 100M days in millisecs
 * 8640000000000000
 * (= 8.64e15)
 */
#define DUK__MS_100M_DAYS         (8.64e15)
#define DUK__MS_100M_DAYS_LEEWAY  (8.64e15 + 24 * 3600e3)

/* Ecmascript year range:
 * > new Date(100e6 * 24 * 3600e3).toISOString()
 * '+275760-09-13T00:00:00.000Z'
 * > new Date(-100e6 * 24 * 3600e3).toISOString()
 * '-271821-04-20T00:00:00.000Z'
 */
#define  DUK__MIN_ECMA_YEAR     (-271821)
#define  DUK__MAX_ECMA_YEAR     275760

/* Part indices for internal breakdowns.  Part order from DUK__IDX_YEAR to
 * DUK__IDX_MILLISECOND matches argument ordering of Ecmascript API calls
 * (like Date constructor call).  A few functions in this file depend
 * on the specific ordering, so change with care.  16 bits are not enough
 * for all parts (year, specifically).
 *
 * (Must be in-sync with genbuiltins.py.)
 */
#define DUK__IDX_YEAR           0  /* year */
#define DUK__IDX_MONTH          1  /* month: 0 to 11 */
#define DUK__IDX_DAY            2  /* day within month: 0 to 30 */
#define DUK__IDX_HOUR           3
#define DUK__IDX_MINUTE         4
#define DUK__IDX_SECOND         5
#define DUK__IDX_MILLISECOND    6
#define DUK__IDX_WEEKDAY        7  /* weekday: 0 to 6, 0=sunday, 1=monday, etc */
#define DUK__NUM_PARTS          8

/* Internal API call flags, used for various functions in this file.
 * Certain flags are used by only certain functions, but since the flags
 * don't overlap, a single flags value can be passed around to multiple
 * functions.
 *
 * The unused top bits of the flags field are also used to pass values
 * to helpers (duk__get_part_helper() and duk__set_part_helper()).
 *
 * (Must be in-sync with genbuiltins.py.)
 */
#define DUK__FLAG_NAN_TO_ZERO          (1 << 0)  /* timeval breakdown: internal time value NaN -> zero */
#define DUK__FLAG_NAN_TO_RANGE_ERROR   (1 << 1)  /* timeval breakdown: internal time value NaN -> RangeError (toISOString) */
#define DUK__FLAG_ONEBASED             (1 << 2)  /* timeval breakdown: convert month and day-of-month parts to one-based (default is zero-based) */
#define DUK__FLAG_EQUIVYEAR            (1 << 3)  /* timeval breakdown: replace year with equivalent year in the [1971,2037] range for DST calculations */
#define DUK__FLAG_LOCALTIME            (1 << 4)  /* convert time value to local time */
#define DUK__FLAG_SUB1900              (1 << 5)  /* getter: subtract 1900 from year when getting year part */
#define DUK__FLAG_TOSTRING_DATE        (1 << 6)  /* include date part in string conversion result */
#define DUK__FLAG_TOSTRING_TIME        (1 << 7)  /* include time part in string conversion result */
#define DUK__FLAG_TOSTRING_LOCALE      (1 << 8)  /* use locale specific formatting if available */
#define DUK__FLAG_TIMESETTER           (1 << 9)  /* setter: call is a time setter (affects hour, min, sec, ms); otherwise date setter (affects year, month, day-in-month) */
#define DUK__FLAG_YEAR_FIXUP           (1 << 10) /* setter: perform 2-digit year fixup (00...99 -> 1900...1999) */
#define DUK__FLAG_SEP_T                (1 << 11) /* string conversion: use 'T' instead of ' ' as a separator */
#define DUK__FLAG_VALUE_SHIFT          12        /* additional values begin at bit 12 */

/* Debug macro to print all parts and dparts (used manually because of debug level). */
#define  DUK__DPRINT_PARTS_AND_DPARTS(parts,dparts)  do { \
		DUK_D(DUK_DPRINT("parts: %ld %ld %ld %ld %ld %ld %ld %ld, dparts: %lf %lf %lf %lf %lf %lf %lf %lf", \
		                 (long) (parts)[0], (long) (parts)[1], \
		                 (long) (parts)[2], (long) (parts)[3], \
		                 (long) (parts)[4], (long) (parts)[5], \
		                 (long) (parts)[6], (long) (parts)[7], \
		                 (double) (dparts)[0], (double) (dparts)[1], \
		                 (double) (dparts)[2], (double) (dparts)[3], \
		                 (double) (dparts)[4], (double) (dparts)[5], \
		                 (double) (dparts)[6], (double) (dparts)[7])); \
	} while (0)
#define  DUK__DPRINT_PARTS(parts)  do { \
		DUK_D(DUK_DPRINT("parts: %ld %ld %ld %ld %ld %ld %ld %ld", \
		                 (long) (parts)[0], (long) (parts)[1], \
		                 (long) (parts)[2], (long) (parts)[3], \
		                 (long) (parts)[4], (long) (parts)[5], \
		                 (long) (parts)[6], (long) (parts)[7])); \
	} while (0)
#define  DUK__DPRINT_DPARTS(dparts)  do { \
		DUK_D(DUK_DPRINT("dparts: %lf %lf %lf %lf %lf %lf %lf %lf", \
		                 (double) (dparts)[0], (double) (dparts)[1], \
		                 (double) (dparts)[2], (double) (dparts)[3], \
		                 (double) (dparts)[4], (double) (dparts)[5], \
		                 (double) (dparts)[6], (double) (dparts)[7])); \
	} while (0)

/* Equivalent year for DST calculations outside [1970,2038[ range, see
 * E5 Section 15.9.1.8.  Equivalent year has the same leap-year-ness and
 * starts with the same weekday on Jan 1.
 * https://bugzilla.mozilla.org/show_bug.cgi?id=351066
 */
#define DUK__YEAR(x) ((duk_uint8_t) ((x) - 1970))
DUK_LOCAL duk_uint8_t duk__date_equivyear[14] = {
#if 1
	/* This is based on V8 EquivalentYear() algorithm (see src/genequivyear.py):
	 * http://code.google.com/p/v8/source/browse/trunk/src/date.h#146
	 */

	/* non-leap year: sunday, monday, ... */
	DUK__YEAR(2023), DUK__YEAR(2035), DUK__YEAR(2019), DUK__YEAR(2031),
	DUK__YEAR(2015), DUK__YEAR(2027), DUK__YEAR(2011),

	/* leap year: sunday, monday, ... */
	DUK__YEAR(2012), DUK__YEAR(2024), DUK__YEAR(2008), DUK__YEAR(2020),
	DUK__YEAR(2032), DUK__YEAR(2016), DUK__YEAR(2028)
#endif

#if 0
	/* This is based on Rhino EquivalentYear() algorithm:
	 * https://github.com/mozilla/rhino/blob/f99cc11d616f0cdda2c42bde72b3484df6182947/src/org/mozilla/javascript/NativeDate.java
	 */

	/* non-leap year: sunday, monday, ... */
	DUK__YEAR(1978), DUK__YEAR(1973), DUK__YEAR(1985), DUK__YEAR(1986),
	DUK__YEAR(1981), DUK__YEAR(1971), DUK__YEAR(1977),

	/* leap year: sunday, monday, ... */
	DUK__YEAR(1984), DUK__YEAR(1996), DUK__YEAR(1980), DUK__YEAR(1992),
	DUK__YEAR(1976), DUK__YEAR(1988), DUK__YEAR(1972)
#endif
};
#undef DUK__YEAR

/*
 *  Platform specific helpers
 */

#ifdef DUK_USE_DATE_NOW_GETTIMEOFDAY
/* Get current Ecmascript time (= UNIX/Posix time, but in milliseconds). */
DUK_INTERNAL duk_double_t duk_bi_date_get_now(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	struct timeval tv;
	duk_double_t d;

	if (gettimeofday(&tv, NULL) != 0) {
		DUK_ERROR(thr, DUK_ERR_INTERNAL_ERROR, "gettimeofday failed");
	}

	d = ((duk_double_t) tv.tv_sec) * 1000.0 +
	    ((duk_double_t) (tv.tv_usec / 1000));
	DUK_ASSERT(DUK_FLOOR(d) == d);  /* no fractions */

	return d;
}
#endif  /* DUK_USE_DATE_NOW_GETTIMEOFDAY */

#ifdef DUK_USE_DATE_NOW_TIME
/* Not a very good provider: only full seconds are available. */
DUK_INTERNAL duk_double_t duk_bi_date_get_now(duk_context *ctx) {
	time_t t = time(NULL);
	return ((duk_double_t) t) * 1000.0;
}
#endif  /* DUK_USE_DATE_NOW_TIME */

#if defined(DUK_USE_DATE_NOW_WINDOWS) || defined(DUK_USE_DATE_TZO_WINDOWS)
/* Shared Windows helpers. */
DUK_LOCAL void duk__convert_systime_to_ularge(const SYSTEMTIME *st, ULARGE_INTEGER *res) {
	FILETIME ft;
	if (SystemTimeToFileTime(st, &ft) == 0) {
		DUK_D(DUK_DPRINT("SystemTimeToFileTime() failed, returning 0"));
		res->QuadPart = 0;
	} else {
		res->LowPart = ft.dwLowDateTime;
		res->HighPart = ft.dwHighDateTime;
	}
}
DUK_LOCAL void duk__set_systime_jan1970(SYSTEMTIME *st) {
	DUK_MEMZERO((void *) st, sizeof(*st));
	st->wYear = 1970;
	st->wMonth = 1;
	st->wDayOfWeek = 4;  /* not sure whether or not needed; Thursday */
	st->wDay = 1;
	DUK_ASSERT(st->wHour == 0);
	DUK_ASSERT(st->wMinute == 0);
	DUK_ASSERT(st->wSecond == 0);
	DUK_ASSERT(st->wMilliseconds == 0);
}
#endif  /* defined(DUK_USE_DATE_NOW_WINDOWS) || defined(DUK_USE_DATE_TZO_WINDOWS) */

#ifdef DUK_USE_DATE_NOW_WINDOWS
DUK_INTERNAL duk_double_t duk_bi_date_get_now(duk_context *ctx) {
	/* Suggested step-by-step method from documentation of RtlTimeToSecondsSince1970:
	 * http://msdn.microsoft.com/en-us/library/windows/desktop/ms724928(v=vs.85).aspx
	 */
	SYSTEMTIME st1, st2;
	ULARGE_INTEGER tmp1, tmp2;

	DUK_UNREF(ctx);

	GetSystemTime(&st1);
	duk__convert_systime_to_ularge((const SYSTEMTIME *) &st1, &tmp1);

	duk__set_systime_jan1970(&st2);
	duk__convert_systime_to_ularge((const SYSTEMTIME *) &st2, &tmp2);

	/* Difference is in 100ns units, convert to milliseconds w/o fractions */
	return (duk_double_t) ((tmp1.QuadPart - tmp2.QuadPart) / 10000LL);
}
#endif  /* DUK_USE_DATE_NOW_WINDOWS */

#if defined(DUK_USE_DATE_TZO_GMTIME) || defined(DUK_USE_DATE_TZO_GMTIME_R)
/* Get local time offset (in seconds) for a certain (UTC) instant 'd'. */
DUK_LOCAL duk_int_t duk__get_local_tzoffset(duk_double_t d) {
	time_t t, t1, t2;
	duk_int_t parts[DUK__NUM_PARTS];
	duk_double_t dparts[DUK__NUM_PARTS];
	struct tm tms[2];
#ifdef DUK_USE_DATE_TZO_GMTIME
	struct tm *tm_ptr;
#endif

	/* For NaN/inf, the return value doesn't matter. */
	if (!DUK_ISFINITE(d)) {
		return 0;
	}

	/* If not within Ecmascript range, some integer time calculations
	 * won't work correctly (and some asserts will fail), so bail out
	 * if so.  This fixes test-bug-date-insane-setyear.js.  There is
	 * a +/- 24h leeway in this range check to avoid a test262 corner
	 * case documented in test-bug-date-timeval-edges.js.
	 */
	if (!duk__timeval_in_leeway_range(d)) {
		DUK_DD(DUK_DDPRINT("timeval not within valid range, skip tzoffset computation to avoid integer overflows"));
		return 0;
	}

	/*
	 *  This is a bit tricky to implement portably.  The result depends
	 *  on the timestamp (specifically, DST depends on the timestamp).
	 *  If e.g. UNIX APIs are used, they'll have portability issues with
	 *  very small and very large years.
	 *
	 *  Current approach:
	 *
	 *  - Stay within portable UNIX limits by using equivalent year mapping.
	 *    Avoid year 1970 and 2038 as some conversions start to fail, at
	 *    least on some platforms.  Avoiding 1970 means that there are
	 *    currently DST discrepancies for 1970.
	 *
	 *  - Create a UTC and local time breakdowns from 't'.  Then create
	 *    a time_t using gmtime() and localtime() and compute the time
	 *    difference between the two.
	 *
	 *  Equivalent year mapping (E5 Section 15.9.1.8):
	 *
	 *    If the host environment provides functionality for determining
	 *    daylight saving time, the implementation of ECMAScript is free
	 *    to map the year in question to an equivalent year (same
	 *    leap-year-ness and same starting week day for the year) for which
	 *    the host environment provides daylight saving time information.
	 *    The only restriction is that all equivalent years should produce
	 *    the same result.
	 *
	 *  This approach is quite reasonable but not entirely correct, e.g.
	 *  the specification also states (E5 Section 15.9.1.8):
	 *
	 *    The implementation of ECMAScript should not try to determine
	 *    whether the exact time was subject to daylight saving time, but
	 *    just whether daylight saving time would have been in effect if
	 *    the _current daylight saving time algorithm_ had been used at the
	 *    time.  This avoids complications such as taking into account the
	 *    years that the locale observed daylight saving time year round.
	 *
	 *  Since we rely on the platform APIs for conversions between local
	 *  time and UTC, we can't guarantee the above.  Rather, if the platform
	 *  has historical DST rules they will be applied.  This seems to be the
	 *  general preferred direction in Ecmascript standardization (or at least
	 *  implementations) anyway, and even the equivalent year mapping should
	 *  be disabled if the platform is known to handle DST properly for the
	 *  full Ecmascript range.
	 *
	 *  The following has useful discussion and links:
	 *
	 *    https://bugzilla.mozilla.org/show_bug.cgi?id=351066
	 */

	duk__timeval_to_parts(d, parts, dparts, DUK__FLAG_EQUIVYEAR /*flags*/);
	DUK_ASSERT(parts[DUK__IDX_YEAR] >= 1970 && parts[DUK__IDX_YEAR] <= 2038);

	d = duk__get_timeval_from_dparts(dparts, 0 /*flags*/);
	DUK_ASSERT(d >= 0 && d < 2147483648.0 * 1000.0);  /* unsigned 31-bit range */
	t = (time_t) (d / 1000.0);
	DUK_DDD(DUK_DDDPRINT("timeval: %lf -> time_t %ld", (double) d, (long) t));

	t1 = t;

	DUK_MEMZERO((void *) tms, sizeof(struct tm) * 2);

#if defined(DUK_USE_DATE_TZO_GMTIME_R)
	(void) gmtime_r(&t, &tms[0]);
	(void) localtime_r(&t, &tms[1]);
#elif defined(DUK_USE_DATE_TZO_GMTIME)
	tm_ptr = gmtime(&t);
	DUK_MEMCPY((void *) &tms[0], tm_ptr, sizeof(struct tm));
	tm_ptr = localtime(&t);
	DUK_MEMCPY((void *) &tms[1], tm_ptr, sizeof(struct tm));
#else
#error internal error
#endif
	DUK_DDD(DUK_DDDPRINT("gmtime result: tm={sec:%ld,min:%ld,hour:%ld,mday:%ld,mon:%ld,year:%ld,"
	                     "wday:%ld,yday:%ld,isdst:%ld}",
	                     (long) tms[0].tm_sec, (long) tms[0].tm_min, (long) tms[0].tm_hour,
	                     (long) tms[0].tm_mday, (long) tms[0].tm_mon, (long) tms[0].tm_year,
	                     (long) tms[0].tm_wday, (long) tms[0].tm_yday, (long) tms[0].tm_isdst));
	DUK_DDD(DUK_DDDPRINT("localtime result: tm={sec:%ld,min:%ld,hour:%ld,mday:%ld,mon:%ld,year:%ld,"
	                     "wday:%ld,yday:%ld,isdst:%ld}",
	                     (long) tms[1].tm_sec, (long) tms[1].tm_min, (long) tms[1].tm_hour,
	                     (long) tms[1].tm_mday, (long) tms[1].tm_mon, (long) tms[1].tm_year,
	                     (long) tms[1].tm_wday, (long) tms[1].tm_yday, (long) tms[1].tm_isdst));

	t1 = mktime(&tms[0]);  /* UTC */
	t2 = mktime(&tms[1]);  /* local */
	if (t1 == (time_t) -1 || t2 == (time_t) -1) {
		/* This check used to be for (t < 0) but on some platforms
		 * time_t is unsigned and apparently the proper way to detect
		 * an mktime() error return is the cast above.  See e.g.:
		 * http://pubs.opengroup.org/onlinepubs/009695299/functions/mktime.html
		 */
		goto error;
	}
	if (tms[1].tm_isdst > 0) {
		t2 += 3600;
	} else if (tms[1].tm_isdst < 0) {
		DUK_D(DUK_DPRINT("tm_isdst is negative: %d", (int) tms[1].tm_isdst));
	}
	DUK_DDD(DUK_DDDPRINT("t1=%ld (utc), t2=%ld (local)", (long) t1, (long) t2));

	/* Compute final offset in seconds, positive if local time ahead of
	 * UTC (returned value is UTC-to-local offset).
	 *
	 * difftime() returns a double, so coercion to int generates quite
	 * a lot of code.  Direct subtraction is not portable, however.
	 * XXX: allow direct subtraction on known platforms.
	 */
#if 0
	return (duk_int_t) (t2 - t1);
#endif
	return (duk_int_t) difftime(t2, t1);

 error:
	/* XXX: return something more useful, so that caller can throw? */
	DUK_D(DUK_DPRINT("mktime() failed, d=%lf", (double) d));
	return 0;
}
#endif  /* DUK_USE_DATE_TZO_GMTIME */

#if defined(DUK_USE_DATE_TZO_WINDOWS)
DUK_LOCAL duk_int_t duk__get_local_tzoffset(duk_double_t d) {
	SYSTEMTIME st1;
	SYSTEMTIME st2;
	SYSTEMTIME st3;
	ULARGE_INTEGER tmp1;
	ULARGE_INTEGER tmp2;
	ULARGE_INTEGER tmp3;
	FILETIME ft1;

	/* XXX: handling of timestamps outside Windows supported range.
	 * How does Windows deal with dates before 1600?  Does windows
	 * support all Ecmascript years (like -200000 and +200000)?
	 * Should equivalent year mapping be used here too?  If so, use
	 * a shared helper (currently integrated into timeval-to-parts).
	 */

	/* Use the approach described in "Remarks" of FileTimeToLocalFileTime:
	 * http://msdn.microsoft.com/en-us/library/windows/desktop/ms724277(v=vs.85).aspx
	 */

	duk__set_systime_jan1970(&st1);
	duk__convert_systime_to_ularge((const SYSTEMTIME *) &st1, &tmp1);
	tmp2.QuadPart = (ULONGLONG) (d * 10000.0);  /* millisec -> 100ns units since jan 1, 1970 */
	tmp2.QuadPart += tmp1.QuadPart;             /* input 'd' in Windows UTC, 100ns units */

	ft1.dwLowDateTime = tmp2.LowPart;
	ft1.dwHighDateTime = tmp2.HighPart;
	FileTimeToSystemTime((const FILETIME *) &ft1, &st2);
	if (SystemTimeToTzSpecificLocalTime((LPTIME_ZONE_INFORMATION) NULL, &st2, &st3) == 0) {
		DUK_D(DUK_DPRINT("SystemTimeToTzSpecificLocalTime() failed, return tzoffset 0"));
		return 0;
	}
	duk__convert_systime_to_ularge((const SYSTEMTIME *) &st3, &tmp3);

	/* Positive if local time ahead of UTC. */
	return (duk_int_t) (((LONGLONG) tmp3.QuadPart - (LONGLONG) tmp2.QuadPart) / 10000000LL);  /* seconds */
}
#endif  /* DUK_USE_DATE_TZO_WINDOWS */

#ifdef DUK_USE_DATE_PRS_STRPTIME
DUK_LOCAL duk_bool_t duk__parse_string_strptime(duk_context *ctx, const char *str) {
	struct tm tm;
	time_t t;
	char buf[DUK__STRPTIME_BUF_SIZE];

	/* copy to buffer with spare to avoid Valgrind gripes from strptime */
	DUK_ASSERT(str != NULL);
	DUK_MEMZERO(buf, sizeof(buf));  /* valgrind whine without this */
	DUK_SNPRINTF(buf, sizeof(buf), "%s", (const char *) str);
	buf[sizeof(buf) - 1] = (char) 0;

	DUK_DDD(DUK_DDDPRINT("parsing: '%s'", (const char *) buf));

	DUK_MEMZERO(&tm, sizeof(tm));
	if (strptime((const char *) buf, "%c", &tm) != NULL) {
		DUK_DDD(DUK_DDDPRINT("before mktime: tm={sec:%ld,min:%ld,hour:%ld,mday:%ld,mon:%ld,year:%ld,"
		                     "wday:%ld,yday:%ld,isdst:%ld}",
		                     (long) tm.tm_sec, (long) tm.tm_min, (long) tm.tm_hour,
		                     (long) tm.tm_mday, (long) tm.tm_mon, (long) tm.tm_year,
		                     (long) tm.tm_wday, (long) tm.tm_yday, (long) tm.tm_isdst));
		tm.tm_isdst = -1;  /* negative: dst info not available */

		t = mktime(&tm);
		DUK_DDD(DUK_DDDPRINT("mktime() -> %ld", (long) t));
		if (t >= 0) {
			duk_push_number(ctx, ((duk_double_t) t) * 1000.0);
			return 1;
		}
	}

	return 0;
}
#endif  /* DUK_USE_DATE_PRS_STRPTIME */

#ifdef DUK_USE_DATE_PRS_GETDATE
DUK_LOCAL duk_bool_t duk__parse_string_getdate(duk_context *ctx, const char *str) {
	struct tm tm;
	duk_small_int_t rc;
	time_t t;

	/* For this to work, DATEMSK must be set, so this is not very
	 * convenient for an embeddable interpreter.
	 */

	DUK_MEMZERO(&tm, sizeof(struct tm));
	rc = (duk_small_int_t) getdate_r(str, &tm);
	DUK_DDD(DUK_DDDPRINT("getdate_r() -> %ld", (long) rc));

	if (rc == 0) {
		t = mktime(&tm);
		DUK_DDD(DUK_DDDPRINT("mktime() -> %ld", (long) t));
		if (t >= 0) {
			duk_push_number(ctx, (duk_double_t) t);
			return 1;
		}
	}

	return 0;
}
#endif  /* DUK_USE_DATE_PRS_GETDATE */

#ifdef DUK_USE_DATE_FMT_STRFTIME
DUK_LOCAL duk_bool_t duk__format_parts_strftime(duk_context *ctx, duk_int_t *parts, duk_int_t tzoffset, duk_small_uint_t flags) {
	char buf[DUK__STRFTIME_BUF_SIZE];
	struct tm tm;
	const char *fmt;

	DUK_UNREF(tzoffset);

	/* If the platform doesn't support the entire Ecmascript range, we need
	 * to return 0 so that the caller can fall back to the default formatter.
	 *
	 * For now, assume that if time_t is 8 bytes or more, the whole Ecmascript
	 * range is supported.  For smaller time_t values (4 bytes in practice),
	 * assumes that the signed 32-bit range is supported.
	 *
	 * XXX: detect this more correctly per platform.  The size of time_t is
	 * probably not an accurate guarantee of strftime() supporting or not
	 * supporting a large time range (the full Ecmascript range).
	 */
	if (sizeof(time_t) < 8 &&
	   (parts[DUK__IDX_YEAR] < 1970 || parts[DUK__IDX_YEAR] > 2037)) {
		/* be paranoid for 32-bit time values (even avoiding negative ones) */
		return 0;
	}

	DUK_MEMZERO(&tm, sizeof(tm));
	tm.tm_sec = parts[DUK__IDX_SECOND];
	tm.tm_min = parts[DUK__IDX_MINUTE];
	tm.tm_hour = parts[DUK__IDX_HOUR];
	tm.tm_mday = parts[DUK__IDX_DAY];       /* already one-based */
	tm.tm_mon = parts[DUK__IDX_MONTH] - 1;  /* one-based -> zero-based */
	tm.tm_year = parts[DUK__IDX_YEAR] - 1900;
	tm.tm_wday = parts[DUK__IDX_WEEKDAY];
	tm.tm_isdst = 0;

	DUK_MEMZERO(buf, sizeof(buf));
	if ((flags & DUK__FLAG_TOSTRING_DATE) && (flags & DUK__FLAG_TOSTRING_TIME)) {
		fmt = "%c";
	} else if (flags & DUK__FLAG_TOSTRING_DATE) {
		fmt = "%x";
	} else {
		DUK_ASSERT(flags & DUK__FLAG_TOSTRING_TIME);
		fmt = "%X";
	}
	(void) strftime(buf, sizeof(buf) - 1, fmt, &tm);
	DUK_ASSERT(buf[sizeof(buf) - 1] == 0);

	duk_push_string(ctx, buf);
	return 1;
}
#endif  /* DUK_USE_DATE_FMT_STRFTIME */

/*
 *  ISO 8601 subset parser.
 */

/* Parser part count. */
#define DUK__NUM_ISO8601_PARSER_PARTS  9

/* Parser part indices. */
#define DUK__PI_YEAR         0
#define DUK__PI_MONTH        1
#define DUK__PI_DAY          2
#define DUK__PI_HOUR         3
#define DUK__PI_MINUTE       4
#define DUK__PI_SECOND       5
#define DUK__PI_MILLISECOND  6
#define DUK__PI_TZHOUR       7
#define DUK__PI_TZMINUTE     8

/* Parser part masks. */
#define DUK__PM_YEAR         (1 << DUK__PI_YEAR)
#define DUK__PM_MONTH        (1 << DUK__PI_MONTH)
#define DUK__PM_DAY          (1 << DUK__PI_DAY)
#define DUK__PM_HOUR         (1 << DUK__PI_HOUR)
#define DUK__PM_MINUTE       (1 << DUK__PI_MINUTE)
#define DUK__PM_SECOND       (1 << DUK__PI_SECOND)
#define DUK__PM_MILLISECOND  (1 << DUK__PI_MILLISECOND)
#define DUK__PM_TZHOUR       (1 << DUK__PI_TZHOUR)
#define DUK__PM_TZMINUTE     (1 << DUK__PI_TZMINUTE)

/* Parser separator indices. */
#define DUK__SI_PLUS         0
#define DUK__SI_MINUS        1
#define DUK__SI_T            2
#define DUK__SI_SPACE        3
#define DUK__SI_COLON        4
#define DUK__SI_PERIOD       5
#define DUK__SI_Z            6
#define DUK__SI_NUL          7

/* Parser separator masks. */
#define DUK__SM_PLUS         (1 << DUK__SI_PLUS)
#define DUK__SM_MINUS        (1 << DUK__SI_MINUS)
#define DUK__SM_T            (1 << DUK__SI_T)
#define DUK__SM_SPACE        (1 << DUK__SI_SPACE)
#define DUK__SM_COLON        (1 << DUK__SI_COLON)
#define DUK__SM_PERIOD       (1 << DUK__SI_PERIOD)
#define DUK__SM_Z            (1 << DUK__SI_Z)
#define DUK__SM_NUL          (1 << DUK__SI_NUL)

/* Rule control flags. */
#define DUK__CF_NEG          (1 << 0)  /* continue matching, set neg_tzoffset flag */
#define DUK__CF_ACCEPT       (1 << 1)  /* accept string */
#define DUK__CF_ACCEPT_NUL   (1 << 2)  /* accept string if next char is NUL (otherwise reject) */

#define DUK__PACK_RULE(partmask,sepmask,nextpart,flags)  \
	((duk_uint32_t) (partmask) + \
	 (((duk_uint32_t) (sepmask)) << 9) + \
	 (((duk_uint32_t) (nextpart)) << 17) + \
	 (((duk_uint32_t) (flags)) << 21))

#define DUK__UNPACK_RULE(rule,var_nextidx,var_flags)  do { \
		(var_nextidx) = (duk_small_uint_t) (((rule) >> 17) & 0x0f); \
		(var_flags) = (duk_small_uint_t) ((rule) >> 21); \
	} while (0)

#define DUK__RULE_MASK_PART_SEP  0x1ffffUL

/* Matching separator index is used in the control table */
DUK_LOCAL const duk_uint8_t duk__parse_iso8601_seps[] = {
	DUK_ASC_PLUS /*0*/, DUK_ASC_MINUS /*1*/, DUK_ASC_UC_T /*2*/, DUK_ASC_SPACE /*3*/,
	DUK_ASC_COLON /*4*/, DUK_ASC_PERIOD /*5*/, DUK_ASC_UC_Z /*6*/, DUK_ASC_NUL /*7*/
};

/* Rule table: first matching rule is used to determine what to do next. */
DUK_LOCAL const duk_uint32_t duk__parse_iso8601_control[] = {
	DUK__PACK_RULE(DUK__PM_YEAR, DUK__SM_MINUS, DUK__PI_MONTH, 0),
	DUK__PACK_RULE(DUK__PM_MONTH, DUK__SM_MINUS, DUK__PI_DAY, 0),
	DUK__PACK_RULE(DUK__PM_YEAR | DUK__PM_MONTH | DUK__PM_DAY, DUK__SM_T | DUK__SM_SPACE, DUK__PI_HOUR, 0),
	DUK__PACK_RULE(DUK__PM_HOUR, DUK__SM_COLON, DUK__PI_MINUTE, 0),
	DUK__PACK_RULE(DUK__PM_MINUTE, DUK__SM_COLON, DUK__PI_SECOND, 0),
	DUK__PACK_RULE(DUK__PM_SECOND, DUK__SM_PERIOD, DUK__PI_MILLISECOND, 0),
	DUK__PACK_RULE(DUK__PM_TZHOUR, DUK__SM_COLON, DUK__PI_TZMINUTE, 0),
	DUK__PACK_RULE(DUK__PM_YEAR | DUK__PM_MONTH | DUK__PM_DAY | DUK__PM_HOUR /*Note1*/ | DUK__PM_MINUTE | DUK__PM_SECOND | DUK__PM_MILLISECOND, DUK__SM_PLUS, DUK__PI_TZHOUR, 0),
	DUK__PACK_RULE(DUK__PM_YEAR | DUK__PM_MONTH | DUK__PM_DAY | DUK__PM_HOUR /*Note1*/ | DUK__PM_MINUTE | DUK__PM_SECOND | DUK__PM_MILLISECOND, DUK__SM_MINUS, DUK__PI_TZHOUR, DUK__CF_NEG),
	DUK__PACK_RULE(DUK__PM_YEAR | DUK__PM_MONTH | DUK__PM_DAY | DUK__PM_HOUR /*Note1*/ | DUK__PM_MINUTE | DUK__PM_SECOND | DUK__PM_MILLISECOND, DUK__SM_Z, 0, DUK__CF_ACCEPT_NUL),
	DUK__PACK_RULE(DUK__PM_YEAR | DUK__PM_MONTH | DUK__PM_DAY | DUK__PM_HOUR /*Note1*/ | DUK__PM_MINUTE | DUK__PM_SECOND | DUK__PM_MILLISECOND | DUK__PM_TZHOUR /*Note2*/ | DUK__PM_TZMINUTE, DUK__SM_NUL, 0, DUK__CF_ACCEPT)

	/* Note1: the specification doesn't require matching a time form with
	 *        just hours ("HH"), but we accept it here, e.g. "2012-01-02T12Z".
	 *
	 * Note2: the specification doesn't require matching a timezone offset
	 *        with just hours ("HH"), but accept it here, e.g. "2012-01-02T03:04:05+02"
	 */
};

DUK_LOCAL duk_bool_t duk__parse_string_iso8601_subset(duk_context *ctx, const char *str) {
	duk_int_t parts[DUK__NUM_ISO8601_PARSER_PARTS];
	duk_double_t dparts[DUK__NUM_PARTS];
	duk_double_t d;
	const duk_uint8_t *p;
	duk_small_uint_t part_idx = 0;
	duk_int_t accum = 0;
	duk_small_uint_t ndigits = 0;
	duk_bool_t neg_year = 0;
	duk_bool_t neg_tzoffset = 0;
	duk_uint_fast8_t ch;
	duk_small_uint_t i;

	/* During parsing, month and day are one-based; set defaults here. */
	DUK_MEMZERO(parts, sizeof(parts));
	DUK_ASSERT(parts[DUK__IDX_YEAR] == 0);  /* don't care value, year is mandatory */
	parts[DUK__IDX_MONTH] = 1;
	parts[DUK__IDX_DAY] = 1;

	/* Special handling for year sign. */
	p = (const duk_uint8_t *) str;
	ch = p[0];
	if (ch == DUK_ASC_PLUS) {
		p++;
	} else if (ch == DUK_ASC_MINUS) {
		neg_year = 1;
		p++;
	}

	for (;;) {
		ch = *p++;
		DUK_DDD(DUK_DDDPRINT("parsing, part_idx=%ld, char=%ld ('%c')",
		                     (long) part_idx, (long) ch,
		                     (int) ((ch >= 0x20 && ch <= 0x7e) ? ch : DUK_ASC_QUESTION)));

		if (ch >= DUK_ASC_0 && ch <= DUK_ASC_9) {
			if (ndigits >= 9) {
				DUK_DDD(DUK_DDDPRINT("too many digits -> reject"));
				goto reject;
			}
			if (part_idx == DUK__PI_MILLISECOND /*msec*/ && ndigits >= 3) {
				/* ignore millisecond fractions after 3 */
			} else {
				accum = accum * 10 + ((duk_int_t) ch) - ((duk_int_t) DUK_ASC_0) + 0x00;
				ndigits++;
			}
		} else {
			duk_uint_fast32_t match_val;
			duk_small_int_t sep_idx;

			if (ndigits <= 0) {
				goto reject;
			}
			if (part_idx == DUK__PI_MILLISECOND) {
				/* complete the millisecond field */
				while (ndigits < 3) {
					accum *= 10;
					ndigits++;
				}
			}
			parts[part_idx] = accum;
			DUK_DDD(DUK_DDDPRINT("wrote part %ld -> value %ld", (long) part_idx, (long) accum));

			accum = 0;
			ndigits = 0;

			for (i = 0; i < (duk_small_uint_t) (sizeof(duk__parse_iso8601_seps) / sizeof(duk_uint8_t)); i++) {
				if (duk__parse_iso8601_seps[i] == ch) {
					break;
				}
			}
			if (i == (duk_small_uint_t) (sizeof(duk__parse_iso8601_seps) / sizeof(duk_uint8_t))) {
				DUK_DDD(DUK_DDDPRINT("separator character doesn't match -> reject"));
				goto reject;
			}

			sep_idx = i;
			match_val = (1UL << part_idx) + (1UL << (sep_idx + 9));  /* match against rule part/sep bits */

			for (i = 0; i < (duk_small_uint_t) (sizeof(duk__parse_iso8601_control) / sizeof(duk_uint32_t)); i++) {
				duk_uint_fast32_t rule = duk__parse_iso8601_control[i];
				duk_small_uint_t nextpart;
				duk_small_uint_t cflags;

				DUK_DDD(DUK_DDDPRINT("part_idx=%ld, sep_idx=%ld, match_val=0x%08lx, considering rule=0x%08lx",
				                     (long) part_idx, (long) sep_idx,
				                     (unsigned long) match_val, (unsigned long) rule));

				if ((rule & match_val) != match_val) {
					continue;
				}

				DUK__UNPACK_RULE(rule, nextpart, cflags);

				DUK_DDD(DUK_DDDPRINT("rule match -> part_idx=%ld, sep_idx=%ld, match_val=0x%08lx, "
				                     "rule=0x%08lx -> nextpart=%ld, cflags=0x%02lx",
				                     (long) part_idx, (long) sep_idx,
				                     (unsigned long) match_val, (unsigned long) rule,
				                     (long) nextpart, (unsigned long) cflags));

				if (cflags & DUK__CF_NEG) {
					neg_tzoffset = 1;
				}

				if (cflags & DUK__CF_ACCEPT) {
					goto accept;
				}

				if (cflags & DUK__CF_ACCEPT_NUL) {
					DUK_ASSERT(*(p - 1) != (char) 0);
					if (*p == DUK_ASC_NUL) {
						goto accept;
					}
					goto reject;
				}

				part_idx = nextpart;
				break;
			}  /* rule match */

			if (i == (duk_small_uint_t) (sizeof(duk__parse_iso8601_control) / sizeof(duk_uint32_t))) {
				DUK_DDD(DUK_DDDPRINT("no rule matches -> reject"));
				goto reject;
			}

			if (ch == 0) {
				/* This shouldn't be necessary, but check just in case
				 * to avoid any chance of overruns.
				 */
				DUK_DDD(DUK_DDDPRINT("NUL after rule matching (should not happen) -> reject"));
				goto reject;
			}
		}  /* if-digit-else-ctrl */
	}  /* char loop */

	/* We should never exit the loop above, but if we do, reject
	 * by falling through.
	 */
	DUK_DDD(DUK_DDDPRINT("fell out of char loop without explicit accept/reject -> reject"));

 reject:
	DUK_DDD(DUK_DDDPRINT("reject"));
	return 0;

 accept:
	DUK_DDD(DUK_DDDPRINT("accept"));

	/* Apply timezone offset to get the main parts in UTC */
	if (neg_year) {
		parts[DUK__PI_YEAR] = -parts[DUK__PI_YEAR];
	}
	if (neg_tzoffset) {
		parts[DUK__PI_HOUR] += parts[DUK__PI_TZHOUR];
		parts[DUK__PI_MINUTE] += parts[DUK__PI_TZMINUTE];
	} else {
		parts[DUK__PI_HOUR] -= parts[DUK__PI_TZHOUR];
		parts[DUK__PI_MINUTE] -= parts[DUK__PI_TZMINUTE];
	}
	parts[DUK__PI_MONTH] -= 1;  /* zero-based month */
	parts[DUK__PI_DAY] -= 1;  /* zero-based day */

	/* Use double parts, they tolerate unnormalized time.
	 *
	 * Note: DUK__IDX_WEEKDAY is initialized with a bogus value (DUK__PI_TZHOUR)
	 * on purpose.  It won't be actually used by duk__get_timeval_from_dparts(),
	 * but will make the value initialized just in case, and avoid any
	 * potential for Valgrind issues.
	 */
	for (i = 0; i < DUK__NUM_PARTS; i++) {
		DUK_DDD(DUK_DDDPRINT("part[%ld] = %ld", (long) i, (long) parts[i]));
		dparts[i] = parts[i];
	}

	d = duk__get_timeval_from_dparts(dparts, 0 /*flags*/);
	duk_push_number(ctx, d);
	return 1;
}

/*
 *  Date/time parsing helper.
 *
 *  Parse a datetime string into a time value.  We must first try to parse
 *  the input according to the standard format in E5.1 Section 15.9.1.15.
 *  If that fails, we can try to parse using custom parsing, which can
 *  either be platform neutral (custom code) or platform specific (using
 *  existing platform API calls).
 *
 *  Note in particular that we must parse whatever toString(), toUTCString(),
 *  and toISOString() can produce; see E5.1 Section 15.9.4.2.
 *
 *  Returns 1 to allow tailcalling.
 *
 *  There is much room for improvement here with respect to supporting
 *  alternative datetime formats.  For instance, V8 parses '2012-01-01' as
 *  UTC and '2012/01/01' as local time.
 */

DUK_LOCAL duk_ret_t duk__parse_string(duk_context *ctx, const char *str) {
	/* XXX: there is a small risk here: because the ISO 8601 parser is
	 * very loose, it may end up parsing some datetime values which
	 * would be better parsed with a platform specific parser.
	 */

	DUK_ASSERT(str != NULL);
	DUK_DDD(DUK_DDDPRINT("parse datetime from string '%s'", (const char *) str));

	if (duk__parse_string_iso8601_subset(ctx, str) != 0) {
		return 1;
	}

#if defined(DUK_USE_DATE_PRS_STRPTIME)
	if (duk__parse_string_strptime(ctx, str) != 0) {
		return 1;
	}
#elif defined(DUK_USE_DATE_PRS_GETDATE)
	if (duk__parse_string_getdate(ctx, str) != 0) {
		return 1;
	}
#else
	/* No platform-specific parsing, this is not an error. */
#endif

	duk_push_nan(ctx);
	return 1;
}

/*
 *  Calendar helpers
 *
 *  Some helpers are used for getters and can operate on normalized values
 *  which can be represented with 32-bit signed integers.  Other helpers are
 *  needed by setters and operate on un-normalized double values, must watch
 *  out for non-finite numbers etc.
 */

DUK_LOCAL duk_uint8_t duk__days_in_month[12] = {
	(duk_uint8_t) 31, (duk_uint8_t) 28, (duk_uint8_t) 31, (duk_uint8_t) 30,
	(duk_uint8_t) 31, (duk_uint8_t) 30, (duk_uint8_t) 31, (duk_uint8_t) 31,
	(duk_uint8_t) 30, (duk_uint8_t) 31, (duk_uint8_t) 30, (duk_uint8_t) 31
};

/* Maximum iteration count for computing UTC-to-local time offset when
 * creating an Ecmascript time value from local parts.
 */
#define DUK__LOCAL_TZOFFSET_MAXITER   4

/* Because 'day since epoch' can be negative and is used to compute weekday
 * using a modulo operation, add this multiple of 7 to avoid negative values
 * when year is below 1970 epoch.  Ecmascript time values are restricted to
 * +/- 100 million days from epoch, so this adder fits nicely into 32 bits.
 * Round to a multiple of 7 (= floor(100000000 / 7) * 7) and add margin.
 */
#define DUK__WEEKDAY_MOD_ADDER  (20000000 * 7)  /* 0x08583b00 */

DUK_LOCAL duk_bool_t duk__is_leap_year(duk_int_t year) {
	if ((year % 4) != 0) {
		return 0;
	}
	if ((year % 100) != 0) {
		return 1;
	}
	if ((year % 400) != 0) {
		return 0;
	}
	return 1;
}

DUK_LOCAL duk_bool_t duk__timeval_in_valid_range(duk_double_t x) {
	return (x >= -DUK__MS_100M_DAYS && x <= DUK__MS_100M_DAYS);
}

DUK_LOCAL duk_bool_t duk__timeval_in_leeway_range(duk_double_t x) {
	return (x >= -DUK__MS_100M_DAYS_LEEWAY && x <= DUK__MS_100M_DAYS_LEEWAY);
}

DUK_LOCAL duk_bool_t duk__year_in_valid_range(duk_double_t x) {
	return (x >= DUK__MIN_ECMA_YEAR && x <= DUK__MAX_ECMA_YEAR);
}

DUK_LOCAL duk_double_t duk__timeclip(duk_double_t x) {
	if (!DUK_ISFINITE(x)) {
		return DUK_DOUBLE_NAN;
	}

	if (!duk__timeval_in_valid_range(x)) {
		return DUK_DOUBLE_NAN;
	}

	x = duk_js_tointeger_number(x);

	/* Here we'd have the option to normalize -0 to +0. */
	return x;
}

/* Integer division which floors also negative values correctly. */
DUK_LOCAL duk_int_t duk__div_floor(duk_int_t a, duk_int_t b) {
	DUK_ASSERT(b > 0);
	if (a >= 0) {
		return a / b;
	} else {
		/* e.g. a = -4, b = 5  -->  -4 - 5 + 1 / 5  -->  -8 / 5  -->  -1
		 *      a = -5, b = 5  -->  -5 - 5 + 1 / 5  -->  -9 / 5  -->  -1
		 *      a = -6, b = 5  -->  -6 - 5 + 1 / 5  -->  -10 / 5  -->  -2
		 */
		return (a - b + 1) / b;
	}
}

/* Compute day number of the first day of a given year. */
DUK_LOCAL duk_int_t duk__day_from_year(duk_int_t year) {
	/* Note: in integer arithmetic, (x / 4) is same as floor(x / 4) for non-negative
	 * values, but is incorrect for negative ones.
	 */
	return 365 * (year - 1970)
	       + duk__div_floor(year - 1969, 4)
	       - duk__div_floor(year - 1901, 100)
	       + duk__div_floor(year - 1601, 400);
}

/* Given a day number, determine year and day-within-year. */
DUK_LOCAL duk_int_t duk__year_from_day(duk_int_t day, duk_small_int_t *out_day_within_year) {
	duk_int_t year;
	duk_int_t diff_days;

	/* estimate year upwards (towards positive infinity), then back down;
	 * two iterations should be enough
	 */

	if (day >= 0) {
		year = 1970 + day / 365;
	} else {
		year = 1970 + day / 366;
	}

	for (;;) {
		diff_days = duk__day_from_year(year) - day;
		DUK_DDD(DUK_DDDPRINT("year=%ld day=%ld, diff_days=%ld", (long) year, (long) day, (long) diff_days));
		if (diff_days <= 0) {
			DUK_ASSERT(-diff_days < 366);  /* fits into duk_small_int_t */
			*out_day_within_year = -diff_days;
			DUK_DDD(DUK_DDDPRINT("--> year=%ld, day-within-year=%ld",
			                     (long) year, (long) *out_day_within_year));
			DUK_ASSERT(*out_day_within_year >= 0);
			DUK_ASSERT(*out_day_within_year < (duk__is_leap_year(year) ? 366 : 365));
			return year;
		}

		/* Note: this is very tricky; we must never 'overshoot' the
		 * correction downwards.
		 */
		year -= 1 + (diff_days - 1) / 366;  /* conservative */
	}
}

/* Given a (year, month, day-within-month) triple, compute day number.
 * The input triple is un-normalized and may contain non-finite values.
 */
DUK_LOCAL duk_double_t duk__make_day(duk_double_t year, duk_double_t month, duk_double_t day) {
	duk_int_t day_num;
	duk_bool_t is_leap;
	duk_small_int_t i, n;

	/* Assume that year, month, day are all coerced to whole numbers.
	 * They may also be NaN or infinity, in which case this function
	 * must return NaN or infinity to ensure time value becomes NaN.
	 * If 'day' is NaN, the final return will end up returning a NaN,
	 * so it doesn't need to be checked here.
	 */

	if (!DUK_ISFINITE(year) || !DUK_ISFINITE(month)) {
		return DUK_DOUBLE_NAN;
	}

	year += DUK_FLOOR(month / 12.0);

	month = DUK_FMOD(month, 12.0);
	if (month < 0.0) {
		/* handle negative values */
		month += 12.0;
	}

	/* The algorithm in E5.1 Section 15.9.1.12 normalizes month, but
	 * does not normalize the day-of-month (nor check whether or not
	 * it is finite) because it's not necessary for finding the day
	 * number which matches the (year,month) pair.
	 *
	 * We assume that duk__day_from_year() is exact here.
	 *
	 * Without an explicit infinity / NaN check in the beginning,
	 * day_num would be a bogus integer here.
	 *
	 * It's possible for 'year' to be out of integer range here.
	 * If so, we need to return NaN without integer overflow.
	 * This fixes test-bug-setyear-overflow.js.
	 */

	if (!duk__year_in_valid_range(year)) {
		DUK_DD(DUK_DDPRINT("year not in ecmascript valid range, avoid integer overflow: %lf", (double) year));
		return DUK_DOUBLE_NAN;
	}
	day_num = duk__day_from_year((duk_int_t) year);
	is_leap = duk__is_leap_year((duk_int_t) year);

	n = (duk_small_int_t) month;
	for (i = 0; i < n; i++) {
		day_num += duk__days_in_month[i];
		if (i == 1 && is_leap) {
			day_num++;
		}
	}

	/* If 'day' is NaN, returns NaN. */
	return (duk_double_t) day_num + day;
}

/* Split time value into parts.  The time value is assumed to be an internal
 * one, i.e. finite, no fractions.  Possible local time adjustment has already
 * been applied when reading the time value.
 */
DUK_LOCAL void duk__timeval_to_parts(duk_double_t d, duk_int_t *parts, duk_double_t *dparts, duk_small_uint_t flags) {
	duk_double_t d1, d2;
	duk_int_t t1, t2;
	duk_int_t day_since_epoch;
	duk_int_t year;  /* does not fit into 16 bits */
	duk_small_int_t day_in_year;
	duk_small_int_t month;
	duk_small_int_t day;
	duk_small_int_t dim;
	duk_int_t jan1_since_epoch;
	duk_small_int_t jan1_weekday;
	duk_int_t equiv_year;
	duk_small_uint_t i;
	duk_bool_t is_leap;
	duk_small_int_t arridx;

	DUK_ASSERT(DUK_ISFINITE(d));    /* caller checks */
	DUK_ASSERT(DUK_FLOOR(d) == d);  /* no fractions in internal time */

	/* The timevalue must be in valid Ecmascript range, but since a local
	 * time offset can be applied, we need to allow a +/- 24h leeway to
	 * the value.  In other words, although the UTC time is within the
	 * Ecmascript range, the local part values can be just outside of it.
	 */
	DUK_UNREF(duk__timeval_in_leeway_range);
	DUK_ASSERT(duk__timeval_in_leeway_range(d));

	/* these computations are guaranteed to be exact for the valid
	 * E5 time value range, assuming milliseconds without fractions.
	 */
	d1 = (duk_double_t) DUK_FMOD(d, (double) DUK__MS_DAY);
	if (d1 < 0.0) {
		/* deal with negative values */
		d1 += (duk_double_t) DUK__MS_DAY;
	}
	d2 = DUK_FLOOR((double) (d / (duk_double_t) DUK__MS_DAY));
	DUK_ASSERT(d2 * ((duk_double_t) DUK__MS_DAY) + d1 == d);
	/* now expected to fit into a 32-bit integer */
	t1 = (duk_int_t) d1;
	t2 = (duk_int_t) d2;
	day_since_epoch = t2;
	DUK_ASSERT((duk_double_t) t1 == d1);
	DUK_ASSERT((duk_double_t) t2 == d2);

	/* t1 = milliseconds within day (fits 32 bit)
	 * t2 = day number from epoch (fits 32 bit, may be negative)
	 */

	parts[DUK__IDX_MILLISECOND] = t1 % 1000; t1 /= 1000;
	parts[DUK__IDX_SECOND] = t1 % 60; t1 /= 60;
	parts[DUK__IDX_MINUTE] = t1 % 60; t1 /= 60;
	parts[DUK__IDX_HOUR] = t1;
	DUK_ASSERT(parts[DUK__IDX_MILLISECOND] >= 0 && parts[DUK__IDX_MILLISECOND] <= 999);
	DUK_ASSERT(parts[DUK__IDX_SECOND] >= 0 && parts[DUK__IDX_SECOND] <= 59);
	DUK_ASSERT(parts[DUK__IDX_MINUTE] >= 0 && parts[DUK__IDX_MINUTE] <= 59);
	DUK_ASSERT(parts[DUK__IDX_HOUR] >= 0 && parts[DUK__IDX_HOUR] <= 23);

	DUK_DDD(DUK_DDDPRINT("d=%lf, d1=%lf, d2=%lf, t1=%ld, t2=%ld, parts: hour=%ld min=%ld sec=%ld msec=%ld",
	                     (double) d, (double) d1, (double) d2, (long) t1, (long) t2,
	                     (long) parts[DUK__IDX_HOUR],
	                     (long) parts[DUK__IDX_MINUTE],
	                     (long) parts[DUK__IDX_SECOND],
	                     (long) parts[DUK__IDX_MILLISECOND]));

	/* This assert depends on the input parts representing time inside
	 * the Ecmascript range.
	 */
	DUK_ASSERT(t2 + DUK__WEEKDAY_MOD_ADDER >= 0);
	parts[DUK__IDX_WEEKDAY] = (t2 + 4 + DUK__WEEKDAY_MOD_ADDER) % 7;  /* E5.1 Section 15.9.1.6 */
	DUK_ASSERT(parts[DUK__IDX_WEEKDAY] >= 0 && parts[DUK__IDX_WEEKDAY] <= 6);

	year = duk__year_from_day(t2, &day_in_year);
	day = day_in_year;
	is_leap = duk__is_leap_year(year);
	for (month = 0; month < 12; month++) {
		dim = duk__days_in_month[month];
		if (month == 1 && is_leap) {
			dim++;
		}
		DUK_DDD(DUK_DDDPRINT("month=%ld, dim=%ld, day=%ld",
		                     (long) month, (long) dim, (long) day));
		if (day < dim) {
			break;
		}
		day -= dim;
	}
	DUK_DDD(DUK_DDDPRINT("final month=%ld", (long) month));
	DUK_ASSERT(month >= 0 && month <= 11);
	DUK_ASSERT(day >= 0 && day <= 31);

	/* Equivalent year mapping, used to avoid DST trouble when platform
	 * may fail to provide reasonable DST answers for dates outside the
	 * ordinary range (e.g. 1970-2038).  An equivalent year has the same
	 * leap-year-ness as the original year and begins on the same weekday
	 * (Jan 1).
	 *
	 * The year 2038 is avoided because there seem to be problems with it
	 * on some platforms.  The year 1970 is also avoided as there were
	 * practical problems with it; an equivalent year is used for it too,
	 * which breaks some DST computations for 1970 right now, see e.g.
	 * test-bi-date-tzoffset-brute-fi.js.
	 */
	if ((flags & DUK__FLAG_EQUIVYEAR) && (year < 1971 || year > 2037)) {
		DUK_ASSERT(is_leap == 0 || is_leap == 1);

		jan1_since_epoch = day_since_epoch - day_in_year;  /* day number for Jan 1 since epoch */
		DUK_ASSERT(jan1_since_epoch + DUK__WEEKDAY_MOD_ADDER >= 0);
		jan1_weekday = (jan1_since_epoch + 4 + DUK__WEEKDAY_MOD_ADDER) % 7;  /* E5.1 Section 15.9.1.6 */
		DUK_ASSERT(jan1_weekday >= 0 && jan1_weekday <= 6);
		arridx = jan1_weekday;
		if (is_leap) {
			arridx += 7;
		}
		DUK_ASSERT(arridx >= 0 && arridx < (duk_small_int_t) (sizeof(duk__date_equivyear) / sizeof(duk_uint8_t)));

		equiv_year = (duk_int_t) duk__date_equivyear[arridx] + 1970;
		year = equiv_year;
		DUK_DDD(DUK_DDDPRINT("equiv year mapping, year=%ld, day_in_year=%ld, day_since_epoch=%ld, "
		                     "jan1_since_epoch=%ld, jan1_weekday=%ld -> equiv year %ld",
		                     (long) year, (long) day_in_year, (long) day_since_epoch,
		                     (long) jan1_since_epoch, (long) jan1_weekday, (long) equiv_year));
	}

	parts[DUK__IDX_YEAR] = year;
	parts[DUK__IDX_MONTH] = month;
	parts[DUK__IDX_DAY] = day;

	if (flags & DUK__FLAG_ONEBASED) {
		parts[DUK__IDX_MONTH]++;  /* zero-based -> one-based */
		parts[DUK__IDX_DAY]++;    /* -""- */
	}

	if (dparts != NULL) {
		for (i = 0; i < DUK__NUM_PARTS; i++) {
			dparts[i] = (duk_double_t) parts[i];
		}
	}
}

/* Compute time value from (double) parts.  The parts can be either UTC
 * or local time; if local, they need to be (conceptually) converted into
 * UTC time.  The parts may represent valid or invalid time, and may be
 * wildly out of range (but may cancel each other and still come out in
 * the valid Date range).
 */
DUK_LOCAL duk_double_t duk__get_timeval_from_dparts(duk_double_t *dparts, duk_small_uint_t flags) {
#if defined(DUK_USE_PARANOID_DATE_COMPUTATION)
	/* See comments below on MakeTime why these are volatile. */
	volatile duk_double_t tmp_time;
	volatile duk_double_t tmp_day;
	volatile duk_double_t d;
#else
	duk_double_t tmp_time;
	duk_double_t tmp_day;
	duk_double_t d;
#endif
	duk_small_uint_t i;
	duk_int_t tzoff, tzoffprev1, tzoffprev2;

	/* Expects 'this' at top of stack on entry. */

	/* Coerce all finite parts with ToInteger().  ToInteger() must not
	 * be called for NaN/Infinity because it will convert e.g. NaN to
	 * zero.  If ToInteger() has already been called, this has no side
	 * effects and is idempotent.
	 *
	 * Don't read dparts[DUK__IDX_WEEKDAY]; it will cause Valgrind issues
	 * if the value is uninitialized.
	 */
	for (i = 0; i <= DUK__IDX_MILLISECOND; i++) {
		/* SCANBUILD: scan-build complains here about assigned value
		 * being garbage or undefined.  This is correct but operating
		 * on undefined values has no ill effect and is ignored by the
		 * caller in the case where this happens.
		 */
		d = dparts[i];
		if (DUK_ISFINITE(d)) {
			dparts[i] = duk_js_tointeger_number(d);
		}
	}

	/* Use explicit steps in computation to try to ensure that
	 * computation happens with intermediate results coerced to
	 * double values (instead of using something more accurate).
	 * E.g. E5.1 Section 15.9.1.11 requires use of IEEE 754
	 * rules (= Ecmascript '+' and '*' operators).
	 *
	 * Without 'volatile' even this approach fails on some platform
	 * and compiler combinations.  For instance, gcc 4.8.1 on Ubuntu
	 * 64-bit, with -m32 and without -std=c99, test-bi-date-canceling.js
	 * would fail because of some optimizations when computing tmp_time
	 * (MakeTime below).  Adding 'volatile' to tmp_time solved this
	 * particular problem (annoyingly, also adding debug prints or
	 * running the executable under valgrind hides it).
	 */

	/* MakeTime */
	tmp_time = 0.0;
	tmp_time += dparts[DUK__IDX_HOUR] * ((duk_double_t) DUK__MS_HOUR);
	tmp_time += dparts[DUK__IDX_MINUTE] * ((duk_double_t) DUK__MS_MINUTE);
	tmp_time += dparts[DUK__IDX_SECOND] * ((duk_double_t) DUK__MS_SECOND);
	tmp_time += dparts[DUK__IDX_MILLISECOND];

	/* MakeDay */
	tmp_day = duk__make_day(dparts[DUK__IDX_YEAR], dparts[DUK__IDX_MONTH], dparts[DUK__IDX_DAY]);

	/* MakeDate */
	d = tmp_day * ((duk_double_t) DUK__MS_DAY) + tmp_time;

	DUK_DDD(DUK_DDDPRINT("time=%lf day=%lf --> timeval=%lf",
	                     (double) tmp_time, (double) tmp_day, (double) d));

	/* Optional UTC conversion. */
	if (flags & DUK__FLAG_LOCALTIME) {
		/* DUK__GET_LOCAL_TZOFFSET() needs to be called with a time
		 * value computed from UTC parts.  At this point we only have
		 * 'd' which is a time value computed from local parts, so it
		 * is off by the UTC-to-local time offset which we don't know
		 * yet.  The current solution for computing the UTC-to-local
		 * time offset is to iterate a few times and detect a fixed
		 * point or a two-cycle loop (or a sanity iteration limit),
		 * see test-bi-date-local-parts.js and test-bi-date-tzoffset-basic-fi.js.
		 *
		 * E5.1 Section 15.9.1.9:
		 * UTC(t) = t - LocalTZA - DaylightSavingTA(t - LocalTZA)
		 *
		 * For NaN/inf, DUK__GET_LOCAL_TZOFFSET() returns 0.
		 */

#if 0
		/* Old solution: don't iterate, incorrect */
		tzoff = DUK__GET_LOCAL_TZOFFSET(d);
		DUK_DDD(DUK_DDDPRINT("tzoffset w/o iteration, tzoff=%ld", (long) tzoff));
		d -= tzoff * 1000L;
		DUK_UNREF(tzoffprev1);
		DUK_UNREF(tzoffprev2);
#endif

		/* Iteration solution */
		tzoff = 0;
		tzoffprev1 = 999999999L;  /* invalid value which never matches */
		for (i = 0; i < DUK__LOCAL_TZOFFSET_MAXITER; i++) {
			tzoffprev2 = tzoffprev1;
			tzoffprev1 = tzoff;
			tzoff = DUK__GET_LOCAL_TZOFFSET(d - tzoff * 1000L);
			DUK_DDD(DUK_DDDPRINT("tzoffset iteration, i=%d, tzoff=%ld, tzoffprev1=%ld tzoffprev2=%ld",
			                     (int) i, (long) tzoff, (long) tzoffprev1, (long) tzoffprev2));
			if (tzoff == tzoffprev1) {
				DUK_DDD(DUK_DDDPRINT("tzoffset iteration finished, i=%d, tzoff=%ld, tzoffprev1=%ld, tzoffprev2=%ld",
				                     (int) i, (long) tzoff, (long) tzoffprev1, (long) tzoffprev2));
				break;
			} else if (tzoff == tzoffprev2) {
				/* Two value cycle, see e.g. test-bi-date-tzoffset-basic-fi.js.
				 * In these cases, favor a higher tzoffset to get a consistent
				 * result which is independent of iteration count.  Not sure if
				 * this is a generically correct solution.
				 */
				DUK_DDD(DUK_DDDPRINT("tzoffset iteration two-value cycle, i=%d, tzoff=%ld, tzoffprev1=%ld, tzoffprev2=%ld",
				                     (int) i, (long) tzoff, (long) tzoffprev1, (long) tzoffprev2));
				if (tzoffprev1 > tzoff) {
					tzoff = tzoffprev1;
				}
				break;
			}
		}
		DUK_DDD(DUK_DDDPRINT("tzoffset iteration, tzoff=%ld", (long) tzoff));
		d -= tzoff * 1000L;
	}

	/* TimeClip(), which also handles Infinity -> NaN conversion */
	d = duk__timeclip(d);

	return d;
}

/*
 *  API oriented helpers
 */

/* Push 'this' binding, check that it is a Date object; then push the
 * internal time value.  At the end, stack is: [ ... this timeval ].
 * Returns the time value.  Local time adjustment is done if requested.
 */
DUK_LOCAL duk_double_t duk__push_this_get_timeval_tzoffset(duk_context *ctx, duk_small_uint_t flags, duk_int_t *out_tzoffset) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h;
	duk_double_t d;
	duk_int_t tzoffset = 0;

	duk_push_this(ctx);
	h = duk_get_hobject(ctx, -1);  /* XXX: getter with class check, useful in built-ins */
	if (h == NULL || DUK_HOBJECT_GET_CLASS_NUMBER(h) != DUK_HOBJECT_CLASS_DATE) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "expected Date");
	}

	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_VALUE);
	d = duk_to_number(ctx, -1);
	duk_pop(ctx);

	if (DUK_ISNAN(d)) {
		if (flags & DUK__FLAG_NAN_TO_ZERO) {
			d = 0.0;
		}
		if (flags & DUK__FLAG_NAN_TO_RANGE_ERROR) {
			DUK_ERROR(thr, DUK_ERR_RANGE_ERROR, "Invalid Date");
		}
	}
	/* if no NaN handling flag, may still be NaN here, but not Inf */
	DUK_ASSERT(!DUK_ISINF(d));

	if (flags & DUK__FLAG_LOCALTIME) {
		/* Note: DST adjustment is determined using UTC time.
		 * If 'd' is NaN, tzoffset will be 0.
		 */
		tzoffset = DUK__GET_LOCAL_TZOFFSET(d);  /* seconds */
		d += tzoffset * 1000L;
	}
	if (out_tzoffset) {
		*out_tzoffset = tzoffset;
	}

	/* [ ... this ] */
	return d;
}

DUK_LOCAL duk_double_t duk__push_this_get_timeval(duk_context *ctx, duk_small_uint_t flags) {
	return duk__push_this_get_timeval_tzoffset(ctx, flags, NULL);
}

/* Set timeval to 'this' from dparts, push the new time value onto the
 * value stack and return 1 (caller can then tailcall us).  Expects
 * the value stack to contain 'this' on the stack top.
 */
DUK_LOCAL duk_ret_t duk__set_this_timeval_from_dparts(duk_context *ctx, duk_double_t *dparts, duk_small_uint_t flags) {
	duk_double_t d;

	/* [ ... this ] */

	d = duk__get_timeval_from_dparts(dparts, flags);
	duk_push_number(ctx, d);  /* -> [ ... this timeval_new ] */
	duk_dup_top(ctx);         /* -> [ ... this timeval_new timeval_new ] */
	duk_put_prop_stridx(ctx, -3, DUK_STRIDX_INT_VALUE);

	/* stack top: new time value, return 1 to allow tailcalls */
	return 1;
}

/* 'out_buf' must be at least DUK_BI_DATE_ISO8601_BUFSIZE long. */
DUK_LOCAL void duk__format_parts_iso8601(duk_int_t *parts, duk_int_t tzoffset, duk_small_uint_t flags, duk_uint8_t *out_buf) {
	char yearstr[8];   /* "-123456\0" */
	char tzstr[8];     /* "+11:22\0" */
	char sep = (flags & DUK__FLAG_SEP_T) ? DUK_ASC_UC_T : DUK_ASC_SPACE;

	DUK_ASSERT(parts[DUK__IDX_MONTH] >= 1 && parts[DUK__IDX_MONTH] <= 12);
	DUK_ASSERT(parts[DUK__IDX_DAY] >= 1 && parts[DUK__IDX_DAY] <= 31);
	DUK_ASSERT(parts[DUK__IDX_YEAR] >= -999999 && parts[DUK__IDX_YEAR] <= 999999);

	/* Note: %06d for positive value, %07d for negative value to include
	 * sign and 6 digits.
	 */
	DUK_SNPRINTF(yearstr,
	             sizeof(yearstr),
	             (parts[DUK__IDX_YEAR] >= 0 && parts[DUK__IDX_YEAR] <= 9999) ? "%04ld" :
	                    ((parts[DUK__IDX_YEAR] >= 0) ? "+%06ld" : "%07ld"),
	             (long) parts[DUK__IDX_YEAR]);
	yearstr[sizeof(yearstr) - 1] = (char) 0;

	if (flags & DUK__FLAG_LOCALTIME) {
		/* tzoffset seconds are dropped; 16 bits suffice for
		 * time offset in minutes
		 */
		if (tzoffset >= 0) {
			duk_small_int_t tmp = tzoffset / 60;
			DUK_SNPRINTF(tzstr, sizeof(tzstr), "+%02d:%02d", (int) (tmp / 60), (int) (tmp % 60));
		} else {
			duk_small_int_t tmp = -tzoffset / 60;
			DUK_SNPRINTF(tzstr, sizeof(tzstr), "-%02d:%02d", (int) (tmp / 60), (int) (tmp % 60));
		}
		tzstr[sizeof(tzstr) - 1] = (char) 0;
	} else {
		tzstr[0] = DUK_ASC_UC_Z;
		tzstr[1] = (char) 0;
	}

	/* Unlike year, the other parts fit into 16 bits so %d format
	 * is portable.
	 */
	if ((flags & DUK__FLAG_TOSTRING_DATE) && (flags & DUK__FLAG_TOSTRING_TIME)) {
		DUK_SPRINTF((char *) out_buf, "%s-%02d-%02d%c%02d:%02d:%02d.%03d%s",
		            (const char *) yearstr, (int) parts[DUK__IDX_MONTH], (int) parts[DUK__IDX_DAY], (int) sep,
		            (int) parts[DUK__IDX_HOUR], (int) parts[DUK__IDX_MINUTE],
		            (int) parts[DUK__IDX_SECOND], (int) parts[DUK__IDX_MILLISECOND], (const char *) tzstr);
	} else if (flags & DUK__FLAG_TOSTRING_DATE) {
		DUK_SPRINTF((char *) out_buf, "%s-%02d-%02d",
		            (const char *) yearstr, (int) parts[DUK__IDX_MONTH], (int) parts[DUK__IDX_DAY]);
	} else {
		DUK_ASSERT(flags & DUK__FLAG_TOSTRING_TIME);
		DUK_SPRINTF((char *) out_buf, "%02d:%02d:%02d.%03d%s",
		            (int) parts[DUK__IDX_HOUR], (int) parts[DUK__IDX_MINUTE],
		            (int) parts[DUK__IDX_SECOND], (int) parts[DUK__IDX_MILLISECOND],
		            (const char *) tzstr);
	}
}

/* Helper for string conversion calls: check 'this' binding, get the
 * internal time value, and format date and/or time in a few formats.
 * Return value allows tail calls.
 */
DUK_LOCAL duk_ret_t duk__to_string_helper(duk_context *ctx, duk_small_uint_t flags) {
	duk_double_t d;
	duk_int_t parts[DUK__NUM_PARTS];
	duk_int_t tzoffset;  /* seconds, doesn't fit into 16 bits */
	duk_bool_t rc;
	duk_uint8_t buf[DUK_BI_DATE_ISO8601_BUFSIZE];

	DUK_UNREF(rc);  /* unreferenced with some options */

	d = duk__push_this_get_timeval_tzoffset(ctx, flags, &tzoffset);
	if (DUK_ISNAN(d)) {
		duk_push_hstring_stridx(ctx, DUK_STRIDX_INVALID_DATE);
		return 1;
	}
	DUK_ASSERT(DUK_ISFINITE(d));

	/* formatters always get one-based month/day-of-month */
	duk__timeval_to_parts(d, parts, NULL, DUK__FLAG_ONEBASED);
	DUK_ASSERT(parts[DUK__IDX_MONTH] >= 1 && parts[DUK__IDX_MONTH] <= 12);
	DUK_ASSERT(parts[DUK__IDX_DAY] >= 1 && parts[DUK__IDX_DAY] <= 31);

	if (flags & DUK__FLAG_TOSTRING_LOCALE) {
		/* try locale specific formatter; if it refuses to format the
		 * string, fall back to an ISO 8601 formatted value in local
		 * time.
		 */
#ifdef DUK_USE_DATE_FMT_STRFTIME
		rc = duk__format_parts_strftime(ctx, parts, tzoffset, flags);
		if (rc != 0) {
			return 1;
		}
#else
		/* No locale specific formatter; this is OK, we fall back
		 * to ISO 8601.
		 */
#endif
	}

	/* Different calling convention than above used because the helper
	 * is shared.
	 */
	duk__format_parts_iso8601(parts, tzoffset, flags, buf);
	duk_push_string(ctx, (const char *) buf);
	return 1;
}

/* Helper for component getter calls: check 'this' binding, get the
 * internal time value, split it into parts (either as UTC time or
 * local time), push a specified component as a return value to the
 * value stack and return 1 (caller can then tailcall us).
 */
DUK_LOCAL duk_ret_t duk__get_part_helper(duk_context *ctx, duk_small_uint_t flags_and_idx) {
	duk_double_t d;
	duk_int_t parts[DUK__NUM_PARTS];
	duk_small_uint_t idx_part = (duk_small_uint_t) (flags_and_idx >> DUK__FLAG_VALUE_SHIFT);  /* unpack args */

	DUK_ASSERT_DISABLE(idx_part >= 0);  /* unsigned */
	DUK_ASSERT(idx_part < DUK__NUM_PARTS);

	d = duk__push_this_get_timeval(ctx, flags_and_idx);
	if (DUK_ISNAN(d)) {
		duk_push_nan(ctx);
		return 1;
	}
	DUK_ASSERT(DUK_ISFINITE(d));

	duk__timeval_to_parts(d, parts, NULL, flags_and_idx);  /* no need to mask idx portion */

	/* Setter APIs detect special year numbers (0...99) and apply a +1900
	 * only in certain cases.  The legacy getYear() getter applies -1900
	 * unconditionally.
	 */
	duk_push_int(ctx, (flags_and_idx & DUK__FLAG_SUB1900) ? parts[idx_part] - 1900 : parts[idx_part]);
	return 1;
}

/* Helper for component setter calls: check 'this' binding, get the
 * internal time value, split it into parts (either as UTC time or
 * local time), modify one or more components as specified, recompute
 * the time value, set it as the internal value.  Finally, push the
 * new time value as a return value to the value stack and return 1
 * (caller can then tailcall us).
 */
DUK_LOCAL duk_ret_t duk__set_part_helper(duk_context *ctx, duk_small_uint_t flags_and_maxnargs) {
	duk_double_t d;
	duk_int_t parts[DUK__NUM_PARTS];
	duk_double_t dparts[DUK__NUM_PARTS];
	duk_idx_t nargs;
	duk_small_uint_t maxnargs = (duk_small_uint_t) (flags_and_maxnargs >> DUK__FLAG_VALUE_SHIFT);  /* unpack args */
	duk_small_uint_t idx_first, idx;
	duk_small_uint_t i;

	nargs = duk_get_top(ctx);
	d = duk__push_this_get_timeval(ctx, flags_and_maxnargs);
	DUK_ASSERT(DUK_ISFINITE(d) || DUK_ISNAN(d));

	if (DUK_ISFINITE(d)) {
		duk__timeval_to_parts(d, parts, dparts, flags_and_maxnargs);
	} else {
		/* NaN timevalue: we need to coerce the arguments, but
		 * the resulting internal timestamp needs to remain NaN.
		 * This works but is not pretty: parts and dparts will
		 * be partially uninitialized, but we only write to them.
		 */
	}

	/*
	 *  Determining which datetime components to overwrite based on
	 *  stack arguments is a bit complicated, but important to factor
	 *  out from setters themselves for compactness.
	 *
	 *  If DUK__FLAG_TIMESETTER, maxnargs indicates setter type:
	 *
	 *   1 -> millisecond
	 *   2 -> second, [millisecond]
	 *   3 -> minute, [second], [millisecond]
	 *   4 -> hour, [minute], [second], [millisecond]
	 *
	 *  Else:
	 *
	 *   1 -> date
	 *   2 -> month, [date]
	 *   3 -> year, [month], [date]
	 *
	 *  By comparing nargs and maxnargs (and flags) we know which
	 *  components to override.  We rely on part index ordering.
	 */

	if (flags_and_maxnargs & DUK__FLAG_TIMESETTER) {
		DUK_ASSERT(maxnargs >= 1 && maxnargs <= 4);
		idx_first = DUK__IDX_MILLISECOND - (maxnargs - 1);
	} else {
		DUK_ASSERT(maxnargs >= 1 && maxnargs <= 3);
		idx_first = DUK__IDX_DAY - (maxnargs - 1);
	}
	DUK_ASSERT_DISABLE(idx_first >= 0);  /* unsigned */
	DUK_ASSERT(idx_first < DUK__NUM_PARTS);

	for (i = 0; i < maxnargs; i++) {
		if ((duk_idx_t) i >= nargs) {
			/* no argument given -> leave components untouched */
			break;
		}
		idx = idx_first + i;
		DUK_ASSERT_DISABLE(idx >= 0);  /* unsigned */
		DUK_ASSERT(idx < DUK__NUM_PARTS);

		if (idx == DUK__IDX_YEAR && (flags_and_maxnargs & DUK__FLAG_YEAR_FIXUP)) {
			duk__twodigit_year_fixup(ctx, (duk_idx_t) i);
		}

		dparts[idx] = duk_to_number(ctx, i);

		if (idx == DUK__IDX_DAY) {
			/* Day-of-month is one-based in the API, but zero-based
			 * internally, so fix here.  Note that month is zero-based
			 * both in the API and internally.
			 */
			/* SCANBUILD: complains about use of uninitialized values.
			 * The complaint is correct, but operating in undefined
			 * values here is intentional in some cases and the caller
			 * ignores the results.
			 */
			dparts[idx] -= 1.0;
		}
	}

	/* Leaves new timevalue on stack top and returns 1, which is correct
	 * for part setters.
	 */
	if (DUK_ISFINITE(d)) {
		return duk__set_this_timeval_from_dparts(ctx, dparts, flags_and_maxnargs);
	} else {
		/* Internal timevalue is already NaN, so don't touch it. */
		duk_push_nan(ctx);
		return 1;
	}
}

/* Apply ToNumber() to specified index; if ToInteger(val) in [0,99], add
 * 1900 and replace value at idx_val.
 */
DUK_LOCAL void duk__twodigit_year_fixup(duk_context *ctx, duk_idx_t idx_val) {
	duk_double_t d;

	/* XXX: idx_val would fit into 16 bits, but using duk_small_uint_t
	 * might not generate better code due to casting.
	 */

	/* E5 Sections 15.9.3.1, B.2.4, B.2.5 */
	duk_to_number(ctx, idx_val);
	if (duk_is_nan(ctx, idx_val)) {
		return;
	}
	duk_dup(ctx, idx_val);
	duk_to_int(ctx, -1);
	d = duk_get_number(ctx, -1);  /* get as double to handle huge numbers correctly */
	if (d >= 0.0 && d <= 99.0) {
		d += 1900.0;
		duk_push_number(ctx, d);
		duk_replace(ctx, idx_val);
	}
	duk_pop(ctx);
}

/* Set datetime parts from stack arguments, defaulting any missing values.
 * Day-of-week is not set; it is not required when setting the time value.
 */
DUK_LOCAL void duk__set_parts_from_args(duk_context *ctx, duk_double_t *dparts, duk_idx_t nargs) {
	duk_double_t d;
	duk_small_uint_t i;
	duk_small_uint_t idx;

	/* Causes a ToNumber() coercion, but doesn't break coercion order since
	 * year is coerced first anyway.
	 */
	duk__twodigit_year_fixup(ctx, 0);

	/* There are at most 7 args, but we use 8 here so that also
	 * DUK__IDX_WEEKDAY gets initialized (to zero) to avoid the potential
	 * for any Valgrind gripes later.
	 */
	for (i = 0; i < 8; i++) {
		/* Note: rely on index ordering */
		idx = DUK__IDX_YEAR + i;
		if ((duk_idx_t) i < nargs) {
			d = duk_to_number(ctx, (duk_idx_t) i);
			if (idx == DUK__IDX_DAY) {
				/* Convert day from one-based to zero-based (internal).  This may
				 * cause the day part to be negative, which is OK.
				 */
				d -= 1.0;
			}
		} else {
			/* All components default to 0 except day-of-month which defaults
			 * to 1.  However, because our internal day-of-month is zero-based,
			 * it also defaults to zero here.
			 */
			d = 0.0;
		}
		dparts[idx] = d;
	}

	DUK_DDD(DUK_DDDPRINT("parts from args -> %lf %lf %lf %lf %lf %lf %lf %lf",
	                     (double) dparts[0], (double) dparts[1],
	                     (double) dparts[2], (double) dparts[3],
	                     (double) dparts[4], (double) dparts[5],
	                     (double) dparts[6], (double) dparts[7]));
}

/*
 *  Helper to format a time value into caller buffer, used by logging.
 *  'out_buf' must be at least DUK_BI_DATE_ISO8601_BUFSIZE long.
 */

DUK_INTERNAL void duk_bi_date_format_timeval(duk_double_t timeval, duk_uint8_t *out_buf) {
	duk_int_t parts[DUK__NUM_PARTS];

	duk__timeval_to_parts(timeval,
	                      parts,
	                      NULL,
	                      DUK__FLAG_ONEBASED);

	duk__format_parts_iso8601(parts,
	                          0 /*tzoffset*/,
	                          DUK__FLAG_TOSTRING_DATE |
	                          DUK__FLAG_TOSTRING_TIME |
	                          DUK__FLAG_SEP_T /*flags*/,
	                          out_buf);
}

/*
 *  Indirect magic value lookup for Date methods.
 *
 *  Date methods don't put their control flags into the function magic value
 *  because they wouldn't fit into a LIGHTFUNC's magic field.  Instead, the
 *  magic value is set to an index pointing to the array of control flags
 *  below.
 *
 *  This must be kept in strict sync with genbuiltins.py!
 */

static duk_uint16_t duk__date_magics[] = {
	/* 0: toString */
	DUK__FLAG_TOSTRING_DATE + DUK__FLAG_TOSTRING_TIME + DUK__FLAG_LOCALTIME,

	/* 1: toDateString */
	DUK__FLAG_TOSTRING_DATE + DUK__FLAG_LOCALTIME,

	/* 2: toTimeString */
	DUK__FLAG_TOSTRING_TIME + DUK__FLAG_LOCALTIME,

	/* 3: toLocaleString */
	DUK__FLAG_TOSTRING_DATE + DUK__FLAG_TOSTRING_TIME + DUK__FLAG_TOSTRING_LOCALE + DUK__FLAG_LOCALTIME,

	/* 4: toLocaleDateString */
	DUK__FLAG_TOSTRING_DATE + DUK__FLAG_TOSTRING_LOCALE + DUK__FLAG_LOCALTIME,

	/* 5: toLocaleTimeString */
	DUK__FLAG_TOSTRING_TIME + DUK__FLAG_TOSTRING_LOCALE + DUK__FLAG_LOCALTIME,

	/* 6: toUTCString */
	DUK__FLAG_TOSTRING_DATE + DUK__FLAG_TOSTRING_TIME,

	/* 7: toISOString */
	DUK__FLAG_TOSTRING_DATE + DUK__FLAG_TOSTRING_TIME + DUK__FLAG_NAN_TO_RANGE_ERROR + DUK__FLAG_SEP_T,

	/* 8: getFullYear */
	DUK__FLAG_LOCALTIME + (DUK__IDX_YEAR << DUK__FLAG_VALUE_SHIFT),

	/* 9: getUTCFullYear */
	0 + (DUK__IDX_YEAR << DUK__FLAG_VALUE_SHIFT),

	/* 10: getMonth */
	DUK__FLAG_LOCALTIME + (DUK__IDX_MONTH << DUK__FLAG_VALUE_SHIFT),

	/* 11: getUTCMonth */
	0 + (DUK__IDX_MONTH << DUK__FLAG_VALUE_SHIFT),

	/* 12: getDate */
	DUK__FLAG_ONEBASED + DUK__FLAG_LOCALTIME + (DUK__IDX_DAY << DUK__FLAG_VALUE_SHIFT),

	/* 13: getUTCDate */
	DUK__FLAG_ONEBASED + (DUK__IDX_DAY << DUK__FLAG_VALUE_SHIFT),

	/* 14: getDay */
	DUK__FLAG_LOCALTIME + (DUK__IDX_WEEKDAY << DUK__FLAG_VALUE_SHIFT),

	/* 15: getUTCDay */
	0 + (DUK__IDX_WEEKDAY << DUK__FLAG_VALUE_SHIFT),

	/* 16: getHours */
	DUK__FLAG_LOCALTIME + (DUK__IDX_HOUR << DUK__FLAG_VALUE_SHIFT),

	/* 17: getUTCHours */
	0 + (DUK__IDX_HOUR << DUK__FLAG_VALUE_SHIFT),

	/* 18: getMinutes */
	DUK__FLAG_LOCALTIME + (DUK__IDX_MINUTE << DUK__FLAG_VALUE_SHIFT),

	/* 19: getUTCMinutes */
	0 + (DUK__IDX_MINUTE << DUK__FLAG_VALUE_SHIFT),

	/* 20: getSeconds */
	DUK__FLAG_LOCALTIME + (DUK__IDX_SECOND << DUK__FLAG_VALUE_SHIFT),

	/* 21: getUTCSeconds */
	0 + (DUK__IDX_SECOND << DUK__FLAG_VALUE_SHIFT),

	/* 22: getMilliseconds */
	DUK__FLAG_LOCALTIME + (DUK__IDX_MILLISECOND << DUK__FLAG_VALUE_SHIFT),

	/* 23: getUTCMilliseconds */
	0 + (DUK__IDX_MILLISECOND << DUK__FLAG_VALUE_SHIFT),

	/* 24: setMilliseconds */
	DUK__FLAG_TIMESETTER + DUK__FLAG_LOCALTIME + (1 << DUK__FLAG_VALUE_SHIFT),

	/* 25: setUTCMilliseconds */
	DUK__FLAG_TIMESETTER + (1 << DUK__FLAG_VALUE_SHIFT),

	/* 26: setSeconds */
	DUK__FLAG_TIMESETTER + DUK__FLAG_LOCALTIME + (2 << DUK__FLAG_VALUE_SHIFT),

	/* 27: setUTCSeconds */
	DUK__FLAG_TIMESETTER + (2 << DUK__FLAG_VALUE_SHIFT),

	/* 28: setMinutes */
	DUK__FLAG_TIMESETTER + DUK__FLAG_LOCALTIME + (3 << DUK__FLAG_VALUE_SHIFT),

	/* 29: setUTCMinutes */
	DUK__FLAG_TIMESETTER + (3 << DUK__FLAG_VALUE_SHIFT),

	/* 30: setHours */
	DUK__FLAG_TIMESETTER + DUK__FLAG_LOCALTIME + (4 << DUK__FLAG_VALUE_SHIFT),

	/* 31: setUTCHours */
	DUK__FLAG_TIMESETTER + (4 << DUK__FLAG_VALUE_SHIFT),

	/* 32: setDate */
	DUK__FLAG_LOCALTIME + (1 << DUK__FLAG_VALUE_SHIFT),

	/* 33: setUTCDate */
	0 + (1 << DUK__FLAG_VALUE_SHIFT),

	/* 34: setMonth */
	DUK__FLAG_LOCALTIME + (2 << DUK__FLAG_VALUE_SHIFT),

	/* 35: setUTCMonth */
	0 + (2 << DUK__FLAG_VALUE_SHIFT),

	/* 36: setFullYear */
	DUK__FLAG_NAN_TO_ZERO + DUK__FLAG_LOCALTIME + (3 << DUK__FLAG_VALUE_SHIFT),

	/* 37: setUTCFullYear */
	DUK__FLAG_NAN_TO_ZERO + (3 << DUK__FLAG_VALUE_SHIFT),

	/* 38: getYear */
	DUK__FLAG_LOCALTIME + DUK__FLAG_SUB1900 + (DUK__IDX_YEAR << DUK__FLAG_VALUE_SHIFT),

	/* 39: setYear */
	DUK__FLAG_NAN_TO_ZERO + DUK__FLAG_YEAR_FIXUP + (3 << DUK__FLAG_VALUE_SHIFT),
};

DUK_LOCAL duk_small_uint_t duk__date_get_indirect_magic(duk_context *ctx) {
	duk_small_int_t magicidx = (duk_small_uint_t) duk_get_current_magic(ctx);
	DUK_ASSERT(magicidx >= 0 && magicidx < (duk_small_int_t) (sizeof(duk__date_magics) / sizeof(duk_uint16_t)));
	return (duk_small_uint_t) duk__date_magics[magicidx];
}

/*
 *  Constructor calls
 */

DUK_INTERNAL duk_ret_t duk_bi_date_constructor(duk_context *ctx) {
	duk_idx_t nargs = duk_get_top(ctx);
	duk_bool_t is_cons = duk_is_constructor_call(ctx);
	duk_double_t dparts[DUK__NUM_PARTS];
	duk_double_t d;

	DUK_DDD(DUK_DDDPRINT("Date constructor, nargs=%ld, is_cons=%ld", (long) nargs, (long) is_cons));

	duk_push_object_helper(ctx,
	                       DUK_HOBJECT_FLAG_EXTENSIBLE |
	                       DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_DATE),
	                       DUK_BIDX_DATE_PROTOTYPE);

	/* Unlike most built-ins, the internal [[PrimitiveValue]] of a Date
	 * is mutable.
	 */

	if (nargs == 0 || !is_cons) {
		d = duk__timeclip(DUK__GET_NOW_TIMEVAL(ctx));
		duk_push_number(ctx, d);
		duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_W);
		if (!is_cons) {
			/* called as a normal function: return new Date().toString() */
			duk_to_string(ctx, -1);
		}
		return 1;
	} else if (nargs == 1) {
		duk_to_primitive(ctx, 0, DUK_HINT_NONE);
		if (duk_is_string(ctx, 0)) {
			duk__parse_string(ctx, duk_to_string(ctx, 0));
			duk_replace(ctx, 0);  /* may be NaN */
		}
		d = duk__timeclip(duk_to_number(ctx, 0));
		duk_push_number(ctx, d);
		duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_W);
		return 1;
	}

	duk__set_parts_from_args(ctx, dparts, nargs);

	/* Parts are in local time, convert when setting. */

	(void) duk__set_this_timeval_from_dparts(ctx, dparts, DUK__FLAG_LOCALTIME /*flags*/);  /* -> [ ... this timeval ] */
	duk_pop(ctx);  /* -> [ ... this ] */
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_date_constructor_parse(duk_context *ctx) {
	return duk__parse_string(ctx, duk_to_string(ctx, 0));
}

DUK_INTERNAL duk_ret_t duk_bi_date_constructor_utc(duk_context *ctx) {
	duk_idx_t nargs = duk_get_top(ctx);
	duk_double_t dparts[DUK__NUM_PARTS];
	duk_double_t d;

	/* Behavior for nargs < 2 is implementation dependent: currently we'll
	 * set a NaN time value (matching V8 behavior) in this case.
	 */

	if (nargs < 2) {
		duk_push_nan(ctx);
	} else {
		duk__set_parts_from_args(ctx, dparts, nargs);
		d = duk__get_timeval_from_dparts(dparts, 0 /*flags*/);
		duk_push_number(ctx, d);
	}
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_date_constructor_now(duk_context *ctx) {
	duk_double_t d;

	d = DUK__GET_NOW_TIMEVAL(ctx);
	DUK_ASSERT(duk__timeclip(d) == d);  /* TimeClip() should never be necessary */
	duk_push_number(ctx, d);
	return 1;
}

/*
 *  String/JSON conversions
 *
 *  Human readable conversions are now basically ISO 8601 with a space
 *  (instead of 'T') as the date/time separator.  This is a good baseline
 *  and is platform independent.
 *
 *  A shared native helper to provide many conversions.  Magic value contains
 *  a set of flags.  The helper provides:
 *
 *    toString()
 *    toDateString()
 *    toTimeString()
 *    toLocaleString()
 *    toLocaleDateString()
 *    toLocaleTimeString()
 *    toUTCString()
 *    toISOString()
 *
 *  Notes:
 *
 *    - Date.prototype.toGMTString() and Date.prototype.toUTCString() are
 *      required to be the same Ecmascript function object (!), so it is
 *      omitted from here.
 *
 *    - Date.prototype.toUTCString(): E5.1 specification does not require a
 *      specific format, but result should be human readable.  The
 *      specification suggests using ISO 8601 format with a space (instead
 *      of 'T') separator if a more human readable format is not available.
 *
 *    - Date.prototype.toISOString(): unlike other conversion functions,
 *      toISOString() requires a RangeError for invalid date values.
 */

DUK_INTERNAL duk_ret_t duk_bi_date_prototype_tostring_shared(duk_context *ctx) {
	duk_small_uint_t flags = duk__date_get_indirect_magic(ctx);
	return duk__to_string_helper(ctx, flags);
}

DUK_INTERNAL duk_ret_t duk_bi_date_prototype_value_of(duk_context *ctx) {
	/* This native function is also used for Date.prototype.getTime()
	 * as their behavior is identical.
	 */

	duk_double_t d = duk__push_this_get_timeval(ctx, 0 /*flags*/);  /* -> [ this ] */
	DUK_ASSERT(DUK_ISFINITE(d) || DUK_ISNAN(d));
	duk_push_number(ctx, d);
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_date_prototype_to_json(duk_context *ctx) {
	/* Note: toJSON() is a generic function which works even if 'this'
	 * is not a Date.  The sole argument is ignored.
	 */

	duk_push_this(ctx);
	duk_to_object(ctx, -1);

	duk_dup_top(ctx);
	duk_to_primitive(ctx, -1, DUK_HINT_NUMBER);
	if (duk_is_number(ctx, -1)) {
		duk_double_t d = duk_get_number(ctx, -1);
		if (!DUK_ISFINITE(d)) {
			duk_push_null(ctx);
			return 1;
		}
	}
	duk_pop(ctx);

	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_TO_ISO_STRING);
	duk_dup(ctx, -2);  /* -> [ O toIsoString O ] */
	duk_call_method(ctx, 0);
	return 1;
}

/*
 *  Getters.
 *
 *  Implementing getters is quite easy.  The internal time value is either
 *  NaN, or represents milliseconds (without fractions) from Jan 1, 1970.
 *  The internal time value can be converted to integer parts, and each
 *  part will be normalized and will fit into a 32-bit signed integer.
 *
 *  A shared native helper to provide all getters.  Magic value contains
 *  a set of flags and also packs the date component index argument.  The
 *  helper provides:
 *
 *    getFullYear()
 *    getUTCFullYear()
 *    getMonth()
 *    getUTCMonth()
 *    getDate()
 *    getUTCDate()
 *    getDay()
 *    getUTCDay()
 *    getHours()
 *    getUTCHours()
 *    getMinutes()
 *    getUTCMinutes()
 *    getSeconds()
 *    getUTCSeconds()
 *    getMilliseconds()
 *    getUTCMilliseconds()
 *    getYear()
 *
 *  Notes:
 *
 *    - Date.prototype.getDate(): 'date' means day-of-month, and is
 *      zero-based in internal calculations but public API expects it to
 *      be one-based.
 *
 *    - Date.prototype.getTime() and Date.prototype.valueOf() have identical
 *      behavior.  They have separate function objects, but share the same C
 *      function (duk_bi_date_prototype_value_of).
 */

DUK_INTERNAL duk_ret_t duk_bi_date_prototype_get_shared(duk_context *ctx) {
	duk_small_uint_t flags_and_idx = duk__date_get_indirect_magic(ctx);
	return duk__get_part_helper(ctx, flags_and_idx);
}

DUK_INTERNAL duk_ret_t duk_bi_date_prototype_get_timezone_offset(duk_context *ctx) {
	/*
	 *  Return (t - LocalTime(t)) in minutes:
	 *
	 *    t - LocalTime(t) = t - (t + LocalTZA + DaylightSavingTA(t))
	 *                     = -(LocalTZA + DaylightSavingTA(t))
	 *
	 *  where DaylightSavingTA() is checked for time 't'.
	 *
	 *  Note that the sign of the result is opposite to common usage,
	 *  e.g. for EE(S)T which normally is +2h or +3h from UTC, this
	 *  function returns -120 or -180.
	 *
	 */

	duk_double_t d;
	duk_int_t tzoffset;

	/* Note: DST adjustment is determined using UTC time. */
	d = duk__push_this_get_timeval(ctx, 0 /*flags*/);
	DUK_ASSERT(DUK_ISFINITE(d) || DUK_ISNAN(d));
	if (DUK_ISNAN(d)) {
		duk_push_nan(ctx);
	} else {
		DUK_ASSERT(DUK_ISFINITE(d));
		tzoffset = DUK__GET_LOCAL_TZOFFSET(d);
		duk_push_int(ctx, -tzoffset / 60);
	}
	return 1;
}

/*
 *  Setters.
 *
 *  Setters are a bit more complicated than getters.  Component setters
 *  break down the current time value into its (normalized) component
 *  parts, replace one or more components with -unnormalized- new values,
 *  and the components are then converted back into a time value.  As an
 *  example of using unnormalized values:
 *
 *    var d = new Date(1234567890);
 *
 *  is equivalent to:
 *
 *    var d = new Date(0);
 *    d.setUTCMilliseconds(1234567890);
 *
 *  A shared native helper to provide almost all setters.  Magic value
 *  contains a set of flags and also packs the "maxnargs" argument.  The
 *  helper provides:
 *
 *    setMilliseconds()
 *    setUTCMilliseconds()
 *    setSeconds()
 *    setUTCSeconds()
 *    setMinutes()
 *    setUTCMinutes()
 *    setHours()
 *    setUTCHours()
 *    setDate()
 *    setUTCDate()
 *    setMonth()
 *    setUTCMonth()
 *    setFullYear()
 *    setUTCFullYear()
 *    setYear()
 *
 *  Notes:
 *
 *    - Date.prototype.setYear() (Section B addition): special year check
 *      is omitted.  NaN / Infinity will just flow through and ultimately
 *      result in a NaN internal time value.
 *
 *    - Date.prototype.setYear() does not have optional arguments for
 *      setting month and day-in-month (like setFullYear()), but we indicate
 *      'maxnargs' to be 3 to get the year written to the correct component
 *      index in duk__set_part_helper().  The function has nargs == 1, so only
 *      the year will be set regardless of actual argument count.
 */

DUK_INTERNAL duk_ret_t duk_bi_date_prototype_set_shared(duk_context *ctx) {
	duk_small_uint_t flags_and_maxnargs = duk__date_get_indirect_magic(ctx);
	return duk__set_part_helper(ctx, flags_and_maxnargs);
}

DUK_INTERNAL duk_ret_t duk_bi_date_prototype_set_time(duk_context *ctx) {
	duk_double_t d;

	(void) duk__push_this_get_timeval(ctx, 0 /*flags*/); /* -> [ timeval this ] */
	d = duk__timeclip(duk_to_number(ctx, 0));
	duk_push_number(ctx, d);
	duk_dup_top(ctx);
	duk_put_prop_stridx(ctx, -3, DUK_STRIDX_INT_VALUE); /* -> [ timeval this timeval ] */

	return 1;
}
