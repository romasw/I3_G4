#include "thread_type.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>

void record(int s){
    int N = 1024;
    FILE *fp_play;
    char *cmd_play = "play -q -t raw -b 16 -c 1 -e s -r 44100 audio/wait.raw ";
    fp_play = popen(cmd_play, "r");

    //popen("play ")録音メッセージを流す
    printf("RECORDING STARTS IN 5 SECONDS.\n");
    sleep(5);

    //control file for popen
    FILE *fp_rec;
    char *cmd_rec = "rec -q -t raw -b 16 -c 1 -e s -r 44100 - ";

    fp_rec = popen(cmd_rec, "r");
    if(fp_rec == NULL){
        perror("popen");
    }

    unsigned char buffer_rec[N];

    clock_t start_time = clock();
    while(1){
        clock_t current_time = clock();
        double elapsed_time = (double)(current_time - start_time) / CLOCKS_PER_SEC;
        if (elapsed_time >= 10) {
            break;
        }
        int n = fread(buffer_rec, 1, N, fp_rec);
        if(n == -1){
            perror("read"); 
            exit(1);
        }
        send(s, buffer_rec, N, 0);
    }
    char eof = 26;
    send(s,&eof,0,0);
    printf("YOUR MESSAGE HAS BEEN SENT.\n");
    fclose(fp_rec);
}

void *call_thread(void *arg) {
    THREAD_ARG *thread_arg = (THREAD_ARG *)arg;
    int s = thread_arg->s;
    int *flag = thread_arg->flag;

    pid_t pid;

    while(1){
        if(*flag == 0){
            printf("PRESS ENTER TO START A CALL\n");
            char data[5] = "call";
            char c[10];
            fgets(c, 10, stdin);
            strcpy(thread_arg->input, c);
            if(*flag == 2){
                *flag = 4;
                continue;
            }
            *flag = 1;
            send(s, data, 5, 0);
            pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            if (pid == 0) {
                // 子プロセス: sox playコマンドを実行
                execlp("play", "play", "-q", "./audio/yobidasi.wav", "repeat", "10",  (char *)NULL);
                perror("execlp");
                exit(EXIT_FAILURE);
            } else {
                printf("NOW CALLING...\n");
                while (*flag != 4) {
                    usleep(200*1000);
                }
                kill(pid, SIGTERM); // 子プロセスを停止
                wait(NULL);
            }
            *flag = 1;
            if(!strcmp(thread_arg->input, "rejected")){ //rejectされた
                printf("YOU GOT NO RESPONSE. LEAVE A MESSAGE.\n");
                record(s);
                //*flag = 0;
                exit(EXIT_SUCCESS);
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