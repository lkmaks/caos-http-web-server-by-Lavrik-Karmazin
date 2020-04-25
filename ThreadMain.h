#ifndef CAOS_HTTP_WEB_SERVER_THREADMAIN_H
#define CAOS_HTTP_WEB_SERVER_THREADMAIN_H


#include <queue>
#include "Epoll.h"
#include "ThreadPool.h"
#include "ConnectionQueue.h"
#include "Config.h"
#include "HTTPServer.h"


class HTTPServer;

Config &GetConf(ThreadData *thread_data) {
  return thread_data->server->GetConf();
}


void *thread_main(void *ptr) {
  ThreadData *thread_data = (ThreadData*)ptr;
  int channel_fd = thread_data->channel[0];
  modify_nonblock(channel_fd);

  // argument does not affect anything for linux >= 2.6.8
  Epoll thread_epoll(GetConf(thread_data).max_thread_conn);
  FdEpollContext channel_context(channel_fd);
  thread_epoll.AddFileDescriptor(channel_fd, EPOLLIN, &channel_context);

  ConnectionQueue conn_queue(thread_epoll, channel_fd); // adds new connections to epoll, stores ones that do not fit in queue

  while (true) {
    std::vector<EpollEvent> events = thread_epoll.Wait(GetConf(thread_data).max_epoll_events_in_iteration);
    for (auto e : events) {
      if (e.epoll_context->context_type == "FdEpollContext" && ((FdEpollContext*)e.epoll_context)->fd == channel_fd) {
        conn_queue.AddNewConnections();
      }
      else {
        // NOT IMPLEMENTED YET
        printf("%d\n", (int)(e.events_mask & EPOLLIN));
        printf("%d\n", (int)(e.events_mask & EPOLLOUT));
      }
    }
  }
}

#endif