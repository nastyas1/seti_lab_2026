#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <signal.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

pid_t *child_pids = NULL;
int pnum_global = 0;

void on_alarm(int sig) {
  write(1, "ALARM\n", 6);
  for (int i = 0; i < pnum_global; i++) {
    if (child_pids[i] > 0) {
      kill(child_pids[i], SIGKILL);
    }
  }
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  int timeout = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"timeout", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
              printf("Ошибка: seed должен быть положительным числом\n");
              return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              printf("Ошибка: array_size должен быть больше 0\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) {
              printf("Ошибка: pnum (число процессов) должно быть больше 0\n");
              return 1;
            }
            break;
          case 3:
            timeout = atoi(optarg);
            if (timeout <= 0) {
              printf("Ошибка: timeout должен быть положительным числом\n");
              return 1;
            }
            break;
          case 4:
            with_files = true;
            break;

          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  if (array == NULL) {
    printf("malloc failed\n");
    return 1;
  }
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  int sub_array_size = array_size / pnum;

  child_pids = malloc(sizeof(pid_t) * pnum);
  if (child_pids == NULL) {
    printf("malloc failed\n");
    free(array);
    return 1;
  }
  for (int i = 0; i < pnum; i++) child_pids[i] = 0;
  pnum_global = pnum;

  if (timeout > 0) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_alarm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);
    alarm(timeout);
  }

  int (*pipes)[2] = NULL;
  if (!with_files) {
    pipes = malloc(sizeof(int[2]) * pnum);
    for (int i = 0; i < pnum; i++) {
      if (pipe(pipes[i]) < 0) {
        printf("Pipe creation failed!\n");
        return 1;
      }
    }
  }

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process
        sleep(5); // <--- ДОБАВЛЕНО ДЛЯ ПРОВЕРКИ
        int start_index = i * sub_array_size;
        int end_index = (i == pnum - 1) ? array_size : (i + 1) * sub_array_size;

        struct MinMax min_max = GetMinMax(array, start_index, end_index);

        // parallel somehow

        if (with_files) {
          char file_name[64];
          sprintf(file_name, "ch_%d.txt", i);
          FILE *f = fopen(file_name, "w");
          if (f) 
          {
            fprintf(f, "%d %d", min_max.min, min_max.max);
            fclose(f);        
          } 
        }
        else 
        {
          close(pipes[i][0]);
          write(pipes[i][1], &min_max, sizeof(struct MinMax));
          close(pipes[i][1]);
        }
        free(array);
        if (!with_files) free(pipes);
        exit(0);
      }

      child_pids[i] = child_pid;

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  while (active_child_processes > 0) {
    wait(NULL);
    active_child_processes -= 1;
  }

  alarm(0);

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      char file_name[64];
      sprintf(file_name, "ch_%d.txt", i);
      FILE *f = fopen(file_name, "r");
      if (f) {
        fscanf(f, "%d %d", &min, &max);
        fclose(f);
        remove(file_name);
      }
    } else {
      struct MinMax local_res;
      close(pipes[i][1]);
      read(pipes[i][0], &local_res, sizeof(struct MinMax));
      close(pipes[i][0]);
      min = local_res.min;
      max = local_res.max;
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  free(child_pids);
  if (!with_files) free(pipes);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}