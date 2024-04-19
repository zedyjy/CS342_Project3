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

char mqname[32] = "mq1";

int
main(int argc, char **argv)
{
    int qid;
    char recvbuffer[MAX_DATALEN];
    int i = 0;
    int n_recv;
    int j;

    if (argc != 1) {
        printf ("usage: ./consumer\n");
        exit(1);
    }

    mf_connect();
    mf_create (mqname, 16);
    qid = mf_open(mqname);
    while (1) {
        n_recv = mf_recv(qid, (void *) recvbuffer, MAX_DATALEN);
        if (recvbuffer[0] == -1)
            break;
        // check if data received correctly
        for (j = 1; j < n_recv; ++j) {
            if (recvbuffer[j] != 'A') {
                printf ("data corruption\n");
                exit (1);
            }
        }
        printf ("data integrity check: success; size of message=%d\n", n_recv);

        i++;
        printf ("received data message %d, size=%d\n", i, n_recv);
    }
    mf_close(qid);
    mf_remove(mqname);
    mf_disconnect();
    return 0;
}