#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
int main(void) {
	pid_t childpid;
	childpid = fork();
	if (childpid == -1) {
	perror("Failed to fork");
	return 1;
	}
	if (childpid == 0)
	/* child code */
	printf("I am child %ld %d\n", (long)getpid(), childpid);
	else
	/* parent code */
	printf("I am parent %ld %d\n", (long)getpid(), childpid);
	return 0;
}

/*
I am parent 6867 6868
I am child 6868 0


*/
