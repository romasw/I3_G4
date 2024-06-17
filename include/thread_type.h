#pragma once

typedef struct thread_arg { 
    int s;
    int *flag;
    int shift;
    char input[256];
} THREAD_ARG;
