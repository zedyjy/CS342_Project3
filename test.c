#include "libmf.h"
#include <stdio.h>

int main() {
    if (mf_init() == -1) {
        fprintf(stderr, "Error initializing message passing facility\n");
        return 1;
    }

    printf("Message passing facility initialized successfully\n");

    return 0;
}
