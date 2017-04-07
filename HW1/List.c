/*
	CSE244 System Programming HW#1
	MUHAMMED SELIM DURSUN
	131044023
*/
//Libraries 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//function take file poiner and word, find word's location as colm and row,
//print these location and return number of total matches
int findLocationOfSeekingWord(FILE *filePtr, const char* word);
int main( int argc, char *argv[] )  {
	if(argc != 3) //control of arguments
    {
        printf("Usage: List string file\n");
        exit(1);
    }
	FILE* fptr;	
	char* stringWillFind = argv[1];
	char* fileName = argv[2];

  	
	if((fptr = fopen(fileName,"r")) != NULL)
	{
		printf("%d adet %s bulundu.\n", 
			findLocationOfSeekingWord(fptr,stringWillFind), stringWillFind);
	}
	else
	{
		printf("The file could not be opened.\n");
		exit(1);
	}
	fclose(fptr);
	return 0; 	
}
//function take file poiner and word, find word's location as colm and row,
//print these location and return number of total matches
int findLocationOfSeekingWord(FILE *filePtr, const char* word)
{
	int lengthOfSeekingWord = 0;
	int numberOfOtherValidChars = 0; // '\n', '\t',' ' or word's chars 
	int row = 1, column = 1;
	int wordRow, wordColumn;
	char character, tempCharacter;
	int wordIndex=0;
	int flag = 0; 
	int tempPtrPosition=0;
	int numberOfSeekingWord = 0;
	int tempp=0;
	
	lengthOfSeekingWord	= strlen(word);
	character = fgetc(filePtr);
	
	while(!feof(filePtr))
	{

		//printf("%c\n",character);
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
			tempp=0;				
		}
		
		/* 
			this conditions work if first letter of target word and again first letter and then others 
			word of target word.
			
			example: target word: araba
			arabaraba it was no problem but when arabaaraba, it was occur problem.
			my code was not working properly before these conditions. 		
			
		*/
		if(flag==0) tempp++;
		if(flag == 0  && tempp==1 && character == word[0] && character == tempCharacter && lengthOfSeekingWord > 1 )
		{
			fseek(filePtr, -sizeof(char)*2, SEEK_CUR);
			if(fgetc(filePtr)=='\n')
				column=0;
			else
				--column;
		}
		
		if(flag == 1)	++numberOfOtherValidChars;		
		if( flag==1 && wordIndex  == lengthOfSeekingWord  )
		{
			printf("[%d, %d] konumunda ilk karakter bulundu.\n", wordRow, wordColumn);
			//Pull back the file pointer
			if(lengthOfSeekingWord > 1)
			{
				tempPtrPosition = ftell(filePtr);
				fseek(filePtr, -(sizeof(char)*(numberOfOtherValidChars-1)), SEEK_CUR);
				column -= (numberOfOtherValidChars-1);

			}
			flag = 0;
			wordIndex=0;
			numberOfOtherValidChars = 0;
			++numberOfSeekingWord;
			tempp=0;
		}
		character = fgetc(filePtr); //reading new char from file

		if( character == '\n' &&  tempPtrPosition<=ftell(filePtr) && numberOfSeekingWord != 0)		
			++row;
		++column;
	}
	return numberOfSeekingWord;
}
