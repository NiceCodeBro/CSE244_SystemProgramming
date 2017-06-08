#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
int main(void)
{
        int     fd[2], nbytes;
        pid_t   childpid;
        char    string[] = "Hello, world!\n";
         char    string2[] = "Hello, dunya!\n";
        char    readbuffer[80];
char    readbuffer2[80];
int     fd2[2];
        pipe(fd);
        pipe(fd2);
        if((childpid = fork()) == -1)
        {
                perror("fork");
                exit(1);
        }

        if(childpid == 0)
        {
                /* Child process closes up input side of pipe */
                close(fd[0]);

                /* Send "string" through the output side of pipe */
                write(fd[1], string, 255/*(strlen(string)+1)*/);
                close(fd[1]);
                
                 close(fd[0]);

                /* Send "string" through the output side of pipe */
                write(fd[1], string, 255/*(strlen(string)+1)*/);
                close(fd[1]);
                
                
        close(fd[1]);
        
                exit(0);
        }
        else
        {
                /* Parent process closes up output side of pipe */
                close(fd[1]);

                /* Read in a string from the pipe */
                nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
                close(fd[0]);
        }
        
        printf("Received string: %s", readbuffer);
        
        
        
       

        /* Read in a string from the pipe */
        nbytes = read(fd2[0], readbuffer2, sizeof(readbuffer2));
        close(fd2[0]);
        printf("aReceived string: %s", readbuffer2);
        
        return(0);
}
