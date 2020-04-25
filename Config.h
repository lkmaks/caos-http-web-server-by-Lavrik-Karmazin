#ifndef CAOS_HTTP_WEB_SERVER_CONFIG_H
#define CAOS_HTTP_WEB_SERVER_CONFIG_H


#include "Utils.h"
#include <vector>
#include <string>

struct Config {
    std::vector<std::pair<std::string, uint16_t> > vhosts;
    int max_main_epoll_queue = 1000;
    int max_thread_conn = 1000;
    int max_conn_queue = 1000;
    const int threads_num = 4;
    int max_epoll_events_in_iteration = 100;
    std::string conf_dir;
    std::string data_dir;
};

#endif