#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> //exit
#include <sys/wait.h> //wait()


int main()
{
	pid_t childPIDorZero = fork();
	if( childPIDorZero < 0){
		perror("fork() error");
		exit(-1);	
	}
	
	if( childPIDorZero != 0) {
	wait(NULL); //wait for child precess to join
		printf("I'am the parent %d, my child is %d\n", getpid(), childPIDorZero);
		
	}else
	{
		printf("I'am the child %d, my parent is %d\n",getpid(), getppid());
		//execl("/bin/echo","echo","Hello, World", NULL);
	}
	
}
