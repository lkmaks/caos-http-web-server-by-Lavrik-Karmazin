#ifndef CAOS_HTTP_WEB_SERVER_THREADPOOL_H
#define CAOS_HTTP_WEB_SERVER_THREADPOOL_H

#include <pthread.h>
#include <vector>


class HTTPServer;

struct ThreadData {
    pthread_t thread;

    // pipe between main thread and worker
    // channel[1] is main thread's socket, channel[0] is worker's,
    // since main thread send messages to workers
    int channel[2];
    HTTPServer* server;
};

struct Connection {
    int sock;
    // maybe also ip address of the client

    Connection() : sock(-1) {};

    explicit Connection(int sock) : sock(sock) {}
};

class ThreadPool {
public:
    ThreadPool() = default;
    explicit ThreadPool(int cnt_threads, HTTPServer* parent);

    void SendNewConn(int thread_id, const Connection &new_conn);

    void SendNewConnToAnyThread(const Connection &new_conn);
private:
    std::vector<ThreadData> threads_;
};

#endif