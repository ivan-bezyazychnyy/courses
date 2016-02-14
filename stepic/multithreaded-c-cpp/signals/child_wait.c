#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

char * PARENT_PID_PATH = "/home/ivan/pid_parent";
char * CHILD_PID_PATH = "/home/ivan/pid_child";

void save_pid(char * path, pid_t pid) {
	FILE *fp = fopen(path, "w");
	if (fp == NULL) {
		printf("Failed to open file, error: %s\n", strerror(errno));
	}
	fprintf(fp, "%d", pid);
	fclose(fp);
}

int main(int argc, char **argv) {
	pid_t fork_pid = fork();
	if (fork_pid) {
		printf("Parent process with pid %d is waiting for child process "
			"with pid %d to finish\n", getpid(), fork_pid);
		save_pid(PARENT_PID_PATH, getpid());
		int status;
		if (waitpid(fork_pid, &status, 0)) {
			printf("Child process finished\n");
		} else {
			printf("Failed to wait child process to finish\n");
		}
	} else {
		printf("Child process with pid %d is running\n", getpid());
		save_pid(CHILD_PID_PATH, getpid());
		while (1) {
		}
	}
}