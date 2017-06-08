#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
	int x;
	x = 0;
	
	x = 1;
	//printf("I am process %ld and my x is %d,, %d ", (long)getpid(), x, getppid());,
		printf("my pid: %d\n", getpid());
	pid_t xx= fork();
	if(xx > 0){ //parent
		wait(NULL);	
		printf("parent\n");
		
	}
	else // child
		printf("child\n");
	printf("my pid: %d\n", getpid());
	
	return 0;
}

/*

I am process 6543 and my x is 1,, 3316  selam
I am process 6544 and my x is 1,, 6543 a selam


*/
