This project is an implementation of a simple http server for hosting static pages and cgi scripts.

Functionality:<br>
Right now only hosting static text/html pages is implemented. Server can be controlled via systemctl command
after installation.

Installation, running and deinstallation:
1) Clone the repository directory. 
2) Run "sudo install.py" from the repository directory. Insert the absolute installation path,
where you want to install the server (and where its configuration and data will be). 
The parent for the target directory should exist, if $installation_path != '/'<br>
3) After that following commands can be used for interacting with the server:
    - sudo systemctl start caos-http-web-server
    - sudo systemctl stop caos-http-web-server
    - sudo systemctl status caos-http-web-server<br>
4) In order to uninstall the server, run "sudo uninstall.py" from anywhere.
Insert the absolute path to installation directory (which will be deleted). You will have to
confirm that you want delete that directory.
    
All data that server serves is defined in $installation_dir/data, which should contain a directory
for each virtual host (which are in turn described in $installation_dir/conf.d/vhosts.conf).<br>
Inside virtual host's directory would be the data of that virtual host. "static/" folder contains static
pages.

If you want to run the server manually, you can:
1) Build the server with CMake that is in the root of the project.
2) Run it according to docs/run.txt
The server will be ran in non-daemon mode.

All information/documantation is available at docs/. The entry point is general.txt.

