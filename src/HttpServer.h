#ifndef CAOS_HTTP_WEB_SERVER_HTTPSERVER_H
#define CAOS_HTTP_WEB_SERVER_HTTPSERVER_H


#include "Config.h"
#include "Utils/ThreadPool.h"
#include "Utils/Epoll.h"

class ThreadPool;

class HttpServer {
public:
    HttpServer(const std::string &conf_dir, const std::string &data_dir);

    Config &GetConf();

    void run();
private:
    bool load_config(const std::string &conf_dir, const std::string &data_dir);

    std::string ipv4_addr_;
    Config config_;

    ThreadPool thread_pool_;
    Epoll epoll_main_;
    std::vector<std::pair<int, uint16_t> > server_sockets_; // {sock, port}
    std::vector<ConnectionEpollContext*> server_socket_epoll_contexts_;
};

#endif