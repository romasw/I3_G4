#include "thread_type.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void *call_thread(void *arg) {
    THREAD_ARG *thread_arg = (THREAD_ARG *)arg;
    int s = thread_arg->s;
    int *flag = thread_arg->flag;
    
    while(1){
        if(*flag == 0){
            printf("電話を開始しますか？\nYES\n");
            char data[5] = "call";
            char c[10];
            scanf("%s", c);
            strcpy(thread_arg->input, c);
            if(*flag == 2){
                *flag = 4;
                continue;
            }
            *flag = 1;
            send(s, data, 5, 0);
            while (*flag != 4) {
                usleep(200*1000);
            }
            *flag = 1;
            if(!strcmp(thread_arg->input, "rejected")){ //rejectされた
                printf("応答が拒否されました\n");
                *flag = 0;
            }else if(!strcmp(thread_arg->input, "accepted")){ //acceptされた
                *flag = 3;
                break;
            }
        }
        else if(*flag == 3){
            break;
        }
    }
    return NULL;
}