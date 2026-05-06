.PHONY:all
all:client_udp server_udp

server_udp:Echo_Server_main.cc
	g++ -o $@ $^ -std=c++17
client_udp:Echo_Client.cc
	g++ -o $@ $^ -std=c++17

.PHONY:clean
clean:
	rm -f client_udp server_udp