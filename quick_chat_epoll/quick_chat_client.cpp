#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

int main()
{
	// Create a socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		std::cerr << "Failed to create socket" << std::endl;
		return -1;
	}

	// Define the server address
	sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8080);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	// Connect to the server
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
	{
		std::cerr << "Failed to connect to server" << std::endl;
		close(sockfd);
		return -1;
	}

	std::cout << "Connected to server. Type your messages and press enter to send. Type 'exit' to quit." << std::endl;

	// Main loop for client input
	std::string message;
	while (true)
	{
		std::getline(std::cin, message);

		// If the user types 'exit', break the loop
		if (message == "exit")
		{
			break;
		}

		// Send the user's input to the server
		send(sockfd, message.c_str(), message.size(), 0);

		// Receive a response from the server
		char buffer[1024];
		ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
		if (bytes_received > 0)
		{
			buffer[bytes_received] = '\0';
			std::cout << "Server replied: " << buffer << std::endl;
		}
	}

	// Clean up and close the socket
	close(sockfd);

	return 0;
}
