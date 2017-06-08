#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h>


void *thread_proc(void* param);
int shared = 1;
int main()
{
	pthread_t tid;
	int i;
	
	if(pthread_create(&tid,NULL,thread_proc,NULL) != 0 )
	{
		fprintf(stderr,"can not create thread!..\n");
		exit(EXIT_FAILURE);
	}
	pthread_join(tid,NULL);
	for(i = 0; i < 10; ++i)
	{
		 printf("MAİN FOR ICI  getpid: %d getpthread_self: %lu \n",getpid(), pthread_self());
		//printf("MyTread: %d\n",shared);
		//sleep(1);

	}
// printf("before calling pthread_create getpid: %d getpthread_self: %lu \n",getpid(), pthread_self());

	return 0;
}


void *thread_proc(void* param)
{
	int i;
	for(i = 0; i < 10; ++i)
	{
		shared *= i+1;
		 printf("THREAD FOR ICI  getpid: %d getpthread_self: %lu \n",getpid(), pthread_self());
		//printf("MyTread22: %d\n",shared);
		//sleep(1);
	}
 //printf("before calling pthread_create getpid: %d getpthread_self: %lu \n",getpid(), pthread_self());

}
