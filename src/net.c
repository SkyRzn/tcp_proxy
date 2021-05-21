#include "net.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>


#define MAX_CONNECTIONS 64


int create_server_socket(struct sockaddr_in *addr)
{
	int sock, enable;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		return -errno;

	enable = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		return -errno;

	if (bind(sock, (const struct sockaddr *)addr, sizeof(struct sockaddr_in)) == -1)
		return -errno;

	if (listen(sock, MAX_CONNECTIONS) < 0)
		return -errno;

	return sock;
}

int create_client_socket(void)
{
	return socket(AF_INET, SOCK_STREAM, 0);
}

int domain_to_ip(struct sockaddr_in *addr, const char *domain)
{
	struct addrinfo hints, *addr_list;
	struct sockaddr_in *addr_p;
	char port_str[8];
	int ret;

	snprintf(port_str, sizeof(port_str), "%hu", ntohs(addr->sin_port));
	memset(&hints, 0, sizeof hints);

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	ret = getaddrinfo(domain, port_str, &hints, &addr_list);
	if (ret != 0)
		return ret;

	if (addr_list == NULL)
		ret = -EINVAL;

	// take the first address
	addr_p = (struct sockaddr_in *)addr_list->ai_addr;
	memcpy(addr, addr_p, sizeof(*addr));

	char buf[256];
	inet_ntop(AF_INET, addr_list->ai_addr, buf, sizeof(buf));

	freeaddrinfo(addr_list);

	return ret;
}

