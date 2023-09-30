#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <unordered_set>

#define MAX_EVENTS 10
class QuickChatServer
{
public:
	QuickChatServer(const int port) : port(port)
	{
		listener = socket(AF_INET, SOCK_STREAM, 0);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = INADDR_ANY;
		bind(listener, (struct sockaddr *)&addr, sizeof(addr));
		if (listen(listener, 10) == -1)
		{
			throw std::runtime_error("Failed to listen");
		}
		epoll_fd = epoll_create1(0);
		event.events = EPOLLIN;
		event.data.fd = listener;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listener, &event);
	};

	~QuickChatServer()
	{
		if (listener != -1)
		{
			close(listener);
		}
		if (epoll_fd != -1)
		{
			close(epoll_fd);
		}
		for (int client : clients)
		{
			removeConnection(client);
		}
	};
	QuickChatServer(const QuickChatServer &) = delete;
	QuickChatServer &operator=(const QuickChatServer &) = delete;

	void poll()
	{
		while (true)
		{
			int num_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

			for (int i = 0; i < num_fds; i++)
			{
				if (events[i].data.fd == listener)
				{
					// new connection
					addConnection();
				}
				else
				{
					char buffer[1024];
					ssize_t bytes_read = read(events[i].data.fd, buffer, sizeof(buffer) - 1);
					if (bytes_read <= 0)
					{
						// disconnect or error
						removeConnection(events[i].data.fd);
					}
					// null terminate the string
					buffer[bytes_read] = '\0';
					handleIncomingMessage(buffer, bytes_read, events[i].data.fd);
				}
			}
		}
	}

	void addConnection()
	{
		int client_fd = accept(listener, NULL, NULL);
		event.events = EPOLLIN;
		event.data.fd = client_fd;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
		clients.insert(client_fd);
	}

	void handleIncomingMessage(const char *buffer, ssize_t bytes_read, int clientfd)
	{
		std::cout << "Received: " << buffer << std::endl;
		// send to all clients that are not the sender
		for (int client : clients)
		{
			if (client != clientfd)
			{
				send(client, buffer, bytes_read, 0);
			}
		}
	}

	void removeConnection(int fd)
	{
		close(fd);
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
		clients.erase(fd);
	};

private:
	const int port;
	sockaddr_in addr;
	int listener = -1;
	int epoll_fd = -1;
	epoll_event event; // data structure to hold events
	epoll_event events[MAX_EVENTS];
	std::unordered_set<int> clients;
};

int main()
{
	QuickChatServer server(8080);
	server.poll();
	return 0;
}
