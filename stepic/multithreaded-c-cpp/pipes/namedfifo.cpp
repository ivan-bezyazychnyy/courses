
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

std::string IN_FIFO = "/home/ivan/in.fifo";
std::string OUT_FIFO = "/home/ivan/out.fifo";

char buffer[128];

int main(int argc, char ** argv) {


	if (mkfifo(IN_FIFO.c_str(), 0666) < 0) {
		std::cout << "Failed to create in.fifo" << std::endl;
		exit(1);
	}
	if (mkfifo(OUT_FIFO.c_str(), 0666) < 0) {
		std::cout << "Failed to create out.fifo" << std::endl;
		exit(1);
	}

	int fd_in = open(IN_FIFO.c_str(), O_RDONLY | O_NONBLOCK);
	if (fd_in < 0) {
		std::cout << "Failed to open in.fifo" << std::endl;
		exit(1);
	}

	int fd_out = open(OUT_FIFO.c_str(), O_WRONLY);
	if (fd_out < 0) {
		std::cout << "Failed to open out.fifo" << std::endl;
		exit(1);	
	}

	while (true) {
		int size = read(fd_in, buffer, 128);
		if (size == -1) {
			break;
		} else if (size > 0) {
			write(fd_out, buffer, size);
		}
	}

	close(fd_in);
	close(fd_out);

	return 0;
}