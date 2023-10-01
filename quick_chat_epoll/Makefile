# Compiler settings
CXX=g++
CXXFLAGS=-std=c++17 -Wall

# Targets
all: quick_chat_client quick_chat_server

quick_chat_client: quick_chat_client.cpp
	$(CXX) $(CXXFLAGS) -o quick_chat_client quick_chat_client.cpp

quick_chat_server: quick_chat_server.cpp
	$(CXX) $(CXXFLAGS) -o quick_chat_server quick_chat_server.cpp

clean:
	rm -f quick_chat_client quick_chat_server
