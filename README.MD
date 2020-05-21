<h3>Overview</h3>
This project is an implementation of a simple http server for hosting static pages and cgi scripts.

<h3>Functionality</h3>
Right now only hosting static text/html pages is implemented. Server can be controlled via systemctl command
after installation.

<h3>Installation, running and deinstallation</h3>
1) Clone the repository directory. 
2) Run "sudo install.py" from repository directory. Insert the absolute or relative installation path, to a directory,
where you want to install the server (and where its configuration and data will be). Server folder will be created
inside the specified directory.<br>
Example data and configuration will be created:
    1. virtual host named 127.0.0.1 listening on port 8080
    2. static/index.html path for this vhost<br>
    * 127.0.0.1:8080/static/index.hmtl should view a page in browser
3) After that following commands can be used for interacting with the server:
    - sudo systemctl start caos-http-web-server
    - sudo systemctl stop caos-http-web-server
    - sudo systemctl status caos-http-web-server<br>
4) In order to uninstall the server, run "sudo uninstall.py" from repository directory.
Insert the absolute or relative path to the installation directory (it will be deleted, among with other server files).

<h3> Configuration</h3>

Let $DIR be server's installation path (/.../caos-http-web-server) 

1) All data served is defined in $DIR/data, which should contain a directory
for each virtual host, with its name (e.g. diht.ru)
2) Inside virtual host's directory would be the data of that virtual host:
    - Everything in data/static will be treated as static content.
    - Other files are ignored (now).
3) Each virtual host is considered enabled if it appears in $DIR/conf.d/vhosts.conf.<br>
The format of vhosts.conf is as follows:<br>
<ipv4_addr><br>
<hostname_1>:<port_1><br>
...<br>
<hostname_n>:<port_n><br>
Which means that server will run under the given ip, and virtual hosts will listen on given ports
(one host may listen on several ports, and many hosts may listen on one port as well).

4) Server runs under user specified in $DIR/conf.d/server_user.conf, it is a newly created server's user
by default ("caos-http-web-server-user"). One has to make sure that the user has rights to the content needed.

<h3>Additional Info</h3>
If you want to run the server manually, you can:
1) Build the server with CMake that is in the root of the project.
2) Run it according to docs/run.txt
The server will be ran in non-daemon mode.

All information/documantation is available at docs/. The entry point is general.txt.

