#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

#define MAX_EVENTS 10

int main()
{
	int listener = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080);
	addr.sin_addr.s_addr = INADDR_ANY;
	bind(listener, (struct sockaddr *)&addr, sizeof(addr));

	listen(listener, 10);

	int epoll_fd = epoll_create1(0);
	epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = listener;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listener, &event);

	epoll_event events[MAX_EVENTS];
	while (true)
	{
		int num_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

		for (int i = 0; i < num_fds; i++)
		{
			if (events[i].data.fd == listener)
			{
				// New connection
				int client_fd = accept(listener, NULL, NULL);
				event.events = EPOLLIN;
				event.data.fd = client_fd;
				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
			}
			else
			{
				// Incoming message
				char buffer[1024];
				ssize_t bytes_read = read(events[i].data.fd, buffer, sizeof(buffer) - 1);
				if (bytes_read <= 0)
				{
					// Disconnection or error
					close(events[i].data.fd);
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
				}
				else
				{
					buffer[bytes_read] = '\0';
					std::cout << "Received: " << buffer << std::endl;
					// Handle the message...
				}
			}
		}
	}

	return 0;
}
