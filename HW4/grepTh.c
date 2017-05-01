/*
	CSE244 System Programming Homework #4
	Muhammed Selim Dursun
	131044023
-----------------------------------------------------------------------
	program do that find all target word repetition in all file content
	do pipe with parent process and child process file information transfer operation
	use thread and semaphore
*/

//libraries
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h> //exit
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <errno.h>
#include <sys/param.h>
#include <fcntl.h> 
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>
//defines
#define MAX_PATH 512
#define PIPE_FIFO_MAX_SIZE 256
#define MAX_WORD_SIZE 50
#define LOG_FILE_NAME "log.txt"
#define HISTORYFILENAME "history.txt"
#define SEMAPHORENAME "mysemaphore_"
#define MAXTHREADFILE "maxthread.txt"
#define ALLPIDFILENAME "allpid.txt"
/*
  function roam all files and folder and if find a file, call grepDirector function
  for find number of targetString in file.
*/
void grepDirector(const char* path, const char* targetString);
/*
	function check path equals directory or file
	referance = book.
*/
int isdirectory(const char *path);
/*
	function find location of string in a file and write log file
*/
int findLocationOfSeekingWord(const char* dirPath ,const char* word, int index, const char* fileName);
/*
	function find number of target in all files and write to log file	properly
*/
int detectNumberOfTargetWord(const char* targetWord);
/*
	function find number of sub directory and file on current directory
*/
void findNumberOfDirectoryAndFile(const char* path, int *directoryNumber, int *fileNumber);
/*
	function write to file - because of tell thread dead
*/
void threadDeleted();
/*
	function write to file - because of tell thread create
*/
void threadCreated();
/*
	function init history struct
*/
void initHistory();
/*
	funct find number of total thread until it called
*/
void findNumberOfTotalThread();
/*
	function print all history informations
*/
void printHistory();
/*
	function write file of the caller func pid 
*/
void savePid();
/*
	function kill all process when signall arrived process
*/
void killAllProcess();
/*
	function remove and unlink files, semaphores end of the program
*/
void removeAndUnlinkAll();

typedef struct information
{
	char filePath[500];
	char fileName[50];
	char word[50];
	int index;
} inf_t;

typedef struct history
{
	int nOfStringFound;
	int nOfSearchedDir;
	int nOfFileSearched;
	int nOfLinesSearched;
	int nOfThCreated;
	int maxThreadRunning;
	char nOfCascadeThCreated[1000];
} his_t;

int determineMainProcces = 0;
inf_t *threadArr = NULL; 
sem_t *semaphore = NULL ;
FILE *inpPtr=NULL, *inpPtr2=NULL;
his_t mainHistory;
struct timeval stop, start;
char tempPath[255]={'\0'};

/*
	when signal arrived, this function will work
*/
static void signalHandler(int signo) 
{
	FILE* fp;
	if(signo == SIGINT || signo == SIGTSTP)
	{
		if(getpid() == determineMainProcces)
		{

			gettimeofday(&stop, NULL);
			killAllProcess();
			 if(threadArr!=NULL)
			 {
			 	free(threadArr);
			 	threadArr = NULL;
			 }

			 printHistory();
			 if(signo == SIGINT)
			 	fprintf(stderr,"Exit condition:SIGINT\n");
			 if(signo == SIGTSTP)
				fprintf(stderr,"Exit condition:SIGTSTP\n");
			
			
			exit(0);
		}

	}
	if(signo == SIGUSR2)
	{	
		if(threadArr!=NULL)
		{
			free(threadArr);
			threadArr = NULL;
		}
		 exit(0);
	}

}
/*
	when thread created, this function will work
*/
void* calculateOfFileContent(void* element)
{
	inf_t *inf = (inf_t *)element;
	sem_wait(semaphore);
	mainHistory.nOfLinesSearched += findLocationOfSeekingWord( inf->filePath, inf->word, inf->index,inf->fileName);
	threadDeleted();
	sem_post(semaphore);
	
	pthread_exit(NULL);
}

int main( int argc, char *argv[] )  {
	
	int i = 0;
	char path[500]={'\0'};
	char targetString[500]={'\0'};
	struct sigaction sigact;
	if(argc != 3) //control of arguments
    {
        printf("Usage: ./grepTh 'string' <dirname> \n");
        exit(1);
    }
    

	sigact.sa_handler = signalHandler;
	sigact.sa_flags = 0;
	if ( (sigemptyset(&sigact.sa_mask) == -1) || (sigaction(SIGINT, &sigact, NULL) == -1)   
		||  (sigaction(SIGTSTP, &sigact, NULL) == -1) ||  (sigaction(SIGUSR2, &sigact, NULL) == -1))  
	{
		perror("Signal handler setting failed.");
		exit(1);
	}
	
	savePid();

	gettimeofday(&start, NULL);
	initHistory();
	inpPtr = fopen(MAXTHREADFILE,"a");

	semaphore = sem_open(SEMAPHORENAME, O_CREAT|O_RDWR, 0644, 1);


	strcpy(path,argv[2]);
	strcpy(targetString,argv[1]);
    determineMainProcces = getpid();

    remove(LOG_FILE_NAME);//if file exist, delete it

    grepDirector( path, targetString ); 
	mainHistory.nOfStringFound = detectNumberOfTargetWord( targetString );	
	
	fclose(inpPtr);
	
	findNumberOfTotalThread();

	gettimeofday(&stop, NULL);
	printHistory();

	fprintf(stderr,"Exit condition:Normal\n");


	return 0; 	
}
/*
	function remove and unlink files, semaphores end of the program
*/
void removeAndUnlinkAll()
{
	remove(HISTORYFILENAME);
	remove(MAXTHREADFILE);
	remove(ALLPIDFILENAME);
	sem_close(semaphore) ;
	sem_unlink(SEMAPHORENAME);
	
	threadArr = NULL; 
    semaphore = NULL ;
    inpPtr=NULL;
    inpPtr2=NULL;
}
/*
	function write file of the caller func pid 
*/
void savePid()
{
	FILE *fp = fopen(ALLPIDFILENAME,"a");
	fprintf(fp,"%d\n",getpid());
	fclose(fp);
	fp = NULL;

}
/*
	function kill all process when signall arrived process
*/
void killAllProcess()
{
	int pid;
	FILE *fp = fopen(ALLPIDFILENAME,"r");
	while (fscanf(fp, "%d", &pid) != EOF) 
	{
		if(pid != getpid())
		{
			kill(pid,SIGUSR2);
		}
	}
	fp = NULL;
}
/*
	function print all history informations
*/
void printHistory()
{
	fprintf(stderr,"Total number of strings found:%d\n",mainHistory.nOfStringFound);
	fprintf(stderr,"Number of directories searched:%d\n",mainHistory.nOfSearchedDir+1);
	fprintf(stderr,"Number of files searched:%d\n",mainHistory.nOfFileSearched);
	fprintf(stderr,"Number of lines searched:%d\n",mainHistory.nOfLinesSearched);
	fprintf(stderr,"Number of cascade threads created:%s\n",mainHistory.nOfCascadeThCreated);
	fprintf(stderr,"Number of search threads created:%d\n",mainHistory.nOfThCreated);
	fprintf(stderr,"Max # of threads running concurrently:%d\n",mainHistory.maxThreadRunning);

	fprintf(stderr,"Total run time, in milliseconds:%lu\n", (stop.tv_usec - start.tv_usec)/100);
	removeAndUnlinkAll();
}
/*
	funct find number of total thread until it called
*/
void findNumberOfTotalThread()
{
	FILE*inp = fopen(MAXTHREADFILE,"r");
	char ch;
	int number = 0;
	int max = 0;
	while(fscanf(inp,"%c",&ch)!=EOF)
	{
		if(ch == '+')
			++number;
	}
	mainHistory.nOfThCreated = number;
	rewind(inp);
	number = 0;
	while(fscanf(inp,"%c",&ch)!=EOF)
	{
		if(ch == '+')
			++number;
		if(ch == '-')
		{
			if(max<number)
			{
				max = number;
			}
			number = 0;
		}
	}
	mainHistory.maxThreadRunning = max;
		
	fclose(inp);
	inp = NULL;
}
/*
	function init history struct
*/	
void initHistory()
{
	mainHistory.nOfStringFound = 0;
	mainHistory.nOfSearchedDir = 0;
	mainHistory.nOfFileSearched = 0;
	mainHistory.nOfLinesSearched = 0;
	mainHistory.nOfThCreated = 0;
	mainHistory.maxThreadRunning = 0;
	strcpy(mainHistory.nOfCascadeThCreated,"\0");
}

/*
	function find number of files and directories. and create fifos
*/
void findNumberOfDirectoryAndFile(const char* path, int *directoryNumber, int *fileNumber)
{
	DIR *dir = NULL;
	struct dirent *readDirent = NULL;
	char tempPath[MAX_PATH]={'\0'};
	char fifoName[MAX_WORD_SIZE]={'\0'};
	
	if ((dir = opendir (path)) == NULL) {
        perror ("Cannot open directory.\n");
        exit (1);
    }
    
    while( (readDirent = readdir(dir)) != NULL )
    {
    	strcpy(tempPath,path); //creating new path
    	strcat(tempPath,"/");
    	strcat(tempPath,readDirent->d_name);
    	if( strcmp(&readDirent->d_name[ (int) strlen(readDirent->d_name) - 1 ],".") != 0  &&  
			strcmp(&readDirent->d_name[ (int) strlen(readDirent->d_name) - 2 ],"..") != 0 &&
			strcmp(&readDirent->d_name[ (int) strlen(readDirent->d_name) - 1 ],"~") != 0 )
    	{
    		if(!isdirectory( tempPath )) //if file
	    	{
	    		++(*fileNumber);	    		
	    	}
	    	else
	    	{
	    		++(*directoryNumber);	
	    	}
	    }
	}
	closedir(dir);
	dir = NULL;
	readDirent = NULL;
 	return;    	
    
}

//function roam all files and folder and if find a file, call grepDirector function
//for find number of targetString in file.
void grepDirector(const char* path, const char* targetString)
{
	FILE *inputFilePointer = NULL;//
	struct dirent *readDirent = NULL;
	DIR *dir = NULL;
	char tempPath[MAX_PATH]={'\0'};
	int childPid = -1; //pid of child

	int i = 0, j = 0; //loop control variables
	int dirNumber = 0, fileNumber = 0;
	int tempDirNumber = -1, tempFileNumber = -1; //The sequence of the processed file or directory in array

	char fifoName[MAX_WORD_SIZE]={'\0'};
	char tempFifoInformationArray[100][1000]={'\0'};
	char tempPipeInformationArray[100][1000]={'\0'};
	char directoryName[MAX_WORD_SIZE]; 
	int status; //variable of waiting all child
	char fileName[MAX_WORD_SIZE]={'\0'};
	pthread_t pthreadIdArr[250]={'\0'};
	char temp[20]={'\0'};
	char ch;
	int tempNumber,tempNumber2,tempNumber3;
	
	//find directory and file numbeer and create fifos
	findNumberOfDirectoryAndFile(path, &dirNumber, &fileNumber);
	initHistory();

	if(fileNumber > 0)
	{
		threadArr = (inf_t *)calloc(fileNumber, sizeof(inf_t));
		
		for(j = 0; j < fileNumber; ++j)
		{
			strcpy(threadArr[j].filePath,"\0");
			strcpy(threadArr[j].fileName,"\0");
			strcpy(threadArr[j].word,"\0");
			threadArr[j].index = 0;
			
		}
	}
		
	if ((dir = opendir (path)) == NULL) 
	{
        fprintf (stderr,"Cannot open directory.\n");
        exit (1);
    }	

	//if directory not null, it is work
    while( (readDirent = readdir(dir)) != NULL && childPid != 0)
    {	
    	strcpy(tempPath,path); //creating new path
    	strcat(tempPath,"/");
    	strcat(tempPath,readDirent->d_name);
    
    	if( strcmp(&readDirent->d_name[ strlen(readDirent->d_name) - 1 ],".") != 0  &&  
			strcmp(&readDirent->d_name[ strlen(readDirent->d_name) - 2 ],"..") != 0 &&
			strcmp(&readDirent->d_name[ strlen(readDirent->d_name) - 1 ],"~") != 0 )
    	{
    		if(!isdirectory( tempPath ) && fileNumber > 0 ) //if file	
    		{		
    			++tempFileNumber;		
				strcpy(threadArr[tempFileNumber].filePath,tempPath);
	    		threadArr[tempFileNumber].index = tempFileNumber;	
	    		strcpy(threadArr[tempFileNumber].word,targetString);
	    		strcpy(threadArr[tempFileNumber].fileName,readDirent->d_name);	
	    		threadCreated();
    			pthread_create(&pthreadIdArr[tempFileNumber],NULL,calculateOfFileContent,&(threadArr[tempFileNumber]));

    		}
    		else //if directory
	    	{		
    			++tempDirNumber;  
    			childPid = fork(); 
    			
				if(childPid  <  0)
				{
					fprintf(stderr,"Failed to fork!\n");
					exit(1);
				}
				if(childPid == 0)
					strcpy(fileName, readDirent->d_name);
			}
    	}
    }

    if(childPid == 0) // if child
	{	
		savePid();
		 if(threadArr!=NULL)
		 {
		 	free(threadArr);
		 	threadArr = NULL;
		 }	
		grepDirector(tempPath, targetString); //recursiv process		
		exit(0);
	}

	i = 0;
	while(i < fileNumber)
	{
		pthread_join(pthreadIdArr[i],NULL);
		++i;
	}
	if(threadArr!=NULL)
	 {
	 	free(threadArr);
	 	threadArr = NULL;
	 }
	 
	while (wait(&status) != -1) ;

	if (getpid() == determineMainProcces) //if process not main process
	{	
		usleep(10*1000);

		mainHistory.nOfSearchedDir += dirNumber;
		mainHistory.nOfFileSearched += fileNumber;
		sprintf(temp,"%d ",fileNumber);	
		strcat(mainHistory.nOfCascadeThCreated,temp);
		inpPtr2 = fopen(HISTORYFILENAME,"r");

		if(inpPtr2 != NULL)
		{
			while ( fscanf(inpPtr2,"%d,%d,%d,",&tempNumber,&tempNumber2,&tempNumber3) != EOF )
			{	
				mainHistory.nOfLinesSearched += tempNumber;
				mainHistory.nOfSearchedDir += tempNumber2;
				mainHistory.nOfFileSearched += tempNumber3;
				sprintf(temp,"%d ",tempNumber3);	
				strcat(mainHistory.nOfCascadeThCreated,temp);
			}
			fclose(inpPtr2);
		}

    }
	if (getpid() != determineMainProcces) //if process not main process
	{	
		mainHistory.nOfSearchedDir = dirNumber;
		mainHistory.nOfFileSearched = fileNumber;
		inpPtr2 = fopen(HISTORYFILENAME,"a");
		fprintf(inpPtr2,"%d,%d,%d,",mainHistory.nOfLinesSearched,mainHistory.nOfSearchedDir,mainHistory.nOfFileSearched );
		fclose(inpPtr2);
     }
    

	closedir(dir);
	dir = NULL;
	readDirent = NULL;
	if(threadArr!=NULL)
	{
		free(threadArr);
		threadArr = NULL;
	} 
	
	inputFilePointer = NULL;
 	return;
}
/*
	function write to file - because of tell thread create
*/
void threadCreated()
{
	fflush(inpPtr);
	fprintf(inpPtr,"%c",'+');
	fflush(inpPtr);
}
/*
	function write to file - because of tell thread dead
*/
void threadDeleted()
{
	fflush(inpPtr);
	fprintf(inpPtr,"%c",'-');
	fflush(inpPtr);
	
}

//function find number of target in all files and write to log file	properly
int detectNumberOfTargetWord(const char* targetWord)
{
	char chr;
	int wordNumber = 0;
	FILE* log_file_pointer=NULL;
	
	log_file_pointer = fopen(LOG_FILE_NAME,"a+");

	fclose(log_file_pointer);
	log_file_pointer = NULL;
	
	if((log_file_pointer = fopen(LOG_FILE_NAME,"a+")) != NULL)
	{
		rewind(log_file_pointer);
		chr = fgetc(log_file_pointer);
		while(!feof(log_file_pointer))
		{
			if(chr == '\n') ++wordNumber;
			chr = fgetc(log_file_pointer);
		}
		
		fprintf(log_file_pointer, "________________________________________________\n");
		
		if(wordNumber > 0)
			fprintf(log_file_pointer, "%d %s were found in total.", wordNumber, targetWord);
		else
			fprintf(log_file_pointer, "Never found the word searched named %s.", targetWord);
		
		fclose(log_file_pointer);
		log_file_pointer = NULL;
	}
	else 
	{
		fprintf(stderr, "Error occur when open the log file.\n");
		exit(1);    			
	}
	log_file_pointer = NULL;
	return wordNumber;
}
//Taken from the course book, referance page: 197
int isdirectory(const char *path) {
	struct stat statbuf;

	if (stat(path, &statbuf) == -1)
		return 0;
	else
		return S_ISDIR(statbuf.st_mode);
}
//function take file poiner and word, find word's location as colm and row,
//print these location and return number of total matches
//write to pipe that input file repetition of target word
int findLocationOfSeekingWord(const char* dirPath ,const char* word, int index, const char* fileName)
{
	int lengthOfSeekingWord = 0;
	int numberOfOtherValidChars = 0; // '\n', '\t',' ' or word's chars 
	int row = 1, column = 1;
	int wordRow=0, wordColumn=0;
	char character=0, tempCharacter=0;
	int wordIndex = 0;
	int flag = 0; 
	int tempPtrPosition=0;
	int numberOfSeekingWord = 0;
	int numberOfUnNecessaryWords=0;
	FILE* inpFilePtr = NULL, *inputFilePointer = NULL;

	lengthOfSeekingWord	= (int)strlen(word);

	
	inpFilePtr = fopen(dirPath,"r");
	character = fgetc(inpFilePtr);
	inputFilePointer = fopen(LOG_FILE_NAME,"a");
	
	
	while(!feof(inpFilePtr))
	{
		if( character == '\n' && numberOfSeekingWord == 0) ++row;
		//if word's char equal with read char
		if(word[wordIndex] == character)
		{
			flag=1;
			//if first char is equal with searcing word's first char, take the current rown and colm
			if(wordIndex == 0 ) 
			{	
				wordRow =  row;
				wordColumn = column;
				tempCharacter= character;
			}
			++wordIndex; //increase the word's index to check with other char
			//printf("Burada geldim %c flag: %d wordIndex: %d\n",character,flag,wordIndex);
		}
		else if(character == '\t' || character == '\n' || character == ' ' )
		{
			if(character == '\n' && flag == 0) column = 0;		
		}
		else //if read char is not equal '\t', '\n', ' ' or word's chars, reset these variabless 
		{
			flag = 0;
			wordIndex=0;
			numberOfOtherValidChars = 0;
			numberOfUnNecessaryWords=0;				
		}
		
		//I add new code block to hw1, code was not working properly before these conditions. 
		//for ex: target word: oto, txt: otooto			
		if(flag==0) numberOfUnNecessaryWords++;
		if(flag == 0  && numberOfUnNecessaryWords==1 && character == word[0] && character == tempCharacter && lengthOfSeekingWord > 1 )
		{
			fseek(inpFilePtr, -sizeof(char)*2, SEEK_CUR);
			if(fgetc(inpFilePtr)=='\n')
				column=0;
			else
				--column;
		}	
		if(flag == 1)	++numberOfOtherValidChars;		
		if( flag==1 && wordIndex  == lengthOfSeekingWord  )
		{
			fflush(inputFilePointer);
			fprintf(inputFilePointer,"%d %lu %s: [%d, %d] %s first character is found.\n", getpid(),pthread_self(),fileName, wordRow, wordColumn, word);

			fflush(inputFilePointer);
			//Pull back the file pointer
			if(lengthOfSeekingWord > 1)
			{
				tempPtrPosition = ftell(inpFilePtr);
				fseek(inpFilePtr, -(sizeof(char)*(numberOfOtherValidChars-1)), SEEK_CUR);
				column -= (numberOfOtherValidChars-1);
			}
			flag = 0;
			wordIndex=0;
			numberOfOtherValidChars = 0;
			++numberOfSeekingWord;
			numberOfUnNecessaryWords=0;
		}
		character = fgetc(inpFilePtr); //reading new char from file

		if( character == '\n' &&  tempPtrPosition<=ftell(inpFilePtr) && numberOfSeekingWord != 0)
			++row;
		++column;
	}

	
	fclose(	inputFilePointer );
	word = NULL;
	inpFilePtr = NULL;
	inputFilePointer = NULL;
	return row-1;
}
