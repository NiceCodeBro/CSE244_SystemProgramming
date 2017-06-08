#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#define MAXHOSTNAME 255
#define DATA "Hello World of socket"

int main(int argc, char *argv[])
{
 //var
 int sock;
 struct sockaddr_in server;
 struct hostent *hp;
 char buff[1024];
char myname[MAXHOSTNAME+1];

 //sock
 sock = socket(AF_INET, SOCK_STREAM, 0); //create socket
 if(sock < 0)
 {
  perror("Failed to create socket.");
  exit(1);
 }
 server.sin_family = AF_INET;
 gethostname(myname, MAXHOSTNAME);
 hp = gethostbyname(myname);
 //fprintf(stderr,"%s",myname);
 if(hp == 0)
 {
 	perror("gethostbyname failed");
 	close(sock);
 	exit(1); 
 }

 memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
 server.sin_port = htons(2046);
 
 if(connect(sock, (struct sockaddr*) &server, sizeof(server)) < 0)
 {
 	perror("connect failed");
 	close(sock);
 	exit(1);
 }	
 
 if(send(sock,DATA, sizeof(DATA),0) < 0)
 {
 	perror("send failed");
 	close(sock);
 	exit(1);
 }
 
 printf("Sent %s\n", DATA);
 close(sock);
 return 0;
}
