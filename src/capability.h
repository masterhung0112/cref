#ifndef _CHELP_CAPABILITY_H
#define _CHELP_CAPABILITY_H 1

/**
 * POSIX capability dropping abstraction layer.
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_SYS_CAPABILITY_H
#include <sys/capability.h>
#elif defined(CAPABILITIES_NATIVE)
#include <linux/capability.h>
#endif

typedef struct capabilities_t capabilities_t;

#ifndef CAP_CHOWN
# define CAP_CHOWN 0
#endif
#ifndef CAP_NET_BIND_SERVICE
# define CAP_NET_BIND_SERVICE 10
#endif
#ifndef CAP_NET_ADMIN
# define CAP_NET_ADMIN 12
#endif
#ifndef CAP_NET_RAW
# define CAP_NET_RAW 13
#endif
#ifndef CAP_DAC_OVERRIDE
# define CAP_DAC_OVERRIDE 1
#endif

/**
 * Register a capability to keep while calling drop().
 * Verifies that the capability is currently held.
 *
 * @note CAP_CHOWN is handled specially as it mmight not be required.
 *
 * @param cap		capability to keep
 * @return			FALSE if the capability is currently not held
 */
bool capabilitiesKeep(capabilities_t *this, uint32_t cap);

/**
 * Drop all capabilities not previously passed to keep(), switch to UID/GID.
 *
 * @return 			TRUE if capability drop successful
 */
bool capabilitiesDrop(capabilities_t *this);

/**
 * Check if the given capability is currently held
 *
 * @note CAP_CHOWN is handled specially as it might not be required.
 *
 * @param cap		capability to check
 * @param			TRUE if the capability is currently held
 */
bool capabilitiesCheck(capabilities_t *this, uint32_t cap);

/**
 * Get the user ID set through setUid/resolveUid
 */
uid_t capabilitiesGetUid(capabilities_t *this);

/**
 * Get the group ID set through setGid/resolveGid
 */
gid_t capabilitiesGetGid(capabilities_t *this);

/**
 * Set the user ID to use during rights dropping
 */
void capabilitiesSetUid(capabilities_t *this, uid_t uid);

/**
 * Set the group ID to use during rights dropping
 */
void capabilitiesSetGid(capabilities_t *this, gid_t gid);

/**
 * Resolve a username and set the user ID accordingly.
 *
 * @param username 	username get the uid for
 * @return			TRUE if username resolved and uid set
 */
bool capabilitiesResolveUid(capabilities_t *this, char *username);

/**
 * Resolve a groupname and set the group ID accordingly.
 *
 * @param groupname	groupname get the gid for
 * @return			TRUE if groupname resolved and gid set
 */
bool capabilitiesResolveGid(capabilities_t *this, char *groupname);

/**
 * Create a capabilities instance.
 */
capabilities_t *capabilitiesCreate();
void capabilitiesDestroy(capabilities_t *this);



#ifdef __cplusplus
}
#endif

#endif /* _CHELP_CAPABILITY_H */
