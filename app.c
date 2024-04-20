#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <time.h>
#include "mf.h"

#define COUNT 10
char *semname1 = "/semaphore1";
char *semname2 = "/semaphore2";
sem_t *sem1, *sem2;
char *mqname1 = "msgqueue1";

int
main(int argc, char **argv)
{
    int ret,  i, qid;
    char sendbuffer[MAX_DATALEN];
    int n_sent, n_received;
    char recvbuffer[MAX_DATALEN];
    int sentcount;
    int receivedcount;
    int totalcount;

    totalcount = COUNT;
    if (argc == 2)
        totalcount = atoi(argv[1]);

    sem1 = sem_open(semname1, O_CREAT, 0666, 0); // init sem
    sem2 = sem_open(semname2, O_CREAT, 0666, 0); // init sem

    srand(time(0));
    printf ("RAND_MAX is %d\n", RAND_MAX);

    ret = fork();
    if (ret > 0) {
        // parent process - P1
        // parent will create a message queue

        mf_connect();

        mf_create (mqname1, 16); //  create mq;  16 KB

        qid = mf_open(mqname1);

        sem_post (sem1);

        while (1) {
            n_sent = 1 + (rand() % MAX_DATALEN);
            ret = mf_send (qid, (void *) sendbuffer, n_sent);
            printf ("app sent message, datalen=%d\n", n_sent);
            sentcount++;
            if (sentcount == totalcount)
                break;
        }
        mf_close(qid);
        sem_wait(sem2);
        // we are sure other process received the messages

        mf_remove(mqname1);   // remove mq
        mf_disconnect();
    }
    else if (ret == 0) {
        // child process - P2
        // child will connect, open mq, use mq
        sem_wait (sem1);
        // we are sure mq was created

        mf_connect();

        qid = mf_open(mqname1);

        while (1) {
            n_received =  mf_recv (qid, (void *) recvbuffer, MAX_DATALEN);
            printf ("app received message, datalen=%d\n", n_received);
            receivedcount++;
            if (receivedcount == totalcount)
                break;
        }
        mf_close(qid);
        mf_disconnect();
        sem_post(sem2);
    }
    return 0;
}