#!/usr/bin/python3
import os
import subprocess as sb
import crypt


class InstallationConfig:
	def __init__(self, install_dir):
		self.workdir = os.getcwd()
		if not self.workdir.endswith('caos-http-web-server-by-Lavrik-Karmazin'):
			raise Exception('You have to run script from repository root!')

		install_dir = os.path.abspath(install_dir)
		self.install_dir = install_dir + '/caos-http-web-server'
		self.conf_dir = self.install_dir + '/conf.d'
		self.data_dir = self.install_dir + '/data'

		self.vhosts_file = self.conf_dir + '/vhosts.conf'
		self.username_file = self.conf_dir + '/server_user.conf'
		self.pid_file = self.install_dir + '/run.pid'

		self.unit_file = '/etc/systemd/system/caos-http-web-server.service'

		# if os.path.exists('/lib/systemd/system'):
		# 	self.unit_file = '/lib/systemd/system/caos-http-web-server.service'
		# elif os.path.exists('/usr/lib/systemd/system'):
		# 	self.unit_file = '/usr/lib/systemd/system/caos-http-web-server.service'

		self.main_executable_name = 'caos-http-web-server'
		self.launcher_name = 'caos-http-web-server-launcher'

		self.user_comment = 'User for installing & running caos-http-web-server http daemon.'
		self.user_password = 'password32'
		self.user_password_salt = '07'
		self.user_login = 'caos-http-web-server-user'
		self.user_uid = None

		with open('install_data/vhosts.conf', 'r') as file:
			self.vhosts_default = file.read()

		with open('install_data/server_user.conf', 'r') as file:
			self.server_user_name_default = file.read().strip()


class Installer:
	def __init__(self, config):
		self.config = config

	def RollBack(self):
		os.chdir(self.config.workdir)
		self.saferun(['systemctl', 'stop', 'caos-http-web-server'], raise_except=False)
		self.saferun(['rm', '-rf', self.config.install_dir], raise_except=False)
		self.saferun(['rm', '-rf', 'build'], raise_except=False)
		self.saferun(['rm', self.config.unit_file], raise_except=False)
		self.saferun(['userdel', self.config.user_login], raise_except=False)

	def Install(self):
		report = self.CheckInstallation()
		if report != 'OK':
			raise Exception('Cannot install to this system: {}'.format(report))
		os.umask(0)
		try:
			self.CreateUser()
			self.CreateDirs()
			self.CreateDefaultConfig()
			self.CreateDefaultData()
			self.BuildExecutable()
			self.CreateUnitFile()
			self.EnableUnitFile()
		except Exception as e:
			print(e)
			print('Installation unsuccessful. Rolling back changes...')
			self.RollBack()
			print('Done')
			return False
		else:
			return True

	def CheckInstallation(self):
		""" Check if current state of the system is suitable for installing the server """

		if not os.path.exists(self.config.install_dir[:len(self.config.install_dir) - len('/caos-http-web-server')]):
			return 'Specified directory does not exist'

		if os.path.exists(self.config.install_dir):
			return 'Directory caos-http-web-server already exists in the specified directory'

		return 'OK'


	def CreateUser(self):
		self.saferun(['useradd',
					'--system',							   # system user
					'--comment', self.config.user_comment, # comment (user's full name)
					'--expiredate', '',                    # no expiration date
					'--user-group',                        # create a group for the user with the same name
					'--password', crypt.crypt(             # password option
					self.config.user_password,             # actual password
					self.config.user_password_salt),       # salt to hash the password with
					self.config.user_login])               # user's name
		# get uid, gid of created user
		ids = self.getids(self.config.user_login)
		self.config.user_uid = ids['uid']
		self.config.user_gid = ids['gid']


	def BecomeUser(self, uid, gid):
		os.setegid(gid)
		os.seteuid(uid)


	def CreateDirs(self):
		os.mkdir(self.config.install_dir, 0o777)
		os.mkdir(self.config.conf_dir, 0o777)
		os.mkdir(self.config.data_dir, 0o777)


	def CreateDefaultConfig(self):
		with open(self.config.vhosts_file, 'w') as file:
			file.write(self.config.vhosts_default)
		os.chmod(self.config.vhosts_file, 0o666)
		with open(self.config.username_file, 'w') as file:
			file.write(self.config.server_user_name_default)
		os.chmod(self.config.username_file, 0o666)


	def CreateDefaultData(self):
		# vhost directory
		vhost_dir = self.config.data_dir + '/127.0.0.1'
		os.mkdir(vhost_dir, 0o777)

		# html file
		os.mkdir(vhost_dir + '/static', 0o777)
		with open(vhost_dir + '/static/index.html', 'w') as dest:
			with open('install_data/index.html', 'r') as src:
				dest.write(src.read())
		os.chmod(vhost_dir + '/static/index.html', 0o666)


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
					  '../launcher.cpp'])
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

		exec_start_cmd = '{} {} {} {} {} {}'.format(
			launcher_file, 		  # run launcher; pass further parameters to it
			main_file, 			  # the server that launcher will launch
			self.config.conf_dir, # server configuration dir
			self.config.data_dir, # server configuration dir
			self.config.pid_file, # pidfile which launcher will write pid to
			self.config.user_uid) # server user id to run under

		unit = unit_template.format(self.config.pid_file,
									exec_start_cmd,
									self.config.pid_file)

		with open(self.config.unit_file, 'w') as file:
			file.write(unit)


	def EnableUnitFile(self):
		self.saferun(['systemctl', 'enable', self.config.unit_file])


	# ============================================================================================================
	# UTILITY FUNTIONS:

	def saferun(self, args, raise_except=True):
		res = sb.run(args, stdout=sb.PIPE, stderr=sb.PIPE)
		if res.returncode != 0:
			if raise_except:
				raise Exception('Error: command {} returned code {} with output:\n'
								'stdout:\n {}\n'
								'stderr:\n {}\n'
								'Exiting...'.format(' '.join(args), res.returncode, res.stdout, res.stderr))
		else:
			return res


	def getids(self, login):
		result = sb.run(['id', login], stdout=sb.PIPE, stderr=sb.PIPE)

		s = str(result.stdout)

		l = s.find('uid=')
		r = s.find('(', l)
		uid = int(s[l + len('uid='):r])

		l = s.find('gid=')
		r = s.find('(', l)
		gid = int(s[l + len('uid='):r])

		return {'uid': uid, 'gid': gid}


def error(mes):
	print(mes)
	exit(0)

def main():
	if os.geteuid() != 0:
		raise Exception('No rights! Add sudo.')
	install_dir = input('Directory to install server to (server will create its directory inside that): ')
	install_conf = InstallationConfig(install_dir)
	installer = Installer(install_conf)
	res = installer.Install()
	if res:
		print('Installation successfull')

if __name__ == '__main__':
	main()
	# install_conf = InstallationConfig()
	# installer = Installer(install_conf)
	# print(installer.getids('root'))
