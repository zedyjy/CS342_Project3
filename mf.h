// mf.h - Header file

#ifndef MF_H
#define MF_H

struct MF {
    // Declare any necessary member variables here
};

typedef struct MF MF;

int mf_init();
int mf_destroy();
int mf_connect();
int mf_disconnect();
int mf_create(char *mqname, int mqsize);
int mf_remove(char *mqname);
int mf_open(char *mqname);
int mf_close(int qid);
int mf_send(int qid, void *bufptr, int datalen);
int mf_recv(int qid, void *bufptr, int bufsize);
int mf_print();

#endif // MF_H
