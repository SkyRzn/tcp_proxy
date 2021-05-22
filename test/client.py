#!/usr/bin/python3


import socket, os, time


host = '127.0.0.1'
port = 8888 #2021
block_size = 65536
block_snd = os.urandom(block_size)
count = 1024

def main():
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.connect((host, port))

	start = time.time()
	for i in range(count):
		sock.send(block_snd)

		block_rcv = b''
		while (len(block_rcv) < block_size):
			block_rcv += sock.recv(block_size)

		if block_snd != block_rcv:
			print('Corrupted data!')

	end = time.time()
	dt = end - start
	mbs = block_size * 2 * count / 1024 / 1024

	print('%.2f MB/s' % (mbs/dt))

	sock.close()

main()
