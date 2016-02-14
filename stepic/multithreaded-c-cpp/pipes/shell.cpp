
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

std::string OUTPUT_FILE = "/home/ivan/result.out";

std::string& trim_right_inplace(std::string& s, const std::string& delimiters = " \f\n\r\t\v") {
  return s.erase(s.find_last_not_of( delimiters ) + 1 );
}

std::string& trim_left_inplace(std::string& s, const std::string& delimiters = " \f\n\r\t\v" ) {
  return s.erase(0, s.find_first_not_of( delimiters ) );
}

std::string& trim_inplace(std::string& s, const std::string& delimiters = " \f\n\r\t\v" ) {
  return trim_left_inplace(trim_right_inplace( s, delimiters ), delimiters );
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(trim_inplace(item));
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

void execute(std::string &command) {
	std::vector<std::string> cmd_parts = split(command, ' ');
	std::vector<char *> argv(cmd_parts.size() + 1);    // one extra for the null

	for (std::size_t i = 0; i != cmd_parts.size(); ++i) {
	    argv[i] = &cmd_parts[i][0];
	}

	argv[cmd_parts.size()] = NULL;

	for (int i = 0; i < argv.size(); ++i) {
		//std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
	}

	execvp(argv[0], argv.data());
}

void execute_last_command(std::string &command) {
	close(STDOUT_FILENO);
	int fd = open(OUTPUT_FILE.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd < 0) {
		std::cout << "Failed to open file " << OUTPUT_FILE << std::endl;
		exit(1);
	}
	dup2(fd, STDOUT_FILENO);
	execute(command);
	close(fd);
}

void execute_commands(std::vector<std::string> &commands, int command_index);

void execute_with_pipe(std::vector<std::string> &commands, int command_index) {
	int pfd[2];
	if (pipe(pfd) < 0) {
		std::cout << "Pipe failed" << std::endl;
		exit(1);
	}
	if (!fork()) {
		close(STDOUT_FILENO);
		dup2(pfd[1], STDOUT_FILENO);
		close(pfd[1]);
		close(pfd[0]);
		execute(commands[command_index]);
	} else {
		close(STDIN_FILENO);
		dup2(pfd[0], STDIN_FILENO);
		close(pfd[1]);
		close(pfd[0]);
		execute_commands(commands, command_index + 1);
	}
}

void execute_commands(std::vector<std::string> &commands, int command_index) {
	if (command_index == commands.size() - 1) {
		// exec right away the last command
		execute_last_command(commands[command_index]);
	} else {
		execute_with_pipe(commands, command_index);
	}
}

int main(int argc, char **argv) {
	std::string command;
	getline(std::cin, command);
	std::cout << "Executing command: " << command << std::endl;

	std::vector<std::string> commands = split(command, '|');
	for (auto i = commands.begin(); i != commands.end(); ++i) {
		std::cout << *i << std::endl;
	}

	execute_commands(commands, 0);

	return 0;
}