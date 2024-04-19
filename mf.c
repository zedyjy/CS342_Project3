#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include "mf.h"
#include <errno.h> // Include errno for error handling

#define CONFIG_FILENAME "config.txt"
#define MAX_MQS 10
#define MAX_MQ_NAME_LENGTH 50
#define MAX_MQ_DATA_LENGTH 100
#define SEM_NAME "/mf_semaphore"

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
int shm_fd;
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

    // Open the semaphore
    mutex = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (mutex == SEM_FAILED) {
        perror("sem_open");
        fclose(config_file);
        return -1;
    }

    // Open or create the shared memory object
    shm_fd = shm_open("/shared_memory_name", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        fclose(config_file);
        return -1;
    }

    // Set the size of the shared memory object
    if (ftruncate(shm_fd, sizeof(struct shared_memory)) == -1) {
        perror("ftruncate");
        fclose(config_file);
        return -1;
    }

    // Map the shared memory object into the address space
    shm_ptr = mmap(NULL, sizeof(struct shared_memory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        fclose(config_file);
        return -1;
    }

    // Read configuration and populate shared memory
    /*fscanf(config_file, "%d", &shm_ptr->num_queues);
    char mqname[MAX_MQ_NAME_LENGTH]; // Temporary buffer to hold the message queue name
    fscanf(config_file, "%s", mqname); // Read the message queue name from the configuration file
    for (int i = 0; i < shm_ptr->num_queues; i++) {
        // Use the read message queue name when creating the message queue
        mf_create(mqname, 16); // Use mqname read from the config file; create mq; 16 KB
    }*/

    // Close the configuration file
    fclose(config_file);

    return 0; // Success
}

int mf_destroy() {
    printf("\nyoure now in mf destroy");
    // Unmap the shared memory
    if (munmap(shm_ptr, sizeof(struct shared_memory)) == -1) {
        perror("munmap");
        return -1;
    }

    // Close and unlink the shared memory object
    if (shm_fd != -1) {
        if (close(shm_fd) == -1) {
            perror("close");
            return -1;
        }
        if (shm_unlink("/shared_memory_name") == -1) {
            perror("shm_unlink");
            return -1;
        }
    }

    // Close and unlink the semaphore
    if (mutex != SEM_FAILED) {
        if (sem_close(mutex) == -1) {
            perror("sem_close");
            return -1;
        }
        if (sem_unlink(SEM_NAME) == -1) {
            perror("sem_unlink");
            return -1;
        }
    }

    return 0; // Return 0 for successful cleanup
}

int mf_connect() {
    printf("\nyoure now in mf connect");

    // Open the semaphore
    mutex = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (mutex == SEM_FAILED) {
        perror("sem_open");
        return -1;
    }

    // Open the shared memory object
    shm_fd = shm_open("/shared_memory_name", O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return -1;
    }

    // Map the shared memory object into the address space
    shm_ptr = mmap(NULL, sizeof(struct shared_memory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    return 0; // Return 0 upon successful initialization
}

int mf_disconnect()
{
    printf("\nyoure now in mf disconnect");

    // Unmap the shared memory
    if (munmap(shm_ptr, sizeof(struct shared_memory)) == -1)
    {
        perror("munmap");
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

int mf_send(int qid, void *bufptr, int datalen) {
    // Acquire the semaphore to ensure mutual exclusion
    if (sem_wait(mutex) == -1) {
        perror("sem_wait");
        return -1;
    }

    // Implementation for mf_send
    if (qid < 0 || qid >= shm_ptr->num_queues) {
        fprintf(stderr, "Invalid queue ID\n");
        // Release the semaphore before returning
        sem_post(mutex);
        return -1;
    }

    // Check if the data length exceeds the maximum message data length
    if (datalen > MAX_MQ_DATA_LENGTH) {
        fprintf(stderr, "Message data exceeds maximum length\n");
        // Release the semaphore before returning
        sem_post(mutex);
        return -1;
    }

    // Check if there is enough space in the message queue buffer
    while (shm_ptr->queues[qid].size + datalen > MAX_MQ_DATA_LENGTH) {
        // Release the semaphore temporarily while waiting for space
        sem_post(mutex);
        // Sleep for a short duration before checking again
        usleep(10000); // Sleep for 10 milliseconds (adjust as needed)
        // Acquire the semaphore again before continuing
        if (sem_wait(mutex) == -1) {
            perror("sem_wait");
            return -1;
        }
    }

    // Copy the message data from the buffer to the message queue
    memcpy(shm_ptr->queues[qid].data + shm_ptr->queues[qid].size, bufptr, datalen);

    // Update the size of the message data in the message queue structure
    shm_ptr->queues[qid].size += datalen;

    // Release the semaphore
    if (sem_post(mutex) == -1) {
        perror("sem_post");
        return -1;
    }

    return 0;
}

int mf_recv(int qid, void *bufptr, int bufsize) {
    // Acquire the semaphore to ensure mutual exclusion
    if (sem_wait(mutex) == -1) {
        perror("sem_wait");
        return -1;
    }

    // Implementation for mf_recv
    if (qid < 0 || qid >= shm_ptr->num_queues) {
        fprintf(stderr, "Invalid queue ID\n");
        // Release the semaphore before returning
        sem_post(mutex);
        return -1;
    }

    // Check if there are any messages available in the message queue buffer
    while (shm_ptr->queues[qid].size == 0) {
        // Release the semaphore temporarily while waiting for a message
        sem_post(mutex);
        // Sleep for a short duration before checking again
        usleep(10000); // Sleep for 10 milliseconds (adjust as needed)
        // Acquire the semaphore again before continuing
        if (sem_wait(mutex) == -1) {
            perror("sem_wait");
            return -1;
        }
    }

    // Check if the buffer size is large enough to hold the received message data
    if (bufsize < shm_ptr->queues[qid].size) {
        fprintf(stderr, "Buffer size is too small\n");
        // Release the semaphore before returning
        sem_post(mutex);
        return -1;
    }

    // Copy the message data from the message queue to the buffer
    memcpy(bufptr, shm_ptr->queues[qid].data, shm_ptr->queues[qid].size);
    int received_data_length = shm_ptr->queues[qid].size; // Store the received data length
    ((char *)bufptr)[shm_ptr->queues[qid].size] = '\0'; // Ensure null-termination

    // Reset the size of the message data in the message queue structure
    shm_ptr->queues[qid].size = 0;

    // Release the semaphore
    if (sem_post(mutex) == -1) {
        perror("sem_post");
        return -1;
    }

    return received_data_length; // Return the actual received data length
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