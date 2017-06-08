#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main (int argc, char *argv[]) {
	pid_t childpid = 0;
	int i, n;
	if (argc != 2){
		/* check for valid number of command-line arguments */
		fprintf(stderr, "Usage: %s processes\n", argv[0]);
		return 1;
	}
	n = atoi(argv[1]);
	for (i = 1; i < n; i++)
		if (childpid = fork())
			break;
	fprintf(stderr, "i:%d process ID:%ld parent ID:%ld child ID:%ld\n",
	i, (long)getpid(), (long)getppid(), (long)childpid);
	return 0;
}


/*
msd@msd-ThinkPad-X1-Carbon:~/Desktop$ ./deneme 5
i:1 process ID:7286 parent ID:3316 child ID:7287
i:2 process ID:7287 parent ID:2366 child ID:7288
i:3 process ID:7288 parent ID:7287 child ID:7289
i:4 process ID:7289 parent ID:7288 child ID:7290
i:5 process ID:7290 parent ID:7289 child ID:0

*/
