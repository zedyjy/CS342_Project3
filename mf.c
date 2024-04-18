#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <string.h>
#include "mf.h"

#define CONFIG_FILENAME "config.txt"
#define MAX_MQS 10
#define MAX_MQ_NAME_LENGTH 50
#define SHM_SIZE sizeof(struct shared_memory)
#define MAX_MQ_DATA_LENGTH 100
#define SEM_NAME "/mf_semaphore"
#define SHARED_MEMORY_KEY 12345 // Define a constant key for shared memory

sem_t *mutex;

// Define structures
struct message_queue {
    char name[MAX_MQ_NAME_LENGTH];
    char data[MAX_MQ_DATA_LENGTH];
    int size;
};

struct shared_memory {
    int num_queues;
    struct message_queue queues[MAX_MQS];
};

// Global variables
int shmid;
struct shared_memory *shm_ptr;

// Function prototypes
int mf_init();
int mf_create(char *mqname, int mqsize);
int mf_remove(char *mqname);
int mf_open(char *mqname);
int mf_close(int qid);
int mf_send(int qid, void *bufptr, int datalen);
int mf_recv(int qid, void *bufptr, int bufsize);

// Function definitions
int mf_init() {
    printf("You're now in mf_init()\n");
    // Read configuration file and populate shared memory
    FILE *config_file = fopen(CONFIG_FILENAME, "r");
    if (config_file == NULL) {
        perror("fopen");
        return -1;
    }

    // Generate key for shared memory
    key_t shm_key = ftok(".", SHARED_MEMORY_KEY);
    if (shm_key == -1) {
        perror("ftok");
        fclose(config_file);
        return -1;
    }

    // Allocate shared memory
    if ((shmid = shmget(shm_key, SHM_SIZE, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        fclose(config_file);
        return -1;
    }

    // Attach to shared memory
    if ((shm_ptr = (struct shared_memory *)shmat(shmid, NULL, 0)) == (struct shared_memory *)-1) {
        perror("shmat");
        fclose(config_file);
        return -1;
    }

    // Initialize synchronization objects
    mutex = sem_open(SEM_NAME, 0); // Open existing semaphore without O_CREAT
    if (mutex == SEM_FAILED) {
        perror("sem_open");
        fclose(config_file);
        return -1;
    }

    // Read configuration and populate shared memory
    fscanf(config_file, "%d", &shm_ptr->num_queues);
    for (int i = 0; i < shm_ptr->num_queues; i++) {
        fscanf(config_file, "%s %d", shm_ptr->queues[i].name, &shm_ptr->queues[i].size);
    }

    // Close the configuration file
    fclose(config_file);

    return 0; // Success
}


int mf_destroy() {
    printf("\nyoure now in mf destroy");
    // Detach from the shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID");
        return -1;
    }

    // Close and unlink the semaphore
    if (sem_close(mutex) == -1) { // Change sem to mutex
        perror("sem_close");
        return -1;
    }
    if (sem_unlink(SEM_NAME) == -1) {
        perror("sem_unlink");
        return -1;
    }

    return 0; // Return 0 for successful cleanup
}

int mf_connect() {
    printf("\nyoure now in mf connect");

    // Read the configuration file to retrieve shared memory name and size
    FILE *config_file = fopen(CONFIG_FILENAME, "r");
    if (config_file == NULL) {
        perror("fopen");
        return -1;
    }

    char shmem_name[MAX_MQ_NAME_LENGTH];
    int shmem_size;

    fscanf(config_file, "%s %d", shmem_name, &shmem_size);
    fclose(config_file);

    // Attach to the shared memory segment
    shmid = shmget(IPC_PRIVATE, shmem_size, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return -1;
    }

    // Attach to the shared memory
    shm_ptr = (struct shared_memory *)shmat(shmid, NULL, 0);
    if (shm_ptr == (struct shared_memory *) -1) {
        perror("shmat");
        return -1;
    }

    // Initialize synchronization objects
    mutex = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (mutex == SEM_FAILED) {
        perror("sem_open");
        return -1;
    }

    return 0; // Return 0 upon successful initialization
}

int mf_disconnect()
{
    printf("\nyoure now in mf disconnect");

    // Implementation for mf_disconnect
    if (shmdt(shm_ptr) == -1)
    {
        perror("shmdt");
        return -1;
    }

    return 0;
}

int mf_create(char *mqname, int mqsize) {
    printf("\nyoure now in mf create");

    // Check if maximum number of message queues reached
    if (shm_ptr->num_queues >= MAX_MQS) {
        fprintf(stderr, "Maximum number of message queues reached\n");
        return -1;
    }

    // Create new message queue
    strcpy(shm_ptr->queues[shm_ptr->num_queues].name, mqname);
    shm_ptr->queues[shm_ptr->num_queues].size = mqsize;
    shm_ptr->num_queues++;

    return 0; // Success
}

int mf_remove(char *mqname)
{
    printf("\nyoure now in mf remove");

    // Implementation for mf_remove
    int i;
    for (i = 0; i < shm_ptr->num_queues; i++)
    {
        if (strcmp(shm_ptr->queues[i].name, mqname) == 0)
        {
            // Shift the queues array to remove the queue
            for (int j = i; j < shm_ptr->num_queues - 1; j++)
            {
                strcpy(shm_ptr->queues[j].name, shm_ptr->queues[j + 1].name);
                shm_ptr->queues[j].size = shm_ptr->queues[j + 1].size;
            }

            shm_ptr->num_queues--;
            return 0;
        }
    }

    // If queue not found
    fprintf(stderr, "Message queue '%s' not found\n", mqname);
    return -1;
}

int mf_open(char *mqname) {
    printf("\nyou're now in mf open");

    // Implementation for mf_open
    for (int i = 0; i < shm_ptr->num_queues; i++) {
        if (strcmp(shm_ptr->queues[i].name, mqname) == 0) {
            // Return the index of the message queue as the queue ID (qid)
            return i;
        }
    }
    // If queue not found
    fprintf(stderr, "Message queue '%s' not found\n", mqname);
    return -1;
}

int mf_close(int qid)
{
    printf("\nyoure now in mf close");

    // Implementation for mf_close
    if (qid < 0 || qid >= shm_ptr->num_queues)
    {
        fprintf(stderr, "Invalid queue ID\n");
        return -1;
    }

    // Operation to close the queue (if required)
    return 0;
}

int mf_send(int qid, void *bufptr, int datalen)
{
    printf("\nyoure now in mf send");
    printf("\nqid in send %d", qid);

    // Implementation for mf_send
    if (qid < 0 || qid >= shm_ptr->num_queues)
    {
        fprintf(stderr, "Invalid queue ID\n");
        return -1;
    }

    // Check if the data length exceeds the queue size
    if (datalen > MAX_MQ_DATA_LENGTH)
    {
        fprintf(stderr, "Message data exceeds queue size\n");
        return -1;
    }

    // Check if the queue buffer is large enough to hold the message data
    if (datalen > shm_ptr->queues[qid].size)
    {
        fprintf(stderr, "Queue buffer is too small\n");
        return -1;
    }

    // Copy the message data from the buffer to the message queue
    memcpy(shm_ptr->queues[qid].data, bufptr, datalen);
    shm_ptr->queues[qid].size = datalen;

    return 0;
}

int mf_recv(int qid, void *bufptr, int bufsize) {
    printf("\nyou're now in mf receive");
    printf("\nqid in receive %d", qid);


    // Implementation for mf_recv
    if (qid < 0 || qid >= shm_ptr->num_queues) {
        fprintf(stderr, "Invalid queue ID\n");
        return -1;
    }

    // Check if the buffer size is large enough to hold the received message data
    if (bufsize < shm_ptr->queues[qid].size) {
        fprintf(stderr, "Buffer size is too small\n");
        return -1;
    }

    // Copy the message data from the message queue to the buffer
    memcpy(bufptr, shm_ptr->queues[qid].data, shm_ptr->queues[qid].size);
    ((char *)bufptr)[shm_ptr->queues[qid].size] = '\0'; // Ensure null-termination

    return 0;
}


int mf_print()
{
    // Implementation for mf_print
    printf("Number of message queues: %d\n", shm_ptr->num_queues);
    for (int i = 0; i < shm_ptr->num_queues; i++)
    {
        printf("Queue %d: Name=%s, Size=%d\n", i, shm_ptr->queues[i].name, shm_ptr->queues[i].size);
    }
    return 0;
}