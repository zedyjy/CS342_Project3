#ifndef MF_H
#define MF_H

#include <semaphore.h> // Include semaphore header

// Configuration file name
#define CONFIG_FILENAME "config.txt"

// Maximum number of message queues
#define MAX_MQS 10

// Maximum length of a message queue name
#define MAX_MQ_NAME_LENGTH 50

// Maximum length of message data
#define MAX_MQ_DATA_LENGTH 100

// Non-configurable parameters
#define MIN_DATALEN 1
#define MAX_DATALEN MAX_MQ_DATA_LENGTH
#define MAX_MQNAME_SIZE MAX_MQ_NAME_LENGTH
#define MIN_MQSIZE 1
#define MAX_MQSIZE (MAX_MQ_DATA_LENGTH / 1024) // Convert from bytes to KB
#define MIN_SHMEMSIZE 1
#define MAX_SHMEMSIZE 1024 // In MB

// Configurable parameters
#define SHMEM_NAME "SHMEM_NAME"
#define SHMEM_SIZE "SHMEM_SIZE"
#define MAX_MSGS_IN_QUEUE "MAX_MSGS_IN_QUEUE"
#define MAX_QUEUES_IN_SHMEM "MAX_QUEUES_IN_SHMEM"

// Define global semaphore mutex
extern sem_t *mutex;

// Function prototypes
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
