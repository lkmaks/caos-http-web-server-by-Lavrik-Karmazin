#include "Config.h"
#include "ThreadPool.h"
#include "Epoll.h"

class HTTPServer {
public:
    HTTPServer(const std::string &ipv4_addr, const std::string &conf_dir, const std::string &data_dir);

    void run();
private:
    bool load_config(const std::string &conf_dir, const std::string &data_dir);

    std::string ipv4_addr_;
    ThreadPool thread_pool_;
    Config config_;
    Epoll epoll_main_;
    std::vector<std::pair<int, int> > server_sockets_;
    std::vector<ServerSocketEpollContext> server_socket_epoll_contexts_; // needless for now
};