#!/usr/bin/python3
"""
Generic Python3 server that can be ran on the CI40 in order to use with the examples which use
UDP client code.
"""
import argparse
parser = argparse.ArgumentParser()
parser.add_argument("-p", "--port", help="Port to bind to", default=3001, type=int)
parser.add_argument("-b", "--buffer", help="Buffer size", default=256, type=int)
args = parser.parse_args()

import socket
import time

# No IP to connect to needed for a server
IP = "::"
PORT = args.port

# Creates a socket using IPV6 and accepting datagrams
sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
sock.bind((IP, PORT))

id_ir = 1

print("Server initialised, awaiting data. Use Ctrl + C to stop")
while True:
    data, address = sock.recvfrom(args.buffer)
    op = data[1] * 256 + data[0]
    id_msg = data[3] * 256 + data[2]
    len_data = data[4]
    data_msg = data[5:]

    print("OP:" + str(op))
    print("ID:" + str(id_msg))
    print("LEN:" + str(len_data))
    print("DATA:" + str(data_msg))
    # Print client data as it arrives. For unpacking the data or converting to a UTF-8 string see below:
    # https://docs.python.org/3.5/library/struct.html                                                   
    # https://docs.python.org/3/howto/unicode.html#the-string-type
    msg = []
    if (op == 4):
        msg.append(0xee)
        msg.append(0xee)
        msg.append(data[2])
        msg.append(data[3])
        msg.append(id_ir)
        msg.append(len(str(id_ir)))
        id_ir += 4
    time.sleep(1)
    sock.sendto(bytes(msg), address) 