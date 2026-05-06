.PHONY:all
all:client_tcp server_tcp

server_tcp:Echo_Server_main.cc
	g++ -o $@ $^ -std=c++17
client_tcp:Echo_Client.cc
	g++ -o $@ $^ -std=c++17

.PHONY:clean
clean:
	rm -f client_tcp server_tcp