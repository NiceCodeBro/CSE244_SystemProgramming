/*
	Sistem Programlama Final Projesi
	Clients.c
	
	Muhammed Selim Dursun
	131044023
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>

#define CLIENT_LOGFILE_NAME "ClientAllLogs.txt"

/*Bilgileri tasimak icin kullanılan structlar*/
typedef struct informations
{
	int clientPid;
	long int threadPid;
	int m;
	int p;

} inf_t;
typedef struct solutions
{
	double arrA[40][40];
	double arrB[40][40];
	double xD[3][40][40];
	double verify[3];
	int m;
	int p;
	int status;
	int indexNumber;
} sol_t;

char sem_name[20];
char sem_name2[20];

sem_t *mutex, *mutex2;
pthread_t tid[100];

char portNumber[100];
/*elde edilen veriler log'a bu metod ile yazılır*/
void writeToLogFile(sol_t *solition, long int tPid);



//time referance https://stackoverflow.com/questions/5141960/get-the-current-time-in-c
static void signalHandler(int signo) 
{
	FILE* fp;
  	time_t tt;
    struct tm * timeinfo;

    time ( &tt );
    timeinfo = localtime ( &tt );


	if(signo == SIGINT)
	{
		fp = fopen(CLIENT_LOGFILE_NAME,"a");
		
		fprintf(fp,"Client terminaline sinyal geldi. Sinyal zamanı: ");
	    fprintf(fp, "[%d %d %d %d:%d:%d]\n",timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		
		fclose(fp);
		exit(0);
	}

}
/*
	clienttaki tüm işi yapan thread fonksiyonu
	öncelikle bir istek yollar sonrasında işlenmiş matrisleri geri alır
	tüm bu haberleşmeler soket üzerinden yapılmaktadır.
*/
void *doSomeThing(void *arg)
{

	int client_socket;
	struct sockaddr_in server;
	int *generateMatrixArr = (int*) arg;
	inf_t inf;
	sol_t sol;
	
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi(portNumber));
	inf.m = generateMatrixArr[0];
	inf.p = generateMatrixArr[1];

	remove(CLIENT_LOGFILE_NAME);
	

	client_socket = socket(AF_INET, SOCK_STREAM, 0); //create socket
	if(client_socket < 0)
	{
		perror("Client socket");
		exit(1);
	}
	sem_wait(mutex);
	if(connect(client_socket, (struct sockaddr*) &server, sizeof(server)) < 0)
	{
		fprintf(stderr,"Port number does not match with server or Server not active yet.\n");
		close(client_socket);
		sem_close(mutex);
		sem_unlink(sem_name);
		sem_close(mutex2);
		sem_unlink(sem_name2);
		exit(1);
	}	

	sem_post(mutex);

	inf.clientPid = getpid();
	inf.threadPid = pthread_self();


	if(send(client_socket, &inf, sizeof(inf), 0) < 0)
	{
		perror("send");
		close(client_socket);
		exit(1);
	}



	recv( client_socket, sol.arrA,    	   sizeof(sol.arrA),		0 );
	recv( client_socket, sol.arrB,   	   sizeof(sol.arrB),		0 );
	recv( client_socket, sol.xD,     	   sizeof(sol.xD),			0 );
	recv( client_socket, sol.verify,	   sizeof(sol.verify),		0 );
	recv( client_socket, &sol.m,    	   sizeof(sol.m), 			0 );
	recv( client_socket, &sol.p,    	   sizeof(sol.p),			0 );
	recv( client_socket, &sol.indexNumber, sizeof(sol.indexNumber), 0 );
	
	sem_wait(mutex2);
	writeToLogFile(&sol,pthread_self());
	sem_post(mutex2);
	
	close( client_socket ); 

	return NULL;
}

int main(int argc, char *argv[])
{	

	pthread_t ozelthread;
    int err,i;
	int generateMatrixArr[2];
	int clientNum;
	struct sigaction act;
	if(argc != 5)
	{
		fprintf(stderr,"You must write four extra arguman. Consider below.\n");
		fprintf(stderr,"First one #of columns of A.\n");
		fprintf(stderr,"Second one #of rows of A.\n");
		fprintf(stderr,"Third one #of clients.\n");
		fprintf(stderr,"Fourth one port #\n");
		fprintf(stderr,"clients <#of columns of A, m> <#of rows of A, p> <#of clients, q> <port #, id>\n");
		exit(1);
	}
	

	
	
	
	act.sa_handler = signalHandler;
	act.sa_flags = 0;

	if ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1)) 
	{
		perror("Failed to initialize the signal mask");
		return 1;
	}
	
	
 	sprintf(sem_name,"%d",getpid());
 	sprintf(sem_name2,"%d",getpid()+getpid());
 	
	generateMatrixArr[0] = atoi(argv[1]);
	generateMatrixArr[1] = atoi(argv[2]);
	clientNum = atoi(argv[3]);
	strcpy(portNumber,argv[4]);
	
	mutex = sem_open(sem_name,O_CREAT,0644,1);//create semaphore
	if(mutex == SEM_FAILED)
	{
		perror("semaphore");
		sem_unlink(sem_name);
		exit(1);
	}
	mutex2 = sem_open(sem_name2,O_CREAT,0644,1);//create semaphore
	if(mutex2 == SEM_FAILED)
	{
		perror("semaphore");
		sem_unlink(sem_name2);
		exit(1);
	}
//******************************************************************************//



	for(i = 0; i < clientNum; ++i)
	pthread_create(&tid[i], NULL, &doSomeThing, generateMatrixArr);
	


	for(i = 0; i < clientNum; ++i)
    pthread_join(tid[i], NULL);



		
//******************************************************************************//

	sem_close(mutex);
	sem_unlink(sem_name);
	sem_close(mutex2);
	sem_unlink(sem_name2);

	return 0;
}


void writeToLogFile(sol_t *solition, long int tPid)
{
	FILE* fptr;
	int i, j;

	fptr = fopen(CLIENT_LOGFILE_NAME,"a");
	fprintf(fptr,"Log ID:%lu\n", tPid);
	fprintf(fptr,"Content Of A Matrix(%dx%d)\n",solition->m, solition->p );	
	for(i = 0; i < solition->m; ++i)
	{
		for(j = 0; j < solition->p; ++j)
		{
			fprintf(fptr,"%lf ", solition->arrA[i][j]);	
		}
		fprintf(fptr,"\n");	
	}
	fprintf(fptr,"\n");
	
	fprintf(fptr,"Content Of Xd1 Matrix(SVD)(%dx%d)\n",solition->p, 1 );	
	for(i = 0; i < solition->p; ++i)
	{
		fprintf(fptr,"%lf\n", solition->xD[0][i][0]);
	}		
	fprintf(fptr,"\n");
	fprintf(fptr,"Content Of Xd2 Matrix(QR)(%dx%d)\n",solition->p, 1 );	
	for(i = 0; i < solition->p; ++i)
	{
		fprintf(fptr,"%lf\n", solition->xD[1][i][0]);
	}		
	fprintf(fptr,"\n");
	fprintf(fptr,"Content Of Xd3 Matrix(Pseudo-Inverse)(%dx%d)\n",solition->p, 1 );	
	for(i = 0; i < solition->p; ++i)
	{
		fprintf(fptr,"%lf\n", solition->xD[2][i][0]);
	}		
	fprintf(fptr,"\n");	
	
	fprintf(fptr,"Content Of b Matrix(%dx%d)\n",solition->m, 1 );
	for(i = 0; i < solition->m; ++i)
	{
		fprintf(fptr,"%lf\n", solition->arrB[i][0]);
	}		
	fprintf(fptr,"\n");
	
	fprintf(fptr,"Error term for SVD %lf\n", solition->verify[0] );
	fprintf(fptr,"Error term for QR %lf\n", solition->verify[1] );
	fprintf(fptr,"Error term for Pseudo-Inverse %lf\n\n", solition->verify[2] );
	fclose(fptr);
}
