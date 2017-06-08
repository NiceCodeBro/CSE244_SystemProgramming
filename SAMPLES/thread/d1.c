#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include <unistd.h> 

#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
pthread_mutex_t lock;
void *myfunc(void* myvar);
int a = 0;
int main()
{

	pthread_t thread1;
	pthread_t thread2;
	int ret1, ret2;

	char* msg1 = "first thread";
	char* msg2 = "second thread";

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
	
	ret1 = pthread_create(&thread1, NULL, (void*) &myfunc, (void*) msg1);
	ret2 = pthread_create(&thread2, NULL, (void*) &myfunc, (void*) msg2);


	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);


	printf("Fİrst thrtead ret1 = %d\n",ret1);
	printf("Fİrst thrtead ret2 = %d\n",ret2);
	
	
	
    pthread_mutex_destroy(&lock);

	return 0;
}

void *myfunc(void* myvar)
{
	 char* msg = (char*) myvar;
	 
	 int i;
	 	printf("QQQQQQQ");
	 for(i = 0; i < 10; ++i)
	 {
	 	printf("%s %d %d\n",msg,i,a);
	 	    pthread_mutex_lock(&lock);

	 	++a;
	 	    pthread_mutex_unlock(&lock);

	 	sleep(1);
	 }
	 
	 return NULL;
}
