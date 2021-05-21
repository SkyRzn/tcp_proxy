#ifndef TCP_PROXY_NET
#define TCP_PROXY_NET


#include <arpa/inet.h>


extern int create_server_socket(struct sockaddr_in *addr);
extern int create_client_socket(void);
extern int domain_to_ip(struct sockaddr_in *addr, const char *domain);


#endif
