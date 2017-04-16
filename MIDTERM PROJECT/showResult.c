/*
	showResult.c
	GTU 2017 SYSTEM PROGRAMMING MIDTERM PROJECT 
	
	Muhammed Selim Dursun
	131044023
	16.04.2017
	
	
	
	This program wait data from client with fifo and print console some informations
	and then write it to log file that defined down
*/
#include <stdio.h>
#include <stdlib.h> //exit
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>// read | write
#include <signal.h>
//fifo perms
#define FIFO_PERMS (S_IRUSR | S_IWUSR)
/*all informations(pid,result1,result2,timelapsed1,timelapsed2) transport fifo*/
#define SHOWRESULT_FIFONAME "srfifo" 
#define CLIENTNUMBER_FILE "clientnum.txt"
#define ALLPIDFILENAME "allpid.txt"
#define SHOWRESULTLOGFILE "logs/showResult.log"
#define ALLFIFONAMESFILENAME "fifonames.txt"
#define SERVERPID_FILENAME "serverpid.txt"

// that will use when transport datas between clients and showresult
typedef struct package
{
	int pid;
	double result1;
	double result2;
	double timeLapsed1;
	double timeLapsed2;
}  showResultPackage_t;

/*
	Function save(fifo) @param name to temp txt file named fifonames.txt that defined at top
*/
void saveFifoName(char* name);
/*
	function save pid of current process
*/
void savePid();
/*
	function send a signal named SIGUSR2 except for current process
*/
void killAllProcess();
/*
	function unlink all fifos
*/
void unlinkAllFifos();
/*
	fuction remove all txt file that defined 
*/
void removeAllTxt();


/*
	signal handler for show result. This function handle SIGINT, SIGTSTP AND SIGUSR2
*/
static void signalHandler(int signo) 
{
	FILE* fp;
	if(signo == SIGINT || signo == SIGTSTP)
	{
		fp = fopen(SHOWRESULTLOGFILE,"a");
		fprintf(fp,"\n\n*************Ctrl^C or Ctrl^Z signal is receipted, program is terminating...*************\n\n");
		fclose(fp);
		killAllProcess();
		unlinkAllFifos();
		removeAllTxt();
		exit(0);
	}
 	if(signo == SIGUSR2)
 	{
		fp = fopen(SHOWRESULTLOGFILE,"a");
		fprintf(fp,"\n\n*************Kill signal is receipted, program is terminating...*************\n\n");
		fclose(fp);
 		exit(0);
 	}

}
int main( int argc, char **argv )
{
	int fifoRead;
	FILE* fp;
	showResultPackage_t showResultPackage;
	struct sigaction sigact;

	if(argc > 1)
	{
		fprintf(stderr,"Usage: ./showresult\n");
		exit(1);
	}
	sigact.sa_handler = signalHandler;
	sigact.sa_flags = 0;
	//adding signal to signal set
	if ( (sigemptyset(&sigact.sa_mask) == -1) || (sigaction(SIGINT, &sigact, NULL) == -1) ||
	 (sigaction(SIGUSR2, &sigact, NULL) == -1) || (sigaction(SIGTSTP, &sigact, NULL) == -1) ) 
	{
		perror("Signal handler setting failed.");
		exit(1);
	}
	//creating fifo
	if(mkfifo(SHOWRESULT_FIFONAME, FIFO_PERMS) == -1) 
	{
		fprintf(stderr,"Fifo creating error!\n");
		exit(1);
	}
	saveFifoName(SHOWRESULT_FIFONAME);
	savePid();
	
	while(1)
	{
		if((fifoRead = open(SHOWRESULT_FIFONAME, O_RDONLY)) == -1 )
		{
			fprintf(stderr,"Error when opening the fifo!\n");
			exit(1);
		}
		
		if(read(fifoRead, &showResultPackage, sizeof(showResultPackage_t)) > 0)
		{
			close(fifoRead);
			fprintf(stderr,"Pid:%d \t Result1:%6.3f\t Result2:%6.3f\n", showResultPackage.pid, showResultPackage.result1, showResultPackage.result2);
			fp = fopen(SHOWRESULTLOGFILE,"a");
			fprintf(fp,"Pid:%d\nResult1:%6.3f\tTime elapsed1:%6.4f\nResult2:%6.3f\tTime elapsed2:%6.4f\n\n", 
					showResultPackage.pid, showResultPackage.result1, showResultPackage.timeLapsed1, showResultPackage.result2, showResultPackage.timeLapsed2);
			fclose(fp);
		}		
	
	}
	return 0;
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
	function save pid of current process
*/
void savePid()
{
	FILE *fp = fopen(ALLPIDFILENAME,"a");
	fprintf(fp,"%d\n",getpid());
	fclose(fp);

}
/*
	function unlink all fifos
*/
void unlinkAllFifos()
{
	char fifoName[20];
	FILE *fp = fopen(ALLFIFONAMESFILENAME,"r");
	while (fscanf(fp, "%s", fifoName) != EOF) 
	{
		unlink(fifoName);
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
	fuction remove all txt file that defined 
*/
void removeAllTxt()
{
	remove(SERVERPID_FILENAME);
	remove(CLIENTNUMBER_FILE);
	remove(ALLPIDFILENAME);
	remove(ALLFIFONAMESFILENAME);
}
