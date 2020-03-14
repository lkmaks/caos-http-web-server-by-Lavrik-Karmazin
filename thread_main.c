#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <inttypes.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>


struct epoll_fd_storage {
	int is_input_pipe;
	union {
		struct input_pipe {
			int sock;
			int bytes_read;
		}
	}
}


void *thread_main(void *thread_data) {
	int epoll_fd = epoll_create(MAX_THREAD_CONN);

	// created EPOLL instance

	epoll_fd_storage input_pipe_storage;
	input_pipe_storage.is_input_pipe = 1;
	input_pipe_storage.input_pipe.sock = 0;
	input_pipe_storage.input_pipe.bytes_read = 0;

	// initialized storage for pipe: haven't started to read socket

	struct epoll_event event;
	event.data.ptr = &input_pipe_storage;
	event.events = EPOLLIN | EPOLLET;

	// created event structure to associate with pipe input file descriptor

	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, thread_data->pipe_into_thread[0], event);

	// associated event structure with pipe input file descriptor

	epoll_event incoming_events[MAX_EPOLL_EVENTS_IN_ITERATION];
	while (1) {
		int events_cnt = epoll_wait(epoll_fd, incoming_events, MAX_EPOLL_EVENTS_IN_ITERATION, -1);
		// acquired some EPOLL events after indefenite blocking, waiting for them to come
		for (int i = 0; i < events_cnt; ++i) {
			epoll_event cur_event = incoming_events[i];
			if (cur_event.data.ptr == input_pipe_storage) {
				// this is an event from input pipe
				// we have to read new socket fd's!
				while (1) {
					int pipe_fd = thread_data->pipe_into_thread[0];
					void *cur_ptr = (void*)(&event.data.ptr) + 4 + event.data.ptr->input_pipe.bytes_read;
					int cnt_to_read = 4 - event.data.ptr->input_pipe.bytes_read;
					int cnt_read = read(pipe_fd, cur_ptr, cnt_to_read);

					if (cnt_to_read == -1 && errno == EAGAIN) {
						// buffer is empty now, so we will return to read later with our state saved in storage
						break;
					}

					event.data.ptr->input_pipe.bytes_read += cnt_read;
					if (event.data.ptr->input_pipe.bytes_read == 4) {
						// finally acquired socketfd that main thread sent us, so register it in our EPOLL
						add_socket_to_epoll(epoll_fd, event.data.ptr->input_pipe.sock);
						event.data.ptr->input_pipe.bytes_read = 0;
					}
				}
			}
			else {
				// cur_event now describes event on socket (if it is read/write opportunity and gives us pointer to epoll_fd_storage associated with socket)
				// so, we should process that events
			}
		}
	}
}
