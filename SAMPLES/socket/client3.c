#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#define SUNUCUPORT 46741
#define SUNUCUIP "127.0.0.1"
#define AZAMIUZUNLUK 1024

int main(){  
    int sockfd; 
    struct sockaddr_in serverAddr;
    char *gonder = "Kaynak hangi site?";
    char str[AZAMIUZUNLUK];
    int gelenBayt, gidenBayt, structSize;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if(-1 == sockfd){
        perror("socket");
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SUNUCUPORT);
    serverAddr.sin_addr.s_addr = inet_addr(SUNUCUIP);
    memset(&(serverAddr.sin_zero), '\0', 8);

   if(-1 == connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr))){
    	perror("connect");
	}

	/*gidenBayt = send(sockfd, gonder, strlen(gonder), 0);
	if(-1 == gidenBayt){  
	    perror("send");
	}
	else if(strlen(gonder) != gidenBayt){
	    printf("Gönderilen: %lu\tGiden: %d\n", strlen(gonder), gidenBayt);
	}
	printf("%d bayt gönderdim:\t%s\n", gidenBayt, gonder);
*/
	gelenBayt = recv(sockfd, &str, AZAMIUZUNLUK-1, 0);
	if(-1 == gelenBayt){  
	    perror("recv");
	}
	else if(0 == gelenBayt){  
	    printf("Bağlantı kapalı.\n");
	}
	printf("%d bayt aldım:\t%s\n", gelenBayt, str);

	close(sockfd);
 sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if(-1 == sockfd){
        perror("socket");
    }
	gelenBayt = recv(sockfd, &str, AZAMIUZUNLUK-1, 0);
	if(-1 == gelenBayt){  
	    perror("recv");
	}
	else if(0 == gelenBayt){  
	    printf("Bağlantı kapalı.\n");
	}
	printf("%d bayt aldım:\t%s\n", gelenBayt, str);

	close(sockfd);
	
 
}
