.PHONY:all
all:Server.exe Client.exe

Server.exe:NetCalServer.cc
	g++ -o $@ $^ -std=c++17 -ljsoncpp

Client.exe:NetCalClient.cc
	g++ -o $@ $^ -std=c++17 -ljsoncpp

.PHONY:clean
clean:
	rm -r Server.exe Client.exe