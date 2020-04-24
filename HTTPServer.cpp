#include "HTTPServer.h"
#include <fstream>
#include "Utils.h"
#include <exception>
#include <set>
#include <algorithm>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef std::string::size_type size_type;


HTTPServer::HTTPServer(const std::string &ipv4_addr, const std::string &conf_dir, const std::string &data_dir) :
                       ipv4_addr_(ipv4_addr) {
  if (!load_config(conf_dir, data_dir)) {
    throw std::logic_error("Bad vhosts.txt file");
  }
  thread_pool_ = ThreadPool(config_.threads_num);

  std::vector<uint16_t> server_ports;
  for (auto &elem : config_.vhosts) {
    server_ports.push_back(elem.second);
  }
  auto last = std::unique(server_ports.begin(), server_ports.end());
  server_ports.erase(last, server_ports.end());

  int n_ports = server_ports.size();
  server_sockets_.resize(n_ports);
  for (int i = 0; i < n_ports; ++i) {
    server_sockets_[i] = {socket(AF_INET, SOCK_STREAM, 0), server_ports[i]};
    epoll_main_.AddFileDescriptor(
            server_sockets_[i].first,
            EPOLLIN,
            &server_socket_epoll_contexts_[i]);
  }
}


bool HTTPServer::load_config(const std::string &conf_dir, const std::string &data_dir) {
  config_.conf_dir = conf_dir;
  config_.data_dir = data_dir;

  std::ifstream vhosts_file;
  vhosts_file.open(conf_dir + "/vhosts.txt");
  std::string cur_string;
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
    config_.vhosts.emplace_back(hostname, strtol(port_str.c_str(), nullptr, 10));
  }

  // deleting duplicates
  std::sort(config_.vhosts.begin(), config_.vhosts.end());
  auto last = std::unique(config_.vhosts.begin(), config_.vhosts.end());
  config_.vhosts.erase(last, config_.vhosts.end());
}

void HTTPServer::run() {
  sockaddr_in address;
  address.sin_family = AF_INET;
  inet_aton(ipv4_addr_.c_str(), &address.sin_addr);
  for (int i = 0; i < server_sockets_.size(); ++i) {
    address.sin_port = htons(server_sockets_[i].second);
    bind(server_sockets_[i].first, (sockaddr*)(&address), sizeof(address));
    listen(server_sockets_[i].first, config_.max_conn_queue);
  }

  while (true) {
    std::vector<EpollEvent> events = epoll_main_.Wait(config_.max_epoll_events_in_iteration, -1);
    for (auto event : events) {
      int conn_socket = accept(((ServerSocketEpollContext*)event.epoll_context)->fd, nullptr, nullptr);
      thread_pool_.SendNewConnToAnyThread(Connection(conn_socket));
    }
  }
}