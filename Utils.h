#ifndef CAOS_HTTP_WEB_SERVER_UTILS_H
#define CAOS_HTTP_WEB_SERVER_UTILS_H

#include <string>

void modify_nonblock(int fd);

bool is_ok_hostname(const std::string &hostname);

bool is_natural_number(const std::string &s);

void write_all(int fd, void *buf, int count);

#endif
