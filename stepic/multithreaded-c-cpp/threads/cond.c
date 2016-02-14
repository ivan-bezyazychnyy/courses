
#include <stdio.h>

#include <sys/types.h>
#include <unistd.h>

#include <string.h>
#include <errno.h>

#include <pthread.h>

const char * PIDFILE = "/home/box/main.pid";

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_barrier_t barrier;

void * thread_cond_wait(void * arg) {
	pthread_cond_wait(&cond, &mutex);
}

void * thread_barrier_wait(void * arg) {
	pthread_barrier_wait(&barrier);
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

	pthread_barrier_init(&barrier, NULL, 2);


	pthread_t thread_id;
	if (pthread_create(&thread_id, NULL, &thread_cond_wait, NULL) < 0) {
		printf("Failed start thread: %s\n", strerror(errno));
		goto exit;
	}
	if (pthread_create(&thread_id, NULL, &thread_barrier_wait, NULL) < 0) {
		printf("Failed start thread: %s\n", strerror(errno));
		goto exit;
	}
	
	printf("Waiting for created thread\n");
	pthread_join(thread_id, NULL);

exit:
	printf("Finished\n");
}