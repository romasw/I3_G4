#include "thread_type.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

void receive_record(int s){
    //make file for store reording
    char filename[] = "REC.raw";
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
    }
    int N = 1024;
    unsigned char buffer_rec[N];
    while(1){
        int n = recv(s, buffer_rec, N, 0);
        if(n==0){break;}
        write(fd,buffer_rec,N);
    }
    close(fd);
}

void *wait_thread(void *arg) {
    THREAD_ARG *thread_arg = (THREAD_ARG *)arg;
    int s = thread_arg->s;
    int *flag = thread_arg->flag;
    int n;
    char buffer[10];
    char sample_yes[9] = "accepted";
    char sample_no[9] = "rejected";

    pid_t pid;

    while(1){
        if(*flag == 0){
            n = recv(s, buffer, 10, 0);
            if (*flag == 1) {
                strcpy(thread_arg->input, buffer);
                *flag = 4;
                continue;
            }
            if(!strcmp(buffer, "call")){// wait for the call
                printf("\033[2JYOU HAVE AN INCOMING CALL.\nANSWER THE CALL?(yes/no)\n");
                *flag = 2;
                pid = fork();
                if (pid == -1) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                if (pid == 0) {
                    // 子プロセス: sox playコマンドを実行
                    execl("/bin/sh", "sh", "-c", "play -q ./audio/chakusin.wav repeat 10 2>/dev/null", (char *)NULL);
                    perror("execlp");
                    exit(EXIT_FAILURE);
                } else {
                    while (*flag != 4) {
                        usleep(200*1000);
                    }
                    kill(pid, SIGKILL); // 子プロセスを停止
                    wait(NULL);
                }
                *flag = 2;
                if(!strcmp(thread_arg->input, "yes\n")){
                    send(s, sample_yes, strlen(sample_yes), 0);
                    *flag = 3;
                    break;
                }// yes: break and start a call
                else{
                    send(s, sample_no, strlen(sample_no), 0);// no: send the message
                    *flag = 5;//receive from record and make file
                }
            }
            if(n == -1){// error handling
                perror("read");
                exit(EXIT_FAILURE);
            }
        }else if(*flag == 3){
            break;
        }else if(*flag == 5){
            printf("\033[2JPLEASE WAIT UNTIL YOU GOT A MESSAAGE.\n");
            receive_record(s);
            printf("\033[2JYOU GOT A MESSAGE. THE MESSAGE HAS BEEN SAVED.\n");
            //*flag = 0;
            exit(EXIT_SUCCESS);
        }
    }
    return NULL;
}