#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>

int main(int argc, char ** argv) {
	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
		std::cout << "Failed to create a socketpair" << std::endl;
		exit(1);
	}

	if (!fork()) {
		// parent
		while (true) {

		}
	} else {
		struct sockaddr_in client_addr;
		socklen_t client_addr_len;
		accept(sv[1], (struct sockaddr*) &client_addr, &client_addr_len);
		while (true) {
			
		}
	}

	return 0;
}