#include <fcntl.h>
#include <string>
#include <unistd.h>

void modify_nonblock(int fd) {
  int flags = fcntl(fd, F_GETFL);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
}


void FAIL(const char *cause) {
  fprintf(stderr, "Failure detected. Printing cause and explanation.\n");
  perror(cause);
  exit(1);
}

void CFAIL(const char *cause) {
  // custom fail
  fprintf(stderr, "%s\n", cause);
  exit(1);
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