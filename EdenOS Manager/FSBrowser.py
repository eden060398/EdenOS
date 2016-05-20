# region ---------- IMPORTS ----------
import sys
import os
from EdenFSManager import *
# endregion

# region ---------- CONSTANTS ----------

# endregion

#  region ---------- FUNCTIONS ----------

# endregion

# region ---------- CLASS ----------


class FSBrowser:
    def __init__(self, dev, dir='/'):
        self.fs = EdenFS(dev)
        self.dir = dir
        self.cmd = {'help': self.__cmd_help,
                    'list': self.__cmd_list,
                    'cd': self.__cmd_cd,
                    'read': self.__cmd_read,
                    'import': self.__cmd_import,
                    'export': self.__cmd_export,
                    'format': self.__cmd_format,
                    'del': self.__cmd_del}

    def run(self):
        while True:
            inp = raw_input(self.dir + ' >>> ').split()
            if inp:
                print
                if inp[0] == 'exit':
                    break
                if inp[0] in self.cmd:
                    self.cmd[inp[0]](inp[1:])
                else:
                    print 'Command not found. Type "help" for more information.'
                print

    def __cmd_help(self, args, desc=False, help=False):
        if desc:
            print 'help\tPRINT HELP INFORMATION'
            return
        if help:
            print 'help [COMMAND]\n' \
                  '\tCOMMAND\t:\tPRINT SPECIFIC COMMAND INFORMATION'
            return

        for arg in args:
            if arg[0] != '-':
                cmd = arg
                if cmd in self.cmd:
                    self.cmd[cmd]([], help=True)
                else:
                    print 'COMMAND DOES NOT EXIST!'
                return

        for cmd in self.cmd.keys():
            self.cmd[cmd]([], desc=True)
        print '\nType "exit" to exit.'

    def __cmd_list(self, args, desc=False, help=False):
        if desc:
            print 'list\tPRINT DIRECTORY CONTENTS'
            return
        if help:
            print 'list [-t] [-s] [PATH]\n' \
                  '\t-t\t:\tLIST AS TREE\n' \
                  '\t-s\t:\tPRINT SIZE\n' \
                  '\tPATH\t:\tTHE PATH TO LIST (DEFAULT: WORKING DIRECTORY)'
            return

        tree = False
        put_size = False
        path = self.dir

        for arg in args:
            if arg[0] != '-':
                if arg[0] == '/':
                    path = arg
                else:
                    path = os.path.join(self.dir, arg)
                if not self.fs.is_path(path, is_dir=True):
                    print 'Path does not exist!'
                    return
            elif arg == '-t':
                tree = True
            elif arg == '-s':
                put_size = True

        print self.fs.list_dir(path, tree=tree, put_type=True, put_size=put_size),

    def __cmd_cd(self, args, desc=False, help=False):
        if desc:
            print 'cd\tCHANGE WORKING DIRECTORY'
            return
        if help:
            print 'cd PATH\n' \
                  '\tPATH\t:\tTHE PATH OF THE NEW WORKING DIRECTORY'
            return

        for arg in args:
            if arg[0] == '/':
                path = arg
            else:
                path = os.path.join(self.dir, arg)
            if self.fs.is_path(path, is_dir=True):
                self.dir = path
                return
            print 'Path does not exist!'
            return

        print 'No path given. Type "help cd" for more information.'

    def __cmd_read(self, args, desc=False, help=False):
        if desc:
            print 'read\tREAD FILE CONTENTS'
            return
        if help:
            print 'read PATH [-sSIZE]\n' \
                  '\tPATH\t:\tTHE PATH OF THE FILE TO READ FROM\n' \
                  '\tSIZE\t:\tTHE COUNT OF BYTES TO READ'
            return

        path = None
        size = None
        for arg in args:
            if arg[0] != '-':
                if arg[0] == '/':
                    path = arg
                else:
                    path = os.path.join(self.dir, arg)
                if not self.fs.is_path(path, is_dir=False):
                    print 'Path does not exist!'
                    return
            elif arg.startswith('-s'):
                size = int(arg[2:])
        if path is None:
            print 'No path given. Type "help read" for more information.'

        f = File(self.fs, path)
        data = f.read(count=size)
        f.close()

        print data

    def __cmd_import(self, args, desc=False, help=False):
        if desc:
            print 'import\tIMPORT A FILE'
            return
        if help:
            print 'import -eEX_PATH -iIN_PATH\n' \
                  '\tEX_PATH\t:\tTHE PATH OF THE FILE TO IMPORT (IN THE LOCAL FS)\n' \
                  '\tIN_PATH\t:\tTHE PATH OF THE FILE IN EDENFS (IN EDENFS)'
            return

        ex_path = None
        in_path = None

        for arg in args:
            if arg.startswith('-e'):
				try:
					open(arg[2:], 'r').close()
					ex_path = arg[2:]
				except IOError:
					print 'Local path does not exist!'
                    return
					"""
                if os.path.isfile(arg[2:]):
                    ex_path = arg[2:]
                else:
                    print 'Local path does not exist!'
                    return
					"""
            elif arg.startswith('-i'):
                path = os.path.join(os.path.split(arg[2:])[:-1])[0]
                if arg[2] == '/':
                    in_path = arg[2:]
                else:
                    path = os.path.join(self.dir, path)
                    in_path = os.path.join(self.dir, arg[2:])
                if not self.fs.is_path(path, is_dir=True):
                    print 'EdenFS path does not exist!'
                    return
        if ex_path is None or in_path is None:
            print 'Illegal format. Type "help import" for more information.'

        self.fs.import_file(ex_path, in_path)

    def __cmd_export(self, args, desc=False, help=False):
        if desc:
            print 'export\tEXPORT A FILE'
            return
        if help:
            print 'export -iIN_PATH -eEX_PATH\n' \
                  '\tIN_PATH\t:\tTHE PATH OF THE FILE IN EDENFS (IN EDENFS)\n' \
                  '\tEX_PATH\t:\tTHE PATH OF THE FILE TO IMPORT (IN THE LOCAL FS)'
            return

        ex_path = None
        in_path = None

        for arg in args:
            if arg.startswith('-e'):
				try:
					open(arg[2:], 'w').close()
					ex_path = arg[2:]
				except IOError:
					print 'Local path does not exist!'
                    return
					"""
                path = os.path.join(os.path.split(arg[2:])[:-1])[0]
                if os.path.isdir(path):
                    ex_path = arg[2:]
                else:
                    print 'Local path does not exist!'
                    return
					"""
            elif arg.startswith('-i'):
                if arg[2] == '/':
                    in_path = arg[2:]
                else:
                    in_path = os.path.join(self.dir, arg[2:])
                if not self.fs.is_path(in_path, is_dir=False):
                    print 'EdenFS path does not exist!'
                    return
        if ex_path is None or in_path is None:
            print 'Illegal format. Type "help import" for more information.'

        self.fs.export_file(in_path, ex_path)

    def __cmd_format(self, args, desc=False, help=False):
        if desc:
            print 'format\tFORMAT EDENFS'
            return
        if help:
            print 'format [-rROOT]\n' \
                  '\tROOT\t:\tTHE PATH OF THE DIRECTORY TO BE USED AS ROOT'
            return

        root_dir = None
        for arg in args:
            if arg.startswith('-r'):
                if os.path.isdir(arg[2:]):
                    root_dir = arg[2:]
                else:
                    print 'Root path does not exist!'
                    return

        self.fs.format_edenfs()
        if root_dir:
            self.fs.import_dir(root_dir, '/')

        self.dir = '/'

    def __cmd_del(self, args, desc=False, help=False):
        if desc:
            print 'del\tDELETE FILE OR FOLDER'
            return
        if help:
            print 'del [PATH] [PATH] [PATH] . . .\n' \
                  '\tPATH\t:\tTHE PATH OF THE TARGET TO BE DELETED'
            return

        for arg in args:
            if arg[0] == '/':
                path = arg
            else:
                path = os.path.join(self.dir, arg)
            if self.fs.is_path(path):
                self.fs.delete(path)
# endregion
