#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h> // For signal handling
#include <string.h>
#include "mf.h"

// Signal handler function for SIGINT
void sigint_handler(int signum) {
    printf("Received SIGINT signal. Exiting...\n");
    exit(signum);
}

int main(int argc, char *argv[]) {
    printf("mfserver pid=%d\n", (int) getpid());

    // Register the signal handler function
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

    // Call mf_init() to create and initialize the shared memory region
    if (mf_init() == -1) {
        fprintf(stderr, "Error initializing message passing facility\n");
        exit(1);
    }

    // Create a message queue
    if (mf_create("test_queue", 10) == -1) {
        fprintf(stderr, "Error creating message queue\n");
        exit(1);
    }

    // Open the message queue
    int qid = mf_open("test_queue");
    if (qid == -1) {
        fprintf(stderr, "Error opening message queue\n");
        exit(1);
    }

    // Prepare a message to send
    char message[] = "what";
    int message_length = strlen(message) + 1;

    // Send the message
    if (mf_send(qid, message, message_length) == -1) {
        fprintf(stderr, "Error sending message\n");
        exit(1);
    }
    printf("Message sent: %s\n", message);

    // Receive a message
    char received_message[100]; // Assuming the buffer size is sufficient
    if (mf_recv(qid, received_message, sizeof(received_message)) == -1) {
        fprintf(stderr, "Error receiving message\n");
        exit(1);
    }
    printf("Message received: %s\n", received_message);

    // Close the message queue
    if (mf_close(qid) == -1) {
        fprintf(stderr, "Error closing message queue\n");
        exit(1);
    }

    // Check if the sent and received messages match
    if (strcmp(message, received_message) == 0) {
        printf("Sent and received messages match!\n");
    } else {
        printf("Error: Sent and received messages do not match\n");
    }

    return 0;
}


