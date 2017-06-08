#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{
 //var
 int sock;
 struct sockaddr_in server;
 int mysock;
 char buff[1024];
 int rval;

 //sock
 sock = socket(AF_INET, SOCK_STREAM, 0); //create socket
 if(sock < 0)
 {
  perror("Failed to create socket.");
  exit(1);
 }
 server.sin_family = AF_INET;
 server.sin_addr.s_addr = INADDR_ANY;
 server.sin_port = htons(2046);

 //bind
 if(bind(sock, (struct sockaddr *)&server, sizeof(server)))
 {
  perror("Bind failed.");
  exit(1);
 }

 //listen
 listen(sock, 5);

 //accept
 do
 {
  mysock = accept(sock, (struct sockaddr *)0,0);
  if(mysock == -1)
   perror("Accept failed.");
  else
  {
   memset(buff, 0, sizeof(buff));
   if((rval = recv(mysock, buff, sizeof(buff),0))<0)
    perror("Reading stream message error.");
   else if(rval == 0)
    printf("Ending connection.\n");
   else
    printf("MSG: %s\n",buff);
   printf("Got the message (rval = %d)\n",rval);
   close(mysock);
  }
 }while(1);
 return 0;
}
