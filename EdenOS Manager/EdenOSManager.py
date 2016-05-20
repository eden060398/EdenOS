# region ---------- IMPORTS ----------
from DeviceManager import *
from EdenFSManager import *
from EdenOSClient import *
from FSBrowser import *
import os
# endregion

# region ---------- CONSTANTS ----------
# endregion

# region ---------- FUNCTIONS ----------
# endregion

# region ---------- CLASS ----------


class EdenOSManager:
    def __init__(self, dev):
        self.dev = dev
        self.fs = EdenFS(dev)
        self.client = EdenOSClient(dev)

    def analyse(self):
        eden_os = self.client.is_edenos()
        if not eden_os:
            print 'The device does not contain EdenOS, or the OS on it is outdated or corrupted.'
            self.client.install_edenos()
        eden_fs = self.fs.is_edenfs()
        if not eden_fs:
            print 'The device does not contain an EdenFS filesystem.'
            resp = raw_input('Would you like to choose a directory that will serve as root? (Y/N) ').upper()
            while resp != 'Y' and resp != 'N':
                resp = raw_input('Would you like to choose a directory that will serve as root? (Y/N) ').upper()
            if resp == 'Y':
                root_dir = raw_input("Please specify the path: ")
                while not os.path.exists(root_dir):
                    root_dir = raw_input("Path does not exist. Please specify the path: ")
            else:
                root_dir = None
            self.fs.format_edenfs()
            if root_dir:
                self.fs.import_dir(root_dir, '/')

# endregion

# region ---------- MAIN ----------


def main():
    devices = find_devs()

    print '{count} devices were detected:\n'.format(count=len(devices))

    for i in xrange(len(devices)):
        print '{i}. {dev}'.format(i=i + 1, dev=devices[i].desc)
    print

    if len(devices):
        msg = 'Please type the number of the device you wish to analyse (1-{max}): '.format(max=len(devices))
        num = raw_input(msg)
        while not num.isdigit() or int(num) < 1 or int(num) > len(devices):
            num = raw_input('Illegal input. ' + msg)
        print

        dev = devices[int(num) - 1]
        os_manager = EdenOSManager(dev)
        os_manager.analyse()

        browser = FSBrowser(dev)
        browser.run()
    else:
        print 'No suitable devices were found. Exiting.'




if __name__ == '__main__':
    main()
# endregion
