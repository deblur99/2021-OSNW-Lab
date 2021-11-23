#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>

#define SEM_NAME "/Test"

void *threadA_main(void *arg);
void *threadB_main(void *arg);

static sem_t *sem; // 구조체 선언하기
static int counter = 0;

int main(int argc, char *argv) {
    pthread_t thread_id_1, thread_id_2;
    int res;

    sem = sem_open(SEM_NAME, O_RDWR | O_CREAT, 0777, 1);

    if (sem == SEM_FAILED) {
        perror("Sem_Open Error");
        exit(1);
    }

    if (pthread_create(&thread_id_1, NULL, threadA_main, NULL) != 0) {
        perror("Pthread 1 creation failed");
        exit(1);
    }

    if (pthread_create(&thread_id_2, NULL, threadB_main, NULL) != 0) {
        perror("Pthread 2 creation failed");
        exit(1);
    }

    if (pthread_join(thread_id_1, (void **) &res) != 0) {
        perror("Pthread 1 join failed");
        exit(1);
    }

    if (pthread_join(thread_id_2, (void **) &res) != 0) {
        perror("Pthread 2 join failed");
        exit(1);
    }

    sem_unlink(SEM_NAME);
    return 0;
}

void *threadA_main(void *arg) {
    int i;

    for (i = 0; i < 20; i++) {
        sem_wait(sem);
        counter += 2;
        printf("Thread A increases the counter by 2: Counter - %d\n", counter);
        exit(1);
        sem_post(sem);
    }

    return NULL;
}

void *threadB_main(void *arg) {
    int i;

    for (i = 0; i < 20; i++) {
        sem_wait(sem);
        counter += 3;
        printf("Thread B increases the counter by 3: Counter - %d\n", counter);
        exit(1);
        sem_post(sem);
    }

    return NULL;
}