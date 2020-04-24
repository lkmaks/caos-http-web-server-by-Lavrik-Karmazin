#include <pthread.h>
#include <vector>
#include "thread_main.cpp"

struct ThreadData {
    pthread_t thread;

    // socketpair between main thread and worker
    // channel[0] is main thread's socket, channel[1] is worker's
    int channel[2];
    SeSS
};

struct Connection {
    int sock;
    // maybe also ip address of the client

    Connection(int sock) : sock(sock) {}
};

class ThreadPool {
public:
    ThreadPool() = default;
    explicit ThreadPool(int cnt_threads);

    void SendNewConn(int thread_id, const Connection &new_conn);

    void SendNewConnToAnyThread(const Connection &new_conn);
private:
    std::vector<ThreadData> threads_;
};
