#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <pwd.h>

int main(int argc, char **argv) {
  // argv:
  // 1) server executable path
  // 2) server conf path (to pass to executable)
  // 3) server data path (to pass to executable)
  // 4) pidfile path


  // read login of user which server will work under
  std::string server_user_name;
  std::ifstream fin;
  fin.open(std::string(argv[2]) + "/" + "server_user.conf");
  fin >> server_user_name;
  fin.close();

  // get uid of that user
  passwd *buf = getpwnam(server_user_name.c_str());
  uid_t server_uid = buf->pw_uid;
  gid_t server_gid = buf->pw_gid;

  int status = daemon(0, 0);
  if (status) {
    std::cerr << "daemon() launch unsuccessful" << std::endl;
  }
  // we are now in the child and detached + all the daemon stuff

  FILE *pid_file = fopen(argv[4], "w");
  std::string s = std::to_string(getpid());
  fwrite(s.c_str(), 1, s.size(), pid_file);
  fclose(pid_file);

  execl(argv[1],
          argv[1],
          argv[2],
          argv[3],
          std::to_string(server_uid).c_str(),
          std::to_string(server_gid).c_str(),
          NULL);
}
