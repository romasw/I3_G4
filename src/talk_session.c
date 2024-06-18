#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>

typedef struct thread_arg { 
    int s;
} TALK_THREAD_ARG;

typedef struct horyu_thread_arg { 
    int *isHoryu;
} HORYU_THREAD_ARG;

void *trigger_horyu(void *arg){
    HORYU_THREAD_ARG *thread_arg = (HORYU_THREAD_ARG *)arg;
    int *isHoryu = thread_arg->isHoryu;

    while(1){
        char c = getchar();
        if(c == 'h'){
            *isHoryu = !*isHoryu;
        }
    }
}

void *rec_send_thread(void *arg) {
    TALK_THREAD_ARG *thread_arg = (TALK_THREAD_ARG *)arg;
    int s = thread_arg->s;

    FILE *fp_rec;
    char *cmd_rec = "rec -t raw -b 16 -c 1 -e s -r 44100 - ";
    fp_rec = popen(cmd_rec, "r");
    if(fp_rec == NULL){
        perror("popen");
        exit(EXIT_FAILURE);
    }
    FILE *fp_horyu;
    fp_horyu = fopen("./audio/horyu_on.raw", "r");
    if(fp_horyu == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int N = 1024;
    unsigned char buffer_rec[N], buffer_horyu[N];

    int isHoryu = 0;
    pthread_t thread_horyu;

    HORYU_THREAD_ARG horyu_thread_arg;
    horyu_thread_arg.isHoryu = &isHoryu;

    if(pthread_create(&thread_horyu, NULL, trigger_horyu, (void *)&horyu_thread_arg) != 0) {
        printf("Failed to create horyu_thread.\n");
        exit(EXIT_FAILURE);
    }


    
    while(1){
        if(isHoryu){
            int n = fread(buffer_horyu, 1, N, fp_rec);
            if(n == -1){
                perror("read");
                exit(EXIT_FAILURE);
            }
            if(n == 0){
                fclose(fp_horyu);
                fp_horyu = fopen("./audio/horyu_on.raw", "r");
                if(fp_horyu == NULL){
                    perror("fopen");
                    exit(EXIT_FAILURE);
                }
            }
            else{
                n = send(s, buffer_rec, n, 0);
                if(n == -1){
                    perror("send"); 
                    exit(EXIT_FAILURE);
                }
            }
        }
        else{
            int n = fread(buffer_rec, 1, N, fp_rec);
            if(n == -1){
                perror("read");
                exit(EXIT_FAILURE);
            }
            if(n == 0){
                break;
            }
            n = send(s, buffer_rec, n, 0);
            if(n == -1){
                perror("send"); 
                exit(EXIT_FAILURE);
            }
        }
    }
    fclose(fp_rec);
}

void *recv_play_thread(void *arg) {
    TALK_THREAD_ARG *thread_arg = (TALK_THREAD_ARG *)arg;
    int s = thread_arg->s;

    FILE *fp_play;
    char *cmd_play = "play -t raw -b 16 -c 1 -e s -r 44100 - ";
    fp_play = popen(cmd_play, "w");
    if(fp_play == NULL){
        perror("popen");
        exit(EXIT_FAILURE);
    }

    int N = 1024;
    unsigned char buffer_play[N];
    while(1){
        int n = recv(s, buffer_play, N, 0);
        if(n == -1){
            perror("recv");
            exit(EXIT_FAILURE);
        }
        if(n == 0){
            break;
        }
        n = fwrite(buffer_play, 1, n, fp_play);
        if(n == -1){
            perror("write");
            exit(EXIT_FAILURE);
        }
    }
    fclose(fp_play);
}


void talk_session(int s){
    pthread_t thread_rec_send, thread_recv_play;

    TALK_THREAD_ARG thread_arg = {s};

    if(pthread_create(&thread_rec_send, NULL, rec_send_thread, (void *)&thread_arg) != 0) {
        printf("Failed to create rec_send_thread.\n");
        exit(EXIT_FAILURE);
    }

    if(pthread_create(&thread_recv_play, NULL, recv_play_thread, (void *)&thread_arg) != 0) {
        printf("Failed to create recv_play_thread.\n");
        exit(EXIT_FAILURE);
    }

    pthread_join(thread_rec_send, NULL);
    pthread_join(thread_recv_play, NULL);
}