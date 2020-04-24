

#include "HTTPServer.h"

int main(int argc, char **argv) {
  // argv:
  // 1: ipv4 address of server
  // 2: configuration directory
  // 3: data directory

  HTTPServer server(argv[1], argv[2], argv[3]);
  server.run();
}