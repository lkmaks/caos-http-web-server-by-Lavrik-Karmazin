#ifndef CAOS_HTTP_WEB_SERVER_EPOLLCONTEXT_H
#define CAOS_HTTP_WEB_SERVER_EPOLLCONTEXT_H

#include <string>
#include <stdexcept>
#include "Config.h"

enum EpollContextType {
    DEFAULT,
    FD,
    PROXY,
    INITIAL_HTTP,
    GET_HTTP
};

class EpollContext {
public:
    EpollContext() : type_(DEFAULT) {}

    EpollContextType GetType() {
      return type_;
    }

protected:
    EpollContextType type_;
};

class FdEpollContext : public EpollContext {
public:
    FdEpollContext() : fd_(-1) {
      type_ = FD;
    }

    explicit FdEpollContext(int fd) : fd_(fd) {
      type_ = FD;
    }

    int GetFd() {
      return fd_;
    }

    void SetFd(int fd) {
      fd_ = fd;
    }

protected:
    int fd_;
};


class HttpEpollContext : public FdEpollContext {
public:
    // never used
    explicit HttpEpollContext(int fd, Config &config) : config_(config), FdEpollContext(fd) {}

    virtual HttpEpollContext *HandleReadEvent() = 0;
    virtual HttpEpollContext *HandleWriteEvent() = 0;

protected:
    Config &config_;
};


class InitialHttpEpollContext : public HttpEpollContext {
public:
    explicit InitialHttpEpollContext(int fd, Config &config) : HttpEpollContext(fd, config) {
      type_ = INITIAL_HTTP;
    }

    HttpEpollContext *HandleReadEvent();
    HttpEpollContext *HandleWriteEvent();
private:
    std::string data_;
};

class GetHttpEpollContext : public HttpEpollContext {
public:
    explicit GetHttpEpollContext(int fd, Config &config, std::string &uri, std::string &data, int ptr) :
                        uri_(std::move(uri)), data_(std::move(data)), data_ptr_(ptr), HttpEpollContext(fd, config) {
      type_ = GET_HTTP;
    }

    HttpEpollContext *HandleReadEvent();
    HttpEpollContext *HandleWriteEvent();
private:
    std::string uri_;
    std::string data_;
    int data_ptr_;
};


struct ProxyEpollContext : public EpollContext {
public:
    ProxyEpollContext() {
      type_ = PROXY;
    }

    explicit ProxyEpollContext(HttpEpollContext *context) : real_(context) {
      type_ = PROXY;
    }

    void HandleReadEvent() {
      real_ = real_->HandleReadEvent();
      if (real_ == nullptr) {
        // an error occurred (?)
        throw std::logic_error("Handling read event unsuccessful.");
      }
    };

    void HandleWriteEvent() {
      real_ = real_->HandleWriteEvent();
      if (real_ == nullptr) {
        // an error occurred (?)
        throw std::logic_error("Handling write event unsuccessful.");
      }
    }
private:
    HttpEpollContext *real_;
};

#endif