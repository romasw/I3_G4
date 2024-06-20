#pragma once

typedef struct thread_arg { 
    int s;
    int *state;
    char input[256];
} THREAD_ARG;
