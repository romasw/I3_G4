#include <band_shift.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct thread_arg {
  int s;
  int shift;
} TALK_THREAD_ARG;

typedef struct horyu_thread_arg {
  int *isHoryu;
} HORYU_THREAD_ARG;

void *trigger_horyu(void *arg) {
  HORYU_THREAD_ARG *thread_arg = (HORYU_THREAD_ARG *)arg;
  int *isHoryu = thread_arg->isHoryu;

  system("/bin/stty raw onlcr");
  while (1) {
    char c = getchar();
    if (c == 'h') {
      *isHoryu = !*isHoryu;
    } else if (c == 'q') {
      system("/bin/stty cooked");
      exit(EXIT_SUCCESS);
    }
  }
}

void *rec_send_thread(void *arg) {
  TALK_THREAD_ARG *thread_arg = (TALK_THREAD_ARG *)arg;
  int s = thread_arg->s;
  int shift = thread_arg->shift;
  FILE *fp_rec;

  char *cmd_rec = "rec -q -V0 -t raw -b 16 -c 1 -e s -r 44100 - 2>/dev/null";

  fp_rec = popen(cmd_rec, "r");
  if (fp_rec == NULL) {
    perror("popen");
    exit(EXIT_FAILURE);
  }

  FILE *fp_horyu;
  fp_horyu = fopen("./audio/horyuu.raw", "rb");
  if (fp_horyu == NULL) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  int N = 1024;
  short buffer_horyu[N];

  int isHoryu = 0;
  pthread_t thread_horyu;

  HORYU_THREAD_ARG horyu_thread_arg;
  horyu_thread_arg.isHoryu = &isHoryu;

  if (pthread_create(&thread_horyu, NULL, trigger_horyu,
                     (void *)&horyu_thread_arg) != 0) {
    printf("Failed to create horyu_thread.\n");
    exit(EXIT_FAILURE);
  }
  short buffer_rec[N];
  short buffer_rec_out[N];
  printf("\n");
  while (1) {
    int n = fread(buffer_rec, 1, N, fp_rec);
    if (n == -1) {
      perror("read");
      exit(EXIT_FAILURE);
    }
    if (n == 0) {
      break;
    }
    printf("\033[2;1H\033[2K");
    if (isHoryu) {
      printf("(PRESS H TO TAKE OFF HOLD / PRESS Q TO QUIT THE CALL)\n");
      n = fread(buffer_rec_out, sizeof(short), N, fp_horyu);
      if (n == 0) {
        fclose(fp_horyu);
        fp_horyu = fopen("./audio/horyuu.raw", "rb");
        if (fp_horyu == NULL) {
          perror("fopen");
          exit(EXIT_FAILURE);
        }
      }
    } else {
      printf("(PRESS H TO PUT ON HOLD / PRESS Q TO QUIT THE CALL)\n");
      band_shift(buffer_rec, buffer_rec_out, N / 2, shift);
    }
    n = send(s, buffer_rec_out, n, 0);
    if (n == -1) {
      perror("send");
      exit(EXIT_FAILURE);
    }
  }
  fclose(fp_rec);
}

void *recv_play_thread(void *arg) {
  TALK_THREAD_ARG *thread_arg = (TALK_THREAD_ARG *)arg;
  int s = thread_arg->s;

  FILE *fp_play;

  char *cmd_play = "play -q -V0 -t raw -b 16 -c 1 -e s -r 44100 - 2>/dev/null";

  fp_play = popen(cmd_play, "w");
  if (fp_play == NULL) {
    perror("popen");
    exit(EXIT_FAILURE);
  }

  int N = 1024;
  short buffer_play[N];
  while (1) {
    int n = recv(s, buffer_play, N, 0);
    if (n == -1) {
      perror("recv");
      exit(EXIT_FAILURE);
    }
    if (n == 0) {
      break;
    }
    n = fwrite(buffer_play, 1, n, fp_play);
    if (n == -1) {
      perror("write");
      exit(EXIT_FAILURE);
    }
  }
  fclose(fp_play);
}

void talk_session(int s, int shift) {
  pthread_t thread_rec_send, thread_recv_play;

  TALK_THREAD_ARG thread_arg = {s, shift};

  FILE *fp_gatcha;

  char *cmd_gatcha = "play -q -V0 ./audio/Gatcha.wav 2>/dev/null";
  fp_gatcha = popen(cmd_gatcha, "w");
  if (fp_gatcha == NULL) {
    perror("popen");
    exit(EXIT_FAILURE);
  }

  printf("\033[H\033[J");
  printf("ON A CALL.\n");

  if (pthread_create(&thread_rec_send, NULL, rec_send_thread,
                     (void *)&thread_arg) != 0) {
    printf("Failed to create rec_send_thread.\n");
    exit(EXIT_FAILURE);
  }

  if (pthread_create(&thread_recv_play, NULL, recv_play_thread,
                     (void *)&thread_arg) != 0) {
    printf("Failed to create recv_play_thread.\n");
    exit(EXIT_FAILURE);
  }

  pthread_join(thread_rec_send, NULL);
  pthread_join(thread_recv_play, NULL);
  fclose(fp_gatcha);
}