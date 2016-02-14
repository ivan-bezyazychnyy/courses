#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 12345


int main(int argc, char **argv) {
	std::cout << "Hello, World!" << std::endl;

	int listen_socket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(listen_socket, (struct sockaddr *) &sa, sizeof(sa));

	listen(listen_socket, SOMAXCONN);

	int client_socket = accept(listen_socket, NULL, NULL);

	char buffer[1024] = {0};
	while (1) {
		int read = recv(client_socket, buffer, 1024, MSG_NOSIGNAL);
		if (read <= 0) {
			break;
		}
		buffer[read] = '\0';
		std::cout << "Received " << read << " bytes: " << buffer << std::endl;
		
		send(client_socket, buffer, read, MSG_NOSIGNAL);
	}

	shutdown(client_socket, SHUT_RDWR);
	shutdown(listen_socket, SHUT_RDWR);

	return 0;
}