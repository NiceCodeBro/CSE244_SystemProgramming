/*
	timerServer.c
	GTU 2017 SYSTEM PROGRAMMING MIDTERM PROJECT 
	
	Muhammed Selim Dursun
	131044023
	16.04.2017
	
	This program simulate a server.
	Wait a signal from clients and generate a matrix.
	Send matris with fifo to clients.
	And wait new signal to repeat this process
*/
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdlib.h> //exit
#include <sys/types.h>
#include <unistd.h> // read | write
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h> 
#include <sys/time.h>

#define SERVERPID_FILENAME "serverpid.txt"
#define CLIENTNUMBER_FILE "clientnum.txt"
#define ALLPIDFILENAME "allpid.txt"
#define TIMERSERVERLOGFILE "logs/timerserver.log"
#define ALLFIFONAMESFILENAME "fifonames.txt"
//fifo perms
#define FIFO_PERMS (S_IRUSR | S_IWUSR)

/*
	function calculate determinant and return it.
	@param f is will calculating determinant of matrix
	@param x is dimension of matrix 
	
	referance site:
	http://study-for-exam.blogspot.com.tr/2013/04/write-c-progrm-to-find-determinant-of.html
*/
double determinant(double f[40][40],int x);

/*
	function generate matrix considering @param n and fill detArr that defined as global variable
*/
double det(int n);
/*
	function save pid of current process
*/
void savePid();
/*
	function send a signal named SIGUSR2 except for current process
*/
void killAllProcess();
/*
	Function save(fifo) @param name to temp txt file named fifonames.txt that defined at top
*/
void saveFifoName(char* name);
/*
	function unlink all fifos
*/
void unlinkAllFifos();
/*
	fuction remove all txt file that defined 
*/
void removeAllTxt();

/* when signal is arrived to server, this variable change to 1 and change return to 0*/
static int got = 0;
char mainPipeName[30];
char pidString[20];
int fifoWrite;
double detArr[40][40];/* orginal determinant array */

//for calculating the orginal determinant generating time
struct timeval stop, start;

/*
	signal handler for show result. This function handle SIGINT,SIGUSR1, SIGTSTP AND SIGUSR2
*/
static void signalHandler(int signo) 
{
	FILE* fp;
	if(signo==SIGUSR1)
	{
		got = 1;	
	}
	if(signo == SIGINT || signo == SIGTSTP)
	{
		fp = fopen(TIMERSERVERLOGFILE,"a");
		fprintf(fp,"\n\n*************Ctrl^C or Ctrl^Z signal is receipted, program is terminating...*************\n\n");
		fclose(fp);
		
		killAllProcess();
		unlinkAllFifos();
		removeAllTxt();
		exit(0);
	}
	if(signo == SIGUSR2)
	{
		fp = fopen(TIMERSERVERLOGFILE,"a");
		fprintf(fp,"\n\n*************Kill signal is receipted, program is terminating...*************\n\n");
		fclose(fp);
		exit(0);
	}
}

int main( int argc, char **argv )
{
	clock_t t1, t2; 
	int t3;  
	int ticksInMilisec;
	int n;
	int pid; //using at fork 
	int fifoRead;
	sigset_t intmask; //for masking signal
	int dif = 0;
	char clientPid[20];
	double resultOfDet; // variable that result of determinant
	FILE *fp ; //file pointer for read or write to text file
	
	if(argc != 4)
	{
		fprintf(stderr,"Usage: ./server ticksInMilisec n mainPipeName\n");
		exit(1);
	}
	
	n = atoi(argv[2]);
	ticksInMilisec = atoi(argv[1]);
	strcpy(mainPipeName,argv[3]);
	
	
	if(n < 1)
	{
		fprintf(stderr,"Usage: ./server ticksInMilisec n mainPipeName\n");
		fprintf(stderr,"n should be greater than 1\n");
		exit(1);
	}
	
	fp = fopen( SERVERPID_FILENAME, "w");
	fprintf(fp,"%d %d",getpid(),n);
	fclose(fp);
	
	fp = fopen( CLIENTNUMBER_FILE, "w");
	fprintf(fp,"1");
	fclose(fp);

	savePid();
	
	struct sigaction act;
	act.sa_handler = signalHandler;
	act.sa_flags = 0;

	if ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGUSR1, &act, NULL) == -1) 
			|| (sigaction(SIGINT, &act, NULL) == -1) || (sigaction(SIGUSR2, &act, NULL) == -1) || 
			(sigaction(SIGTSTP, &act, NULL) == -1) )
	{
		perror("Failed to initialize the signal mask");
		return 1;
	}
	//creating main pipe for get client pid
	if (mkfifo(mainPipeName, FIFO_PERMS) == -1) 
	{
	    fprintf(stderr,"Fifo creating error!\n");
		exit(1);
	}
	saveFifoName(mainPipeName);
	t1 = clock(); 	
	while(1)
	{	
		//referance page: http://stackoverflow.com/questions/8558625/how-to-get-the-current-time-in-milliseconds-in-c-programming
		t2 = clock();
		dif = ((int)(t2 - t1) / 1000000.0F ) * 1000;  
		//masking signal
		if (sigprocmask(SIG_BLOCK, &intmask, NULL) < 0) {
			perror ("sigprocmask");
			return 1;	
		}

		//unmasking only ticksInMilisec 
		if( dif >= ticksInMilisec )
		{
			if (sigprocmask(SIG_UNBLOCK, &intmask, NULL) < 0) 
			{
				perror ("sigprocmask");
				return 1;
			}
			t1 = clock();
		}
		if(got == 1) //signal is detected 
		{
			got = 0;
			if((fifoRead = open(mainPipeName, O_RDONLY)) != -1 )
			{
				read(fifoRead, clientPid, sizeof(clientPid));
				close(fifoRead);			
				pid = fork();
				if(pid == 0)
				{
					savePid();
					if((fifoWrite = open(clientPid, O_WRONLY)) == -1 )
					{
						fprintf(stderr,"Error when opening the fifo_!\n");
						exit(1);
					}
					resultOfDet = det(n);
					write(fifoWrite, detArr, sizeof(int)*1600); //write orginal determinant to client's fifo 
					fp = fopen(TIMERSERVERLOGFILE,"a");				
					fprintf(fp,"Matris generating time:%lu ms\t\tClient pid:%s\tDetereminant of matris:%f\n",stop.tv_usec - start.tv_usec,clientPid,resultOfDet);
					fclose(fp);
					close(fifoWrite);
					exit(0);
				}
				else if(pid > 0)
				{		
				}
				else
				{
					perror("Fork error!");
					exit(1);
				}
			}
		}
	}
	//remove file
	return 0;
}
/*
	function generate matrix considering @param n and fill detArr that defined as global variable
*/
double det(int n)
{
	int i,j;
	double result = 0;
	clock_t t1,t2,t3;
	srand(time(NULL));
	
	gettimeofday(&start, NULL);
	while(result == 0)
	{
		for(i = 0; i < (2*n); ++i)
		{
			for(j = 0; j < (2*n); ++j)
			{
				t1 = clock();
				detArr[i][j] = (double)((rand()%75) + (int)t1%10 + 5);
			}
		}
		result =  determinant(detArr,n);
	}
	gettimeofday(&stop, NULL);

	return result;
}
/*
	function calculate determinant and return it.
	@param f is will calculating determinant of matrix
	@param x is dimension of matrix 
	
	referance site:
	http://study-for-exam.blogspot.com.tr/2013/04/write-c-progrm-to-find-determinant-of.html
*/
double determinant(double f[40][40],int x)
{
  double c[40],d=0,b[40][40];
  int pr,j,p,q,t;
  int r=1,s=1;
  if(x==2)
  {
    d=0;
    d=(f[1][1]*f[2][2])-(f[1][2]*f[2][1]);
    return(d);
   }
  else
  {
    for(j=1;j<=x;j++)
    {        
      r=1; 
      s=1;
      for(p=1;p<=x;p++)
        {
          for(q=1;q<=x;q++)
            {
              if(p!=1&&q!=j)
              {
                b[r][s]=f[p][q];
                s++;
                if(s>x-1)
                 {
                   r++;
                   s=1;
                  }
               }
             }
         }
     for(t=1,pr=1;t<=(1+j);t++)
     pr=(-1)*pr;
     c[j]=pr*determinant(b,x-1);
     }
     for(j=1,d=0;j<=x;j++)
     {
       d=d+(f[1][j]*c[j]);
      }
     return(d);
   }
}

/*
	function save pid of current process
*/
void savePid()
{
	FILE *fp = fopen(ALLPIDFILENAME,"a");
	fprintf(fp,"%d\n",getpid());
	fclose(fp);

}
/*
	function send a signal named SIGUSR2 except for current process
*/
void killAllProcess()
{
	int pid;
	FILE *fp = fopen(ALLPIDFILENAME,"r");
	while (fscanf(fp, "%d", &pid) != EOF) {
		if(pid != getpid())
		{
			kill(pid,SIGUSR2);
		}
	}
}
/*
	Function save(fifo) @param name to temp txt file named fifonames.txt that defined at top
*/
void saveFifoName(char* name)
{
	FILE *fp = fopen(ALLFIFONAMESFILENAME,"a");
	fprintf(fp,"%s\n",name);
	fclose(fp);
}
/*
	function unlink all fifos
*/
void unlinkAllFifos()
{
	char fifoName[20];
	FILE *fp = fopen(ALLFIFONAMESFILENAME,"r");
	while (fscanf(fp, "%s", 	fifoName) != EOF) {
		unlink(fifoName);
	}
}

/*
	fuction remove all txt file that defined 
*/
void removeAllTxt()
{
	remove(SERVERPID_FILENAME);
	remove(CLIENTNUMBER_FILE);
	remove(ALLPIDFILENAME);
	remove(ALLFIFONAMESFILENAME);
}
