/**
 * @defgroup printf_hook printf_hook
 * @{ @ingroup utils
 */

#ifndef _CHELP_PRINTFHOOK_H
#define _CHELP_PRINTFHOOK_H	1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct printfHook_t printfHook_t;
typedef struct printfHookSpec_t printfHookSpec_t;
typedef struct printfHookData_t printfHookData_t;
typedef enum printfHookArgType_t printfHookArgType_t;

/**
 * Argument types to pass to printf hook.
 */
enum printfHookArgType_t {
	PRINTF_HOOK_ARGTYPE_END,
	PRINTF_HOOK_ARGTYPE_INT,
	PRINTF_HOOK_ARGTYPE_POINTER,
};

/**
 * Callback function type for printf hooks.
 *
 * @param data		hook data, to pass to printInHook()
 * @param spec		format specifier
 * @param args		arguments array
 * @return			number of characters written
 */
typedef int32_t (*printfHookFunc_t)(printfHookData_t *data,
									  printfHookSpec_t *spec,
									  const void *const *args);

/**
 * Properties of the format specifier.
 */
struct printfHookSpec_t {
	int32_t hash;	/**!< TRUE if a '#' was used in the format specifier */
	int32_t minus;	/**!< TRUE if a '-' was used in the format specifier */
	int32_t plus;	/**!< TRUE if a '+' was used in the format specifier */
	int32_t width;	/**!< The width as given in the format specifier */
};

/**
 * Create a printfHook instance.
 */
printfHook_t *printfHookCreate();

/**
 * Destroy a printfHook instance.
 */
void printfHookDestroy(printfHook_t *this);

/**
 * Register a printf handler.
 *
 * @param spec		printf hook format character
 * @param hook		hook function
 * @param ...		list of PRINTF_HOOK_ARGTYPE_*, MUST end with PRINTF_HOOK_ARGTYPE_END
 *
 * Ex: 
 * (this, 'B', chunk_printf_hook, PRINTF_HOOK_ARGTYPE_POINTER, PRINTF_HOOK_ARGTYPE_END)
 * (this, 'T', time_printf_hook, PRINTF_HOOK_ARGTYPE_POINTER, PRINTF_HOOK_ARGTYPE_INT, PRINTF_HOOK_ARGTYPE_END)
 */
void printfHookAddHandler(printfHook_t *this, char spec, printfHookFunc_t hook, ...);

/**
 * Print with format string within a printf hook.
 *
 * @param data		hook data, as passed to printf hook
 * @param fmt		printf format string
 * @param ...		arguments to format string
 * @return			number of characters written
 */
size_t printInHook(printfHookData_t *data, char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* _CHELP_PRINTFHOOK_H @} */
