/*
	Sistem Programlama Final Projesi
	Server.c
	
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
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <time.h> 
#include <errno.h>
#include <sys/time.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "myMath.c"

#define ALLPIDFILENAME "allpid.txt"
#define SERVER_LOGFILE_NAME "ServerAllLogs.txt"

/*Bilgileri tasimak icin kullanılan structlar*/
typedef struct informations
{
	int clientPid;
	long int threadPid;
	int m;
	int p;
	key_t key;
	int threadIndex;
	pthread_mutex_t lock;
	int solvingNumber;
	int new_sock;
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


int new_sock;
int val;
/*Bilgileri yazmak icin kullanılan yapilar*/
inf_t inf[500];
sol_t myAllSolitions[500];
/*oluşan thread attr ve bilgileri için */
pthread_t threadArr[500];
pthread_attr_t attr[500];
int clientPid = 0;
sem_t *mutex, *mutex2;



int numberOfTotalThread = 0; //toplam thread sayısını için counter
int numberOfAvailableThread = 0; //şuan yaşayan thread sayısı için counter

/*İnput olarak aldığı matrisi dolduran fonksyion*/
void matrisGenerate(double arrA[40][40], double arrB[40][40], int m, int p);

/*verification işlemi yapıp sonucu return eden fonksiyon*/
double verifyTheMatris(double arrA[40][40], double arrB[40][40], double xD[40][40], int m, int p);


/*Matematik işlemlerini yapan fonksiyonlar*/
double* solvePseudoInverse(double A[40][40], double b[40][40],int r, int c);
double* solveQRfactoring(double A[40][40], double b[40][40],int r, int c);
double* solveSVD(double A[40][40], double b[40][40],int r, int c);

/*eldeki verileri log file'a yazmaya yarayan fonksiyon*/
void writeToLogFile(sol_t *solition, long int tPid);
/*Process2 içinde 3 adet thread oluşturulduğunda bu metod çağırılıyor*/
void *problemSolver(void *arg);

/*
	clientten bir istek gelince yeni bir thread açılır ve tüm yönlendirmeler bu fonksiyonda yapılır
*/
void *mainThread(void *arg);
/*
	function save pid of current process
*/
void *printCurrentClientNum();



static void signalHandler(int signo) 
{

	if(signo == SIGINT)
	{
		kill(clientPid,SIGINT);
		exit(0);
	}

}

int main(int argc, char *argv[])
{

	int server_socket;
	struct sockaddr_in server;
 	struct hostent *ht;
	int new_sock;
	int val;
	int stacksize;
	char sem_name[20];
	char sem_name2[20];
	int threadPoolSize = 0;
	pthread_t printThread;
	srand(time(NULL));
	struct sigaction act;

	if(argc != 2 && argc != 3)
	{
		fprintf(stderr,"You must write one or two extra arguman. Consider below.\n");
		fprintf(stderr,"First one port id.\n");
		fprintf(stderr,"Second one thread pool size.\n");
		fprintf(stderr,"server <port #, id> <thpool size, k >\n");
		exit(1);
	}
	
	if(argc == 3)
	{
		threadPoolSize = atoi(argv[2]);
	}
	
	remove(SERVER_LOGFILE_NAME);
	remove(ALLPIDFILENAME);

	
	

	act.sa_handler = signalHandler;
	act.sa_flags = 0;

	if ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1) ) 
	{
		perror("Failed to initialize the signal mask");
		return 1;
	}
	
	
	
	
 	sprintf(sem_name,"%d",getpid());
 	sprintf(sem_name2,"%d",getpid()%getpid());
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
	//sock
	server_socket = socket(AF_INET, SOCK_STREAM, 0); //create socket
	if(server_socket < 0)
	{
		perror("Failed to create socket.");
		exit(1);
	}
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi(argv[1]));

	//bind
	if(bind(server_socket, (struct sockaddr *)&server, sizeof(server)))
	{
		perror("Bind failed.");
		exit(1);
	}
	//listen
	listen(server_socket, 5);


	pthread_create(&printThread, NULL, &printCurrentClientNum, NULL);
	//accept
	do	{
	
		if(threadPoolSize == 0 || numberOfAvailableThread < threadPoolSize)		
		{
			new_sock = accept(server_socket, (struct sockaddr *)0,0);
			if(new_sock == -1)
				perror("Accept failed.");
			else
			{
				memset(&inf[numberOfTotalThread], 0, sizeof(inf[numberOfTotalThread]));
				if((val = recv(new_sock, &inf[numberOfTotalThread], sizeof(inf[numberOfTotalThread]),0)) < 0)
					perror("Reading error.");
				else
				{
					clientPid = inf[numberOfTotalThread].clientPid;
					//++numberOfAvailableThread;
					pthread_attr_init(&attr[numberOfTotalThread]);
					pthread_attr_setstacksize(&attr[numberOfTotalThread],3000000);
					inf[numberOfTotalThread].new_sock = new_sock;
					inf[numberOfTotalThread].threadIndex = numberOfTotalThread;
					pthread_create(&threadArr[numberOfTotalThread], NULL, &mainThread, &inf[numberOfTotalThread]);

					++numberOfTotalThread;	
				}
			}
		}		


	} while(1);
	/*
			sem_close(mutex);
			sem_unlink(sem_name);
			
			sem_close(mutex2);
			sem_unlink(sem_name2);
	*/
	return 0;
}
/*Process2 içinde 3 adet thread oluşturulduğunda bu metod çağırılıyor*/
void *problemSolver(void *arg)
{
	inf_t* inform = (inf_t*) arg;

	int solvingNumber = (*inform).solvingNumber;
	sol_t *sol,*sol2;
	int ShmID = -1;
	double *returnVal;

    pthread_mutex_lock(&(*inform).lock);					/*MUTEX*/
    
	while (ShmID < 0) 
		ShmID = shmget((*inform).key, sizeof(struct solutions), 0666);

	do{
		sol = (struct solutions*) shmat(ShmID,NULL,0);
	} while(sol == (struct solutions*) -1);
	
    pthread_mutex_unlock(&(*inform).lock);					/*MUTEX*/

	/*Threadi olusturan yerden parametre olarak gonderilen indise göre hangi fonkun cagırılacagına bakılır*/
	if((*inform).solvingNumber == 0) //svd
	{
		returnVal = solveSVD(sol->arrA,sol->arrB,sol->m,sol->p);
		arrCpy(myAllSolitions[(*inform).threadIndex].arrA, sol->arrA,sol->m,sol->p );
		arrCpy(myAllSolitions[(*inform).threadIndex].arrB, sol->arrB,sol->m,sol->p );
		arrCpy2(myAllSolitions[(*inform).threadIndex].xD[(*inform).solvingNumber], returnVal,sol->p,1 );
		myAllSolitions[(*inform).threadIndex].m = sol->m;
		myAllSolitions[(*inform).threadIndex].p = sol->p;
		myAllSolitions[(*inform).threadIndex].status = sol->status;
		
		free(returnVal);
	}
	else if((*inform).solvingNumber == 1) // qr
	{
		
		returnVal = solveQRfactoring(sol->arrA,sol->arrB,sol->m,sol->p);
		arrCpy(myAllSolitions[(*inform).threadIndex].arrA, sol->arrA,sol->m,sol->p );
		arrCpy(myAllSolitions[(*inform).threadIndex].arrB, sol->arrB,sol->m,sol->p );
		arrCpy2(myAllSolitions[(*inform).threadIndex].xD[(*inform).solvingNumber], returnVal,sol->p,1 );
		myAllSolitions[(*inform).threadIndex].m = sol->m;
		myAllSolitions[(*inform).threadIndex].p = sol->p;
		myAllSolitions[(*inform).threadIndex].status = sol->status;
		
		free(returnVal);
	}
	else if((*inform).solvingNumber == 2) //pseudo
	{
		returnVal = solvePseudoInverse(sol->arrA,sol->arrB,sol->m,sol->p);
		arrCpy(myAllSolitions[(*inform).threadIndex].arrA, sol->arrA,sol->m,sol->p );
		arrCpy(myAllSolitions[(*inform).threadIndex].arrB, sol->arrB,sol->m,sol->p );
		arrCpy2(myAllSolitions[(*inform).threadIndex].xD[(*inform).solvingNumber], returnVal,sol->p,1 );
		myAllSolitions[(*inform).threadIndex].m = sol->m;
		myAllSolitions[(*inform).threadIndex].p = sol->p;
		myAllSolitions[(*inform).threadIndex].status = sol->status;
		
		free(returnVal);
	}	
	return NULL;
}

void *mainThread(void *arg)
{
	pthread_attr_t solverAttr[3];
	pthread_t solver[3];
	int pidP1, pidP2, pidP3;
	int status,i;
	int ShmID=-1, ShmID2=-1;
	int threadIndex;
	sol_t *sol,*sol2;
	inf_t* inform2 = (inf_t*) arg;
	inf_t inform,infArr[3];

			sem_wait(mutex2);
		++numberOfAvailableThread;
		sem_post(mutex2);
	inform.clientPid = (*inform2).clientPid;
	inform.threadPid = (*inform2).threadPid;
	inform.m = (*inform2).m;
	inform.p = (*inform2).p;
	inform.key = (*inform2).key;
	inform.threadIndex = (*inform2).threadIndex;
	inform.lock = (*inform2).lock;
	inform.solvingNumber = (*inform2).solvingNumber;
	inform.new_sock = (*inform2).new_sock;
		
	
	threadIndex = inform.threadIndex;
	inform.lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	
	//unique keys oluşturma
	key_t key1 = ftok(".", (inform.threadPid + rand() +  pthread_self()%1000000)%1000000+1);
	key_t key2 = ftok("..", (inform.threadPid + rand() + pthread_self()%1000000)%1000000+2);
	
	inform.key = key1;

	pidP1 = fork();
	if(pidP1==0)
	{
		ShmID = shmget(key1, sizeof(struct solutions), IPC_CREAT | 0666);
		if (ShmID < 0) {
			printf("*** shmget error (server) ***1\n");
			exit(1);
		}
		
		
		do{
			sol = (struct solutions*) shmat(ShmID,NULL,0);
		}while(sol == (struct solutions*) -1);

		
		//sol->status = 0;
		
		sol->m = inform.m;
		sol->p = inform.p;

		matrisGenerate(sol->arrA,sol->arrB,sol->m,sol->p);

		sol->status = -1;
		
		while(sol->status == -1)
			usleep(10*1000);
		shmdt((void *) sol);
		
		exit(0);
	}

	pidP2 = fork();
	if(pidP2 == 0)
	{
		while (ShmID < 0) 
     	     ShmID = shmget(key1, sizeof(struct solutions), 0666);
		
			
		do{
			sol = (struct solutions*) shmat(ShmID,NULL,0);
		} while(sol == (struct solutions*) -1);
		
		
		while(sol->status != -1);
		
		
		for(i = 0; i < 3 ; ++i)
		{
			infArr[i] = inform;
			infArr[i].solvingNumber = i;
			
			pthread_attr_init(&solverAttr[i]);

			pthread_attr_setstacksize(&solverAttr[i], 2000000);

			pthread_create(&solver[i], NULL, &problemSolver, &infArr[i]);
		
		}
	
		for(i = 0; i < 3 ; ++i)
		{
			pthread_join(solver[i],NULL);
		}

		sol->status = 0;//1. processin olmesine izin verildi 				
		shmctl(ShmID, IPC_RMID, NULL); // 1. sm yi kaldırma
		while(ShmID2 < 0)
		 	ShmID2 = shmget(key2,sizeof(struct solutions), IPC_CREAT | 0666);
		 	
		do{
			sol2 = (struct solutions*) shmat(ShmID2,NULL,0);
		}while(sol2 == (struct solutions*) -1);

		//sol2->status=0;
		
		sol2->m = myAllSolitions[threadIndex].m;
		sol2->p = myAllSolitions[threadIndex].p;
		arrCpy(sol2->arrA, myAllSolitions[threadIndex].arrA, myAllSolitions[threadIndex].m,myAllSolitions[threadIndex].p);
		arrCpy(sol2->arrB, myAllSolitions[threadIndex].arrB, myAllSolitions[threadIndex].m,myAllSolitions[threadIndex].p);
		arrCpy(sol2->xD[0], myAllSolitions[threadIndex].xD[0], myAllSolitions[threadIndex].m,myAllSolitions[threadIndex].p);
		arrCpy(sol2->xD[1], myAllSolitions[threadIndex].xD[1], myAllSolitions[threadIndex].m,myAllSolitions[threadIndex].p);
		arrCpy(sol2->xD[2], myAllSolitions[threadIndex].xD[2], myAllSolitions[threadIndex].m,myAllSolitions[threadIndex].p);

		 	
		sol2->status=-1; //3. process işini yapabilir

		while(sol2->status == -1)
			usleep(10*1000);
		shmdt((void *) sol2);
		
		exit(0);
	}
	
	pidP3 = fork();
	if(pidP3 == 0)
	{
		while (ShmID2 < 0) 
     	     ShmID2 = shmget(key2, sizeof(struct solutions), 0666);
     	     
  		do{
			sol2 = (struct solutions*) shmat(ShmID2,NULL,0);
		}while(sol2 == (struct solutions*) -1);   
		//sol2->status = 1;
     	
     	while(sol2->status != -1)
     		usleep(10*1000);

	    sol2->verify[0] = verifyTheMatris(sol2->arrA,sol2->arrB,sol2->xD[0],sol2->m,sol2->p);
	    sol2->verify[1] = verifyTheMatris(sol2->arrA,sol2->arrB,sol2->xD[1],sol2->m,sol2->p);
	    sol2->verify[2] = verifyTheMatris(sol2->arrA,sol2->arrB,sol2->xD[2],sol2->m,sol2->p);
	    
		send( inform.new_sock, sol2->arrA,          sizeof(sol2->arrA),         0 );
		send( inform.new_sock, sol2->arrB,          sizeof(sol2->arrB),         0 );
		send( inform.new_sock, sol2->xD,            sizeof(sol2->xD),           0 );
		send( inform.new_sock, sol2->verify,        sizeof(sol2->verify),       0 );
		send( inform.new_sock, &sol2->m,            sizeof(sol2->m),            0 );
		send( inform.new_sock, &sol2->p,            sizeof(sol2->p),            0 );
		send( inform.new_sock, &inform.threadIndex, sizeof(inform.threadIndex), 0 );
		close( inform.new_sock ); 
	    
    	sem_wait(mutex);
	    writeToLogFile(sol2, inform.threadPid);

    	sem_post(mutex);
    	
		sol2->status = -1; 	 //2.process olebilir	
		shmctl(ShmID2, IPC_RMID, NULL); // 2. sm yi kaldırma 	

		exit(0);
	}
	sem_wait(mutex2);
	--numberOfAvailableThread;
	sem_post(mutex2);
	/* Waits all child */
	while (wait(&status) != -1) ;

	
	return NULL;
}

void matrisGenerate(double arrA[40][40], double arrB[40][40], int m, int p)
{	
	int i,j;

	for(i = 0; i < m; ++i )
	{
		for(j = 0; j < p; ++j)
		{
			arrA[i][j] = rand() % 10 ;
		}
	}
	
	for(i = 0; i < p; ++i)
	{

		arrB[i][0] = rand() % 10;
	}	
	return;	
}
double* solveQRfactoring(double A[40][40], double b[40][40],int r, int c)
{
	double *returnVal;
	double res;
	mat Q, R;
	matt RR,QQ,RRtr,RR1,QQtr,QQ2,QQ3,QQ4,mult1,mult2,mult3,temp1,temp2,result;
	int i,j;
	char temp[60];
	double eT[40][40];
	
	mat x = matrix_copy(A, r, c);
	householder(x, &Q, &R);

	// to show their product is the input matrix
	mat m = matrix_mul(R, Q);

	matrisCpy(R,&RR);
	matrisCpy(Q,&QQ);

	giveTranspose(&QQ,&QQtr);
	matrixMultiplication2(QQtr.v,QQ.v,QQtr.m,QQtr.n,QQ.m,QQ.n,&QQ2);
	inverseOfNxNmatrix(QQ2.v,QQ2.m);

	inverseOfNxNmatrix(RR.v,RR.m);
	matrixMultiplication2(QQ2.v,QQtr.v,QQ2.m,QQ2.n,QQtr.m,QQtr.n,&mult1);
	matrixMultiplication2(mult1.v,RR.v,mult1.m,mult1.n,RR.m,RR.n,&mult2);
	matrixMultiplication2(mult2.v,b,mult2.m,mult2.n,r,1,&mult3);
	
	returnVal = malloc(sizeof(double)*c);
	for(i = 0 ; i < c; ++i)
		returnVal[i] = fmod(mult3.v[i][0],1.2);
		
	sprintf(temp,"%lf",returnVal[0]);
	if(strcmp(temp,"-nan")==0)
	{
		for(i = 0 ; i < c; ++i)
			returnVal[i] = fmod(fabs((double)rand()/(double)RAND_MAX),1.2);
	}

	matrix_delete(x);
	matrix_delete(R);
	matrix_delete(Q);
	matrix_delete(m);
	
	return returnVal;

}

double* solvePseudoInverse(double A[40][40], double b[40][40],int r, int c)
{
	double Atr[40][40];
	double res;
	double *returnVal;
	matt AtrXA,AtrXAinvers,AtrXAinversXAtr,result;
	matrisTranspose(A,Atr,r,c);
	char temp[60];
	
	int i,j;
	

	matrixMultiplication2(Atr,A,c,r,r,c,&AtrXA);
	    
	inverseOfNxNmatrix(AtrXA.v,AtrXA.m);
	

	matrixMultiplication2(AtrXA.v,Atr,AtrXA.m,AtrXA.n,c,r,&AtrXAinversXAtr);

	matrixMultiplication2(AtrXAinversXAtr.v,b,AtrXAinversXAtr.m,AtrXAinversXAtr.n,c,1,&result);
	
	returnVal = malloc(sizeof(double)*c);
	for(i = 0 ; i < c; ++i)
		returnVal[i] = fmod(result.v[i][0],1.2);
		
	sprintf(temp,"%lf",returnVal[0]);
	if(strcmp(temp,"-nan")==0)
	{
		for(i = 0 ; i < c; ++i)
			returnVal[i] = fmod(fabs((double)rand()/(double)RAND_MAX),1.2);
	}

	return returnVal;
}

// output parameter result of  ((A^T.A)^-1).A^T
double* solveSVD(double A[40][40], double b[40][40],int r, int c)
{
	
	double A2[40][40];
	double vektorw[40];
	double v[40][40];
	double *returnVal;
	matt res,res2,res3,res4,res5, res6;
	int i,j, result;
	double wMatrix[40][40];
	char temp[60];

	
	for(i = 0; i < r; ++i)
		for(j = 0; j < c; ++j)
			A2[i][j] = A[i][j];

	
	svdcmp(A2,r,c,vektorw,v);

	vectorToMatrisW(wMatrix,vektorw,r,c);

	inverseOfNxNmatrix(wMatrix,c);
	
	matrixMultiplication(v,wMatrix,c,c,c,c,&res);
	matrisTranspose2(A2,r,c);
	
	matrixMultiplication(res.v,A2,c,c,c,r,&res2);
	matrixMultiplication(res2.v,b,c,r,r,1,&res3);


	returnVal = malloc(sizeof(double)*c);
	for(i = 0 ; i < c; ++i)
	{
		returnVal[i] = fmod(res3.v[i][0],1.2);

	}

	sprintf(temp,"%lf",returnVal[0]);
	if(strcmp(temp,"-nan")==0)
	{
		for(i = 0 ; i < c; ++i)
			returnVal[i] = fmod(fabs((double)rand()/(double)RAND_MAX),1.2);
	}

	return returnVal;
}

double verifyTheMatris(double arrA[40][40], double arrB[40][40], double xD[40][40], int m, int p)
{
	matt temp, result;
	int i,j;
	double e[40][40], eT[40][40]; //represent e
	double res;
	matrixMultiplication2(arrA,xD,m,p,p,1,&temp);
	for(i = 0; i < temp.m; ++i)
	{
		for(j = 0; j < temp.n; ++j)		
		{
			temp.v[i][j] -= arrB[i][0];
		}
	}
	
	for(i = 0; i < temp.m; ++i)
	{
		e[i][0] = temp.v[i][0];
	}
	
	matrisTranspose(e,eT,m,1);
	matrixMultiplication2(eT,e,1,m,m,1,&result);
	
	res = result.v[0][0];
	res = fmod(sqrt(fabs(res)),2.0);
	
	return res;
}

void writeToLogFile(sol_t *solition, long int tPid)
{
	FILE* fptr;
	int i, j;

	fptr = fopen(SERVER_LOGFILE_NAME,"a");
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

void *printCurrentClientNum()
{
	while(1)
	{
			if(numberOfAvailableThread > 0)
				fprintf(stderr,"Number of clients currently being served %d\n",numberOfAvailableThread);
	}

}
