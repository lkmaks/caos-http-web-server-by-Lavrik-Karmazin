#!/usr/bin/python3
import os


install_dir = '/home/max/caos-http-web-server'
conf_dir = install_dir + '/conf.d'
data_dir = install_dir + '/data'
vhosts_file = conf_dir + '/vhosts.conf'
pidfile = install_dir + '/run.pid'
unit_file = '/lib/systemd/system/caos-http-web-server.service'

def create_files():
	os.system("mkdir {}".format(install_dir))
	os.system("mkdir {}".format(conf_dir))
	os.system("mkdir {}".format(data_dir))
	with open(vhosts_file, 'w+') as file:
		file.write('0.0.0.0\n127.0.0.1:8080\n')


create_files()

os.system('mkdir build')
os.chdir('build')
os.system('cmake ../')
os.system('make')
os.system('mv {} {}'.format('caos-http-web-server', install_dir + '/caos-http-web-server'))
os.system('g++ --std=c++11 -o caos-http-web-server-launcher ../launcher.cpp')
os.system('mv {} {}'.format('caos-http-web-server-launcher', install_dir + '/caos-http-web-server-launcher'))

server_file = install_dir + '/caos-http-web-server'
launcher_file = install_dir + '/caos-http-web-server-launcher'

exec_start = '{} {} {} {} {}'.format(launcher_file, server_file, conf_dir, data_dir, pidfile)

unit = '''[Unit]
Description=A simple http server hosting static content and cgi scripts
After=network.target

[Service]
Type=forking
PIDFile={}
ExecStart={}
ExecStop=/sbin/start-stop-daemon --quiet --stop --retry QUIT/5 --pidfile {}
TimeoutStopSec=5
KillMode=mixed

[Install]
WantedBy=multi-user.target
'''.format(pidfile, exec_start, pidfile)

os.chdir('..')
os.system('rm -rf build')

os.system('sudo -s')

os.system("mkdir {}".format(unit_file))
with open(unit_file, 'w') as file:
	file.write(unit)

