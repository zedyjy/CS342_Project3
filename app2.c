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

int totalcount = COUNT;

void test_messageflow_2p1mq();
void test_messageflow_4p2mq();


int
main(int argc, char **argv)
{
    totalcount = COUNT;
    if (argc != 2) {
        printf ("usage: app2 numberOfMessages\n");
        exit(1);
    }
    if (argc == 2)
        totalcount = atoi(argv[1]);

    srand(time(0));

    test_messageflow_2p1mq();
    //test_messageflow_4p2mq();

    return 0;
}


void test_messageflow_2p1mq()
{
    int ret1,   qid;
    char sendbuffer[MAX_DATALEN];
    int n_sent, n_received;
    char recvbuffer[MAX_DATALEN];
    int sentcount = 0;
    int receivedcount = 0;
    int i;

    mf_connect();
    mf_create ("mq1", 16); //  create mq;  size in KB

    printf("test messageflow 2 process, 1 message queue\n");

    ret1 = fork();
    if (ret1 ==  0) {
        //  process - P1
        //  will create a message queue
        mf_connect();
        qid = mf_open("mq1");
        while (1) {
            n_sent = 1 + (rand() % MAX_DATALEN);
            mf_send(qid, (void *) sendbuffer, n_sent);
            printf ("app sent message, datalen=%d\n", n_sent);
            sentcount++;
            if (sentcount == totalcount)
                break;
        }
        mf_close(qid);
        mf_disconnect();
        exit(0);
    }
    ret1  = fork();
    if (ret1 == 0) {
        //  process - P2
        mf_connect();
        qid = mf_open("mq1");
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

    for (i = 0; i < 2; ++i)
        wait(NULL);

    mf_remove("mq1");
    mf_disconnect();
}




void test_messageflow_4p2mq()
{
    int ret1, qid;
    char sendbuffer[MAX_DATALEN];
    int n_sent, n_received;
    char recvbuffer[MAX_DATALEN];
    int sentcount = 0;
    int receivedcount = 0;
    int i;

    mf_connect();
    mf_create ("mq1", 16); //  create mq;  size in KB
    mf_create ("mq2", 16); //  create mq;  size in KB

    printf("test messageflow 4 process, 2 message queue\n");
    ret1 = fork();
    if (ret1 == 0) {
        // P1
        // P1 will send
        srand(time(0));
        mf_connect();
        qid = mf_open("mq1");
        while (1) {
            n_sent = 1 + (rand() % MAX_DATALEN);
            mf_send(qid, (void *) sendbuffer, n_sent);
            printf ("app sent message, datalen=%d\n", n_sent);
            sentcount++;
            if (sentcount == totalcount)
                break;
        }
        mf_close(qid);
        mf_disconnect();
        exit(0);
    }
    ret1 = fork();
    if (ret1 == 0) {
        // P2
        // P2 will receive
        srand(time(0));
        mf_connect();
        qid = mf_open("mq1");
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


    ret1 = fork();
    if (ret1 == 0) {
        // P3
        // P3 will send
        srand(time(0));
        mf_connect();
        qid = mf_open("mq2");
        while (1) {
            n_sent = 1 + (rand() % MAX_DATALEN);
            mf_send(qid, (void *) sendbuffer, n_sent);
            printf ("app sent message, datalen=%d\n", n_sent);
            sentcount++;
            if (sentcount == totalcount)
                break;
        }
        mf_close(qid);
        mf_disconnect();
        exit(0);
    }
    ret1 = fork();
    if (ret1 == 0) {
        // P4
        // P4 will receive
        srand(time(0));
        mf_connect();
        qid = mf_open("mq2");
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

    for (i = 0; i < 4; ++i)
        wait(NULL);

    mf_remove("mq1");
    mf_remove("mq2");
    mf_disconnect();
}