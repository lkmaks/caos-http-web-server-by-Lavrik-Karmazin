#include "HttpServer.h"
#include <fstream>
#include "Utils/GeneralUtils.h"
#include <exception>
#include <set>
#include <algorithm>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "Utils/Debug.hpp"

typedef std::string::size_type size_type;


Config &HttpServer::GetConf() {
  return config_;
}

HttpServer::HttpServer(const std::string &conf_dir, const std::string &data_dir) {
  if (!load_config(conf_dir, data_dir)) {
    throw std::logic_error("Bad vhosts.txt file");
  }

  std::vector<uint16_t> server_ports;
  for (auto &elem : config_.vhosts) {
    server_ports.push_back(elem.second);
  }
  auto last = std::unique(server_ports.begin(), server_ports.end());
  server_ports.erase(last, server_ports.end());

  // argument does not affect anything for linux >= 2.6.8
  epoll_main_ = Epoll(config_.max_main_epoll_queue);
  int n_ports = server_ports.size();
  server_sockets_.resize(n_ports);
  for (int i = 0; i < n_ports; ++i) {
    server_sockets_[i] = {socket(AF_INET, SOCK_STREAM, 0), server_ports[i]};
    #ifdef LOCAL
        int opt = 1;
        setsockopt(server_sockets_[i].first, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    #endif
    auto *ptr = new ConnectionEpollContext(Connection(server_sockets_[i].first, server_ports[i]));
    server_socket_epoll_contexts_.push_back(ptr);
    epoll_main_.AddFileDescriptor(
            server_sockets_[i].first,
            EPOLLIN,
            server_socket_epoll_contexts_[i]);
  }

  // bind on ports now

  sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  inet_aton(ipv4_addr_.c_str(), &address.sin_addr);
  for (int i = 0; i < server_sockets_.size(); ++i) {
    address.sin_port = htons(server_sockets_[i].second);
    bind(server_sockets_[i].first, (sockaddr*)(&address), sizeof(address));
    listen(server_sockets_[i].first, config_.max_conn_queue);
  }
}


bool HttpServer::load_config(const std::string &conf_dir, const std::string &data_dir) {
  config_.conf_dir = conf_dir;
  config_.data_dir = data_dir;

  std::ifstream vhosts_file;
  vhosts_file.open(conf_dir + "/vhosts.conf");
  std::string cur_string;
  getline(vhosts_file, ipv4_addr_);
  if (!is_ok_ipv4_address(ipv4_addr_)) {
    throw std::logic_error("Incorrect ip address.");
  }
  while (getline(vhosts_file, cur_string)) {
    if (cur_string.empty()) {
      continue;
    }
    size_type pos = cur_string.find(':');
    if (pos == std::string::npos) {
      return false;
    }
    std::string hostname = cur_string.substr(0, pos);
    std::string port_str = cur_string.substr(pos + 1, cur_string.size());
    if (!is_natural_number(port_str) || !is_ok_hostname(hostname)) {
      return false;
    }
    config_.vhosts.emplace_back(hostname, (uint16_t)strtol(port_str.c_str(), nullptr, 10));
  }

  // deleting duplicates
  std::sort(config_.vhosts.begin(), config_.vhosts.end());
  auto last = std::unique(config_.vhosts.begin(), config_.vhosts.end());
  config_.vhosts.erase(last, config_.vhosts.end());

  return true;
}

void HttpServer::run() {
  thread_pool_ = ThreadPool(config_.threads_num, this);

  while (true) {
    std::vector<EpollEvent> events = epoll_main_.Wait(config_.max_epoll_events_in_iteration, -1);
    for (auto event : events) {
      int sock = ((ConnectionEpollContext*)event.epoll_context)->GetConn().sock;
      int conn_socket = accept(sock, nullptr, nullptr);
      thread_pool_.SendNewConnToAnyThread(Connection(conn_socket));
    }
  }
}