/*
	CSE244 System Programming Homework #2
	Muhammed Selim Dursun
	131044023
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
//defines
#define MAX_PATH 512
#define LOG_FILE_NAME "log.txt"

void grepDirector(const char* path, const char* targetString);
int isdirectory(const char *path);
int findLocationOfSeekingWord(FILE *inpFilePtr, FILE *logFilePtr, const char* fileName ,const char* word);
int detectNumberOfTargetWord(const char* targetWord);

int main( int argc, char *argv[] )  {
	
	if(argc != 3) //control of arguments
    {
        printf("Usage: ./exe string directory\n");
        exit(1);
    }
    char* path = argv[2];
	char* targetString = argv[1];
	
    remove(LOG_FILE_NAME);//if file exist, delete it
    grepDirector( path, targetString ); 
	detectNumberOfTargetWord( targetString );
	return 0; 	
}
//function roam all files and folder and if find a file, call grepDirector function
//for find number of targetString in file.
void grepDirector(const char* path, const char* targetString)
{
	FILE *log_file_pointer = NULL, *input_file_pointer = NULL;
	struct dirent *readDirent = NULL;
	DIR *dir = NULL;
	char tempPath[MAX_PATH];
	int childPid;
	
	if ((dir = opendir (path)) == NULL) {
        perror ("Cannot open directory.\n");
        exit (1);
    }
	//if directory not null, it is work
    while( (readDirent = readdir(dir)) != NULL )
    {
    	strcpy(tempPath,path); //creating new path
    	strcat(tempPath,"/");
    	strcat(tempPath,readDirent->d_name);
		//missing top and upper directory. missing txt saved file
    	if( strcmp(&readDirent->d_name[ strlen(readDirent->d_name) - 1 ],".") != 0  &&  
			strcmp(&readDirent->d_name[ strlen(readDirent->d_name) - 2 ],"..") != 0 &&
			strcmp(&readDirent->d_name[ strlen(readDirent->d_name) - 1 ],"~") != 0 )
    	{
    		if(!isdirectory( tempPath )) //if file
	    	{
	    		childPid = fork();
	    		if(childPid == 0)
	    		{
	    			if((log_file_pointer = fopen(LOG_FILE_NAME,"a+")) != NULL)
					{
						if((input_file_pointer = fopen(tempPath,"r")) != NULL)
						{
							findLocationOfSeekingWord(input_file_pointer, log_file_pointer, readDirent->d_name ,targetString);
							fclose( input_file_pointer );
						}
						else
						{
							fprintf(stderr, "Error occur when open the input file.\n");
							exit(1);
						}
						fclose( log_file_pointer );
					}
					else 
					{
						fprintf(stderr, "Error occur when open the log file.\n");
						exit(1);    			
					}
					exit(0);
	    		}
	    		else if(childPid > 0)
	    		{	
	    			wait(NULL); //parent wait for child death
	    		}
	    		else
	    		{
	    			fprintf(stderr,"Fork error. \n");
					exit(1);
	    		}
	    	}
	    	else //if directory
	    	{
				childPid = fork();
				if(childPid >= 0) //fork success
				{
					if(childPid > 0) //parent process
					{
						wait(NULL); //parent wait for child death
					}
					else if(childPid == 0) // child process
					{
						grepDirector(tempPath, targetString ); //recursiv process
						exit(0);
					}
				}
				else
				{
					fprintf(stderr,"Fork error. \n");
					exit(1);
				}
	    	}
    	}
    }
    closedir(dir);
 	return;
}
//function find number of target in all files and write to log file	properly
int detectNumberOfTargetWord(const char* targetWord)
{
	char chr;
	int wordNumber = 0;
	FILE* log_file_pointer=NULL;
	if((log_file_pointer = fopen(LOG_FILE_NAME,"a+")) != NULL)
	{
		rewind(log_file_pointer);
		chr = fgetc(log_file_pointer);
		while(!feof(log_file_pointer))
		{
			if(chr == '\n') ++wordNumber;
			chr = fgetc(log_file_pointer);
		}
		
		fprintf(log_file_pointer, "----------------------------------------------------\n");
		if(wordNumber>0)
			fprintf(log_file_pointer, "%d %s were found in total.", wordNumber, targetWord);
		else
			fprintf(log_file_pointer, "Never found the word searched named %s.", targetWord);
		
		fclose(log_file_pointer);
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
int findLocationOfSeekingWord(FILE *inpFilePtr, FILE *logFilePtr, const char* fileName ,const char* word)
{
	int lengthOfSeekingWord = 0;
	int numberOfOtherValidChars = 0; // '\n', '\t',' ' or word's chars 
	int row = 1, column = 1;
	int wordRow, wordColumn;
	char character, tempCharacter;
	int wordIndex = 0;
	int flag = 0; 
	int tempPtrPosition=0;
	int numberOfSeekingWord = 0;
	int numberOfUnNecessaryWords=0;
	
	lengthOfSeekingWord	= strlen(word);
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
			fprintf(logFilePtr,"%s: [%d, %d] %s first character is found.\n", fileName, wordRow, wordColumn, word);
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
	return numberOfSeekingWord;
}
