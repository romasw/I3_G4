#include <netinet/in.h>
#include <netinet/ip.h> 
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

int setup(int argc, char **argv, int* shift){
    int s;
    struct sockaddr_in addr;
        addr.sin_family = AF_INET;
    if(argc == 3){
        int ss = socket(PF_INET, SOCK_STREAM, 0);
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(atoi(argv[1]));
        bind(ss, (struct sockaddr *)&addr, sizeof(addr));
        listen(ss, 10);
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(struct sockaddr_in);
        s = accept(ss, (struct sockaddr *)&client_addr, &len);
        *shift = atoi(argv[2]);
    }else if(argc == 4){
        s = socket(PF_INET, SOCK_STREAM, 0);
        if (inet_aton(argv[1], &addr.sin_addr) == 0){
            perror("ERROR: Invalid IP passed.");
            return -1;
        }
        addr.sin_port = htons(atoi(argv[2]));
        connect(s, (struct sockaddr *)&addr, sizeof(addr));
        *shift = atoi(argv[3]);
    }else{
        perror("usage: ./i1i2i3_phone [IP ADDRESS] [PORT] [SHIFT] or ./i1i2i3_phone [PORT] [SHIFT]");
        exit(EXIT_FAILURE);
    }

    return s;
}