
#include <stdio.h>

#include <sys/types.h>
#include <unistd.h>

#include <string.h>
#include <errno.h>

#include <pthread.h>

const char * PIDFILE = "/home/box/main.pid";

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
pthread_spinlock_t spinlock;

void * thread_mutex_wait(void * arg) {
	pthread_mutex_lock(&mutex);
}

void * thread_mutex_wait_spin(void * arg) {
	pthread_spin_lock(&spinlock);
}

void * thread_mutex_wait_read(void * arg) {
	pthread_rwlock_rdlock(&rwlock);
}

void * thread_mutex_wait_write(void * arg) {
	pthread_rwlock_wrlock(&rwlock);
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

	// lock all mutexes
	pthread_mutex_lock(&mutex);
	pthread_rwlock_wrlock(&rwlock);
	pthread_spin_init(&spinlock, 0);
	pthread_spin_lock(&spinlock);


	pthread_t thread_id;
	if (pthread_create(&thread_id, NULL, &thread_mutex_wait, NULL) < 0) {
		printf("Failed start thread: %s\n", strerror(errno));
		goto exit;
	}
	if (pthread_create(&thread_id, NULL, &thread_mutex_wait_spin, NULL) < 0) {
		printf("Failed start thread: %s\n", strerror(errno));
		goto exit;
	}
	if (pthread_create(&thread_id, NULL, &thread_mutex_wait_read, NULL) < 0) {
		printf("Failed start thread: %s\n", strerror(errno));
		goto exit;
	}
	if (pthread_create(&thread_id, NULL, &thread_mutex_wait_write, NULL) < 0) {
		printf("Failed start thread: %s\n", strerror(errno));
		goto exit;
	}
	printf("Waiting for created thread\n");
	pthread_join(thread_id, NULL);

exit:
	printf("Finished\n");
}