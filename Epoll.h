#ifndef CAOS_HTTP_WEB_SERVER_EPOLL_H
#define CAOS_HTTP_WEB_SERVER_EPOLL_H


#include <vector>
#include "EpollContext.h"
#include <inttypes.h>
#include <sys/epoll.h>

struct EpollEvent {
    uint32_t events_mask;
    EpollContext *epoll_context;

    EpollEvent() = default;

    EpollEvent(uint32_t events, epoll_data_t epoll_data) : events_mask(events) {
      epoll_context = (EpollContext*)(epoll_data.ptr);
    }
};

class Epoll {
public:
    Epoll() = default;

    explicit Epoll(int size);

    std::vector<EpollEvent> Wait(int max_events, int timeout = -1);

    int AddFileDescriptor(int fd, uint32_t flags, EpollContext *epoll_context);

    int RemoveFileDescriptor(int fd);

    // void ChangeEpollContext(int fd, EpollContext &epoll_context);

private:
    int epoll_fd;
};

#endif