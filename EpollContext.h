#ifndef CAOS_HTTP_WEB_SERVER_EPOLLCONTEXT_H
#define CAOS_HTTP_WEB_SERVER_EPOLLCONTEXT_H

#include <string>

struct EpollContext {
    std::string context_type;
    EpollContext() : context_type("EpollContext") {}
};

struct FdEpollContext : EpollContext {
    int fd;

    FdEpollContext() : fd(-1) {
      context_type = "FdEpollContext";
    }

    FdEpollContext(int fd) : fd(fd) {
      context_type = "FdEpollContext";
    }
};

struct InitialHttpEpollContext : EpollContext {
    InitialHttpEpollContext() {
      context_type = "InitialHttpEpollContext";
    }
};

#endif