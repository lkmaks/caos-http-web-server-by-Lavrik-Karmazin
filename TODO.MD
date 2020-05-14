1) More optimal data/conf location
2) Server should shut down if it could not configure itself to listen on ports, load config, etc.
It should also output the problem.
3) Input validation for installation scripts
4) Code documentation
5) Which rights do we want to have for different files/dirs of server?
6) Avoid active port binding
7) Graceful handling of systemctl stop (for correct status)
