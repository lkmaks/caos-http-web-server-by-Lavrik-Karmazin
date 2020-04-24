//
// Created by max on 4/24/20.
//

#include "Epoll.h"
#include <sys/epoll.h>


Epoll::Epoll(int size) {
  epoll_fd = epoll_create(size);
}

std::vector<EpollEvent> Epoll::Wait(int max_events, int timeout) {
  epoll_event events[max_events];
  int cnt = epoll_wait(epoll_fd, events, max_events, timeout);
  std::vector<EpollEvent> result(max_events);
  for (int i = 0; i < max_events; ++i) {
    result[i] = EpollEvent(events[i].events, events[i].data);
  }
  return result;
}

int Epoll::AddFileDescriptor(int fd, uint32_t flags, EpollContext *epoll_context) {
  epoll_event event;
  event.data.ptr = (void*)(epoll_context);
  event.events = flags;
  return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
}

int Epoll::RemoveFileDescriptor(int fd) {
  return epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
}
