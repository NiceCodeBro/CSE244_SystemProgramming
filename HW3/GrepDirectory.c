/*
	CSE244 System Programming Homework #3
	Muhammed Selim Dursun
	131044023
-----------------------------------------------------------------------
	program do that find all target word repetition in all file content
	do pipe with parent process and child process file information transfer operation
	do fifo with parent process and child process directory information transfer operation
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
//defines
#define MAX_PATH 512
#define PIPE_FIFO_MAX_SIZE 256
#define MAX_WORD_SIZE 50
#define LOG_FILE_NAME "log.txt"

#define FIFO_PERMS (S_IRUSR | S_IWUSR)

void grepDirector(const char* path, const char* targetString);
int isdirectory(const char *path);
int findLocationOfSeekingWord(FILE *inpFilePtr, const char* fileName ,const char* word, int pipe);
int detectNumberOfTargetWord(const char* targetWord);
void findNumberOfDirectoryAndFileAndCreateFifo
	(const char* path, int *directoryNumber, int *fileNumber, char fifoArray[PIPE_FIFO_MAX_SIZE][MAX_WORD_SIZE]);

char arr[5000]={'\0'}; //read informations to arr and write upper process fifos
int determineMainProcces = 0;

int main( int argc, char *argv[] )  {
	
	int i = 0;
	char path[500]={'\0'};
	char targetString[500]={'\0'};
	if(argc != 3) //control of arguments
    {
        printf("Usage: ./exe 'string' <dirname> \n");
        exit(1);
    }
    
	strcpy(path,argv[2]);
	strcpy(targetString,argv[1]);
    determineMainProcces = getpid();
	
    remove(LOG_FILE_NAME);//if file exist, delete it
    grepDirector( path, targetString ); 
	detectNumberOfTargetWord( targetString );	
	
	return 0; 	
}
/*
	function find number of files and directories. and create fifos
*/
void findNumberOfDirectoryAndFileAndCreateFifo
	(const char* path, int *directoryNumber, int *fileNumber, char fifoArray[PIPE_FIFO_MAX_SIZE][50])
{
	DIR *dir = NULL;
	struct dirent *readDirent = NULL;
	char tempPath[MAX_PATH]={'\0'};;
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
	    	{	//fifo name must be unique
                sprintf(fifoName,"%d_%s", getpid(), readDirent->d_name);
	    		strcpy(fifoArray[*directoryNumber],fifoName);
	    		if (mkfifo(fifoName, FIFO_PERMS) == -1) 
	    		{
                    fprintf(stderr,"Fifo creating error!\n");
					exit(1);
                }
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
	int pipeArray[PIPE_FIFO_MAX_SIZE][2]; //
	char fifoArray[PIPE_FIFO_MAX_SIZE][MAX_WORD_SIZE];
	char fifoName[MAX_WORD_SIZE]={'\0'};
	char tempFifoInformationArray[100][1000]={'\0'};
	char tempPipeInformationArray[100][1000]={'\0'};
	int fifoRead=0, fifoWrite=0;
	char *sign=NULL; //path tokenizing variable
	char directoryName[MAX_WORD_SIZE]; 
	int status; //variable of waiting all child
	char fileName[MAX_WORD_SIZE];
	
	//find directory and file numbeer and create fifos
	findNumberOfDirectoryAndFileAndCreateFifo(path, &dirNumber, &fileNumber,fifoArray);

	for (i = 0; i < PIPE_FIFO_MAX_SIZE; ++i) 
	{
        pipeArray[i][0] = -1;
        pipeArray[i][1] = -1;
	}
	
	i = 0;
	while(i < fileNumber) //creating pipe that number of file in directory
	{
		if ( pipe(pipeArray[i]) < 0 )
		{
			fprintf (stderr, "Pipe failed.\n");
			return ;
		}
		++i;
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
    	
    		if(!isdirectory( tempPath )) //if file	
    			++tempFileNumber;
    		else 						//if directory
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
        
    if(childPid == 0) // if child
	{	
		if(!isdirectory( tempPath )) //if file
		{	
			if((inputFilePointer = fopen(tempPath,"r")) != NULL)
			{	
				close(pipeArray[tempFileNumber][0]);
				findLocationOfSeekingWord(inputFilePointer, fileName, targetString, pipeArray[tempFileNumber][1]);
				close(pipeArray[tempFileNumber][1]);
				
				fclose( inputFilePointer );
				inputFilePointer = NULL;
			}
			else
			{
				fprintf(stderr, "Error occur when open the input file.\n");
				exit(1);
			}
		}
		else //if directory
		{
			grepDirector(tempPath, targetString); //recursiv process //directory boşmu bak	
		}			
		exit(0);
	}

	//read named pipe
	j = 0;
	while( j < dirNumber)
	{
		sprintf(fifoName,"%s",fifoArray[j]);
		if( (fifoRead = open(fifoName, O_RDONLY)) != -1)
		{
			strcpy(tempFifoInformationArray[j],"");	
			read(fifoRead, tempFifoInformationArray[j], sizeof(tempFifoInformationArray[j]));
			strcat(arr,tempFifoInformationArray[j]);
			close(fifoRead);
        	unlink(fifoName);
		}
		++j;
   }
      
	/* Waits all child */
	while (wait(&status) != -1) 
	{
        if (status != EXIT_SUCCESS) 
        {
            fprintf(stderr, "Error in fork procces!\n");
            exit(1);
        }
    }
    
	i = 0;
	while( i < fileNumber)
	{   
		strcpy(tempPipeInformationArray[i],"");				
		close(pipeArray[i][1]);
		read(pipeArray[i][0], tempPipeInformationArray[i], sizeof(tempPipeInformationArray[i])); 
		close(pipeArray[i][0]);	
		strcat(arr,tempPipeInformationArray[i]);
		++i;
	}
	//write to named pipe
	j = 0;
	if (getpid() != determineMainProcces) //if process not main process
	{	
		strcpy(tempPath,path);
		sign = strtok(tempPath, "/");
        while (sign != NULL ) 
        {
            strcpy(directoryName, sign);

            sign = strtok(NULL, "/");
        }
		sprintf(fifoName, "%d_%s", getppid(), directoryName); 
		fifoWrite = open(fifoName, O_WRONLY);
		write(fifoWrite, arr, sizeof (arr));
		close(fifoWrite); 
		sign = NULL;    
    }
   
    closedir(dir);
    dir = NULL;
    readDirent = NULL;
 	return;
}
//function find number of target in all files and write to log file	properly
int detectNumberOfTargetWord(const char* targetWord)
{
	char chr;
	int wordNumber = 0;
	FILE* log_file_pointer=NULL;
	
	log_file_pointer = fopen(LOG_FILE_NAME,"a+");
	fprintf(log_file_pointer,"%s",arr );
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
int findLocationOfSeekingWord(FILE *inpFilePtr, const char* fileName ,const char* word, int pipe)
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
	char tempFifoInformationArray[4096];
	char tempPipeInformationArray[4096];
	
	lengthOfSeekingWord	= (int)strlen(word);
	character = fgetc(inpFilePtr);
	
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
			strcpy(tempPipeInformationArray,"");	
			sprintf(tempPipeInformationArray,"%s: [%d, %d] %s first character is found.\n", fileName, wordRow, wordColumn, word);
			strcat(tempFifoInformationArray,tempPipeInformationArray);
			

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

	
	write(pipe,tempFifoInformationArray, (int)strlen(tempFifoInformationArray));

	word = NULL;
	
	return numberOfSeekingWord;
}
