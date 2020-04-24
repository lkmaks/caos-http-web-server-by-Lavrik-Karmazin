#include "Utils.h"
#include <vector>
#include <string>

struct Config {
    std::vector<std::pair<std::string, uint16_t> > vhosts;
    int max_thread_conn = 1000;
    int max_conn_queue = 1000;
    const int threads_num = 4;
    int max_epoll_events_in_iteration = 100;
    std::string conf_dir;
    std::string data_dir;
};
