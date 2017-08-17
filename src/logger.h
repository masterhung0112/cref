#ifndef _CHELP_LOGGER_H
#define _CHELP_LOGGER_H 1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct logger_t logger_t;

/**
 * Logger interface, listens for log events on the bus.
 *
 * Calls to bus_t.log() are handled separately from calls to other functions.
 * Logger functions may be called concurrently by multiple threads. Also
 * recursive calls are not prevented, loggers that may cause recursive log
 * messages are responsible to avoid infinite loops.
 *
 * Both the log() and the vlog() methods are optional to implement. With many
 * loggers, using log() may be faster as printf() format substitution is done
 * only once for all loggers.
 */
struct logger_t {
	
	/**
	 * Log a debugging message.
	 *
	 * @param group		kind of the signal (up, down, rekeyed, ...)
	 * @param level		verbosity level of the signal
	 * @param thread	ID of the thread raised this signal
	 * @param ike_sa	IKE_SA associated to the event
	 * @param message	log message
	 */
	void (*log)(logger_t *this, debug_t group, level_t level, int thread, 
		ikeSa_t *ike_sa, const char *message);
	
	/**
	 * Log a debugging message with a format string.
	 *
	 * @note Calls to bus_t.log() are handled separately from calls to
	 * other functions.  This callback may be called concurrently by
	 * multiple threads.  Also recursive calls are not prevented, loggers that
	 * may cause recursive log messages are responsible to avoid infinite loops.
	 *
	 * @param group		kind of the signal (up, down, rekeyed, ...)
	 * @param level		verbosity level of the signal
	 * @param thread	ID of the thread raised this signal
	 * @param ike_sa	IKE_SA associated to the event
	 * @param fmt		log message format string
	 * @param args		variable arguments to format string
	 */
	void (*vlog)(logger_t *this, debug_t group, level_t level, int thread,
				 ike_sa_t *ike_sa, const char *fmt, va_list args);
	
	/**
	 * Get the desired log level for a debug group.  This is called during
	 * registration.
	 *
	 * If the desired log levels have changed, re-register the logger with
	 * the bus.
	 *
	 * @param group		debug group
	 * @return			max level to log (0..4) or -1 for none (see debug.h)
	 */
	level_t (*getLevel)(logger_t *this, debug_t group);	
};

#ifdef __cplusplus
}
#endif

#endif /* _CHELP_LOGGER_H */
