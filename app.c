#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "mf.h" // Include mf.h to access shared variables and function prototypes

#define SEM_NAME "/mf_semaphore"

#define COUNT 5
char *semname1 = "/semaphore1";
char *semname2 = "/semaphore2";
sem_t *sem1, *sem2;
#define MQ_NAME_MAX_LENGTH 50
char mqname1[MQ_NAME_MAX_LENGTH] = "my_queue";

int main() {
    int ret, i, qid;

    // Initialize semaphores
    sem1 = sem_open(semname1, O_CREAT, 0666, 0);
    sem2 = sem_open(semname2, O_CREAT, 0666, 0);
    mutex = sem_open(SEM_NAME, O_CREAT, 0666, 1); // Initialize mutex semaphore

    ret = fork();
    if (ret > 0) {
        // parent process - P1
        char *bufptr = (char *) malloc (MAX_DATALEN);
        sem1 = sem_open(semname1, 0);
        sem2 = sem_open(semname2, 0);
        mf_connect();
        mf_create (mqname1, 16); // Use mqname1 here

        qid = mf_open(mqname1);
        printf("\nanan5");

        sem_wait(mutex); // Acquire mutex before accessing shared resources
        for (i = 0; i < COUNT; ++i) {
            sprintf (bufptr, "%s-%d", "MessageData", i);
            mf_send (qid, (void *) bufptr, strlen(bufptr)+1);
        }
        sem_post(mutex); // Release mutex after accessing shared resources

        free (bufptr);
        mf_close(qid);
        mf_disconnect();
        sem_wait(sem2);
        mf_remove(mqname1);
    }
    else if (ret == 0) {
        // child process - P2
        char *bufptr = (char *) malloc (MAX_DATALEN);
        sem1 = sem_open(semname1, 0);
        sem2 = sem_open(semname2, 0);

        sem_wait(mutex); // Acquire mutex before accessing shared resources
        mf_connect();
        qid = mf_open(mqname1);
        for (i = 0; i < COUNT; ++i) {
            mf_recv (qid, (void *) bufptr, MAX_DATALEN);
            printf ("%s\n", bufptr);
        }
        sem_post(mutex); // Release mutex after accessing shared resources

        free (bufptr);
        mf_close(qid);
        mf_disconnect();
        sem_post(sem2);
    }
    return (0);
}