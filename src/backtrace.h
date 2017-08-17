#ifndef _CHELP_BACKTRACE_H
#define _CHELP_BACKTRACE_H 1

/* FILE */

#ifdef HAVE_ENUMERATOR_H
#include "enumerator.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A backtrace registers the frames on the stack during creation.
 */
typedef struct backtrace_t backtrace_t;


/**
 * Initialize backtracing framework.
 */
void backtraceInit();

/**
 * Deinitialize backtracing framework.
 */
void backtraceDeinit();

/**
 * Create a backtrace of the current stack.
 *
 * @param skip		how many of the innerst frames to skip
 * @return			backtrace
 */
backtrace_t *backtraceCreate(int32_t skip);

/**
 * Create a backtrace, dump it and clean it up.
 *
 * @param label		description to print for this backtrace, or NULL
 * @param file		FILE to log backtrace to, NULL to dbg() function
 * @param detailed	TRUE to resolve line/file using addr2line (slow)
 */
void backtraceDump(char *label, FILE *file, bool detailed);

/**
 * Log the backtrace to a FILE stream.
 *
 * If no file pointer is given, the backtrace is reported over the debug
 * framework to the registered dbg() callback function.
 *
 * @param file		FILE to log backtrace to, NULL for dbg() function
 * @param detailed	TRUE to resolve line/file using addr2line (slow)
 */
void backtraceLog(backtrace_t *this, FILE *file, bool detailed);

/**
 * Check if the backtrace contains a frame having a function in a list.
 *
 * @param		function name array
 * @param		number of elements in function array
 * @return		TRUE if one of the functions is in the stack
 */
bool backtraceContainsFunction(backtrace_t *this, char *function[], int32_t count);

/**
 * Check two backtraces for equality.
 *
 * @param other	backtrace to compare to this
 * @return		TRUE if backtraces are equal
 */
bool backtraceEquals(backtrace_t *this, backtrace_t *other);

/**
 * Create a copy of this backtrace.
 *
 * @return		cloned copy
 */
backtrace_t* backtraceClone(backtrace_t *this);

/**
 * Destroy a backtrace instance.
 */
void backtraceDestroy(backtrace_t *this);

#ifdef HAVE_ENUMERATOR_H

/**
 * Create an enumerator over the stack frame addresses.
 *
 * @return		enumerator_t over void*
 */
enumerator_t* backtraceCreateFrameEnumerator(backtrace_t *this);

#endif /* ifdef HAVE_ENUMERATOR_H */

#ifdef __cplusplus
}
#endif

#endif /* _CHELP_BACKTRACE_H */
