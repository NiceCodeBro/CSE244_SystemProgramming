/*
	seeWhat.c
	GTU 2017 SYSTEM PROGRAMMING MIDTERM PROJECT 
	
	Muhammed Selim Dursun
	131044023
	16.04.2017
	
	
	This program send a signal (SIGUSR1) to server and wait a generated matrix from server
	with fifo and create 2 process. This processes handle this matrix in 2 way and give (with fifo) to parents. 
	Parents send this matrix to showresult program to printing to terminal.
*/

#include <stdio.h>
#include <signal.h>
#include <sys/stat.h> 
#include <stdlib.h> //exit
#include <sys/types.h>
#include <unistd.h> // read | write
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h> 
#include <math.h>
#include <sys/wait.h>
#include <errno.h>


#define SERVERPID_FILENAME "serverpid.txt"
#define SHOWRESULT_FIFONAME "srfifo"
#define CLIENTNUMBER_FILE "clientnum.txt"
#define ALLPIDFILENAME "allpid.txt"
#define ALLFIFONAMESFILENAME "fifonames.txt"
//fifo perms
#define FIFO_PERMS (S_IRUSR | S_IWUSR)

double shiftedInverseMatrix[40][40]; // shifted inverse matrix
double convolutionMatrix[40][40]; // convolution matrix
double detArr[40][40]; //orginal  matrix
double kernel[3][3]; //kernel matrix that will use generate 2d convolution matrix
char lastCreatedClientLogFileName[20];
int sizeOfMatris = 0;
char logFileName[20];
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
	fuction remove all txt file that defined 
*/
void removeAllTxt();
/*
	Function save(fifo) @param name to temp txt file named fifonames.txt that defined at top
*/
void saveFifoName(char* name);
/*
	function send a signal named SIGUSR2 except for current process
*/
void killAllProcess();
/*
	function save pid of current process
*/
void savePid();
/*
	function unlink all fifos
*/
void unlinkAllFifos();
/*
	function calculate determinant and return it.
	@param a is will calculating determinant of matrix
	@param k is dimension of matrix 
	
	referance site:
	http://study-for-exam.blogspot.com.tr/2013/04/write-c-progrm-to-find-determinant-of.html
*/
double determinant(double a[40][40], double k);
/*
	Function find cofactor of matrix
	referance : http://www.sanfoundry.com/c-program-find-inverse-matrix/
*/
void cofactor(double num[40][40], double f);
/*
	Function find transpose of matrix
	referance : http://www.sanfoundry.com/c-program-find-inverse-matrix/
*/
void transpose(double num[40][40], double fac[40][40], double r);
/*
	function divides the matrix into four separate pieces, inverse separate pieces
	and put its back
*/
void shiftedInverser(int n);
/*
	function divides the matrix into four separate pieces, take 2d convolution
	 separate pieces and put its back
*/
void convertConvMatris(int n, double conv[3][3]);

/*
	function calculate and generate that 2d convolotion matrix
	@param conv  > kernel matrix
	@param source > orginal matrix
	@param dest > will fill matrix
	@param n > number of dimension of matrixs (n x n)
*/
void resultOfCon(double conv[3][3], double source[40][40],double dest[40][40], int n);
/*
	x1 and y1 source matris cord, x2 and y2 conv. matris cord
*/
int isInRange( int x1,int y1, int x2, int y2, int n);
/*
	function init the kernel matrix(3x3) properly
	1 0 0
	0 1 0
	0 0 1
*/
void initKernel();
/*
	function print all matrix's last version to log file
*/
void printAllMatrixToLogFile();


/*
	signal handler for show result. This function handle SIGINT, SIGTSTP AND SIGUSR2
*/
static void signalHandler(int signo) 
{
	FILE* fp;
	if(signo == SIGINT || signo == SIGTSTP)
	{
		fp = fopen(lastCreatedClientLogFileName,"a");
		fprintf(fp,"\n\n*************Ctrl^C or Ctrl^Z signal is receipted, program is terminating...*************\n\n");
		fclose(fp);
		killAllProcess();
		unlinkAllFifos();
		removeAllTxt();
		exit(0);
	}
	if(signo == SIGUSR2)
	{
		fp = fopen(lastCreatedClientLogFileName,"w");
		fprintf(fp,"\n\n*************Kill signal is receipted, program is terminating...*************\n\n");
		fclose(fp);

		exit(0);
	}
}

int main( int argc, char **argv )
{	
	char *mainPipeName;
	int fifoWrite;
	int fifoRead;
	int serverPid, childPid;
	char pidString[20];
	char childFifoName[20];
	int i,j;
	int num ;
	int status; //variable of waiting all child
	double orginalMatrisDet;	
	int numberOfClient;
	FILE *fp;
	time_t t1,t2;
	showResultPackage_t showResultPackage;
	struct sigaction sigact;
		
	if(argc != 2)
	{
		fprintf(stderr,"Usage: ./client mainPipeName\n");
		exit(1);
	}
	
	mainPipeName = argv[1];
	
	srand(time(NULL));
	
	initKernel();

	sigact.sa_handler = signalHandler;
	sigact.sa_flags = 0;

	savePid();
	num = getpid() % 10 + 2;

	if ( (sigemptyset(&sigact.sa_mask) == -1) || (sigaction(SIGUSR1, &sigact, NULL) == -1) || 
		(sigaction(SIGINT, &sigact, NULL) == -1)  || (sigaction(SIGUSR2, &sigact, NULL) == -1)  || 
		(sigaction(SIGTSTP, &sigact, NULL) == -1)) 
	{
		perror("Signal handler setting failed.");
		exit(1);
	}
	
	fp = fopen( SERVERPID_FILENAME, "r") ;
	
	if(!fp)
	{
		fprintf(stderr,"Server not found, program was terminated.\n");
		exit(1);
	}
	fscanf(fp,"%d %d",&serverPid, &sizeOfMatris);
	fclose(fp);

	sprintf(pidString,"%d",getpid());

	saveFifoName(pidString);
	while(1)
	{	
		//every time each client sleep that 50 ms times it's pid's modded 10%  so I prevented conflict
		usleep(num*50000); 

		fp = fopen( CLIENTNUMBER_FILE, "r");
		fscanf(fp,"%d",&numberOfClient);
		fclose(fp);
			

		kill(serverPid,SIGUSR1);
		sprintf(logFileName,"logs/sw%d.log",numberOfClient);
		sprintf(lastCreatedClientLogFileName,"%s",logFileName);
		
		fp = fopen( CLIENTNUMBER_FILE, "w");
		fprintf(fp,"%d",++numberOfClient);
		fclose(fp);

		if(mkfifo(pidString, FIFO_PERMS) == -1) 
		{
			fprintf(stderr,"Fifo creating error!\n");
			exit(1);
		}	
		if( (fifoWrite=open(mainPipeName, O_WRONLY)) != -1 )
		{
			if(write(fifoWrite,pidString,sizeof(pidString)) < 0) 
			{
				perror("write error, client to server");
			}
			close(fifoWrite);
		
			if((fifoRead = open(pidString, O_RDONLY)) == -1 )
			{
				fprintf(stderr,"Error when opening the fifo!\n");
				exit(1);
			}
			read(fifoRead, detArr, sizeof(int)*1600);
			close(fifoRead);

			childPid = fork(); //first fork
			if(childPid == 0)
			{
				savePid();
				shiftedInverser(sizeOfMatris);
				sprintf(childFifoName,"child%d",getppid());
				if(mkfifo(childFifoName, FIFO_PERMS) != -1) 
				{
			
					if( (fifoWrite=open(childFifoName, O_WRONLY)) == -1 )
					{
						fprintf(stderr,"Error when opening the fifo!aa_\n");
						exit(1);
					}
					if(write(fifoWrite,shiftedInverseMatrix,sizeof(int)*1600) < 0) 
					{
						perror("write error, client to server");
						exit(1);
					}
					close(fifoWrite);
				}
				exit(0);
			}
			else if(childPid > 0)
			{
				usleep(30000);
				sprintf(childFifoName,"child%d",getpid());
				saveFifoName(childFifoName);
				if( (fifoRead=open(childFifoName, O_RDONLY)) != -1 )
				{
					if(read(fifoRead,shiftedInverseMatrix,sizeof(int)*1600) < 0) 
					{
						perror("write error, client to server");
						exit(1);
					}
					close(fifoRead);
					unlink(childFifoName);
				}
			}
			else
			{
				perror("Fork error!");
				exit(1);
			}
			
			childPid = fork(); //second fork
			if(childPid == 0)
			{
				savePid();
				convertConvMatris(sizeOfMatris,kernel);
				sprintf(childFifoName,"child%d",getppid());
				if(mkfifo(childFifoName, FIFO_PERMS) == -1) 
				{
					fprintf(stderr,"Fifo creating error!\n");
					exit(1);
				}
				if( (fifoWrite=open(childFifoName, O_WRONLY)) == -1 )
				{
					fprintf(stderr,"Error when opening the fifo!\n");
					exit(1);
				}
				if(write(fifoWrite,convolutionMatrix,sizeof(int)*1600) < 0) 
				{
					perror("write error, client to server");
					exit(1);
				}
				close(fifoWrite);
				exit(0);
			}
			else if(childPid > 0)
			{
				usleep(30000);
				sprintf(childFifoName,"child%d",getpid());
				if( (fifoRead=open(childFifoName, O_RDONLY)) == -1 )
				{
					fprintf(stderr,"Error when opening the fifo!\n");
					exit(1);
				}
				if(read(fifoRead,convolutionMatrix,sizeof(int)*1600) < 0) 
				{
					perror("write error, client to server");
					exit(1);
				}
				close(fifoRead);
				unlink(childFifoName);
			} 
			else
			{
				perror("Fork error!");
				exit(1);
			}
			orginalMatrisDet = determinant(detArr,sizeOfMatris);
			showResultPackage.pid = getpid();
			
			t1 = clock();
			showResultPackage.result1 = orginalMatrisDet - determinant(shiftedInverseMatrix,sizeOfMatris);
			t2 = clock();
			showResultPackage.timeLapsed1 = ((int)(t2 - t1) / 1000000.0F ) * 1000;   
					
			t1 = clock();
			showResultPackage.result2 = orginalMatrisDet - determinant(convolutionMatrix,sizeOfMatris);
			t2 = clock();
			showResultPackage.timeLapsed2 = ((int)(t2 - t1) / 1000000.0F ) * 1000;   
		
			//write all information to show result
			if( (fifoWrite=open(SHOWRESULT_FIFONAME, O_WRONLY)) != -1 )
			{
				write(fifoWrite,&showResultPackage,sizeof(showResultPackage_t));
				close(fifoWrite);
			}
			printAllMatrixToLogFile();
		
			unlink(pidString);
		}
					
	}
	return 0;
}
/*
	function print all matrix's last version to log file
*/
void printAllMatrixToLogFile()
{
	int i,j;
	FILE* fp = fopen( logFileName, "w");
	fprintf(fp ,"Orginal Matrix=\n[");		
	for(i = 0; i < sizeOfMatris*2; ++i)
	{
		for(j = 0; j < sizeOfMatris*2 ; ++j)
		{
		 	fprintf(fp ,"%f ",detArr[i][j]);		
		}
		if(i+1 == sizeOfMatris*2)
			fprintf(fp ,"]\n");	
		else
			fprintf(fp ,";");	
	}
	fprintf(fp ,"\n");	
	fprintf(fp ,"Shifted Inverse Matrix=\n[");	
	for(i = 0; i < sizeOfMatris*2; ++i)
	{
		for(j = 0; j < sizeOfMatris*2 ; ++j)
		{
		 	fprintf(fp ,"%f ",shiftedInverseMatrix[i][j]);		
		}
		if(i+1 == sizeOfMatris*2)
			fprintf(fp ,"]\n");	
		else
			fprintf(fp ,";");
	}

	fprintf(fp ,"\n");
	fprintf(fp ,"Convolution Matrix=\n[");		
	for(i = 0; i < sizeOfMatris*2; ++i)
	{
		for(j = 0; j < sizeOfMatris*2 ; ++j)
		{
		 	fprintf(fp ,"%f ",convolutionMatrix[i][j]);		
		}
		if(i+1 == sizeOfMatris*2)
			fprintf(fp ,"]\n");	
		else
			fprintf(fp ,";");
	}

	fclose(fp);
}
/*
	function divides the matrix into four separate pieces, inverse separate pieces
	and put its back
*/
void shiftedInverser(int n)
{
	double tempArr1[40][40];
	double tempArr2[40][40];
	double tempArr3[40][40];
	double tempArr4[40][40];

	int i,j;

	for(i = 0; i < n; ++i)
	{
		for(j = 0; j < n; ++j)
		{
			tempArr1[i][j] = detArr[i][j];
			tempArr2[i][j] = detArr[i][j+n];
			tempArr3[i][j] = detArr[i+n][j];
			tempArr4[i][j] = detArr[i+n][j+n];
		}	
	}
	cofactor(tempArr1,(double)n);
	cofactor(tempArr2,(double)n);
	cofactor(tempArr3,(double)n);
	cofactor(tempArr4,(double)n);
	
	for(i = 0; i < n; ++i)
	{
		for(j = 0; j < n; ++j)
		{
			shiftedInverseMatrix[i][j] = tempArr1[i][j];
			shiftedInverseMatrix[i][j+n] = tempArr2[i][j];
			shiftedInverseMatrix[i+n][j] = tempArr3[i][j];
			shiftedInverseMatrix[i+n][j+n] = tempArr4[i][j];
		}	
	}	
}
/*
	function divides the matrix into four separate pieces, take 2d convolution
	 separate pieces and put its back
*/
void convertConvMatris(int n, double conv[3][3])
{
	double tempArr1[40][40];
	double tempArr2[40][40];
	double tempArr3[40][40];
	double tempArr4[40][40];
	
	double tempArrResult1[40][40];
	double tempArrResult2[40][40];
	double tempArrResult3[40][40];
	double tempArrResult4[40][40];
	int i,j;

	for(i = 0; i < n; ++i)
	{
		for(j = 0; j < n; ++j)
		{
			tempArr1[i][j] = detArr[i][j];
			tempArr2[i][j] = detArr[i][j+n];
			tempArr3[i][j] = detArr[i+n][j];
			tempArr4[i][j] = detArr[i+n][j+n];
		}	
	}
	
	resultOfCon(conv, tempArr1, tempArrResult1, n);
	resultOfCon(conv, tempArr2, tempArrResult2, n);
	resultOfCon(conv, tempArr3, tempArrResult3, n);
	resultOfCon(conv, tempArr4, tempArrResult4, n);
	
	
	for(i = 0; i < n; ++i)
	{
		for(j = 0; j < n; ++j)
		{
			convolutionMatrix[i][j] = tempArrResult1[i][j];
			convolutionMatrix[i][j+n] = tempArrResult2[i][j];
			convolutionMatrix[i+n][j] = tempArrResult3[i][j];
			convolutionMatrix[i+n][j+n] = tempArrResult4[i][j];
		}	
	}	
}
/*
	function init the kernel array(3x3) properly
	1 0 0
	0 1 0
	0 0 1
*/
void initKernel()
{
	kernel[0][0] = 1;
	kernel[0][1] = 0;
	kernel[0][2] = 0;
	kernel[1][0] = 0;
	kernel[1][1] = 1;
	kernel[1][2] = 0;
	kernel[2][0] = 0;
	kernel[2][1] = 0;
	kernel[2][2] = 1;
}
/*
	function calculate determinant and return it.
	@param a is will calculating determinant of matrix
	@param k is dimension of matrix 
	
	referance site:
	http://study-for-exam.blogspot.com.tr/2013/04/write-c-progrm-to-find-determinant-of.html
*/
double determinant(double a[40][40], double k)
{
  double s = 1, det = 0, b[40][40];
  int i, j, m, n, c;
  if (k == 1)
    {
     return (a[0][0]);
    }
  else
    {
     det = 0;
     for (c = 0; c < k; c++)
       {
        m = 0;
        n = 0;
        for (i = 0;i < k; i++)
          {
            for (j = 0 ;j < k; j++)
              {
                b[i][j] = 0;
                if (i != 0 && j != c)
                 {
                   b[m][n] = a[i][j];
                   if (n < (k - 2))
                    n++;
                   else
                    {
                     n = 0;
                     m++;
                     }
                   }
               }
             }
          det = det + s * (a[0][c] * determinant(b, k - 1));
          s = -1 * s;
          }
    }
 
    return (det);
}
/*
	function calculate and generate that 2d convolotion matrix
	@param conv  > kernel matrix
	@param source > orginal matrix
	@param dest > will fill matrix
	@param n > number of dimension of matrixs (n x n)
*/
void resultOfCon(double conv[3][3], double source[40][40], double dest[40][40], int n)
{
	int row, col;
	int i,j;
	double sum;
	for ( row = 0; row < n; row++ ) 
	{ 
		for ( col = 0; col < n; col++ ) 
		{
			sum = 0.0 ;
			for ( i = -1; i <= 1; i++ ) 
			{
				for ( j = -1; j <= 1; j++ ) 
				{
					if(isInRange(row,col,i,j,n))
					{
						sum += source[i + row][j + col] * conv[i+1][j+1];
					}
				}
			}
			dest[row][col] = sum;
		}
	}

}
/*
	x1 and y1 source matris cord, x2 and y2 conv. matris cord
*/
int isInRange( int x1,int y1, int x2, int y2, int n)
{
	int sum1 = x1 + x2;
	int sum2 = y1 + y2;
	if(sum1 >= 0 && sum1 < n && sum2 >= 0 && sum2 < n )
		return 1;
	else
		return 0;
}

/*
	Function find cofactor of matrix
	referance : http://www.sanfoundry.com/c-program-find-inverse-matrix/
*/
void cofactor(double num[40][40], double f)
{
 double b[40][40], fac[40][40];
 int p, q, m, n, i, j;
 for (q = 0;q < f; q++)
 {
   for (p = 0;p < f; p++)
    {
     m = 0;
     n = 0;
     for (i = 0;i < f; i++)
     {
       for (j = 0;j < f; j++)
        {
          if (i != q && j != p)
          {
            b[m][n] = num[i][j];
            if (n < (f - 2))
             n++;
            else
             {
               n = 0;
               m++;
               }
            }
        }
      }
      fac[q][p] = pow(-1, q + p) * determinant(b, f - 1);
    }
  }
  transpose(num, fac, f);
}
/*
	Function find transpose of matrix
	referance : http://www.sanfoundry.com/c-program-find-inverse-matrix/
*/
void transpose(double num[40][40], double fac[40][40], double r)
{
  int i, j;
  double b[40][40], inverse[40][40], d;
 
  for (i = 0;i < r; i++)
    {
     for (j = 0;j < r; j++)
       {
         b[i][j] = fac[j][i];
        }
    }
  d = determinant(num, r);
  for (i = 0;i < r; i++)
    {
     for (j = 0;j < r; j++)
       {
        inverse[i][j] = b[i][j] / d;
        }
    }

 
   for (i = 0;i < r; i++)
    {
     for (j = 0;j < r; j++)
       {
         	num[i][j] = inverse[i][j];
        }
     }

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
	while (fscanf(fp, "%s", fifoName) != EOF) {
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
