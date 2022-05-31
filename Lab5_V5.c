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

#define SEEK_SEQUENCE_LEN 256
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

#define ERROR_CHECK_THREAD(cond, err_msg, exit_code) {\
  if ((cond)){\
    if (errno != ERROR_SUCCESS){\
      perror((err_msg));\
    }else{\
      fprintf(stderr, (err_msg));\
    }\
    sem_post(semIsFreeThread);\
    pthread_exit((exit_code));\
  }\
}

typedef struct _TThreadArg
{
  char seekFileName[FILENAME_MAX];
  char sequence[SEEK_SEQUENCE_LEN];
}TThreadArg, *PThreadArg;

void* ThreadMain(void* arg);

int main(int argc, char* argv[])
{
  //Delete later
    argc = 3;
    argv[1] = "TestFolder";
    argv[2] = "aaa";
    argv[3] = "1";
  //Delete later


  errno = ERROR_SUCCESS;
  int iResult = 0;

  //Check amount of parametrs
  ERROR_CHECK(argc < 3, "Not enough actual parametrs:\n"  \
                        "./{FileName} {Dir} {Sequence} {ThAm}\n"  \
                        "FileName - Name of exec file\n"  \
                        "Dir - Where seek\n" \
                        "Sequence - What seek\n" \
                        "ThAm - Thread amount\n", EXIT_ERROR); 
  
  //Check correct directory name
  DIR* seekDirStrm = opendir(argv[1]);
  ERROR_CHECK(seekDirStrm == NULL, "Incorrect directory", EXIT_ERROR); 
  
  //Check seek byte sequence length
  iResult = strlen(argv[2]);
  ERROR_CHECK(iResult >= 255, "To long sequence to seek" \
                              "Length({Contenr}) < 255", EXIT_ERROR);
 
  //Check amount of threads
  char *endptr;
  long thAm = strtol(argv[3], &endptr, 10);
  ERROR_CHECK(errno != ERROR_SUCCESS, "Incorrect thread amount", EXIT_ERROR);
  ERROR_CHECK(endptr == argv[3], "No digits where found", EXIT_ERROR);
  ERROR_CHECK(thAm <= 0, "Incorrect thread number", EXIT_ERROR);

  //Create semaphore
  iResult = sem_unlink(IS_FREE_THREAD_SEM_NAME);
  //ERROR_CHECK(iResult < 0, "Cannot unlink semaphore", EXIT_ERROR)

  sem_t *semIsFreeThread = sem_open(IS_FREE_THREAD_SEM_NAME, O_CREAT, 0777, (int)thAm);
  ERROR_CHECK(semIsFreeThread == SEM_FAILED, "Cannot create semaphore", EXIT_ERROR);
 
  struct dirent* PDirStruct;
  int isDirEnd = 0;
  while (!isDirEnd){
    //semwait
    errno = ERROR_SUCCESS;
    PDirStruct = readdir(seekDirStrm);
    if (PDirStruct == NULL){
      isDirEnd = 1;
      ERROR_CHECK(errno != ERROR_SUCCESS, "Cannot read file from directory stream", EXIT_ERROR);
    }
    if (!isDirEnd && PDirStruct->d_type == DT_REG){
      iResult = sem_wait(semIsFreeThread);
      ERROR_CHECK(iResult < 0, "Cannot wait semaphore", EXIT_ERROR);
      TThreadArg threadArg;
      strcpy(threadArg.seekFileName, argv[1]);
      strcat(threadArg.seekFileName, "/");
      strcat(threadArg.seekFileName, PDirStruct->d_name);
//      printf("%s\n", threadArg.seekFileName);
      strcpy(threadArg.sequence, argv[2]); 
      pthread_t thread;
      iResult = pthread_create(&thread, NULL, ThreadMain, &threadArg);
      ERROR_CHECK(iResult != 0, "Could not create thread", EXIT_ERROR);
    }
  }

  iResult = sem_wait(semIsFreeThread);

  ERROR_CHECK(iResult < 0, "Cannot wait semaphore", EXIT_ERROR);
  //Delete semaphore
  iResult = sem_close(semIsFreeThread);
  ERROR_CHECK(iResult < 0, "Cannot close semaphore", EXIT_ERROR);
  iResult = sem_unlink(IS_FREE_THREAD_SEM_NAME);
  ERROR_CHECK(iResult < 0, "Cannot unlink semaphore", EXIT_ERROR);
  return 0;
}

void* ThreadMain(void* arg)
{

  //Delete later
//  printf("Thread id: %lu\n", pthread_self()); 
  //Delete later


  //Local
  TThreadArg threadArg = (*(TThreadArg*)arg);
  int iResult = 0;
  int fd = 0;
  FILE* fp;

  //Open semapthore
  sem_t *semIsFreeThread = sem_open(IS_FREE_THREAD_SEM_NAME, 0);
  ERROR_CHECK_THREAD(semIsFreeThread == SEM_FAILED, "Cannot create semaphore", NULL);

  fd = open(threadArg.seekFileName, O_RDONLY);
  ERROR_CHECK_THREAD(fd < 0, "Cannot get file discriptor", NULL);
  fp = fdopen(fd, "rb");
  ERROR_CHECK_THREAD(fp == NULL, "Cannot get file pointer", NULL);
   
  int idSequence = 0;

  char c = getc(fp);
  int nxtSeekPos = -1;
  int byteCount = 1, findCount = 0;
  int sequenceLen = strlen(threadArg.sequence);
  while(c != EOF){

    if(nxtSeekPos != -1){
      iResult = fseek(fp, nxtSeekPos, SEEK_SET);
      ERROR_CHECK_THREAD(iResult < 0, "Cannot set file position pointer", NULL);
      nxtSeekPos = -1;
    }

    if (c == threadArg.sequence[0]){
      nxtSeekPos = ftell(fp);
      printf("%d,", nxtSeekPos);
      nxtSeekPos++;
      ERROR_CHECK_THREAD(nxtSeekPos < 0, "Cannot get file position pointer", NULL);
      idSequence = 0;
      while (c == threadArg.sequence[idSequence] && idSequence < sequenceLen){
        c = getc(fp);
        idSequence++;
        byteCount++;
      }
      if (idSequence == sequenceLen){
        findCount++;
      }
    }
    c = getc(fp);
    byteCount++;
  }

  printf("Thread id: %lu Byte Cout: %05d Find Count: %d File Name: %s\n", pthread_self(), byteCount, findCount, threadArg.seekFileName); 

  iResult = fclose(fp);
  ERROR_CHECK_THREAD(iResult != 0, "Cannot close file pointer", NULL);

  //Set semphore
  iResult = sem_post(semIsFreeThread);
  ERROR_CHECK_THREAD(iResult < 0, "Cannot set semaphore in thread", NULL);

  //Close semaphore
  iResult = sem_close(semIsFreeThread);
  ERROR_CHECK_THREAD(iResult < 0, "Cannot close semapthore in thread", NULL);

  //pthread_exit(0);
}