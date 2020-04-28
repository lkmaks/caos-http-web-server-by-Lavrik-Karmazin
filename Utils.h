#ifndef CAOS_HTTP_WEB_SERVER_UTILS_H
#define CAOS_HTTP_WEB_SERVER_UTILS_H

#include <string>
#include <vector>

void modify_nonblock(int fd);

bool is_ok_hostname(const std::string &hostname);

bool is_natural_number(const std::string &s);

void write_all(int fd, void *buf, int count);

std::vector<std::string> split(const std::string &str, char c);

bool is_ok_ipv4_address(const std::string &ipv4_addr);

#endif
