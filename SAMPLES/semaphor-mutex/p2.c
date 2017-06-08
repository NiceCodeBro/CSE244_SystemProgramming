#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
pthread_t tid[50];
int counter;
pthread_mutex_t lock;
  sem_t *mutex;
  char SEM_NAME[]= "vikasqqqd";

void* doSomeThing(void *arg)
{
    pthread_mutex_lock(&lock);
   //   sem_wait(mutex);

    unsigned long i = 0;
    counter += 1;
    printf("\n Job %d started\n", counter);

    for(i=0; i<(50000000);i++);

    printf("\n Job %d finished\n", counter);
     // sem_post(mutex);

    pthread_mutex_unlock(&lock);

    return NULL;
}

int main(void)
{
    int i = 0;
    int err;


  mutex = sem_open(SEM_NAME,O_CREAT,0644,1);
  if(mutex == SEM_FAILED)
    {
      perror("unable to create semaphore");
      sem_unlink(SEM_NAME);
      exit(-1);
    }


    while(i < 50)
    {
        err = pthread_create(&(tid[i]), NULL, &doSomeThing, NULL);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        i++;
    }

	for(i = 0; i < 50; ++i)
    	pthread_join(tid[i], NULL);

    pthread_mutex_destroy(&lock);
  sem_close(mutex);
  sem_unlink(SEM_NAME);
    return 0;
}
