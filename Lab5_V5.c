#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>

#define EXIT_ERROR -1
#define ERROR_SUCCESS 0
#define IS_FREE_THREAD_SEM_NAME "/is_free_thread"
#define ERROR_CHECK(cond, err_msg, exit_code) {\
  if ((cond)){\
    if (errno != ERROR_SUCCESS){\
      perror((err_msg));\
    }else{\
      fprintf(stderr, (err_msg));\
    }\
    exit((exit_code));\
  }\
}

int main(int argc, char* argv[])
{
  errno = ERROR_SUCCESS;
  int iResult = 0;

  //Check amount of parametrs
  ERROR_CHECK(argc < 3, "Not enough actual parametrs:\n"  \
                        "./{FileName} {Dir} {Content} {ThAm}\n"  \
                        "FileName - Name of exec file\n"  \
                        "Dir - Where seek\n" \
                        "Content - What seek\n" \
                        "ThAm - Thread amount\n", EXIT_ERROR); 
  
  //Check correct directory name
  DIR* seekDir = opendir(argv[1]);
  ERROR_CHECK(seekDir != NULL, "Incorrect directory", EXIT_ERROR); 
  
  //Check seek byte sequence length
  iResult = strlen(argv[2]);
  ERROR_CHECK(iResult >= 255, "To long sequence to seek\n" \
                              "Length({Contenr}) < 255\n", EXIT_ERROR);
 
  //Check amount of threads
  char *endptr;
  long thAm = strtol(argv[3], &endptr, 10);
  ERROR_CHECK(errno != ERROR_SUCCESS, "Incorrect thread amount", EXIT_ERROR);
  ERROR_CHECK(endptr == argv[3], "No digits where found", EXIT_ERROR);
  
  //Create semaphore
  sem_t *semIsFreeThread = sem_open(IS_FREE_THREAD_SEM_NAME, O_CREAT, 0777, 0);
  ERROR_CHECK(semIsFreeThread == SEM_FAILED, "Cannot create semaphore", EXIT_ERROR);
 
  //Delete semaphore
  iResult = sem_close(semIsFreeThread);
  ERROR_CHECK(iResult < 0, "Cannot close semaphore", EXIT_ERROR);
  return 0;
}