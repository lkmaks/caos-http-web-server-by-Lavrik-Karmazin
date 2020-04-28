#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>


int main(int argc, char **argv) {
  // argv:
  // 1) server executable path
  // 2) server conf path (to pass to executable)
  // 3) server data path (to pass to executable)
  // 4) pidfile path

  int status = daemon(0, 0);
  if (status) {
    std::cerr << "daemon() launch unsuccessful" << std::endl;
  }
  // we are now in the child and detached + all the daemon stuff

  FILE *pid_file = fopen(argv[4], "w");
  std::string s = std::to_string(getpid());
  fwrite(s.c_str(), 1, s.size(), pid_file);
  fclose(pid_file);

  execl(argv[1], argv[1], argv[2], argv[3]);
}
