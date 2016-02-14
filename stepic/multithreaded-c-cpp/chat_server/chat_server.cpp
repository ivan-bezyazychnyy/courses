
#include <iostream>
#include <sstream>
#include <algorithm>
#include <set>
#include <map>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>	
#include <fcntl.h>

#define PORT 12345

std::set<int> slave_sockets;
std::map<int, std::string> clients_info;

std::string get_client_info(int fd) {
	std::stringstream sstream;
	if (clients_info.count(fd) > 0) {
		sstream << "Client(descr=" << fd << ", addr=" << clients_info[fd] << ")";
	} else {
		sstream << "Client(descr=" << fd << ", no addr info";
	}
	return sstream.str();
}

std::string merge_client_info_with_msg(int fd, char buffer[], int size) {
	std::stringstream sstream;
	sstream << get_client_info(fd) << ": " << std::string(buffer, size);
	return sstream.str();
}

int set_nonblock(int fd) {
	int flags;
#if defined(O_NONBLOCK)
	if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
		flags = 0;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
	flags = 1;
	return ioctl(fd, FIOBIO, &flags);
#endif
} 

int create_and_start_master_socket() {
	int master_socket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(master_socket, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		return -1;
	}

	set_nonblock(master_socket);

	listen(master_socket, SOMAXCONN);

	return master_socket;
}


int main(int argc, char **argv) {
	std::cout << "Chat server started." << std::endl;

	

	int master_socket = create_and_start_master_socket();
	if (master_socket <= 0) {
		std::cout << "Failed to start listening for clients." << std::endl;
		goto exit;
	}

	while (true) {
		fd_set fd_set;
		FD_ZERO(&fd_set);
		FD_SET(master_socket, &fd_set);
		for (auto iter = slave_sockets.begin(); iter != slave_sockets.end(); ++iter) {
			FD_SET(*iter, &fd_set);
		}

		int max_slave_fd = -1;
		if (!slave_sockets.empty()) {
			max_slave_fd = *slave_sockets.rbegin();
		}
		int max_fd = std::max(master_socket, max_slave_fd);
		select(max_fd + 1, &fd_set, NULL, NULL, NULL);

		if (FD_ISSET(master_socket, &fd_set)) {
			struct sockaddr_in client_addr;
			socklen_t client_addr_len;
			int slave_socket = accept(master_socket, (struct sockaddr*) &client_addr, &client_addr_len);
			int client_port = ntohs(client_addr.sin_port);
			static char ipstr[17] = {0};
			const char * result = inet_ntop(AF_INET, &(client_addr.sin_addr), ipstr, sizeof(ipstr));
			if (result == NULL) {
				std::cout << "Cannot parse ip address of incoming connection" << std::endl;
			} else {
				std::stringstream sstream;
				sstream << result << ":" << client_port;
				std::string client_info = sstream.str();
				std::cout << "New connection: " << client_info << std::endl;
				clients_info[slave_socket] = client_info;
			}

			set_nonblock(slave_socket);
			slave_sockets.insert(slave_socket);
		}

		for (auto iter = slave_sockets.begin(); iter != slave_sockets.end(); /* move iter manually */) {
			bool iter_moved = false;
			int slave_socket = *iter;
			std::cout << "Checking client " << slave_socket << std::endl;
			if (FD_ISSET(slave_socket, &fd_set)) {
				static char buffer[1024];
				int attempts = 0;
				int read  = recv(slave_socket, buffer, 1024, MSG_NOSIGNAL);
				//std::cout << "Received " << read << " bytes from " << slave_socket << std::endl;
				bool client_discontected = false;
				if (read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
					// should retry
					client_discontected = true;
					std::cout << "Client " << slave_socket
					          << " timed out. Should try to read again."
					          << std::endl;
				} else if (read <= 0) {
					client_discontected = true;
				}
				if (client_discontected) {
					std::cout << "Client " << slave_socket << " discontected."
					           << std::endl;
					//FD_CLR(slave_socket, &fd_set);
					shutdown(slave_socket, SHUT_RDWR);
					close(slave_socket);
					clients_info.erase(slave_socket);
					iter = slave_sockets.erase(iter);
					iter_moved = true;
				} else {
					for (auto iter_to_send = slave_sockets.begin();
						iter_to_send != slave_sockets.end();
						++iter_to_send) {
						
						std::string msg = merge_client_info_with_msg(slave_socket, buffer, read);
						send(*iter_to_send, msg.c_str(), msg.length() + 1, MSG_NOSIGNAL);
					}
				}
			}
			if (!iter_moved) {
				++iter;	
			}
		}
	}


exit:
    if (master_socket > 0) {
    	shutdown(master_socket, SHUT_RDWR);
    	close(master_socket);
    }

	return 0;
}