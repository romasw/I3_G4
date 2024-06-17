#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

void talk_session(int s){
    int N = 1024;

    FILE *fp_rec, *fp_play;
    char *cmd_rec = "rec -t raw -b 16 -c 1 -e s -r 44100 - ";
    char *cmd_play = "play -t raw -b 16 -c 1 -e s -r 44100 - ";

    fp_rec = popen(cmd_rec, "r");
    fp_play = popen(cmd_play, "w");
    if(fp_rec == NULL || fp_play == NULL){
        perror("popen");
        exit(EXIT_FAILURE);
    }

    unsigned char buffer_rec[N];
    unsigned char buffer_play[N];
    while(1){
        int n = fread(buffer_rec, 1, N, fp_rec);
        if(n == -1){
            perror("read"); 
            exit(EXIT_FAILURE);
        }
        n = send(s, buffer_rec, n, 0);
        if(n == -1){
            perror("send"); 
            exit(EXIT_FAILURE);
        }
        n = recv(s, buffer_play, N, 0);
        if(n == -1){
            perror("recv");
            exit(EXIT_FAILURE);
        }
        n = fwrite(buffer_play, 1, n, fp_play);
        if(n == -1){
            perror("write");
            exit(EXIT_FAILURE);
        }
    }
    fclose(fp_rec);
    fclose(fp_play);
}