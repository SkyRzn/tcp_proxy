#ifndef TCP_PROXY_URING
#define TCP_PROXY_URING


#include <arpa/inet.h>
#include <liburing.h>


#define URING_DATA_BUFFER_SIZE 65536 //TODO maybe 256k is more reasonable

#define URING_CQE_DATA(cqe) ((uring_data_t *)(cqe)->user_data);


typedef enum {
	URING_DATA_TYPE_UNKNOWN,
	URING_DATA_TYPE_ACCEPT,
	URING_DATA_TYPE_CONNECT,
	URING_DATA_TYPE_RECV,
	URING_DATA_TYPE_SEND,
	URING_DATA_TYPE_CANCEL,
	URING_DATA_TYPE_CLOSE
} uring_data_type_t;

typedef struct uring_data {
	struct uring_data *sibling;
	uring_data_type_t type;
	int src_sock;
	int dst_sock;
	size_t size;
	char buffer[URING_DATA_BUFFER_SIZE];
} uring_data_t;


extern int uring_init(void);
extern void uring_free(void);
extern uring_data_t *uring_alloc_data(uring_data_t *sibling);
extern struct io_uring_cqe *uring_wait_cqe(void);
extern void uring_cqe_seen(struct io_uring_cqe *cqe);
extern int uring_request_accept(uring_data_t *data, int sock, struct sockaddr_in *addr, socklen_t *addrlen);
extern int uring_request_connect(uring_data_t *data, struct sockaddr_in *addr);
extern int uring_request_recv(uring_data_t *data);
extern int uring_request_send(uring_data_t *data);
extern int uring_request_cancel_sibling(uring_data_t *data);
extern int uring_request_close(uring_data_t *data);


#endif
