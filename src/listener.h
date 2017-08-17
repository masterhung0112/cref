#ifndef _CHELP_LISTENER_H
#define _CHELP_LISTENER_H 1

/**
 * Listener interface, listens to events if registered to the bus.
 */

#ifdef __cplusplus
extern "C" {
#endif

struct listener_t {
	/*****************************
	 * List of API that bus will call
	 *****************************/
	 
	/**
	 * Virtual IP address assignment hook.
	 *
	 * This hook gets invoked after virtual IPs have been assigned to a peer
	 * for a specific IKE_SA, and again before they get released.
	 *
	 * @param ikeSa		IKE_SA the VIPs are assigned to
	 * @param assign	TRUE if assigned to IKE_SA, FALSE if released
	 * @return			TRUE to stay registered, FALSE to unregister
	 */
	bool (*assignVips)(listener_t *this, ikeSa_t *ikeSa, bool assign);
	
	/**
	 * Virtual IP and configuration attribute handler hook.
	 *
	 * This hook gets invoked after virtual IP and other configuration
	 * attributes just got installed or are about to get uninstalled on a peer
	 * receiving them.
	 *
	 * @param ikeSa		IKE_SA the VIPs/attributes are handled on
	 * @param handle	TRUE if handled by IKE_SA, FALSE on release
	 * @return			TRUE to stay registered, FALSE to unregister
	 */
	bool (*handleVips)(listener_t *this, ikeSa_t *ikeSa, bool handle);
};

#ifdef __cplusplus
}
#endif

#endif /* _CHELP_LISTENER_H */
