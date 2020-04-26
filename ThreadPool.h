#ifndef CAOS_HTTP_WEB_SERVER_THREADPOOL_H
#define CAOS_HTTP_WEB_SERVER_THREADPOOL_H

#include <pthread.h>
#include <vector>
#include <inttypes.h>

class HttpServer;

struct ThreadData {
    pthread_t thread;

    // pipe between main thread and worker
    // channel[1] is main thread's socket, channel[0] is worker's,
    // since main thread send messages to workers
    int channel[2];
    HttpServer* server;
};

struct Connection {
    int sock;
    uint16_t port;
    // maybe also ip address of the client

    Connection() : sock(-1), port(0) {};

    explicit Connection(int sock) : sock(sock), port(0) {}

    Connection(int sock, uint16_t port) : sock(sock), port(port) {}
};

class ThreadPool {
public:
    ThreadPool() = default;
    explicit ThreadPool(int cnt_threads, HttpServer* parent);

    void SendNewConn(int thread_id, const Connection &new_conn);

    void SendNewConnToAnyThread(const Connection &new_conn);
private:
    std::vector<ThreadData> threads_;
};

#endif