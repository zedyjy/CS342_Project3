#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <wait.h>

#define SHM_SIZE 1024
#define SHM_KEY 1234

int main() {
    int shmid;
    char *shm_ptr;
    char message[] = "Hello from Process 1!";

    // Create shared memory segment
    if ((shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    // Attach to the shared memory segment
    if ((shm_ptr = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    // Write message to shared memory
    strcpy(shm_ptr, message);
    printf("Message written to shared memory by Process 1: %s\n", message);

    // Detach from shared memory
    if (shmdt(shm_ptr) == -1) {
        perror("shmdt");
        exit(1);
    }

    // Fork a child process
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) { // Child process
        // Attach to the shared memory segment
        if ((shm_ptr = shmat(shmid, NULL, 0)) == (char *) -1) {
            perror("shmat");
            exit(1);
        }

        // Read message from shared memory and print
        printf("Message read from shared memory by Process 2: %s\n", shm_ptr);

        // Detach from shared memory
        if (shmdt(shm_ptr) == -1) {
            perror("shmdt");
            exit(1);
        }

        exit(0);
    } else { // Parent process
        wait(NULL); // Wait for child process to finish
    }

    // Delete the shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }

    return 0;
}
