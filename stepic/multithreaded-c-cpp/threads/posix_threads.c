
#include <stdio.h>

#include <sys/types.h>
#include <unistd.h>

#include <string.h>
#include <errno.h>

#include <pthread.h>

const char * PIDFILE = "/home/ivan/main.pid";

void * thread_func(void * arg) {
	printf("Thread started\n");
	sleep(10);
	printf("Thread finished\n");
}

int main(int argc, char **argv) {
	pid_t pid = getpid();
	printf("Started with pid %d\n", pid);

	FILE * file = fopen(PIDFILE, "w+");
	if (file == NULL) {
		printf("Failed to open file: %s\n", strerror(errno));
		goto exit;
	}

	fprintf(file, "%d", pid);
	fclose(file);


	pthread_t thread_id;
	if (pthread_create(&thread_id, NULL, &thread_func, NULL) < 0) {
		printf("Failed start thread: %s\n", strerror(errno));
		goto exit;
	}
	printf("Waiting for created thread\n");
	pthread_join(thread_id, NULL);

exit:
	printf("Finished\n");
}