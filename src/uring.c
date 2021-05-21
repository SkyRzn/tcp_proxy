#include "uring.h"
#include <stdlib.h>


#define RING_ENTRIES_COUNT 64


static struct io_uring ring;


int uring_init(void)
{
	return io_uring_queue_init(RING_ENTRIES_COUNT, &ring, 0);
}

void uring_free(void)
{
	io_uring_queue_exit(&ring);
}

uring_data_t *uring_alloc_data(uring_data_t *sibling)
{
	uring_data_t *data;

	data = malloc(sizeof(uring_data_t));
	if (!data)
		return NULL;

	data->type = URING_DATA_TYPE_UNKNOWN;
	data->sibling = NULL;

	// there is a sibling data structure, so we are creating
	// a backward data with a reversed src/dst pair
	if (sibling) {
		data->src_sock = sibling->dst_sock;
		data->dst_sock = sibling->src_sock;
		data->sibling = sibling;
		sibling->sibling = data;
	} else
		data->sibling = NULL;

	data->size = URING_DATA_BUFFER_SIZE;

	return data;
}

struct io_uring_cqe *uring_wait_cqe(void)
{
	struct io_uring_cqe *cqe;

	if (io_uring_wait_cqe(&ring, &cqe) != 0)
		return NULL;

	return cqe;
}

void uring_cqe_seen(struct io_uring_cqe *cqe)
{
	io_uring_cqe_seen(&ring, cqe);
}

int uring_request_accept(uring_data_t *data, int sock, struct sockaddr_in *addr, socklen_t *addrlen)
{
	struct io_uring_sqe *sqe;

	sqe = io_uring_get_sqe(&ring);
	if (sqe == NULL)
		return -ENOMEM;

	data->type = URING_DATA_TYPE_ACCEPT;

	io_uring_prep_accept(sqe, sock, (struct sockaddr *)addr, addrlen, 0);
	io_uring_sqe_set_data(sqe, data);
	io_uring_submit(&ring);

	return 0;
}

int uring_request_connect(uring_data_t *data, struct sockaddr_in *addr)
{
	struct io_uring_sqe *sqe;

	sqe = io_uring_get_sqe(&ring);
	if (sqe == NULL)
		return -ENOMEM;

	data->type = URING_DATA_TYPE_CONNECT;

	io_uring_prep_connect(sqe, data->dst_sock, (struct sockaddr *)addr, sizeof(*addr));
	io_uring_sqe_set_data(sqe, data);
	io_uring_submit(&ring);

	return 0;
}

int uring_request_recv(uring_data_t *data)
{
	struct io_uring_sqe *sqe;

	sqe = io_uring_get_sqe(&ring);
	if (sqe == NULL)
		return -ENOMEM;

	data->type = URING_DATA_TYPE_RECV;

	io_uring_prep_recv(sqe, data->src_sock, data->buffer, URING_DATA_BUFFER_SIZE, 0);
	io_uring_sqe_set_data(sqe, data);
	io_uring_submit(&ring);

	return 0;
}

int uring_request_send(uring_data_t *data)
{
    struct io_uring_sqe *sqe;

	sqe = io_uring_get_sqe(&ring);
	if (sqe == NULL)
		return -ENOMEM;

    data->type = URING_DATA_TYPE_SEND;

    io_uring_prep_send(sqe, data->dst_sock, data->buffer, data->size, 0);
    io_uring_sqe_set_data(sqe, data);
    io_uring_submit(&ring);

    return 0;
}

int uring_request_cancel_sibling(uring_data_t *data)
{
    struct io_uring_sqe *sqe;

	sqe = io_uring_get_sqe(&ring);
	if (sqe == NULL)
		return -ENOMEM;

    data->type = URING_DATA_TYPE_CANCEL;

    io_uring_prep_cancel(sqe, data->sibling, 0);
    io_uring_sqe_set_data(sqe, data);
    io_uring_submit(&ring);

    return 0;
}

int uring_request_close(uring_data_t *data)
{
    struct io_uring_sqe *sqe;

	sqe = io_uring_get_sqe(&ring);
	if (sqe == NULL)
		return -ENOMEM;

    data->type = URING_DATA_TYPE_CLOSE;

    io_uring_prep_close(sqe, data->src_sock);
    io_uring_sqe_set_data(sqe, data);
    io_uring_submit(&ring);

    return 0;
}
