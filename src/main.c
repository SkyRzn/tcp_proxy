#include "uring.h"
#include "net.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>


#define err_exit(fmt, ...) \
	do { \
		fprintf(stderr, (fmt), ##__VA_ARGS__); \
		exit(-1); \
	} while (0)

#define LOCAL_MASK  1
#define REMOTE_MASK 2


typedef struct {
	struct sockaddr_in local;
	struct sockaddr_in remote;
} options_t;


static options_t options;


static void main_loop(void);
static int init_signals(void);
static void signal_handler(int signal);
static int parse_args(int argc, char *argv[]);
static int parse_addr(struct sockaddr_in *sa, char *addr_str);
static void close_connection(uring_data_t *data);
static void print_usage(const char *name);


int main(int argc, char *argv[])
{

	if (parse_args(argc, argv) != 0) {
		fprintf(stderr, "Incorrect args\n");
		print_usage(argv[0]);
		exit(-1);
	}

	if (uring_init() != 0)
		err_exit("Uring init error\n");

	if (init_signals() != 0)
		err_exit("Signal error (%s)\n", strerror(errno));

	main_loop();

	return 0;
}

static void main_loop(void)
{
	struct io_uring_cqe *cqe;
	uring_data_t *data, *data2;
	int srv_sock;

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	srv_sock = create_server_socket(&options.local);
	if (srv_sock < 0)
		err_exit("Server socket creation error\n");

	data = uring_alloc_data(NULL);
	if (!data)
		err_exit("Data allocation error\n");

	uring_request_accept(data, srv_sock, &addr, &addrlen);

	while (1) {
        cqe = uring_wait_cqe();
		if (cqe == NULL)
			err_exit("CQE getting error\n");

		data = URING_CQE_DATA(cqe);
		if (data == NULL)
			err_exit("CQE data error\n");

		if (cqe->res < 0 && cqe->res != -ECANCELED)
            fprintf(stderr, "CQE error: %s \n", strerror(-cqe->res));

        switch (data->type) {
            case URING_DATA_TYPE_ACCEPT:
				data2 = uring_alloc_data(NULL);
				if (!data2)
					err_exit("Data allocation error\n");

				uring_request_accept(data2, srv_sock, &addr, &addrlen);

				if (cqe->res < 0) {
					free(data);
					break;
				}

				data->src_sock = cqe->res;
				data->dst_sock = create_client_socket();
				if (data->dst_sock < 0) {
					fprintf(stderr, "Client socket creation error\n");
					uring_request_close(data);
					break;
				}

				uring_request_connect(data, &(options.remote));

				break;

			case URING_DATA_TYPE_CONNECT:
				if (cqe->res < 0) {
					free(data);
					break;
				}

				uring_request_recv(data);

				data2 = uring_alloc_data(data);
				if (!data2)
					err_exit("Data allocation error\n");

				uring_request_recv(data2);

				break;

			case URING_DATA_TYPE_RECV:
				if (cqe->res <= 0) {
					close_connection(data);
					break;
				}

				data->size = cqe->res;

				uring_request_send(data);

				break;

			case URING_DATA_TYPE_SEND:
				if (cqe->res <= 0) {
					close_connection(data);
					break;
				}

				uring_request_recv(data);

				break;

			case URING_DATA_TYPE_CANCEL:
				uring_request_close(data);
				break;

			case URING_DATA_TYPE_CLOSE:
				free(data);
				break;

			default:
				err_exit("Unknown request type: %d\n", data->type);
		}

		uring_cqe_seen(cqe);
	}
}

static int init_signals(void)
{
	struct sigaction act;

	memset(&act, 0, sizeof(act));
    act.sa_handler = signal_handler;

	return sigaction(SIGINT, &act, NULL);
}

static void signal_handler(int signal)
{
    uring_free();
	printf("\nExit.\n");
    exit(0);
}

static int parse_args(int argc, char *argv[])
{
	int opt, presented = 0;

	while ((opt = getopt(argc, argv, "l:d:")) != -1)
		switch (opt) {
			case 'l':
				if (presented & LOCAL_MASK)
					return -EINVAL;

				if (parse_addr(&options.local, optarg) != 0)
					err_exit("Incorrect proxy address\n");

				presented |= LOCAL_MASK;

				break;
			case 'd':
				if (presented & REMOTE_MASK)
					return -EINVAL;

				if (parse_addr(&options.remote, optarg) != 0)
					err_exit("Incorrect remote address\n");

				presented |= REMOTE_MASK;

				break;
			default:
				return -EINVAL;
	}

	return (presented == (LOCAL_MASK | REMOTE_MASK)) ? 0 : -EINVAL;
}

static int parse_addr(struct sockaddr_in *sa, char *addr_str)
{
	char *port_str;

	port_str = strchr(addr_str, ':');
	if (port_str == NULL)
		return -EINVAL;

	*port_str = '\0';
	port_str++;

	sa->sin_family = AF_INET;

	sa->sin_port = htons(atoi(port_str));
	if (sa->sin_port == 0)
		return -EINVAL;

	if (inet_pton(AF_INET, addr_str, &(sa->sin_addr)) != 1) {
		if (domain_to_ip(sa, addr_str) != 0)
			return -EINVAL;
	}

	return 0;
}

static void close_connection(uring_data_t *data)
{
	if (data->sibling) {
		data->sibling->sibling = NULL;
		uring_request_cancel_sibling(data);
	} else
		uring_request_close(data);
}

static void print_usage(const char *name)
{
	printf("Usage: %s -l proxy_address:proxy_port -d remote_address:remote_port\n", name);
}
