#!/usr/bin/python3


import socket
from threading import Thread


host = '0.0.0.0'
port = 2021
block_size = 65536

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind((host, port))
threads = []


class ClientThread(Thread):
	def __init__(self, conn, host, port):
		Thread.__init__(self)
		self.conn = conn
		print('New connection from %s:%d' % (host, port))

	def run(self):
		while True :
			block = self.conn.recv(block_size)
			self.conn.send(block)


def main():
	print('Server started')
	while True:
		sock.listen(10)
		(conn, (host, port)) = sock.accept()
		thread = ClientThread(conn, host, port)
		thread.start()
		threads.append(thread)

	for thread in threads:
		thread.join()

main()
