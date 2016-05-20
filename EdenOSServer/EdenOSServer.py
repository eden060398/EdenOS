# region ----------IMPORTS----------
import socket
import threading
import hashlib
import struct
import os
# endregion

# region ----------CONSTANTS----------
IP = '0.0.0.0'
PORT = 6398
LISTEN_NUM = 10
MAX_CLIENTS = 100

KERNEL_FILENAME = 'kernel32.img'

KEEP_ALIVE = '\x00'

READ_SIZE = 1024
# endregion

# region ----------FUNCTIONS----------


def create_server():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind((IP, PORT))
    sock.listen(LISTEN_NUM)

    return sock


def simple_response(data):
    return '{len}{data}'.format(len=struct.pack('I', len(data)), data=data)


def keep_alive(client):
    client.send(simple_response(KEEP_ALIVE))


def send_md5(client):
    size = os.path.getsize(KERNEL_FILENAME)
    kernel = open(KERNEL_FILENAME, 'rb')

    data = struct.pack('I', size)
    md5 = hashlib.md5()
    while size >= READ_SIZE:
        md5.update(kernel.read(READ_SIZE))
        size -= READ_SIZE
    if size:
        md5.update(kernel.read(size))
    kernel.close()
    data += md5.digest()
    client.send(simple_response(data))


def send_os(client):
    size = os.path.getsize(KERNEL_FILENAME)
    kernel = open(KERNEL_FILENAME, 'rb')

    client.send(struct.pack('I', size))
    while size >= READ_SIZE:
        client.send(kernel.read(READ_SIZE))
        size -= READ_SIZE
    if size:
        client.send(kernel.read(size))
    kernel.close()


def handle_client(client, addr):

    try:
        msg = client.recv(1)
        if msg:
            req = ord(msg)
            if 0 < req < len(REQ):
                REQ[req](client)
                if REQ_MSG[req]:
                    print '\t---> {ip}:{port} {msg}'.format(ip=addr[0], port=addr[1], msg=REQ_MSG[req])
            else:
                print '\t---> {ip}:{port} HAS SENT AN ILLEGAL REQUEST. CONNECTION TERMINATED.'.format(ip=addr[0], port=addr[1])
    except socket.error:
        '\t---> {ip}:{port} HAS TERMINATED THE CONNECTION.'.format(ip=addr[0], port=addr[1])
    client.close()
    print '\t\t---> THE CONNECTION WITH {ip}:{port} WAS TERMINATED.\n'.format(ip=addr[0], port=addr[1])
# endregion

# region ----------MAIN----------


def main():
    global REQ
    global REQ_MSG
    REQ = [keep_alive, send_md5, send_os]
    REQ_MSG = [None, 'HAS REQUESTED THE KERNEL\'S MD5 HASH.', 'HAS DOWNLOADED THE OS.']

    server = create_server()
    while True:
        if True:
            client, addr = server.accept()
            print "---> {ip}:{port} CONNECTED.".format(ip=addr[0], port=addr[1])
            threading.Thread(target=handle_client, args=(client, addr)).start()

if __name__ == '__main__':
    main()
# endregion
