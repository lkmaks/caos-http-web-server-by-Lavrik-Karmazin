#include "HttpServer.h"
#include <unistd.h>
#include <iostream>

int main(int argc, char **argv) {
  // argv:
  // 1: configuration directory
  // 2: data directory

  HttpServer server(argv[1], argv[2]);
  server.run();
}