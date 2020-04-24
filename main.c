#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <inttypes.h>
#include <signal.h>
#include <pthread.h>

#include "Utils.c"
#include "config.c"


typedef struct thread_data {
	pthread_t thread;
	int pipe_into_thread[2];
	//int pipe_from_thread[2];
} thread_data;

thread_data thread_pool[THREADS_NUM];


extern void *thread_main(void *arg);

void create_thread_pool() {
	pthread_attr_t thread_attributes;
	pthread_attr_init(&thread_attributes);
	for (int i = 0; i < THREADS_NUM; ++i) {
		pipe(&thread_pool[i].pipe_into_thread, O_NONBLOCK);
		modify_nonblock(thread_pool[i].pipe_into_thread[0]); // make thread input end of pipe nonblocking to use in EPOLL
		pthread_create((pthread_t*)(thread_pool + i), &thread_attributes, thread_main, thread_pool + i);
	}
	pthread_attr_destroy(&thread_attributes);
}

void send_socket_to_one_of_threads(int socket, struct sockaddr_in client_address) {
	int thread_id = rand() % THREADS_NUM;
	write(thread_pool[thread_id].pipe_into_thread[1], &socket, sizeof(socket));
}


void parse_args(int argc, char **argv, struct in_addr *ipv4_server_addr) {
	inet_aton(argv[1], ipv4_server_addr);
}


int main(int argc, char **argv) {
	int status;

	// argv: ip address, conf dir, data dir
	init_conf(argv[2], argv[3]);
	create_thread_pool(thread_pool);
	srand(time(NULL));

	struct in_addr ipv4_server_addr;
	parse_args(argc, argv, ipv4_server_addr);

	// parsed ip address of the server

	int server_sockets[CONF_DATA.vhosts_cnt];
	for (int i = 0; i < CONF_DATA.vhosts_cnt; ++i) {
		server_sockets[i] = socket(AF_INET, SOCK_STREAM, 0); // tcp socket with ipv4 addressing
		struct sockaddr_in sock_address;
		sock_address.sin_family = AF_INET;
		sock_address.sin_addr = ipv4_server_addr;
		sock_address.sin_port = htons((uint16_t)CONF_DATA.ports[i]);
		status = bind(server_socket, (struct sockaddr*)(&sock_address), sizeof(sock_address));
		if (status != 0) {
			FAIL("Binding server on one of the ports");
		}
		status = listen(server_socket, MAX_CONN_QUEUE);
		if (status != 0) {
			FAIL("Listening on socket");
		}
	}

	// enabled listening in socket

	while (1) {
		struct sockaddr client_address;
		socklen_t cient_address_len;
		int new_socket = accept(server_sockets[0], &client_address, &cient_address_len);
		send_socket_to_one_of_threads(new_socket, (sockaddr_in)client_address, thread_pool);
	}
}