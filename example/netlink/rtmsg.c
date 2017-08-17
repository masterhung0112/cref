#include <sys/socket.h>
#include <linux/netlink.h> /* nlmsghdr, NETLINK_ROUTE, sockaddr_nl, AF_NETLINK */
#include <linux/rtnetlink.h> /* rtmsg */
#include <linux/fib_rules.h> /* FIB_RULE_INVERT */
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/if_addr.h> /* ifaddrmsg */
#define FALSE 0
#define TRUE 1

typedef union {
	struct nlmsghdr hdr;
	unsigned char bytes[1024];
} netlink_buf_t __attribute__((aligned(RTA_ALIGNTO)));

int socketfd;
char msgbuf[65535] = {0};
int msgbufIdx = 0;
int msgbufLen = 0;

static bool write_msg(int socketfd, struct nlmsghdr *msg)
{
	struct sockaddr_nl addr = {
		.nl_family = AF_NETLINK
	};
	int len;
	
	while (TRUE) {
		len = sendto(socketfd, msg, msg->nlmsg_len, 0, 
			(struct sockaddr *)&addr, sizeof(addr));
		if(len != msg->nlmsg_len) {
			if (errno == EINTR) {
				continue;
			}
			printf("netlink write error: %s\n", strerror(errno));
			return FALSE;
		}
		return TRUE;
	}
}

/* Read a single Netlink message from socketfd, return 0 on error, -1 on timeout */
static ssize_t read_msg(int socketfd, char *buf, size_t buf_len, bool block)
{
	ssize_t len;
	if (block) {
		fd_set set;
		struct timeval tv = {};
		FD_ZERO(&set);
		FD_SET(socketfd, &set);
		tv.tv_sec = 1000;
		//timeval_add_ms(&tv, 1000);
		if (select(socketfd + 1, &set, NULL, NULL, &tv) < 0) {
			printf("%s:%d read_msg TIMEOUT\n", __func__, __LINE__);
			return -1; /* timeout */
		}
	}
	len = recv(socketfd, buf, buf_len, MSG_TRUNC|(block ? 0 : MSG_DONTWAIT));
	if (len > buf_len) {
		printf("netlink response exeeds buffer size");
		return 0;
	}
	if (len < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
			printf("netlink read error: %s\n", strerror(errno));
		}
		return 0;
	}
	return len;
}

static bool queue(int socketfd, struct nlmsghdr *hdr)
{
	int ret = FALSE;
	memcpy(msgbuf + msgbufIdx, hdr, hdr->nlmsg_len);
	msgbufIdx += hdr->nlmsg_len;
	msgbufLen += hdr->nlmsg_len;
	if (hdr->nlmsg_type == NLMSG_DONE || !(hdr->nlmsg_flags & NLM_F_MULTI)) {
		//printf("done:%d multi:0x%X\n", 
		//	hdr->nlmsg_type == NLMSG_DONE,
		//	hdr->nlmsg_flags & NLM_F_MULTI);
		ret = TRUE;
	}
	return ret;
}

static bool read_and_queue(int socketfd, bool block)
{
	struct nlmsghdr *hdr;
	char buf[65535];
	ssize_t len;
	bool is_completed = FALSE;
	
	while (!is_completed) {
		len = read_msg(socketfd, buf, sizeof(buf), block);
		if (len == -1) {
			return TRUE;
		}
		if (len) {
			hdr = (struct nlmsghdr *)buf;
			//printf("len:%d hdr->len:%d\n", len, hdr->nlmsg_len);
			while (NLMSG_OK(hdr, len)) {
				if ((is_completed = queue(socketfd, hdr))) {
					break;
				}
				hdr = NLMSG_NEXT(hdr, len);
			}
		}
	}
	return is_completed;
}

int send_once(int socketfd, struct nlmsghdr *in, uintptr_t seq, struct nlmgshdr **out, size_t *out_len)
{
	struct nlmsghdr * hdr;
	in->nlmsg_seq = seq;
	in->nlmsg_pid = getpid();
	
	if (!write_msg(socketfd, in)) {
		printf("%s:%d write_msg FAIL\n", __func__, __LINE__);
		return -1;
	}
	bool block = TRUE;
	
	msgbufLen = 0;
	msgbufIdx = 0;
	while (!read_and_queue(socketfd, block)) {
	
	}
	//printf("msgbufLen:%d msgBufIdx:%d\n", msgbufLen, msgbufIdx);
	*out = msgbuf;
	*out_len = msgbufLen;
	return 0;
	
}

void netlink_add_attribute(struct nlmsghdr *hdr, int rta_type, char *data, size_t datalen, size_t buflen)
{
	struct rtattr *rta;
	
	if ((NLMSG_ALIGN(hdr->nlmsg_len) + RTA_LENGTH(datalen)) > buflen) {
		printf("Unable to add attribute, buffer too small\n");
		return;
	}
	
	rta = (struct rtattr*)(((char *)hdr) + NLMSG_ALIGN(hdr->nlmsg_len));
	rta->rta_type = rta_type;
	rta->rta_len = RTA_LENGTH(datalen);
	memcpy(RTA_DATA(rta), data, datalen);
	hdr->nlmsg_len = NLMSG_ALIGN(hdr->nlmsg_len) + rta->rta_len;
}

int netlink_send(int socketfd, struct nlmsghdr *in, struct nlmsghdr **out, size_t *out_len)
{
	static int _seq = 0;
	
	uintptr_t seq = ++_seq;
	uint32_t try;
	int status;
	
	for (try = 0; try <= 3; ++try) {
		struct nlmsghdr *hdr;
		int status;
		size_t len;
		if (try > 0) {
			printf("retransmitting netlink request (%u)\n", try);
		}
		
		status = send_once(socketfd, in, seq, &hdr, &len);
		if (status == 0) {
		} else {
			printf("%s:%d send_once FAIL\n", __func__, __LINE__);
			return status;
		}
		//printf("%s:%d hdr->nlmsg_type:%d\n", __func__, __LINE__, hdr->nlmsg_type);
		if (hdr->nlmsg_type == NLMSG_ERROR) {
			struct nlmsgerr *err = NLMSG_DATA(hdr);
			//printf("%s:%d err->error:%d\n", __func__, __LINE__, err->error);
			if (err->error) {
				if (err->error == -EBUSY) {
					free(hdr);
					--try;
					continue;
				}
				/*TODO: ignore errors for message types that might have completed previously */
			}
		}
		*out = hdr;
		*out_len = len;
		return 0;
	}
	/* Netlink request timed out after several transmit */
	return -1;
}

/* 0: Success; -1: Fail */
int netlink_send_ack(int socketfd, struct nlmsghdr *in)
{
	struct nlmsghdr *out, *hdr;
	size_t len;
	
	if (netlink_send(socketfd, in, &out, &len) != 0) {
		return -1;
	}
	hdr = out;
	while (NLMSG_OK(hdr, len)) {
		switch(hdr->nlmsg_type) {
			case NLMSG_ERROR:
			{
				struct nlmsgerr *err = NLMSG_DATA(hdr);
				if (err->error) {
					if (-err->error == EEXIST) {
						//free(out);
						printf("%s:%d ALREADY_DONE\n", __func__, __LINE__);
						return -1; /*ALREADY_DONE */
					}
					if (-err->error == ESRCH) {
						//free(out);
						/* do not report missing entries */
						printf("%s:%d NOTFOUND\n", __func__, __LINE__);
						return -1;/* NOT_FOUND */
					}
					printf("%s:%d UNKNOWN(%d)\n", __func__, __LINE__, err->error);
					//free(out);
					return -1;
				}
				//free(out);
				return 0;
			}
			default:
				hdr = NLMSG_NEXT(hdr, len);
				continue;
			case NLMSG_DONE:
				break;
		}
		break;
	}
	printf("%s:%d netlink request not acknowledged\n", __func__, __LINE__);
	//free(out);
	return -1;
}

/* Create or delete a rule to use our routing table */
static int manage_rule(int nlmsg_type, int family, uint32_t table, uint32_t prio)
{
	netlink_buf_t request = {0};
	struct nlmsghdr *hdr;
	struct rtmsg *msg;
	
	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	hdr->nlmsg_type = nlmsg_type;
	
	if (nlmsg_type == RTM_NEWRULE) {
		hdr->nlmsg_flags |= NLM_F_CREATE | NLM_F_EXCL;
	}
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	
	msg = NLMSG_DATA(hdr);
	msg->rtm_table = table;
	msg->rtm_family = family;
	msg->rtm_protocol = RTPROT_BOOT;
	msg->rtm_scope = RT_SCOPE_UNIVERSE;
	msg->rtm_type = RTN_UNICAST;
	
	netlink_add_attribute(hdr, RTA_PRIORITY, (char *)&prio, sizeof(prio), sizeof(request));
	
	
	return netlink_send_ack(socketfd, hdr);
}

static int getIfIndex(char *ifname)
{
	struct ifreq req = {0};
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	
	strcpy(req.ifr_name, ifname);
	
	if (ioctl(sock, SIOCGIFINDEX, &req) != 0) {
		close(sock);
		printf("%s:%d Fail to get if index: %s\n", __func__, __LINE__, ifname);
		return 0;
	}
	close(sock);
	return req.ifr_ifindex;
}

/*
 * Manage source routes in the routing table
 * By setting the appropriate nlmsg_type, the route gets added or removed.
 */
static int manage_srcroute(int routingtable, int nlmsg_type, int flags, char *dstnet, unsigned char prefixlen, char *gateway, char *srcip, char *ifname, uint32_t mtu, uint32_t mss)
{
	netlink_buf_t  request;
	struct nlmsghdr *hdr;
	struct rtmsg *msg;
	struct rtattr *rta;
	int ifindex;
	char *chunk;
	
	/* If route is 0.0.0.0/0, we cannot install it, as it would
	 * overwrite the default route. Instead we add two routes:
	 * 0.0.0.0/1 and 128.0.0.0/1
	 */
	if(routingtable == 0 && prefixlen == 0) {
		/*TODO: Add impl */
		return -1;
	}
	
	memset(&request, 0, sizeof(request));
	
	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
	hdr->nlmsg_type = nlmsg_type;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	
	msg = NLMSG_DATA(hdr);
	msg->rtm_family = AF_INET;
	msg->rtm_dst_len = prefixlen;
	msg->rtm_table = routingtable;
	msg->rtm_protocol = RTPROT_STATIC;
	msg->rtm_type = RTN_UNICAST;
	msg->rtm_scope = RT_SCOPE_UNIVERSE;
	__be32 ip;
	inet_pton(AF_INET, dstnet, &ip);
	netlink_add_attribute(hdr, RTA_DST, &ip, sizeof(ip), sizeof(request));
	inet_pton(AF_INET, srcip, &ip);
	netlink_add_attribute(hdr, RTA_PREFSRC, &ip, sizeof(ip), sizeof(request));
	if (gateway) {
		inet_pton(AF_INET, gateway, &ip);
		netlink_add_attribute(hdr, RTA_GATEWAY, &ip, sizeof(ip), sizeof(request));
	}
	#if 0
	/*TODO: how to get the ifindex from if_name */
	ifindex = getIfIndex(ifname);
	netlink_add_attribute(hdr, RTA_OIF, &ifindex, sizeof(ifindex), sizeof(request));
	#endif
	#if 1
	char metricbuf[128] = {0};
	ssize_t metriclen = 0;
	rta = (struct rtattr *)metricbuf;
	rta->rta_type = RTAX_MTU;
	rta->rta_len = RTA_LENGTH(sizeof(unsigned int));
	memcpy(RTA_DATA(rta), &mtu, sizeof(uint32_t));
	metriclen += rta->rta_len;
	rta = (struct rtattr *)(metricbuf + RTA_ALIGN(metriclen));
	rta->rta_type = RTAX_ADVMSS;
	rta->rta_len = RTA_LENGTH(sizeof(uint32_t));
	memcpy(RTA_DATA(rta), &mss, sizeof(uint32_t));
	metriclen += rta->rta_len;
	netlink_add_attribute(hdr, RTA_METRICS, metricbuf, metriclen, sizeof(request));
	#endif
	return netlink_send_ack(socketfd, hdr);
}

/**
 * Manages the creation and deletion of ip addresses on an interface.
 * By setting the appropriate nlmsg_type, the ip will be set or unset.
 */
static int manage_ipaddr(int nlmsg_type, int flags, char *ifname, char *ip, int prefix)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr;
	struct ifaddrmsg *msg;
	
	memset(&request, 0, sizeof(request));
	
	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
	hdr->nlmsg_type = nlmsg_type;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	
	msg = NLMSG_DATA(hdr);
	msg->ifa_family = AF_INET;
	msg->ifa_prefixlen = prefix;
	msg->ifa_flags = 0;
	msg->ifa_scope = RT_SCOPE_UNIVERSE;
	msg->ifa_index = getIfIndex(ifname);
	
	__be32 uip;
	inet_pton(AF_INET, ip, &uip);
	netlink_add_attribute(hdr, IFA_LOCAL, &uip, sizeof(uip), sizeof(request));
	
	return netlink_send_ack(socketfd, hdr);
}

/*
 * get_source_addr: Get our outgoing source address for a destination
 * Does a route lookup to get the source address used to reach dest.
 *
 * dest: target address
 * src: source address, or NULL
 */
char *get_source_addr(char *host, char *src)
{
	
}

/* get_next_hop: Get the next hop for a destination
 * Does a route lookup to get the next hop used to reach dest.
 *
 * dest: target address
 * prefix: prefix length if dest if subnet, -1 for auto
 * src: source address, or NULL
 * return: next hope address, NULL if unreachable
 */

/*
 * get_interface: Get the interface name of a local address. 
 * Interfaces that are down or ignored by config are not considered.
 * 
 * host: address to get interface name from
 * name: allocated interface name (optional)
 * return: TRUE if interface found and usable
 */

/*
 * Creates an enumerator over all local addresses
 *
 * which: a combination of address type to enumerate
 * return enumerator over address
 */

/*
 * add_ip: Add a virtual IP to an interface
 *
 * Virtual IPs are attached to an interface. If an IP is added multiple
 * times, the IP is refcounted and not removed until del_ip() was called as many times as add_ip()
 *
 * virtual_ip virtual IP address to assign
 * prefix		prefix length to install with IP address, -1 for auto
 * ifacename interface name to install virtual IP on
 * return TRUE: if operation completed
 */
int add_ip(char *virtual_ip, int prefix, char *ifacename)
{
	if (manage_ipaddr(RTM_NEWADDR, NLM_F_CREATE | NLM_F_REPLACE,
		ifacename, virtual_ip, prefix) == 0) {
		
	} else {
		printf("%s:%d Fail to add IP: %s %s\n", 
			__func__, __LINE__, virtual_ip, ifacename);
	}
}

/*
 * del_ip: remove a IP from an interface
 * The kernel interface uses refcounting, see add_ip
 *
 * virtual_ip
 * prefix
 * wait: TRUE to wait until IP is gone
 * return: TRUE: if operation completed
 */

/*
 * add_route: add a route
 *
 * dst_net destination net
 * prefixlen destination net prefix length
 * gateway gateway for this route
 * src_ip source ip of the route
 * if_name name of the interface the route is bound to
 * return TRUE: operation completed
 */
 
 /*
  * del_route: del a route
  *
  * dst_net: destination net
  * prefixlen
  * gateway
  * src_ip
  * if_name
  * return
  */

/*
 * process RTM_NEWADDR/RTM_DELADDR from kernel
 */
static void process_addr(struct nlmsghdr *hdr, bool event)
{
	struct ifaddrmsg *msg = NLMSG_DATA(hdr);
	struct rtattr *rta = IFA_RTA(msg);
	size_t rtasize = IFA_PAYLOAD(hdr);
	uint32_t localip, addressip;
	char localipStr[32], addressipStr[32];
	char *localptr = NULL, *addressptr = NULL;
	ssize_t locallen, addresslen;
	
	while (RTA_OK(rta, rtasize)) {
		switch(rta->rta_type) {
			case IFA_LOCAL:
				localptr = RTA_DATA(rta);
				locallen = RTA_PAYLOAD(rta);
				memcpy(&localip, localptr, locallen);
				break;
			case IFA_ADDRESS:
				addressptr = RTA_DATA(rta);
				addresslen = RTA_PAYLOAD(rta);
				
				memcpy(&addressip, addressptr, addresslen);
				break;
		}
		rta = RTA_NEXT(rta, rtasize);
	}
	
	/*
	 * For PPP interfaces, we need the IFA_LOCAL address,
	 * IFA_ADDRESS is the peers address. But IFA_LOCAL is
	 * not included in all cases (IPv6?), so fallback to IFA_ADDRESS
	 */
	if (localptr) {
		inet_ntop(AF_INET, &localip, localipStr, 32);
		printf("%s:%d %d Local address: %s\n", __func__, __LINE__, msg->ifa_index, localipStr);
	} else if (addressptr) {
		inet_ntop(AF_INET, &addressip, addressipStr, 32);
		printf("%s:%d %d Destination address: %s\n", __func__, __LINE__, msg->ifa_index, addressipStr);
	}
	
	if (!localptr && !addressptr) {
		printf("%s:%d No local and destination address\n", __func__, __LINE__);
	}
}

/*
 * Receive events from kernel
 */
static bool receive_events()
{
	char response[1536];
	struct nlmsghdr *hdr = (struct nlmsghdr *)response;
	struct sockaddr_nl addr= {
		.nl_family = AF_NETLINK,
	};
	socklen_t addr_len = sizeof(addr);
	int len;
	int socket_events = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (socket_events < 0) {
		printf("%s:%d Fail to create socket for netlink\n", __func__, __LINE__);
		return FALSE;
	}
	
	addr.nl_groups = RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR |
				RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE | RTMGRP_LINK;
	if (bind(socket_events, (struct sockaddr *)&addr, sizeof(addr))) {
		printf("unable to find netlink socket_events\n");
		return -1;
	}
	//printf("Start receiving\n");
	memset(&addr, 0, sizeof(addr));
	len = recvfrom(socket_events, response, sizeof(response), 0, (struct sockaddr *)&addr, &addr_len);
	//printf("Receive: %d\n", len);
	if (len < 0) {
		switch (errno) {
		case EINTR: /* interrupted, try again */
			return TRUE;
		case EAGAIN: /* no data ready, select again */
			return TRUE;
		default:
			printf("Unable to receive from rt event socket\n");
			sleep(1);
			return TRUE;
		}
	}
	
	if (addr.nl_pid != 0) {
		printf("not from kernel. not interested, try another one\n");
		return TRUE;
	}
	
	while (NLMSG_OK(hdr, len)) {
		/* look good so far, dispatch netlink message */
		switch(hdr->nlmsg_type) {
		case RTM_NEWADDR:
			printf("%s:%d type: NEWADDR\n", __func__, __LINE__);
			process_addr(hdr, TRUE);
			break;
		case RTM_DELADDR:
			printf("%s:%d type: RTM_DELADDR\n", __func__, __LINE__);
			process_addr(hdr, TRUE);
			break;
		case RTM_NEWLINK:
			printf("%s:%d type: RTM_NEWLINK\n", __func__, __LINE__);
			break;
		case RTM_DELLINK:
			printf("%s:%d type: RTM_DELLINK\n", __func__, __LINE__);
			break;
		case RTM_NEWROUTE:
			printf("%s:%d type: RTM_NEWROUTE\n", __func__, __LINE__);
			break;
		case RTM_DELROUTE:
			printf("%s:%d type: RTM_DELROUTE\n", __func__, __LINE__);
			break;
		default:
			printf("%s:%d Unknown type: %d\n", __func__, __LINE__, hdr->nlmsg_type);
			break;
		}
		hdr = NLMSG_NEXT(hdr, len);
	}
	return TRUE;
}

static void* thread_do(struct thread* thread_p)
{
	while (TRUE)
		receive_events();
}

static void process_link(struct nlmsghdr *hdr, bool event)
{
	struct ifinfomsg *msg = NLMSG_DATA(hdr);
	struct rtattr *rta = IFLA_RTA(msg);
	size_t rtasize = IFLA_PAYLOAD(hdr);
	char *name = NULL;
	
	while (RTA_OK(rta, rtasize)) {
		switch(rta->rta_type) {
			case IFLA_IFNAME:
				name = RTA_DATA(rta);
				break;
		}
		rta = RTA_NEXT(rta,rtasize);
	}
	if (!name) {
		name = "(unknown)";
	}
	
	switch(hdr->nlmsg_type) {
		case RTM_NEWLINK:
		{
			printf("    RTM_NEWLINK %d %s %s\n", msg->ifi_index, name,
				(msg->ifi_flags & IFF_UP) ? "up" : "down");
			break;
		}
		case RTM_DELLINK:
		{
			printf("    RTM_DELLINK %d %s %s\n", msg->ifi_index, name);
			break;
		}
	}	
}

static int init_address_list(void)
{
	netlink_buf_t request;
	struct nlmsghdr *out, *current, *in;
	struct rtgenmsg *msg;
	size_t len;
	
	memset(&request, 0, sizeof(request));
	
	in = &request.hdr;
	in->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg));
	in->nlmsg_flags = NLM_F_REQUEST | NLM_F_MATCH | NLM_F_ROOT;
	msg = NLMSG_DATA(in);
	msg->rtgen_family = AF_UNSPEC;
	
	/* get all links */
	in->nlmsg_type = RTM_GETLINK;
	if (netlink_send(socketfd, in, &out, &len) != 0) {
		printf("%s:%d Fail to send RTM_GETLINK\n", __func__, __LINE__);
		return -1;
	}
	current = out;
	
	printf("All interfaces:\n");
	while (NLMSG_OK(current, len)) {
		switch(current->nlmsg_type) {
			case NLMSG_DONE:
				break;
			case RTM_NEWLINK:
				process_link(current, FALSE);
			default:
				current = NLMSG_NEXT(current, len);
				continue;
		}
		break;
	}
	
	/* get all interface addresses */
	in->nlmsg_type = RTM_GETADDR;
	if (netlink_send(socketfd, in, &out, &len) != 0) {
		printf("%s:%d Fail to send RTM_GETADDR\n", __func__, __LINE__);
		return -1;
	}
	current = out;
	
	printf("All addresses:\n");
	while (NLMSG_OK(current, len)) {
		switch(current->nlmsg_type) {
			case NLMSG_DONE:
				break;
			case RTM_NEWADDR:
				process_addr(current, FALSE);
			default:
				current = NLMSG_NEXT(current, len);
				continue;
		}
		break;
	}
}

/**
 * Store information about a route retrieved via RTNETLINK
 */
typedef struct rt_entry_t {
	char gtw[32];
	char src[32];
	char dst[32];
	char src_host[32];
	uint8_t dst_len;
	uint32_t table;
	uint32_t oif;
	uint32_t priority;
} rt_entry_t;

/**
 * Parse route received with RTM_NEWROUTE. The given rt_entry_t object will be
 * reused if not NULL.
 *
 * Returned chunks point to internal data of the Netlink message.
 */
rt_entry_t *parse_route(struct nlmsghdr *hdr, rt_entry_t *route)
{
	struct rtattr *rta;
	struct rtmsg *msg;
	size_t rtasize;
	uint32_t ipN;
	msg = NLMSG_DATA(hdr);
	rta = RTM_RTA(msg);
	rtasize = RTM_PAYLOAD(rta);
	
	if (route) {
		memset(route->gtw, 0, 32);
		memset(route->src, 0, 32);
		memset(route->dst, 0, 32);
		memset(route->src_host, 0, 32);
		route->dst_len = msg->rtm_dst_len;
		route->table = msg->rtm_table;
		route->oif = 0;
		route->priority = 0;
	} else {
		route = malloc(sizeof(*route));
	}
	
	while (RTA_OK(rta, rtasize)) {
		switch(rta->rta_type) {
			case RTA_PREFSRC:
				ipN = *(uint32_t *)RTA_DATA(rta);
				inet_pton(AF_INET, &ipN, route->src);
				break;
			case RTA_GATEWAY:
				ipN = *(uint32_t *)RTA_DATA(rta);
				inet_pton(AF_INET, &ipN, route->gtw);
				break;
			case RTA_DST:
				ipN = *(uint32_t *)RTA_DATA(rta);
				inet_pton(AF_INET, &ipN, route->dst);
				break;
			case RTA_OIF:
				if (RTA_PAYLOAD(rta) == sizeof(route->oif)) {
					route->oif = *(uint32_t *)RTA_DATA(rta);
				}
				break;
			case RTA_PRIORITY:
				if (RTA_PAYLOAD(rta) == sizeof(route->oif)) {
					route->priority = *(uint32_t *)RTA_DATA(rta);
				}
				break;
			case RTA_TABLE:
				if (RTA_PAYLOAD(rta) == sizeof(route->oif)) {
					route->table = *(uint32_t *)RTA_DATA(rta);
				}
				break;
		}
		rta = RTA_NEXT(rta, rtasize);
	}
	return route;
}

static char *get_route(char *dest, int prefix, bool nexthop, char *candicate, uint32_t recusion)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr, *out, *current;
	struct rtmsg *msg;
	bool match_net;
	uint32_t ip;
	rt_entry_t *route = NULL, *best = NULL;
	
#define MAX_ROUTE_RECUSION 2
	if (recusion > MAX_ROUTE_RECUSION) {
		return NULL;
	}
	
	memset(&request, 0, sizeof(request));
	
	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST;
	hdl->nlmsg_type = RTM_GETROUTE;
	hdl->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	
	msg = RTA_DATA(hdr);
	msg->rtm_family = AF_INET;
	
	len = 4 * 8;
	prefix = prefix < 0 ? len : min(prefix, len);
	match_net = prefix != len;
	
	hdr->nlmsg_flags |= NLM_F_DUMP;
	if (candidate) {
		inet_pton(AF_INET, candidate, &ip);
		netlink_add_attribute(hdr, RTA_PREFSRC, &ip, sizeof(ip));
	}
	
	if(!match_net) {
		inet_pton(AF_INET, dest, &ip);
		netlink_add_attribute(hdr, RTA_DST, &ip, sizeof(ip));
	}
	
	if (netlink_send(socketfd, hdr, &out, &len) != 0) {
		printf("%s:%d getting %s to reach %s%d failed\n",
			__func__, __LINE__,
			nexthop ? "nexthop" : "address", dest, prefix);
		return -1;
	}
	for (current = out; NLMSG_OK(hdr, len); current = NLMSG_NEXT(current, len)) {
		switch(hdr->nlmsg_type) {
			case NLMSG_DONE:
				break;
			case RTM_NEWROUTE:
			{
				route = parse_route(current, route);
				/* route->priority < other->priority ||
				   route->priority == other->priority &&
				   route->dst_len > other->dst_len */
				   
				/* got a source address with the route, if no preferred source
				 * is given or it matches we are done, as this is the best route */
				if (route->src[0]) {
					if (!candidate || strcmp(candidate, route->src) == 0) {
						best = route;
						printf("Found best route by candidate: %s\n", candidate);
					}
				} else if(route->oif) {
					/* no match yet, may be it is assigned to the same interface */
					//TODO: Add impl
				}
				route = NULL;
				break;
			}
		}	
	}
	
	/* now we have a list of routes matching dest, sorted by net prefix.
	 * we will look for source addresses for these routes and select the one
	 * with the preferred source address, if possible */
	
}

int main()
{
	struct rtmsg msg;
	msg.rtm_family = AF_INET;
	msg.rtm_table = RT_TABLE_MAIN;
	msg.rtm_protocol = RTPROT_UNSPEC;
	msg.rtm_scope = RT_SCOPE_UNIVERSE;
	msg.rtm_type = RTN_UNICAST;
	
	struct sockaddr_nl addr = {
		.nl_family = AF_NETLINK,
	};
	
	pthread_t pthread;
	pthread_create(&pthread, NULL, (void *)thread_do, NULL);
	
	/* create and bind RT socket for events (address/interface/route changes) */
	socketfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	
	if (bind(socketfd, (struct sockaddr *)&addr, sizeof(addr))) {
		printf("unable to find netlink socketfd\n");
		return -1;
	}
	
	/* Display all interfaces */
	init_address_list();
	
	
	#if 0
	if (manage_rule(RTM_NEWRULE, AF_INET, 28, 0) != 0) {
		printf("Fail to create IPv4 routing table rule\n");
	}
	#endif
	
	
	if (manage_srcroute(RT_TABLE_MAIN, RTM_NEWROUTE, NLM_F_CREATE | NLM_F_REPLACE, "192.168.1.2", 32, "192.168.0.1", "192.168.1.3", "enp8s0", 1340, 0) != 0) {
		printf("Fail to add new route\n");
	}
	sleep(1);
	if (manage_srcroute(RT_TABLE_MAIN, RTM_DELROUTE, NLM_F_CREATE | NLM_F_REPLACE, "192.168.1.2", 32, "192.168.0.1", "192.168.1.3", "enp8s0", 1340, 0) != 0) {
		printf("Fail to add new route\n");
	}
	sleep(1);
	if (add_ip("5.0.2.4", 32, "enp8s0") != 0) {
		printf("Fail to add ip 5.0.2.1 to enp8s0\n");
	}
	
	sleep(1);
	
	#if 0
	if (manage_rule(RTM_DELRULE, AF_INET, 28, 0) != 0) {
		printf("Fail to del IPv4 routing table rule\n");
	}
	#endif
	close(socketfd);
	return 0;
}
