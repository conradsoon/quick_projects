#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
	int sock = 0;
	struct sockaddr_in serv_addr;
	char buffer[BUFFER_SIZE] = {0};

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cerr << "Socket creation failed." << std::endl;
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
	{
		std::cerr << "Invalid address or Address not supported." << std::endl;
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		std::cerr << "Connection failed." << std::endl;
		return -1;
	}

	std::cout << "Connected to server." << std::endl;
	while (true)
	{
		std::cout << "Client: ";
		std::cin.getline(buffer, BUFFER_SIZE);
		send(sock, buffer, strlen(buffer), 0);

		memset(buffer, 0, BUFFER_SIZE);
		int valread = read(sock, buffer, BUFFER_SIZE);
		if (valread == 0)
		{
			std::cout << "Server disconnected." << std::endl;
			break;
		}
		std::cout << "Server: " << buffer << std::endl;
	}

	close(sock);
	return 0;
}
