#ifndef _CHELP_HOST_H
#define _CHELP_HOST_H 1

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represents a host
 *
 * Host object, identifies a address:port pair and defines some
 * useful functions on it
 */
typedef struct host_t host_t;

/**
 * Constructor to create host_t object from address string
 *
 * @param string		string of an address, "152.96.192.130"
 * @param port			port number
 * @return				host_t, NULL if string not an address
 */
host_t *hostCreateFromString(char *string, uint16_t port);

/**
 *
 * @param family		address family, or AF_UNSPEC
 */
host_t *hostCreateFromStringAndFamily(char *string, int family, uint16_t port);

/**
 * @param string		hostname to resolve
 * @param family		family to prefer, 0 for first match
 */
host_t *hostCreateFromDns(char *string, int family, uint16_t port);

/*
 * @param family		Address family, E.g.. AF_INET or AF_INET6
 * @param address		address as chunk_t in network order
 * @param port			port number
 */
host_t *hostCreateFromChunk(int family, chunk_t address, uint16_t port);

/**
 * @param sockaddr		sockaddr struct which contains family, address, and port
 */
host_t *hostCreateFromSockaddr(sockaddr_t *sockaddr);

/**
 * Create a range definition (1.2.3.0-1.2.3.5), return the two hosts.
 *
 * The two hosts are not orderrerd, from is simply the first, to is the second,
 * from is not necessarily smaller.
 *
 * @param string		string to parse
 * @param from			return the first address (out)
 * @param to			return the second address (out)
 * @return				TRUE if parsed successfully, FALSE otherwise
 */
bool hostCreateFromRange(char *string, host_t **from, host **to);

/**
 * Create a host from a CIDR subnet definition (1.2.3.0/24), return bits
 *
 * @param bits			gets the number of network bits in CIDR notation
 */
host_t *hostCreateFromSubnet(char *string, int *bit);

/**
 * Create a netmask host having the first netbits bits set
 * @param family		family of the netmask host
 * @param netbits		number of leading bits set in the host
 * @return				netmask host
 */
host_t *hostCreateNetmask(int family, int netbits);

/**
 * Create a host without an address, a "any" host
 */
host_t *hostCreateAny(int family);

/**
 * Build a clone of this host object
 */
host_t *hostClone(host_t *this);

/**
 * Get a pointer to the internal sockaddr struct
 *
 * This is used for sending and receiving via sockets
 *
 * @return		pointer to the internal sockaddr structure
 */
sockaddr_t *hostGetSockaddr(host_t *this);

/**
 * Get the length of the sockaddr struct.
 *
 * Depending on the family, the length of the sockaddr struct
 * is different. use this function to get the length of the sockaddr
 * struct returned by GetSockaddr
 */
socklen_t *hostGetSockaddrLen(host_t *this);

int hostGetFamily(host_t *this);
int hostIsAnyAddr(host_t *this);
chunk_t hostGetAddress(host_t *this);
uint16_t hostGetPort(host_t *this);
void hostSetPort(host_t *this, uint16_t port);
bool hostIpEquals(host_t *this, host_t *other);
bool hostEquals(host_t *this, host_t *other);
void hostDestroy(host_t *this);


#ifdef __cplusplus
}
#endif

#endif /* _CHELP_HOST_H */
