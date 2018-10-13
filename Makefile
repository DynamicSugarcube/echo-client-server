all: client server

clean:
	rm -rf client server *.o
	
client: client.cpp connection.h
	g++ -o client client.cpp
	
server: server.cpp connection.h
	g++ -o server -std=c++11 server.cpp
