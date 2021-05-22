# tcp_proxy
TCP proxy server example, based on io_uring

Compilation:
	mkdir build
	cd build
	cmake ..
	make

Test (run in different terminals):
	./tcp_proxy -l 127.0.0.1:8888 -d 127.0.0.1:2021
	./server.py
	./start_clients.sh

Test results:
	direct connections - 60.7 MB/s
	proxy              - 36.7 MB/s
