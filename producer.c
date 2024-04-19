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

char mqname[32] = "mq1";
int totalcount = COUNT;

int
main(int argc, char **argv)
{
    int sentcount, qid, n_sent;
    char sendbuffer[MAX_DATALEN];
    struct timespec t1;
    int j;

    totalcount = COUNT;
    if (argc != 2) {
        printf ("usage: ./producer numberOfMessages\n");
        exit(1);
    }
    if (argc == 2)
        totalcount = atoi(argv[1]);

    clock_gettime(CLOCK_REALTIME, &t1);
    srand(t1.tv_nsec);

    mf_connect();
    qid = mf_open(mqname);
    sentcount = 0;
    while (1) {
        if (sentcount < totalcount) {
            n_sent = 1 + (rand() % (MAX_DATALEN - 1));
            sendbuffer[0] = 1;
            if (n_sent > 1) {
                for (j = 1; j < n_sent; ++j)
                    sendbuffer[j] = 'A'; // just place some data/content
            }
            mf_send(qid, (void *) sendbuffer, n_sent);
            sentcount++;
            printf ("sent data message %d\n", sentcount);
        }
        else {
            sendbuffer[0] = -1;
            mf_send(qid, (void *) sendbuffer, 1);
            printf ("sent END OF DATA message\n");
            break;
        }
    }

    mf_close(qid);
    mf_disconnect();
    return 0;
}