#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

typedef struct thread_data {
   int result;

} thread_data;

int buffer = 0;
pthread_mutex_t lock;

void* increment(void* arg) {
    pid_t tid = syscall(__NR_gettid);
    pid_t pid = getpid();
    int increments = 15;
    thread_data *counter = (thread_data *)arg; 
    counter->result = 0;

    while (1){
        pthread_mutex_lock(&lock);

        if (increments - 1 < buffer){
            pthread_mutex_unlock(&lock);
            break;
        }

        printf("TID: %d\tPID: %d\tBuffer: %d\n", tid, pid, buffer);
        buffer += 1;
        counter->result++;

        pthread_mutex_unlock(&lock);
        sleep(0.1);
    }

    printf("TID: %d worked on the buffer %d times\n", tid, counter->result);

    return NULL;
}

int main(void) {
    int amount_of_threads = 3;
    pthread_t threads[amount_of_threads];
    thread_data res[amount_of_threads];

   if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return 1;
    }
  
    int i;
    for(i = 0; i < amount_of_threads; i++){
        pthread_create(&(threads[i]), NULL, &increment, (void *)&res[i]);
    }

    for(i = 0; i < amount_of_threads; i++){
	    pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&lock);

    int tot = 0;
    for(i = 0; i < amount_of_threads; i++){
        tot += res[i].result;
    }

    printf("Total buffer accesses: %d\n", tot);

	return 0;
}
