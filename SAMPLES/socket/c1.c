#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <netdb.h>
#include <netinet/in.h> 
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#define PORTNUM 2016

int main(int argc,char *argv[]) {

  int sd,i,sz,num;
  char *buffer= (char *) malloc(18*sizeof(char));
  
  struct sockaddr_in server;
  struct hostent *host;
  time_t srvrtime;
  struct utsname name;

  /* create the socket for talking to server*/
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { 
    perror("socket"); 
    exit(1);
  }

  /* get server internet address and put into addr
   * structure fill in the socket address structure 
   * and connect to server
   */
  memset((char *) &server, '\0', sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = PORTNUM;

  /* Server is local system.  Get its name. */
  if (uname(&name) == -1) {
    perror("uname");
    exit (1);
  }

  if ((host = gethostbyname(name.nodename)) == NULL) { 
    perror("gethostbyname"); 
    exit(1); 
  }
  memcpy((char *)&server.sin_addr,host->h_addr,host->h_length);

  /* connect to server */
  if( connect(sd, (struct sockaddr *)&server,
     sizeof(server))) { 
    perror("connect"); 
    exit(1);
  }

  /* Communicate with server */
  num = atoi(argv[1]);
  write(sd,&num,sizeof(num));
  for (i=0;i<num;++i){
     do {
 sz= read (sd, buffer, sizeof(18*sizeof(char)));
 buffer[sz]='\0';
 printf("%s",buffer);
     } while(sz>0);
  }
  printf("\n");  
  
    write(sd,&num,sizeof(num));
  for (i=0;i<num;++i){
     do {
 sz= read (sd, buffer, sizeof(18*sizeof(char)));
 buffer[sz]='\0';
 printf("%s",buffer);
     } while(sz>0);
  }
  printf("\n");  
  close(sd);
  
  return 0;
}
