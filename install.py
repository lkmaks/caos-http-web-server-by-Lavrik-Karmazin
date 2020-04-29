#!/usr/bin/python3
import os


def error(mes):
	print(mes)
	exit(0)


# pre-set server directories/files
install_dir = '/home/max/caos-http-web-server'
conf_dir = install_dir + '/conf.d'
data_dir = install_dir + '/data'
vhosts_file = conf_dir + '/vhosts.conf'

# created by launcher
pid_file = install_dir + '/run.pid'

# (probably) needs privelleges
unit_file = '/lib/systemd/system/caos-http-web-server.service'


def init_dirs():
	global install_dir
	global conf_dir
	global data_dir
	global vhosts_file
	global pid_file
	s = input('Install directory: ')
	s = s.strip()
	if s == '':
		print('Using default script value (empty string provided)')
		return
	directory = os.path.abspath(s)
	install_dir = directory
	conf_dir = install_dir + '/conf.d'
	data_dir = install_dir + '/data'
	vhosts_file = conf_dir + '/vhosts.conf'
	pid_file = install_dir + '/run.pid'
	print('Installing into {}'.format(install_dir))


def init_data():
	os.system("mkdir {}".format(data_dir + '/127.0.0.1'))
	os.system("mkdir {}".format(data_dir + '/127.0.0.1/static'))
	with open(data_dir + '/127.0.0.1/static/index.html', 'w+') as file:
		file.write('<h1>Hello, world!</h1>')


def create_files():
	os.system("mkdir {}".format(install_dir))
	os.system("mkdir {}".format(conf_dir))
	os.system("mkdir {}".format(data_dir))
	with open(vhosts_file, 'w+') as file:
		file.write('0.0.0.0\n127.0.0.1:8080\n')
	init_data();


init_dirs()
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

exec_start = '{} {} {} {} {}'.format(launcher_file, server_file, conf_dir, data_dir, pid_file)

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
'''.format(pid_file, exec_start, pid_file)

os.chdir('..')
os.system('rm -rf build')

os.system('sudo -s')

os.system("mkdir {}".format(unit_file))
with open(unit_file, 'w') as file:
	file.write(unit)

os.system('systemctl enable {}'.format(unit_file))
