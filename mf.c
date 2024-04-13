#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <string.h>

#define CONFIG_FILENAME "config.txt" // Configuration file name
#define MAX_MQS 10                  // Maximum number of message queues
#define MAX_MQ_NAME_LENGTH 50       // Maximum length of a message queue name
#define SHM_SIZE sizeof(struct shared_memory) // Size of the shared memory segment
#define MAX_MQ_DATA_LENGTH 100 // Example size, adjust as needed

// Define semaphore name
#define SEM_NAME "/mf_semaphore"

// Global variable for semaphore
sem_t *sem;

// Structure to represent a message queue
struct message_queue
{
    char name[MAX_MQ_NAME_LENGTH];
    char data[MAX_MQ_DATA_LENGTH]; // New field to hold message data
    int size;
    // Add any other necessary fields
};


// Structure to represent the shared memory
struct shared_memory
{
    int num_queues;
    struct message_queue queues[MAX_MQS];
    // Add any other necessary fields
};

// Global variables for shared memory and synchronization objects
int shmid;
struct shared_memory *shm_ptr;
sem_t *mutex; // Mutex for thread safety

int mf_init()
{
    // Create the semaphore
    sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED)
    {
        perror("sem_open");
        return -1;
    }

    // Create shared memory segment
    if ((shmid = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666)) < 0)
    {
        perror("shmget");
        return -1;
    }

    // Attach to the shared memory segment
    if ((shm_ptr = (struct shared_memory *)shmat(shmid, NULL, 0)) == (struct shared_memory *)-1)
    {
        perror("shmat");
        return -1;
    }

    // Initialize synchronization objects
    mutex = (sem_t *)malloc(sizeof(sem_t)); // Allocate memory for mutex
    if (mutex == NULL)
    {
        perror("malloc");
        return -1;
    }
    if (sem_init(mutex, 1, 1) == -1)
    { // Initialize mutex with value 1 for thread safety
        perror("sem_init");
        return -1;
    }

    // Read configuration file and populate shared memory
    FILE *config_file = fopen(CONFIG_FILENAME, "r");
    if (config_file == NULL)
    {
        perror("fopen");
        return -1;
    }

    // Read configuration and populate shared memory
    fscanf(config_file, "%d", &shm_ptr->num_queues);
    for (int i = 0; i < shm_ptr->num_queues; i++)
    {
        fscanf(config_file, "%s %d", shm_ptr->queues[i].name, &shm_ptr->queues[i].size);
    }

    // Close the configuration file
    fclose(config_file);

    return 0; // MF_SUCCESS
}

int mf_destroy() {
    // Detach from the shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID");
        return -1;
    }

    // Close and unlink the semaphore
    if (sem_close(sem) == -1) {
        perror("sem_close");
        return -1;
    }
    if (sem_unlink(SEM_NAME) == -1) {
        perror("sem_unlink");
        return -1;
    }

    return 0; // Return 0 for successful cleanup
}

int mf_disconnect()
{
    // Implementation for mf_disconnect
    if (shmdt(shm_ptr) == -1)
    {
        perror("shmdt");
        return -1;
    }

    return 0;
}

int mf_create(char *mqname, int mqsize)
{
    // Implementation for mf_create
    if (shm_ptr->num_queues >= MAX_MQS)
    {
        fprintf(stderr, "Maximum number of message queues reached\n");
        return -1;
    }

    strcpy(shm_ptr->queues[shm_ptr->num_queues].name, mqname);
    shm_ptr->queues[shm_ptr->num_queues].size = mqsize;

    // Increment the number of queues
    shm_ptr->num_queues++;

    return 0;
}

int mf_remove(char *mqname)
{
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

int mf_open(char *mqname)
{
    // Implementation for mf_open
    for (int i = 0; i < shm_ptr->num_queues; i++)
    {
        if (strcmp(shm_ptr->queues[i].name, mqname) == 0)
        {
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

    // Copy the message data from the buffer to the message queue
    memcpy(shm_ptr->queues[qid].data, bufptr, datalen);
    shm_ptr->queues[qid].size = datalen;

    return 0;
}

int mf_recv(int qid, void *bufptr, int bufsize)
{
    // Implementation for mf_recv
    if (qid < 0 || qid >= shm_ptr->num_queues)
    {
        fprintf(stderr, "Invalid queue ID\n");
        return -1;
    }

    // Copy the message data from the message queue to the buffer
    if (bufsize < shm_ptr->queues[qid].size)
    {
        fprintf(stderr, "Buffer size is too small\n");
        return -1;
    }

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