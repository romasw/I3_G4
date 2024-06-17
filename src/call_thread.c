#include "thread_type.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

void record(int s){
    int N = 1024;
    //popen("play ")録音メッセージを流す
    printf("5秒後に録音を開始いたします。\n");
    sleep(5);

    //make file for store reording
    char filename[] = "REC";
    /*
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
    }
    */

    //control file for popen
    FILE *fp_rec;
    char *cmd_rec = "rec -t raw -b 16 -c 1 -e s -r 44100 - ";

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
    fclose(fp_rec);
}

void *call_thread(void *arg) {
    THREAD_ARG *thread_arg = (THREAD_ARG *)arg;
    int s = thread_arg->s;
    int *flag = thread_arg->flag;

    FILE *fp_play;
    char *cmd_play = "play ./audio/yobidasi.wav repeat 1";
    pid_t pid;

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
            pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            if (pid == 0) {
                // 子プロセス: sox playコマンドを実行
                execlp("play", "play", "-q", "./audio/yobidasi.wav", "repeat", "10", (char *)NULL);
                perror("execlp");
                exit(EXIT_FAILURE);
            } else {
                while (*flag != 4) {
                    usleep(200*1000);
                }
                kill(pid, SIGTERM); // 子プロセスを停止
                wait(NULL);
            }
            *flag = 1;
            if(!strcmp(thread_arg->input, "rejected")){ //rejectされた
                printf("応答が拒否されました、録音を開始いたします。\n");
                record(s);
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