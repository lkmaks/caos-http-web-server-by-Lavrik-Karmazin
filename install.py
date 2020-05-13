#!/usr/bin/python3
import os
import subprocess as sb
import crypt


class Config:
	def __init__(self):
		self.install_dir = '/home/max/caos-http-web-server'
		self.conf_dir = self.install_dir + '/conf.d'
		self.data_dir = self.install_dir + '/data'
		self.vhosts_file = self.conf_dir + '/vhosts.conf'
		self.pid_file = self.install_dir + '/run.pid'
		self.unit_file = '/lib/systemd/system/caos-http-web-server.service'
		self.main_executable_name = 'caos-http-web-server'
		self.launcher_name = 'caos-http-web-server-launcher'
		self.user_comment = 'User for installing & running caos-http-web-server http daemon.'
		self.user_password = 'password32'
		self.user_password_salt = '07'
		self.user_login = 'caos-httpd-lavrik'
		self.user_uid = None

class Installer:
	def __init__(self, config):
		self.config = config

	def Install(self):
		report = self.CheckInstallation()
		if report != 'OK':
			raise Exception('Cannot install to this system: {}'.format(report))
		self.CreateUser()
		self.BecomeUser(self.config.user_uid, self.config.user_gid)
		self.CreateFiles()
		self.CreateExampleConfig()
		self.CreateExampleData()
		self.BuildExecutable()
		self.BecomeUser(0, 0) # BECOME ROOT AGAIN!
		self.CreateUnitFile()

	def CheckInstallation(self):
		return 'OK'

	def CreateUser(self):
		sb.run(['useradd',
				'--home-dir', self.config.install_dir, # home directory
				'--no-create-home',                    # don't create home dir yet
				'--comment', self.config.user_comment, # comment (user's full name)
				'--expiredate', '',                    # no expiration date
				'--user-group',                        # create a group for the user with the same name
				'--password', crypt.crypt(             # password option
				self.config.user_password,             # actual password
				self.config.user_password_salt),       # salt to hash the password with
				self.config.user_login])               # user's name
		# get uid, gid of created user
		self.config.user_uid, self.config.user_gid = self.getids(self.config.user_login)

	def BecomeUser(self, uid, gid):
		os.seteuid(uid)
		os.setegid(gid)

	def CreateFiles(self):
		os.mkdir(self.config.install_dir, 0o777)
		os.mkdir(self.config.conf_dir, 0o777)
		os.mkdir(self.config.data_dir, 0o777)

	def getids(self, login):
		result = sb.run(['id', login], stdout=sb.PIPE, stderr=sb.PIPE)

		l = result.stdout.find('uid=')
		r = result.stdout.find('(', l)
		uid = result[l + len('uid='):r]

		l = result.stdout.find('gid=')
		r = result.stdout.find('(', l)
		gid = result[l + len('uid='):r]

		return {'uid': uid, 'gid': gid}


def error(mes):
	print(mes)
	exit(0)





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
	init_data()


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
