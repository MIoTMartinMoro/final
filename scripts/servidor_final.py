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

def build_msg(id, data, error):
    msg_bytes = bytes()
    op = 0
    if (error):
        msg_bytes += bytes([0xff, 0xff])
        op = "ffff"
    else:
        msg_bytes += bytes([0xee, 0xee])
        op = "eeee"
    msg_bytes += bytes([id[0], id[1], len(data)]) + bytes(data, 'utf-8')
    id_msg = id[1] * 256 + id[0]
    write_msg(op, id_msg, len(data), data, "Enviado")

    return msg_bytes

def write_msg(op, id, len, data, sentido):
    print("[{}] ({}) OP: {} ID: {} Len: {} Data: {}".format(time.strftime("%c"), sentido, hex(int(op, base=16)), id, len, data))

# No IP to connect to needed for a server
IP = "::"
PORT = args.port

# Creates a socket using IPV6 and accepting datagrams
sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
sock.bind((IP, PORT))

id_ir = 1
id_mesa = 1

print("Server initialised, awaiting data. Use Ctrl + C to stop")
while True:
    data, address = sock.recvfrom(args.buffer)
    if (data[0] == 'dummy'):
        continue
    op = data[1] * 256 + data[0]
    id_msg = data[3] * 256 + data[2]
    len_data = data[4]
    data_msg = data[5:]

    write_msg(str(op), str(id_msg), str(len_data), data_msg, "Recibido")
    # Print client data as it arrives. For unpacking the data or converting to a UTF-8 string see below:
    # https://docs.python.org/3.5/library/struct.html                                                   
    # https://docs.python.org/3/howto/unicode.html#the-string-type
    msg = bytes()
    if (op == 4):
        id_msg = [data[2], data[3]]
        msg = build_msg(id_msg, str(id_ir), False)
        id_ir += 4
        time.sleep(1)
        sock.sendto(msg, address)
    elif (op == 1):
        id_msg = [data[2], data[3]]
        msg = build_msg(id_msg, str(id_mesa), False)
        id_mesa += 1
        time.sleep(1)
        sock.sendto(msg, address)
    elif (op > 10):
        id_msg = [data[2], data[3]]
        msg = build_msg(id_msg, "Operacion no identificada", True)
        time.sleep(1)
        sock.sendto(msg, address)
