#include <fcntl.h>
#include <unistd.h>
#include <string>

#include "Utils.h"

void modify_nonblock(int fd) {
  int flags = fcntl(fd, F_GETFL);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
}


bool is_ok_hostname(const std::string &hostname) {
  for (char c : hostname) {
    if (!isalnum(c) && c != '.') {
      return false;
    }
  }
  return true;
}

bool is_natural_number(const std::string &s) {
  if (s.empty()) {
    return false;
  }

  for (char c : s) {
    if (!isdigit(c)) {
      return false;
    }
  }

  return s[0] != '0';
}

void write_all(int fd, void *buf, int count) {
  int total = 0;
  while (total < count) {
    total += write(fd, buf, count - total);
  }
}

std::vector<std::string> split(const std::string &str, char c) {
  std::vector<std::string> res;
  int i = 0;
  while (i < str.size()) {
    int j = i;
    while (j < str.size() && str[j] != c) {
      ++j;
    }
    res.push_back(str.substr(i, j - i));
    i = j + 1;
  }
  return res;
}
