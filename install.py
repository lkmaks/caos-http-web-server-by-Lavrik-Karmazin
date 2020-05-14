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

		with open('install_data/vhosts.conf', 'r') as file:
			self.vhosts_default = file.read()


class Installer:
	def __init__(self, config):
		self.config = config


	def Install(self):
		report = self.CheckInstallation()
		if report != 'OK':
			raise Exception('Cannot install to this system: {}'.format(report))
		self.CreateUser()
		self.BecomeUser(self.config.user_uid, self.config.user_gid)
		self.CreateDirs()
		self.CreateDefaultConfig()
		self.CreateDefaultData()
		self.BuildExecutable()
		self.BecomeUser(0, 0) # BECOME ROOT AGAIN!
		self.CreateUnitFile()
		self.EnableUnitFile()


	def CheckInstallation(self):
		""" Check if current state of the system is suitable for installing the server """
		return 'OK'


	def CreateUser(self):
		self.saferun(['useradd',
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


	def CreateDirs(self):
		os.mkdir(self.config.install_dir, 0o777)
		os.mkdir(self.config.conf_dir, 0o777)
		os.mkdir(self.config.data_dir, 0o777)


	def CreateDefaultConfig(self):
		with open(self.config.vhosts_file, 'w') as file:
			file.write(self.config.vhosts_default)


	def CreateDefaultData(self):
		# vhost directory
		vhost_dir = self.config.install_dir + '/127.0.0.1'
		os.mkdir(vhost_dir, 0o777)
		# html file
		with open(vhost_dir + '/index.html', 'w') as dest:
			with open('install_data/index.html', 'r') as src:
				dest.write(src.read())


	def BuildExecutable(self):
		# go into build dir
		self.saferun(['mkdir', 'build'])
		os.chdir('build')

		# compile server executable and put it into installation dir
		self.saferun(['cmake', '../'])
		self.saferun(['make'])
		self.saferun(['mv',
					  self.config.main_executable_name,
					  '{}/{}'.format(self.config.install_dir, self.config.main_executable_name)])

		# compile launcher executable and put it there too
		self.saferun(['g++',
					  '--std=c++11',
					  '-o', self.config.launcher_name,
					  'launcher.cpp'])
		self.saferun(['mv',
					  self.config.launcher_name,
					  '{}/{}'.format(self.config.install_dir, self.config.launcher_name)])

		# clean build files
		os.chdir('..')
		self.saferun(['rm', '-rf', 'build'])


	def CreateUnitFile(self):
		unit_template = ''
		with open('install_data/unit.txt', 'r') as file:
			unit_template = file.read()

		launcher_file = '{}/{}'.format(self.config.install_dir, self.config.launcher_name)
		main_file = '{}/{}'.format(self.config.install_dir, self.config.main_executable_name)

		exec_start_cmd = '{} {} {} {} {}'.format(
			launcher_file, 		  # run launcher; pass further parameters to it
			main_file, 			  # the server that launcher will launch
			self.config.conf_dir, # server configuration dir
			self.config.data_dir, # server configuration dir
			self.config.pid_file) # pidfile which launcher will write pid to

		unit = unit_template.format(self.config.pid_file,
									exec_start_cmd,
									self.config.pid_file)

		with open(self.config.unit_file, 'w') as file:
			file.write(unit)


	def EnableUnitFile(self):
		self.saferun(['systemctl', 'enable', self.config.unit_file])


	# ============================================================================================================
	# UTILITY FUNTIONS:

	def saferun(self, args):
		res = sb.run(args, stdout=sb.PIPE, stdin=sb.PIPE)
		if res.returncode != 0:
			raise Exception('Error: command {} returned code {} with output:\n'
							'stdout:\n {}\n'
							'stderr:\n {}\n'
							'Exiting...'.format(' '.join(args), res.returncode, res.stdout, res.stderr))
		else:
			return res


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

