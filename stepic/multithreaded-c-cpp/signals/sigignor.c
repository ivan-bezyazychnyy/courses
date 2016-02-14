

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

char * PATH = "/home/ivan/pid";

int main(int argc, char ** argv) {
	pid_t pid = getpid();
	printf("PID: %d\n", pid);
	FILE *fp = fopen(PATH, "w");
	if (fp == NULL) {
		printf("Failed to open file, error: %s\n", strerror(errno));
	}
	fprintf(fp, "%d", pid);
	fclose(fp);

	struct sigaction ignore_action;
	ignore_action.sa_handler = SIG_IGN;
	if (sigaction(SIGTERM, &ignore_action, NULL) == -1) {
		printf("Failed assign sigaction to SIGTERM, error: %s\n", strerror(errno));	
	}
	if (sigaction(SIGINT, &ignore_action, NULL) == -1) {
		printf("Failed assign sigaction to SIGINT, error: %s\n", strerror(errno));	
	}
	
	while (1) {

	}

}