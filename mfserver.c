#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h> // For signal handling
#include <string.h>
#include "mf.h"

// Signal handler function for SIGINT
void sigint_handler(int signum) {
    printf("Received SIGINT signal. Exiting...\n");
    mf_destroy(); // Call mf_destroy() for cleanup
    exit(signum);
}

int main() {
    printf("mfserver pid=%d\n", (int) getpid());

    // Call mf_init() to create and initialize the shared memory region
    if (mf_init() == -1) {
        fprintf(stderr, "Error initializing message passing facility\n");
        exit(1);
    }

    // Register the signal handler function
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

    printf("You're now in 1\n");

    // Do some initialization if needed

    while (1) {
        sleep(1000); // Sleep for most of the time
    }

    return 0;
}

