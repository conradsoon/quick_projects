CXX = g++
CXXFLAGS = -Wall -std=c++20

all: tcp_client tcp_server

tcp_client: tcp_client.cpp
	$(CXX) $(CXXFLAGS) -o tcp_client tcp_client.cpp

tcp_server: tcp_server.cpp
	$(CXX) $(CXXFLAGS) -o tcp_server tcp_server.cpp

clean:
	rm -f tcp_client tcp_server
