# region ---------- IMPORTS ----------
import socket
import hashlib
from EdenFSManager import *
import UI
# endregion

# region ---------- CONSTANTS ----------
IP = '192.168.94.1'
PORT = 6398

KEEP_ALIVE = b'\x00'
GET_MD5 = b'\x01'
GET_OS = b'\x02'

READ_LEN = 1024
BLOCK_SIZE = 512
RECV_LEN = 1024

EDENFS_DATA_START = 488
EDENFS_DATA_END = 506
EDENFS_DATA_LEN = EDENFS_DATA_END - EDENFS_DATA_START
# endregion

#  region ---------- FUNCTIONS ----------

# endregion

# region ---------- CLASS ----------


class EdenOSClient:
    def __init__(self, dev):
        """
        Initializes the EdenOSClient object.
        :param dev: the device that the client will handle
        """

        self.dev = dev

    def is_edenos(self):
        """
        Checks whether the device contains EdenOS.
        :return: whether the device contains EdenOS (boolean)
        """

        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.connect((IP, PORT))

        client.send(GET_MD5)

        length = struct.unpack('I', client.recv(4))[0]
        size = struct.unpack('I', client.recv(4))[0]
        md5_hash = client.recv(length - 4)
        client.close()

        print 'Verifying OS...'
        prog = UI.Progress(0)
        done = 0
        total = size

        md5 = hashlib.md5()
        lba = 0
        if is_edenfs(self.dev):
            data = self.dev.read_blocks(lba=lba, count=1, block_size=BLOCK_SIZE)
            data = data[:EDENFS_DATA_START] + \
                b'\x00' * (EDENFS_DATA_LEN) + \
                data[EDENFS_DATA_END:]
            md5.update(data)
            lba += 1
            size -= BLOCK_SIZE
            done += BLOCK_SIZE_BYTES
            prog.update(done * 100 / total)
        while size >= BLOCK_SIZE:
            md5.update(self.dev.read_blocks(lba=lba, count=1, block_size=BLOCK_SIZE))
            lba += 1
            size -= BLOCK_SIZE
            done += BLOCK_SIZE_BYTES
            prog.update(done * 100 / total)
        if size:
            md5.update(self.dev.read_blocks(lba=lba, count=1, block_size=BLOCK_SIZE)[:size])
            done += size
            prog.update(done * 100 / total)
        prog.done()
        print

        return md5.digest() == md5_hash

    def install_edenos(self):
        """
        Installs EdenOS on the device.
        :return: None
        """

        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.connect((IP, PORT))
        client.send(GET_OS)

        length = struct.unpack('I', client.recv(4))[0]

        print 'Installing OS...'
        prog = UI.Progress(0)
        done = 0
        total = length

        lba = 0
        if is_edenfs(self.dev):
            data = client.recv(RECV_LEN)
            edenfs_data = self.dev.read(EDENFS_DATA_START, EDENFS_DATA_LEN)
            data = data[:EDENFS_DATA_START] + edenfs_data + data[EDENFS_DATA_END:]
            self.dev.write_blocks(lba=lba, data=data, block_size=BLOCK_SIZE)
            lba += RECV_LEN / BLOCK_SIZE
            length -= RECV_LEN
            done += RECV_LEN
            prog.update(done * 100 / total)

        while length >= RECV_LEN:
            data = client.recv(RECV_LEN)
            self.dev.write_blocks(lba=lba, data=data, block_size=BLOCK_SIZE)
            lba += RECV_LEN / BLOCK_SIZE
            length -= RECV_LEN
            done += RECV_LEN
            prog.update(done * 100 / total)
        if length:
            data = client.recv(length)
            self.dev.write_blocks(lba=lba * BLOCK_SIZE_BYTES, data=data, block_size=1)
            done += length
            prog.update(done * 100 / total)
        prog.done()
        print

# endregion
