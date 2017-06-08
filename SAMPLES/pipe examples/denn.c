#include <stdio.h>
#include <dirent.h>
#include <stdlib.h> //exit
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
 #include <limits.h>
 #include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
void tempFunc(int *arr);

int main()
{
	int arr[10];
	arr[0] = 5;
	arr[1] = 5;
	arr[2] = 5;
	arr[3] = 5;
	arr[4] = 5;	
	tempFunc(arr);
	
	printf("%d\n",arr[2]);
	printf("%d\n",arr[3]);
	printf("%d\n",arr[4]);
}


void tempFunc(int *arr)
{
	arr[2] = 7;
	
   pid_t pid;
   
   pid = fork();
   
   if(pid)
   {
   	wait(NULL);
   	arr[3] = 9;
   }
   else
   {
   	arr[4] = 17;
   	exit(0);
   }

}



