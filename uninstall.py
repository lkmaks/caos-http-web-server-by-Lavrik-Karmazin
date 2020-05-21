#!/usr/bin/python3
import os
import subprocess as sb

class Deinstallator:
	def __init__(self, install_dir):
		self.install_dir = install_dir
		self.unit_file = '/lib/systemd/system/caos-http-web-server.service'
		self.server_user = 'caos-http-web-server-user'

	def deinstall(self):
		self.saferun(['systemctl', 'stop', 'caos-http-web-server'], raise_except=False)
		self.saferun(['rm', '-rf', self.install_dir], raise_except=False)
		self.saferun(['rm', self.unit_file], raise_except=False)
		self.saferun(['deluser', self.server_user], raise_except=False)

	# ===================================================================================================
	# Utiility functions

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

def main():
	install_dir = input('Installation directory (path to server\'s folder, e.g. /home/seriy/caos-http-web-server): ')
	confirm = input('Are you sure that this is the absolute path to'
					' the installation directory of caos-http-web-server?\n'
		  			'{}\n'
		  			'Answer (y or n): '.format(os.path.abspath(install_dir)))
	if confirm == 'y':
		deinstallator = Deinstallator(os.path.abspath(install_dir))
		deinstallator.deinstall()
	else:
		print('Aborting')


if __name__ == '__main__':
	main()