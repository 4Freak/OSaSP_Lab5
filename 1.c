#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/wait.h>

#define AM_OF_THREADS 2
#define EXIT_ERROR -1
#define ERROR_SUCCESS 0
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

void GetTime(int threadNum);

//Maint thread function
void* ThreadMain(void* arg);

int main()
{
  errno = ERROR_SUCCESS;
  int iResult = 0;
  pthread_t procThreads[AM_OF_THREADS];
  int i = 0;

  //Create threads
  for (; i < AM_OF_THREADS; i++){
    iResult = pthread_create(&procThreads[i], NULL, ThreadMain, &i); 
    ERROR_CHECK(iResult != 0, "Could not create thread", EXIT_ERROR);
  }
  GetTime(0);

  //Join threads
  for (i = 0; i < AM_OF_THREADS; i++){
    iResult = pthread_join(procThreads[i], NULL);
    ERROR_CHECK(iResult != 0, "Could not join thread", EXIT_ERROR);
  }
  return 0;
}

void GetTime(int threadNum){
	struct timeval TV;
	
	if (gettimeofday(&TV, NULL) == -1)
		perror("Can not get current time\n");
	else {
		int MSec = TV.tv_usec / 1000;
		int Sec = TV.tv_sec % 60;
		int Min = (TV.tv_sec / 60) % 60;
		int Hrs = (TV.tv_sec / 60 * 60 + 23) % 24;
	
		printf("Thread num: %d ID: %lu Parrent Pid: %d Time: %02d:%02d:%02d:%03d\n",threadNum, pthread_self(), getppid(), Hrs, Min, Sec, MSec);	
	}
}

void* ThreadMain(void* arg)
{
  int threadNum = *((int*)arg);
  threadNum++;
  GetTime(threadNum);
  pthread_exit(0);
}
