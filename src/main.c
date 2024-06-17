#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <pthread.h>

#include "setup.h"
#include "thread_type.h"
#include "call_thread.h"
#include "wait_thread.h"
#include "talk_session.h"
//IP: YR 10.100.235.229
int main(int argc, char ** argv){
    int shift = 0;
    int s = setup(argc, argv);
    if (argc == 3) {
        shift = atoi(argv[2]);
    }
    if (argc == 4) {
        shift = atoi(argv[3]);
    }
    pthread_t thread_call, thread_wait;
    int flag = 0;
    int *pflag = &flag;
    
    THREAD_ARG thread_arg;
    thread_arg.flag = pflag;
    thread_arg.s = s;
    strcpy(thread_arg.input, "");

    if(pthread_create(&thread_call, NULL, call_thread, (void *)&thread_arg) != 0) {
        printf("Failed to create thread 1.\n");
        return 1;
    }

    if(pthread_create(&thread_wait, NULL, wait_thread, (void *)&thread_arg) != 0) {
        printf("Failed to create thread 2.\n");
        return 1;
    }

    pthread_join(thread_call, NULL);
    pthread_join(thread_wait, NULL);

    talk_session(s, &shift);

    return 0;
}