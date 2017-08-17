#ifndef _HOSTRESOLVER_H
#define _HOSTRESOLVER_H 1

/**
 * Resolve hosts by DNS name but do so in the separate thread
 * (calling getaddrinfo(3) directly might block indefinitely,
 * or at least a very long time if no DNS servers are reachable)
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hostResolver_t hostResolver_t;

/**
 * Create host resolver
 */
hostResolver_t *hostResolverCreate();

/**
 * Resolve host from the given DNS name
 *
 * @param name		name to lookup
 * @param family	requested address family
 * @return			resolved host or NULL if failed or canceled
 */
host_t *hostResolverResolve(hostResolver_t *this, char *name, int family);

/**
 * Flush the queue of queries. No new queries will be accepted afterwards
 */
void hostResolverFlush(hostResolver_t *this);

/**
 * Destroy host resolver
 */
void hostResolverDestroy(hostResolver_t *this);

#ifdef __cplusplus
}
#endif

#endif /* _HOSTRESOLVER_H */
