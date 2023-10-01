#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
class QuickChatClient
{
public:
	QuickChatClient(const int port) : port(port)
	{
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		// Create a socket
		if (sockfd == -1)
		{
			throw std::runtime_error("Failed to create socket");
		}
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(port);
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
		{
			throw std::runtime_error("Failed to connect to server");
		}
		if (epoll_fd == -1)
		{
			throw std::runtime_error("Failed to create epoll fd");
		}

		epoll_fd = epoll_create1(0);
		// add socket and stdin to epoll
		ev.events = EPOLLIN;
		ev.data.fd = sockfd;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &ev);
		ev.data.fd = STDIN_FILENO;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
	};
	~QuickChatClient()
	{
		if (sockfd != -1)
		{
			close(sockfd);
		}
		if (epoll_fd != -1)
		{
			close(epoll_fd);
		}
	};
	QuickChatClient(const QuickChatClient &) = delete;
	QuickChatClient operator=(const QuickChatClient &) = delete;
	int poll()
	{
		std::cout << "Connected to server. Type your messages and press enter to send." << std::endl;
		while (true)
		{
			epoll_event events[2];
			int num_events = epoll_wait(epoll_fd, events, 2, -1); // wait indefinitely

			for (int i = 0; i < num_events; i++)
			{
				if (events[i].data.fd == STDIN_FILENO)
				{
					std::string message;
					std::getline(std::cin, message);
					send(sockfd, message.c_str(), message.size(), 0);
				}
				else if (events[i].data.fd == sockfd)
				{
					char buffer[1024];
					ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
					if (bytes_received <= 0)
					{
						std::cerr << "Connection closed or error occurred" << std::endl;
						return -1;
					}
					buffer[bytes_received] = '\0';
					handleIncomingMessage(buffer, bytes_received);
				}
			}
		}
		return 0;
	};
	void handleIncomingMessage(const char *buffer, const ssize_t bytes_read)
	{
		std::cout << "Client received: " << buffer << std::endl;
	};

private:
	const int port;
	int sockfd = -1;
	sockaddr_in serv_addr;
	int epoll_fd = -1;
	epoll_event ev;
};

int main()
{
	QuickChatClient client(8080);
	client.poll();
	return 0;
}
