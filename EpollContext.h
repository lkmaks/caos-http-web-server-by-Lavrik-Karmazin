#ifndef CAOS_HTTP_WEB_SERVER_EPOLLCONTEXT_H
#define CAOS_HTTP_WEB_SERVER_EPOLLCONTEXT_H

#include <string>
#include <stdexcept>
#include "Config.h"
#include "ThreadPool.h"
#include "Epoll.h"
#include "HttpParser.h"

class Epoll;


enum EpollContextType {
    DEFAULT,
    FD,
    CONN,
    HTTP
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

private:
    int fd_;
};

class ConnectionEpollContext : public EpollContext {
public:
    ConnectionEpollContext() : conn_() {
      type_ = CONN;
    }

    ConnectionEpollContext(Connection conn) : conn_(conn) {
      type_ = CONN;
    }

    Connection GetConn() {
      return conn_;
    }

private:
    Connection conn_;
};

class HttpEpollContext : public EpollContext {
public:
    HttpEpollContext(Connection &conn, Epoll &epoll, Config &config);

    void HandleRead();
    void HandleWrite();

private:
    Connection conn_;
    Epoll &epoll_;
    Config &config_;

    std::string data_;
    int data_ptr_; // on first unprocessed character

    // utility methods
    void DestroyContext();
    int ReadSome();
    void Rearm();
    void TransitToError(int error_no);
    void TransitToWrite(const std::string &write_data);

    void InitReadFields();
    void InitWriteFields(const std::string &write_data);

    void ParseRequestHeaders();

    // current stage field
    bool is_read_stage_;

    // read fields
    bool first_line_ready_;
    int first_line_end_;
    HttpFirstLine first_line_;
    HttpHeaders request_headers_;

    // write fields
    int write_data_ptr_;
    std::string write_data_;
};


#endif