/* Net module for JustCoderdev Core library v1
 * */

#include <core.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>


/* 255.255.255.255 */
struct in_addr addr_to_bytes(n8 A, n8 B, n8 C, n8 D)
{
	struct in_addr address = {0};

	n32 ip = A;
	ip <<= 8;
	ip |= B;
	ip <<= 8;
	ip |= C;
	ip <<= 8;
	ip |= D;

	address.s_addr = htonl(ip);
	return address;
}

error hostname_resolve(const char *hostname, struct in_addr *address)
{
	struct hostent *host_info = {0};

	host_info = gethostbyname(hostname);
	if(host_info == NULL)
	{
		printf("ERROR:%s:%d Could not resolve hostname %s\n%s",
		       __FILE__,
		       __LINE__,
		       hostname,
		       strerror(h_errno));
		return success;
	}

#if DEBUG_ENABLE
	printf("DEBUG: Resolved hostname, %s => %d.%d.%d.%d\n",
	       hostname,
	       (n8)(*host_info->h_addr_list)[0],
	       (n8)(*host_info->h_addr_list)[1],
	       (n8)(*host_info->h_addr_list)[2],
	       (n8)(*host_info->h_addr_list)[3]);
#endif

	assert(host_info->h_length == 4);
	*address = addr_to_bytes((n8)(*host_info->h_addr_list)[0],
	                         (n8)(*host_info->h_addr_list)[1],
	                         (n8)(*host_info->h_addr_list)[2],
	                         (n8)(*host_info->h_addr_list)[3]);

	return success;
}

