# region ---------- IMPORTS ----------
from subprocess import check_output, Popen, PIPE
# endregion

# region ---------- CONSTANTS ----------
LETTER_TO_SIZE = {'K': 10**3,
                  'M': 10**6,
                  'G': 10**9}
# endregion

#  region ---------- FUNCTIONS ----------


def find_devs():
    """
    Finds the connected USB BULK devices.
    :return: a list of Device-type objects
    """

    data_headers = ['NAME', 'TYPE', 'TRAN']
    cmd = 'lsblk -S -nl -o {data_headers}'.format(data_headers=','.join(data_headers))
    data = check_output(cmd.split())
    data_lines = data.splitlines()
    devs = []
    for line in data_lines:
        dev = {}
        parts = line.split()
        for i in xrange(len(data_headers)):
            dev[data_headers[i]] = parts[i]
        if dev['TYPE'] == 'disk' and dev['TRAN'] == 'usb':
            path = '/dev/{name}'.format(name=dev['NAME'])
            cmd = 'lsblk {path} -nl -o SIZE'.format(path=path)
            data = check_output(cmd.split()).splitlines()[0]
            size_desc = data
            if data.isdigit():
                size = int(data)
            else:
                size = int(float(data[:-1]) * LETTER_TO_SIZE[data[-1]])
            cmd = 'lsblk {path} -nl -o VENDOR,MODEL'.format(path=path)
            data = check_output(cmd.split())
            desc = ' '.join(data.split() + [size_desc])
            devs.append(Device(path, size, desc))

    return devs
# endregion

# region ---------- CLASS ----------


class Device:
    def __init__(self, path, size, desc=None):
        """
        Initializes the Device object.
        :param path: the path of the device in Linux
        :param size: the size of the device (bytes)
        :param desc: a textual description of the device
        """

        self.path = path
        self.size = size
        self.desc = desc

    def read_blocks(self, lba, count=1, block_size=512):
        """
        Reads data from the device.
        :param lba: the LBA of the first block to read
        :param count: the amount of blocks to read
        :param block_size: the size of each block
        :return: the data read from the device
        """

        cmd = 'dd if={path} bs={block_size} skip={lba} count={count} status=none'.format(path=self.path,
                                                                                         block_size=block_size,
                                                                                         lba=lba,
                                                                                         count=count)
        return check_output(cmd.split())

    def write_blocks(self, lba, data, block_size=512):
        """
        Write data to the device.
        :param lba: the LBA of the first block to write
        :param data: the data to write to the device
        :param block_size: the size of each block
        :return: None
        """

        cmd = 'dd of={path} bs={block_size} seek={lba} status=none'.format(path=self.path,
                                                                           block_size=block_size,
                                                                           lba=lba)
        proc = Popen(cmd.split(), stdin=PIPE)
        proc.communicate(data)

    def read(self, addr, len):
        """
        Read data from the device.
        :param addr: the address of the first byte to read
        :param len: the amount of bytes to read
        :return: the data read from the device
        """

        return self.read_blocks(lba=addr, count=len, block_size=1)

    def write(self, addr, data):
        """
        Write data to the device.
        :param addr: the address of the first byte to write to
        :param data: the data to write to the device
        :return: None
        """

        self.write_blocks(lba=addr, data=data, block_size=1)

# endregion
