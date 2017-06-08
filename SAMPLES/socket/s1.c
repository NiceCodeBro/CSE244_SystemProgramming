#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define PORTNUM 2016

int compare_ints(const void * elem1, const void * elem2){
 int f = *((int*)elem1);
 int s = *((int*)elem2);
 if (f > s) return  1;
 if (f < s) return -1;
 return 0;
}

int find(int element,int *array,int length){
 int i;
 for (i=0;i<length;++i){
  if (element == array[i])
   return i;
 }
 return -1;
}



int populateLottery(char *buffer){
  int i,k=0,candidate,numbers[6]={50,50,50,50,50,50};
  numbers[0]= rand() % 49 + 1;
  for (i=1;i<6;++i){
     do {
       candidate= rand() % 49 + 1;
     } while(find(candidate,numbers,6)>=0);
     numbers[i]= candidate;
  }
  qsort(numbers,6,sizeof(int),compare_ints);
  for (i=0;i<6;++i){
     if (i<5)
        k += sprintf(buffer+k,"%d,",numbers[i]);
     else
        k += sprintf(buffer+k,"%d\n",numbers[i]);
  }
  return k;
}
void service(int connection){
 static int buffer_size,num,i;
 char *buffer= malloc(18*sizeof(char));
 read(connection,&num,sizeof(num));
 for (i=0;i<num;++i){
  buffer_size= populateLottery(buffer);
  write( connection, buffer,sizeof(buffer));
 }
    close(connection); 
 free(buffer);
}
int main() {

  struct utsname name;
  struct sockaddr_in socketname, client;
  int sd, ns, clientlen = sizeof(client);
  struct hostent *host;
  time_t today;

  /* determine server system name and internet address */
  if (uname(&name) == -1) {
    perror("uname");
    exit(1);
  }

  if ((host = gethostbyname(name.nodename)) == NULL) {
    perror("gethostbyname");
    exit(1);
  }

  /* fill in socket address structure */
  memset((char *) &socketname, '\0', sizeof(socketname));
  socketname.sin_family = AF_INET;
  socketname.sin_port = PORTNUM;
  memcpy( (char *) &socketname.sin_addr, host->h_addr, host->h_length);

  /* open socket */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  /* bind socket to a name */
  if (bind(sd, (struct sockaddr *) & socketname, sizeof(socketname))) {
    perror("bind");
    exit(1);
  }

  /* prepare to receive multiple connect requests */
  if (listen(sd, 128)) {
    perror("listen");
    exit(1);
  }
 
  while (1) {
    if ((ns = accept(sd, (struct sockaddr *)&client, &clientlen)) == -1) {
      perror("accept");
      exit(1);
    }
    service(ns);
  }
}

