#include "HttpServer.h"
#include <unistd.h>
#include <iostream>

int main(int argc, char **argv) {
  // argv:
  // 1: configuration directory
  // 2: data directory
  // 3: uid to run under
  // 4: gid to run under

  // create server (load configs) - as root
  HttpServer server(argv[1], argv[2]);

  //seteuid(strtol(argv[3], nullptr, 10));
  //setegid(strtol(argv[4], nullptr, 10));

  // run - as requested user
  server.run();
}