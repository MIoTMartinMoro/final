#!/usr/bin/python3
"""
Generic Python3 server that can be ran on the CI40 in order to use with the examples which use
UDP client code.
"""
import argparse
import socket
import time

parser = argparse.ArgumentParser()
parser.add_argument("-p", "--port", help="Port to bind to", default=3001, type=int)
parser.add_argument("-b", "--buffer", help="Buffer size", default=256, type=int)
args = parser.parse_args()

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
    msg = "[{}] ({}) OP: {} ID: {} Len: {} Data: {}".format(time.strftime("%c"), sentido, hex(int(op, base=16)), id, len, data)
    log_file.write("{}\n".format(msg))
    print(msg)

def asignar_mesa(id_mesa):
    camarero_asignado = mesas_camareros[0]
    for camarero in mesas_camareros:
        if len(camarero) < len(camarero_asignado):  # Si un camarero tiene menos mesas se le asigna
            camarero_asignado = camarero
    camarero_asignado.append(id_mesa)
    id_camarero_asignado = camarero_asignado[0]

# No IP to connect to needed for a server
IP = "::"
PORT = args.port

# Creates a socket using IPV6 and accepting datagrams
sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
sock.bind((IP, PORT))

id_ir = 1
id_mesa = 1
id_pulsera = 1
mesas_camareros = []  # Array de arrays con la relación mesa-camarero, el primer elemento es el id de la pulsera y el resto de sus mesas (IMPORTANTE mantener el prefijo en el id)

log_file = open("restaurante.log", "a")

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
    if (op == 1):  # OP_WHOAMI_MESA
        id_msg = [data[2], data[3]]
        msg = build_msg(id_msg, str(id_mesa), False)
        id_mesa += 1
        time.sleep(1)
        sock.sendto(msg, address)
    elif (op == 4):  # OP_MESA_OCUPADA
    elif (op == 5):  # OP_MESA_VACIA
    elif (op == 6):  # OP_WHOAMI_IR
        id_msg = [data[2], data[3]]
        msg = build_msg(id_msg, str(id_ir), False)
        id_ir += 4  # Suma de 4 en 4 porque a cada clicker se le conectan 4 sensores IR
        time.sleep(1)
        sock.sendto(msg, address)
    elif (op == 9):  # OP_WHOAMI_PULSERA
        id_msg = [data[2], data[3]]
        msg = build_msg(id_msg, str(id_pulsera), False)
        mesas_camareros.append([id_pulsera])
        id_pulsera += 1
        time.sleep(1)
        sock.sendto(msg, address)
    elif (op > 12):
        id_msg = [data[2], data[3]]
        msg = build_msg(id_msg, "Operacion no identificada", True)
        time.sleep(1)
        sock.sendto(msg, address)

log_file.close()
