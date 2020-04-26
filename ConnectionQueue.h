#ifndef CAOS_HTTP_WEB_SERVER_CONNECTIONQUEUE_H
#define CAOS_HTTP_WEB_SERVER_CONNECTIONQUEUE_H

#include <vector>
#include "ThreadPool.h"
#include <unistd.h>


class ConnectionsReader {
public:
    explicit ConnectionsReader(int fd) : fd_(fd), cur_context() {}

    std::vector<Connection> GetConnections() {
      std::vector<Connection> result;
      while (ReadSomeConnection(cur_context, fd_) == 0) {
        result.push_back(cur_context.connection);
        cur_context.refresh();
      }
      return result;
    }

    int GetFd() {
      return fd_;
    }

private:
    struct ConnectionReadContext {
        int ptr;
        Connection connection;
        ConnectionReadContext() : ptr(0), connection() {};
        void refresh() {
          ptr = 0;
        }
    };

    ConnectionReadContext cur_context;
    int fd_;

    int ReadSomeConnection(ConnectionReadContext &context, int fd) {
      // returns how many bytes left to read of the object
      int len = sizeof(Connection);
      int cnt = read(fd, (char*)(&context.connection) + context.ptr, len - context.ptr);
      if (cnt == -1) {
        return len - context.ptr;
      }
      context.ptr += cnt;
      return len - context.ptr;
    }
};

class ConnectionQueue {
public:
    ConnectionQueue(Epoll &thread_epoll, int channel_fd, Config &config) :
            thread_epoll_(thread_epoll), connection_reader(channel_fd), config_(config) {}

    int AddConnection(Connection &conn) {
      modify_nonblock(conn.sock); // socket will be treated with EPOLLET semantics
      auto *new_context = new HttpEpollContext(conn, thread_epoll_, config_);
      return thread_epoll_.AddFileDescriptor(conn.sock, EPOLLET | EPOLLIN | EPOLLOUT, new_context);
    }

    void AddNewConnections() {
      while (!conn_queue.empty()) {
        int status = AddConnection(conn_queue.front());
        if (status == -1) {
          break;
        }
        else {
          conn_queue.pop();
        }
      }

      if (conn_queue.empty()) {
        std::vector<Connection> new_conns = connection_reader.GetConnections();
        int i = 0;
        for (; i < new_conns.size(); ++i) {
          int status = AddConnection(new_conns[i]);
          if (status == -1) {
            break;
          }
        }
        while (i < new_conns.size()) {
          conn_queue.push(new_conns[i]);
        }
      }
    }

private:
    Epoll &thread_epoll_;
    ConnectionsReader connection_reader;
    std::queue<Connection> conn_queue; // connections that do not fit in EPOLL for some reason
    Config &config_;
};

#endif //CAOS_HTTP_WEB_SERVER_CONNECTIONQUEUE_H
