//
// Created by max on 4/26/20.
//

#include "ThreadMain.h"


#ifndef CAOS_HTTP_WEB_SERVER_THREADMAIN_HPP
#define CAOS_HTTP_WEB_SERVER_THREADMAIN_HPP


#include <queue>
#include "Epoll.h"
#include "ThreadPool.h"
#include "ConnectionQueue.h"
#include "Config.h"
#include "HttpServer.h"
#include <string.h>
#include <signal.h>


thread_local sig_atomic_t sigpipe_flag = 0;

void sigpipe_signal_handler(int signum) {
  sigpipe_flag = 1;
}

void sigpipe_protection() {
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = sigpipe_signal_handler;
  sigaction(SIGPIPE, &action, NULL);
}


class HttpServer;

Config &GetConf(ThreadData *thread_data) {
  return thread_data->server->GetConf();
}


void HandleEvent(EpollEvent &event) {
  if (event.events_mask & EPOLLIN) {
    ((HttpEpollContext*)event.epoll_context)->HandleRead();
  }
  if (event.events_mask & EPOLLOUT) {
    ((HttpEpollContext*)event.epoll_context)->HandleWrite();
  }
}

void *thread_main(void *ptr) {
  auto *thread_data = (ThreadData*)ptr;
  int channel_fd = thread_data->channel[0];
  modify_nonblock(channel_fd);

  // argument does not affect anything for linux >= 2.6.8
  Epoll thread_epoll(GetConf(thread_data).max_thread_conn);
  FdEpollContext channel_context(channel_fd);
  thread_epoll.AddFileDescriptor(channel_fd, EPOLLIN, &channel_context);

  // adds new connections to epoll, stores ones that do not fit in queue
  ConnectionQueue conn_queue(thread_epoll, channel_fd, GetConf(thread_data));

  sigpipe_protection();

  while (true) {
    std::vector<EpollEvent> events = thread_epoll.Wait(GetConf(thread_data).max_epoll_events_in_iteration);
    for (auto e : events) {
      if (e.epoll_context->GetType() == FD && ((FdEpollContext*)e.epoll_context)->GetFd() == channel_fd) {
        conn_queue.AddNewConnections();
      }
      else {
        // NOT IMPLEMENTED YET
        printf("%d\n", (int)(e.events_mask & EPOLLIN));
        printf("%d\n", (int)(e.events_mask & EPOLLOUT));
        printf("%d\n", (int)(e.events_mask & EPOLLRDHUP));
        printf("%d\n", (int)(e.events_mask & EPOLLHUP));

        HandleEvent(e);
      }
    }
  }
}

#endif