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
#include <sys/types.h>
#include <sys/wait.h>
#include "mf.h"

#define COUNT 10

char *mqname1 = "msgqueue1";

int
main(int argc, char **argv)
{
    int ret, qid;
    char sendbuffer[MAX_DATALEN];
    int n_sent, n_received;
    char recvbuffer[MAX_DATALEN];
    int sentcount = 0;
    int receivedcount = 0;
    int totalcount;

    totalcount = COUNT;
    if (argc == 2)
        totalcount = atoi(argv[1]);

    srand(time(0));

    mf_connect();
    mf_create (mqname1, 16); //  create mq;  16 KB

    ret = fork();
    if (ret > 0) {
        // parent process - P1

        qid = mf_open(mqname1);

        while (1) {
            n_sent = 1 + (rand() % MAX_DATALEN);
            ret = mf_send (qid, (void *) sendbuffer, n_sent);
            printf ("app sent message, datalen=%d\n", n_sent);
            sentcount++;
            if (sentcount == totalcount)
                break;
        }
        mf_close(qid);

        wait(NULL); // wait for child to terminate

        mf_remove(mqname1);   // remove mq
        mf_disconnect();
        exit(0);
    }
    else if (ret == 0) {
        // child process - P2

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
        exit(0);
    }
    return 0;
}