#include "thread_type.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

void *wait_thread(void *arg) {
    THREAD_ARG *thread_arg = (THREAD_ARG *)arg;
    int s = thread_arg->s;
    int *flag = thread_arg->flag;
    int n;
    char buffer[10];
    char sample_yes[9] = "accepted";
    char sample_no[9] = "rejected";
    while(1){
        if(*flag == 0){
            n = recv(s, buffer, 10, 0);
            if (*flag == 1) {
                strcpy(thread_arg->input, buffer);
                *flag = 4;
                continue;
            }
            if(!strcmp(buffer, "call")){// wait for the call
                printf("Would you accept this call?\n");
                *flag = 2;
                while(*flag != 4){}
                *flag = 2;
                if(!strcmp(thread_arg->input, "yes")){
                    send(s, sample_yes, strlen(sample_yes), 0);
                    *flag = 3;
                    break;
                }// yes: break and start a call
                else{
                    send(s, sample_no, strlen(sample_no), 0);// no: send the message
                    *flag = 0;
                }
            }
            if(n == -1){// error handling
                perror("read");
                exit(EXIT_FAILURE);
            }
        }else if(*flag == 3){
            break;
        }
    }
    return NULL;
}