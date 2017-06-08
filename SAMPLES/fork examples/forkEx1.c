#include <stdio.h>
#include <unistd.h>


int main()
{
	int x=1;
	

	if(fork() == 0)
		printf("Child, x = %d\n", ++x);
	else
		printf("Parent, x = %d\n", --x);
		
	printf("Exiting with x = %d\n", x);
	
	return 0;
}
