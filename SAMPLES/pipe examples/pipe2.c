#include <sys/types.h>
 #include <unistd.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <limits.h>
 
char arr[1000];
 /* Read characters from the pipe and echo them to stdout. */

 void read_from_pipe (int file)
 {
   FILE *stream;
   int c;
   int index2 =0 ;
   stream = fdopen (file, "r");
   while ((c = fgetc (stream)) != EOF)
   {
   	arr[index2] = c ;
   	index2++;
   }

    //   printf("_read %d\n",myIndex);
   fclose (stream);
 }

 /* Write some random text to the pipe. */

 void write_to_pipe (int file)
 {
   FILE *stream;
   stream = fdopen (file, "w");
   fprintf (stream, "hello, world!\n");
   fprintf (stream, "goodbye, world!\n");
   fclose (stream);
 }
 
 int main (void)
 {
 	printf("%d\n",PIPE_BUF);
   pid_t pid;
   int mypipe[2];
   int i = 0,j;

   /* Create the pipe. */
   if (pipe (mypipe))
     {
       fprintf (stderr, "Pipe failed.\n");
       return EXIT_FAILURE;
     }

   /* Create the child process. */
   pid = fork ();
   if (pid == (pid_t) 0)
     {
     	printf("cocuk %d\n",5);
       /* This is the child process.
          Close other end first. */
       close (mypipe[0]);

       
       write_to_pipe (mypipe[1]);
      
       return EXIT_SUCCESS;
     }
   else if (pid < (pid_t) 0)
     {
       /* The fork failed. */
       fprintf (stderr, "Fork failed.\n");
      return EXIT_FAILURE;
      
     }
   else
     {
     printf("parent %d\n",5);
       /* This is the parent process.
          Close other end first. */
       close (mypipe[1]);
       
       read_from_pipe (mypipe[0]);
       printf("parent %d\n",5);
       //return EXIT_SUCCESS;
     }
     
     	
      printf("_%s",arr);
 }
