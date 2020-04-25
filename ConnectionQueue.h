#ifndef CAOS_HTTP_WEB_SERVER_CONNECTIONQUEUE_H
#define CAOS_HTTP_WEB_SERVER_CONNECTIONQUEUE_H

#include <vector>
#include "ThreadPool.h"
#include <unistd.h>


class ConnectionsReader {
public:
    ConnectionsReader(int fd) : fd(fd), cur_context() {}

    std::vector<Connection> GetConnections() {
      std::vector<Connection> result;
      while (ReadSomeConnection(cur_context, fd) == 0) {
        result.push_back(cur_context.connection);
        cur_context.refresh();
      }
      return result;
    }

    int GetFd() {
      return fd;
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
    int fd;

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
    ConnectionQueue(Epoll &thread_epoll, int channel_fd) :
            thread_epoll_(thread_epoll), connection_reader(channel_fd) {}

    void AddNewConnections() {
      while (!conn_queue.empty()) {
        auto conn = conn_queue.front();
        InitialHttpEpollContext *new_context = new InitialHttpEpollContext();
        int status = thread_epoll_.AddFileDescriptor(conn.sock, EPOLLET | EPOLLIN | EPOLLOUT, new_context);
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
          InitialHttpEpollContext *new_context = new InitialHttpEpollContext();
          int status = thread_epoll_.AddFileDescriptor(new_conns[i].sock, EPOLLET | EPOLLIN | EPOLLOUT, new_context);
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
};

#endif //CAOS_HTTP_WEB_SERVER_CONNECTIONQUEUE_H
