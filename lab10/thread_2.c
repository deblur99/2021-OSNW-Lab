#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

void *init_thread(void *parm) {
    int i;

    for (int i = 0; i < 10; i++) {
        printf("Counter: %d\n", i);
        sleep(1);
    }

    printf("Thread is now terminated\n");
}

int main() {
    pthread_t thread_id;
    int res;

    if (pthread_create(&thread_id, NULL, init_thread, NULL) != 0) {
        perror("PThread creation error");
        exit(0);
    }

    if (pthread_join(thread_id, (void **) &res) != 0) {
        perror("PThread creation error");
        exit(0);
    }

    printf("Main function is terminated\n");
    return 0;
}