#!/usr/bin/python3
import os

install_dir = '/home/max/caos-http-web-server'


def error(mes):
	print(mes)
	exit(0)


def init_dirs():
	global install_dir
	s = input('Install directory: ')
	s = s.strip()
	if s == '':
		print('Using default script value (empty string provided)')
	else:
		directory = os.path.abspath(s)
		if not os.path.isdir(directory):
			error("Bad install path")
		install_dir = directory
	print('This directory will be deleted with all its contents: {}'.format(install_dir))
	print('Do you agree? (y or n): ')
	reply = input()
	if reply != 'y':
		error('Permission to uninstall not granted')


init_dirs()
os.system('rm -rf {}'.format(install_dir))