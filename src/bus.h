#ifndef _CHELP_BUS_H
#define _CHELP_BUS_H 1

#include "listener.h"/* listener_t */
/* debug_t, level_t */

/**
 * The bus receives events and sends them to all registered listeners.
 *
 * Loggers are handled separately.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bus_t bus_t;

/**
 * Create the event bus which forwards events to its listeners.
 *
 * @return		event bus instance
 */
bus_t *busCreate();

/**
 * Register a listener to the bus.
 *
 * A registered listener receives all events which are sent to the bus.
 * The listener is passive; the thread which emitted the event
 * processes the listener routine.
 *
 * @param listener	listener to register.
 */
void busAddListener(bus_t *this, listener_t *listener);

/**
 * Unregister a listener from the bus.
 *
 * @param listener	listener to unregister.
 */
void busRemoveListener(bus_t *this, listener_t *listener);

/**
 * Register a logger with the bus.
 *
 * The logger is passive; the thread which emitted the event
 * processes the logger routine.  This routine may be called concurrently
 * by multiple threads.  Recursive calls are not prevented, so logger that
 * may cause recursive calls are responsible to avoid infinite loops.
 *
 * During registration getLevel() is called for all log groups and the
 * logger is registered to receive log messages for groups for which
 * the requested log level is > LEVEL_SILENT and whose level is lower
 * or equal than the requested level.
 *
 * To update the registered log levels call addLogger again with the
 * same logger and return the new levels from getLevel().
 *
 * @param logger	logger to register.
 */
void busAddLogger(bus_t *this, logger_t *logger);

/**
 * Unregister a logger from the bus.
 *
 * @param logger	logger to unregister.
 */
void busRemoveLogger(bus_t *this, logger_t *logger);

/**
 * Set the IKESa the calling thread is using.
 *
 * To associate a received log message with an IKESa without passing it as
 * parameter each time, the thread registers the currently used IKESa
 * during check-out. Before check-in, the thread unregisters the IKESa.
 * This IKESa is stored per-thread, so each thread has its own IKESa
 * registered.
 *
 * @param ikeSa	ikeSa to register, or NULL to unregister
 */
void busSetSa(bus_t *this, ikeSa_t *ikeSa);

/**
 * Get the IKESa the calling thread is currently using.
 *
 * If a thread currently does not know what IKESa it is processing,
 * it can call getSa() to look up the SA set during checkout via setSa().
 *
 * @return			registered ikeSa, NULL if none registered
 */
ikeSa_t* busGetSa(bus_t *this);

/**
 * Send a log message to the bus.
 *
 * The format string specifies an additional informational or error
 * message with a printf() like variable argument list.
 * Use the DBG() macros.
 *
 * @param group		debugging group
 * @param level		verbosity level of the signal
 * @param format	printf() style format string
 * @param ...		printf() style argument list
 */
void busLog(bus_t *this, debug_t group, level_t level, char* format, ...);

/**
 * Send a log message to the bus using va_list arguments.
 *
 * Same as bus_t.log(), but uses va_list argument list.
 *
 * @param group		kind of the signal (up, down, rekeyed, ...)
 * @param level		verbosity level of the signal
 * @param format	printf() style format string
 * @param args		va_list arguments
 */
void busVlog(bus_t *this, debug_t group, level_t level,
			 char* format, va_list args);

/**
 * Raise an alert over the bus.
 *
 * @param alert		kind of alert
 * @param ...		alert specific arguments
 */
void busAlert(bus_t *this, alert_t alert, ...);

/**
 * Send a IKESa state change event to the bus.
 *
 * @param ikeSa	IKESa which changes its state
 * @param state		new state IKESa changes to
 */
void busIkeStateChange(bus_t *this, ikeSa_t *ikeSa,
						 ikeSa_state_t state);
/**
 * Send a CHILDSa state change event to the bus.
 *
 * @param childSa	CHILDSa which changes its state
 * @param state		new state CHILDSa changes to
 */
void busChildStateChange(bus_t *this, childSa_t *childSa,
						   childSa_state_t state);
/**
 * Message send/receive hook.
 *
 * The hook is invoked twice for each message: Once with plain, parsed data
 * and once encoded and encrypted.
 *
 * @param message	message to send/receive
 * @param incoming	TRUE for incoming messages, FALSE for outgoing
 * @param plain		TRUE if message is parsed and decrypted, FALSE it not
 */
void busMessage(bus_t *this, message_t *message, bool incoming, bool plain);

/**
 * IKESa authorization hook.
 *
 * @param final		TRUE if this is the final invocation
 * @return			TRUE to establish IKESa, FALSE to send AUTH_FAILED
 */
bool busAuthorize(bus_t *this, bool final);

/**
 * CHILDSa traffic selector narrowing hook.
 *
 * @param childSa	CHILDSa set up with these traffic selectors
 * @param type		type of hook getting invoked
 * @param local		list of local traffic selectors to narrow
 * @param remote	list of remote traffic selectors to narrow
 */
void busNarrow(bus_t *this, childSa_t *childSa, narrow_hook_t type,
			   linked_list_t *local, linked_list_t *remote);

/**
 * IKESa keymat hook.
 *
 * @param ikeSa	IKESa this keymat belongs to
 * @param dh		diffie hellman shared secret
 * @param dh_other	others DH public value (IKEv1 only)
 * @param nonce_i	initiators nonce
 * @param nonce_r	responders nonce
 * @param rekey		IKESa we are rekeying, if any (IKEv2 only)
 * @param shared	shared key used for key derivation (IKEv1-PSK only)
 */
void busIkeKeys(bus_t *this, ikeSa_t *ikeSa, diffie_hellman_t *dh,
				 chunk_t dh_other, chunk_t nonce_i, chunk_t nonce_r,
				 ikeSa_t *rekey, shared_key_t *shared);

/**
 * CHILDSa keymat hook.
 *
 * @param childSa	CHILDSa this keymat is used for
 * @param initiator	initiator of the CREATE_CHILDSa exchange
 * @param dh		diffie hellman shared secret
 * @param nonce_i	initiators nonce
 * @param nonce_r	responders nonce
 */
void busChildKeys(bus_t *this, childSa_t *childSa, bool initiator,
				   diffie_hellman_t *dh, chunk_t nonce_i, chunk_t nonce_r);

/**
 * IKESa up/down hook.
 *
 * @param ikeSa	IKESa coming up/going down
 * @param up		TRUE for an up event, FALSE for a down event
 */
void busIkeUpdown(bus_t *this, ikeSa_t *ikeSa, bool up);

/**
 * IKESa rekeying hook.
 *
 * @param old		rekeyed and obsolete IKESa
 * @param new		new IKESa replacing old
 */
void busIkeRekey(bus_t *this, ikeSa_t *old, ikeSa_t *new);

/**
 * IKESa peer endpoint update hook.
 *
 * @param ikeSa	updated IKESa, having old endpoints set
 * @param local		TRUE if local endpoint gets updated, FALSE for remote
 * @param new		new endpoint address and port
 */
void busIkeUpdate(bus_t *this, ikeSa_t *ikeSa, bool local, host_t *new);

/**
 * IKESa reestablishing hook (before resolving hosts).
 *
 * @param old		reestablished and obsolete IKESa
 * @param new		new IKESa replacing old
 */
void busIkeReestablish_pre(bus_t *this, ikeSa_t *old, ikeSa_t *new);

/**
 * IKESa reestablishing hook (after configuring and initiating the new
 * IKESa).
 *
 * @param old		reestablished and obsolete IKESa
 * @param new		new IKESa replacing old
 * @param initiated	TRUE if initiated successfully, FALSE otherwise
 */
void busIkeReestablishPost(bus_t *this, ikeSa_t *old, ikeSa_t *new,
							 bool initiated);

/**
 * CHILDSa up/down hook.
 *
 * @param childSa	CHILDSa coming up/going down
 * @param up		TRUE for an up event, FALSE for a down event
 */
void busChildUpdown(bus_t *this, childSa_t *childSa, bool up);

/**
 * CHILDSa rekeying hook.
 *
 * @param old		rekeyed and obsolete CHILDSa
 * @param new		new CHILDSa replacing old
 */
void busChildRekey(bus_t *this, childSa_t *old, childSa_t *new);

/**
 * CHILDSa migration hook.
 *
 * @param new		ID of new SA when called for the old, NULL otherwise
 * @param uniue		unique ID of new SA when called for the old, 0 otherwise
 */
void busChildrenMigrate(bus_t *this, ikeSa_id_t *new, uint32_t unique);

/**
 * Virtual IP assignment hook.
 *
 * @param ikeSa	IKESa the VIPs are assigned to
 * @param assign	TRUE if assigned to IKESa, FALSE if released
 */
void busAssignVips(bus_t *this, ikeSa_t *ikeSa, bool assign);

/**
 * Virtual IP handler hook.
 *
 * @param ikeSa	IKESa the VIPs/attributes got handled on
 * @param assign	TRUE after installing attributes, FALSE on release
 */
void busHandleVips(bus_t *this, ikeSa_t *ikeSa, bool handle);

/**
 * Destroy the event bus.
 */
void busDestroy(bus_t *this);


#ifdef __cplusplus
}
#endif


#endif /* _CHELP_BUS_H */
